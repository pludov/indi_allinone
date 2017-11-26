/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustrate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.cpp
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.cpp
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#include <iostream>
#include <map>
#include "simpledevice.h"
#include "Lock.h"

#include "indicom.h"
#include "indiapi.h"
#include "indidevapi.h"
#include "connectionplugins/connectionserial.h"

#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiVector.h"
#include "IndiProtocol.h"
#include "IndiVectorMember.h"
#include "IndiNumberVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"
#include "IndiSwitchVector.h"
#include "IndiSwitchVectorMember.h"
#include "IndiProtocol.h"
#include "IndiDeviceMutator.h"
#include "BinSerialProtocol.h"
#include "BinSerialReadBuffer.h"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstring>
#include <memory>

std::unique_ptr<SimpleDevice> simpleDevice(new SimpleDevice());

/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties(const char *dev)
{
    simpleDevice->ISGetProperties(dev);
}

/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    simpleDevice->ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    simpleDevice->ISNewText(dev, name, texts, names, n);
}

/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    simpleDevice->ISNewNumber(dev, name, values, names, n);
}

/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    simpleDevice->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice(XMLEle *root)
{
    INDI_UNUSED(root);
}

SimpleDevice::SimpleDevice() : INDI::DefaultDevice(), backgroundProcessorThread(){
	sharingMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	doneCond = PTHREAD_COND_INITIALIZER;
	backgroundProcessorStarted = false;
	terminateBackgroundProcessorThread = false;
	backgroundProcessorThreadDone = false;
	currentIndiProtocol = 0;
	currentIndiDevice = 0;
}

class DisconnectHookConnectionSerial : public Connection::Serial
{
	SimpleDevice * callOnDisconnect;
public:
	DisconnectHookConnectionSerial(SimpleDevice * device) : Connection::Serial(device) {
		this->callOnDisconnect = device;
	}

	virtual ~DisconnectHookConnectionSerial() {}
	virtual bool Disconnect() {
		this->callOnDisconnect->beforeDisconnect();

		return Connection::Serial::Disconnect();
	}
};

bool SimpleDevice::initProperties()
{
    INDI::DefaultDevice::initProperties();

    addAuxControls();

    serialConnection = new DisconnectHookConnectionSerial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    registerConnection(serialConnection);
    return true;
}

struct BackgroundProcessorContext {
    SimpleDevice * device;
    int port;
};

struct IndiVectorImage {
	bool announced;
	IndiVector * vector;
	INumberVectorProperty * numberVectorProperty;
	ITextVectorProperty * textVectorProperty;
	ISwitchVectorProperty * switchVectorProperty;

	IndiVectorImage(IndiVector * vector)
	{
		this->announced = false;
		this->vector = vector;
		this->numberVectorProperty = nullptr;
		this->textVectorProperty = nullptr;
		this->switchVectorProperty = nullptr;
	}

	~IndiVectorImage()
	{
		cleanVectorProperty();
		if (vector) {
			delete vector;
		}
	}

	void cleanVectorProperty() {
		if (numberVectorProperty) {
			delete [] numberVectorProperty->np;
			delete(numberVectorProperty);
			numberVectorProperty = nullptr;
		}
		if (textVectorProperty) {
			for(int i = 0 ; i < textVectorProperty->ntp; ++i) {
				free(textVectorProperty->tp[i].text);
			}
			delete [] textVectorProperty->tp;
			delete(textVectorProperty);
			textVectorProperty = nullptr;
		}
		if (switchVectorProperty) {
			delete [] switchVectorProperty->sp;
			delete(switchVectorProperty);
			switchVectorProperty = nullptr;
		}
	}
};


class CustomIndiProtocol : public IndiProtocol, public IndiDeviceMutator
{
    IndiDevice * device;
    SimpleDevice * target;
    int wakeFd;
public:
    CustomIndiProtocol(IndiDevice * device, SimpleDevice * target, int wakeFd) : IndiDeviceMutator(){
        this->device = device;
        this->target = target;
        this->wakeFd = wakeFd;
        this->requestPacket = (uint8_t*)malloc(REQUEST_PACKET_MAX_SIZE);
    }
    
    bool canRead()
    {
        return !incomingPacketReady;
    }
    
    bool canWrite()
    {
        if (writeBufferLeft) {
            return true;
        }
        fillBuffer();
        return writeBufferLeft > 0;
    }

    uint8_t write() {
        uint8_t r = *writeBuffer;
        writeBuffer ++;
		writeBufferLeft --;
		return r;
    }
    
    void flushIncomingMessages()
    {
    	if (incomingPacketReady) {
    		BinSerialReadBuffer bsrb(incomingPacket, incomingPacketSize);
    		BinSerialWriteBuffer answer(ackPacket, ACK_PACKET_MAX_SIZE);

    		incomingPacketSize = 0;
    		incomingPacketReady = false;
    		if (!bsrb.readAndApply(*device, *this, answer)) {
    			// FIXME: when proto is fully implemented, this will really be an error
    			//reset();
    		} else {
    			if (!answer.isEmpty()) {
    				ackPacketSize = answer.size();
    			}
    		}
    	}
    }

    virtual IndiDeviceMutator * getMutator() {
    	std::cerr << "getMutator called\n";
    	return this;
    }

    IPerm getIPerm(const IndiVector * vector) const {
    	switch (vector->flag & (VECTOR_WRITABLE | VECTOR_READABLE)) {
    	case VECTOR_WRITABLE:
    		return IPerm::IP_WO;
    	case VECTOR_WRITABLE|VECTOR_READABLE:
			return IPerm::IP_RW;
    	default:
    		return IPerm::IP_RO;
    	}
    }

    IPState getIPState(const IndiVector * vector) const {
    	if (vector->flag & VECTOR_BUSY) {
    		return IPState::IPS_BUSY;
    	}
    	return IPState::IPS_IDLE;
    }

    std::map<std::string, IndiVectorImage*> propsByName;
    std::map<uint8_t, IndiVectorImage*> propsByUid;

    INumberVectorProperty * buildFromVector(IndiNumberVector * vector)
    {
		INumberVectorProperty * newVector = new INumberVectorProperty();
		IPerm perm = getIPerm(vector);
		IPState state = getIPState(vector);
		int count = vector->getMemberCount();

		INumber * numbers = new INumber[count];

		int pos = 0;
		for(IndiVectorMember * member = vector->first; member ; member = member->next)
		{
			IUFillNumber(numbers + (pos++), member->name.c_str(), member->label.c_str(), "%.f", 0, 10000, 1, ((IndiNumberVectorMember*)member)->getDoubleValue());

		}
		IUFillNumberVector(newVector, numbers, count, target->getDeviceName(), vector->name.c_str(), vector->label.c_str(), "badgroup", perm, 60, state);

		return newVector;
    }

    ITextVectorProperty * buildFromVector(IndiTextVector * vector)
    {
		ITextVectorProperty * newVector = new ITextVectorProperty();
		IPerm perm = getIPerm(vector);
		IPState state = getIPState(vector);
		int count = vector->getMemberCount();

		IText * texts = new IText[count];

		int pos = 0;
		for(IndiVectorMember * member = vector->first; member ; member = member->next)
		{
			IUFillText(texts + (pos++), member->name.c_str(), member->label.c_str(), ((IndiTextVectorMember*)member)->getTextValue());
		}
		IUFillTextVector(newVector, texts, count, target->getDeviceName(), vector->name.c_str(), vector->label.c_str(), "badgroup", perm, 60, state);

		return newVector;
    }

    ISwitchVectorProperty * buildFromVector(IndiSwitchVector * vector)
    {
		ISwitchVectorProperty * newVector = new ISwitchVectorProperty();
		IPerm perm = getIPerm(vector);
		IPState state = getIPState(vector);
		int count = vector->getMemberCount();

		ISwitch * switches = new ISwitch[count];

		int pos = 0;
		for(IndiVectorMember * member = vector->first; member ; member = member->next)
		{
			ISState value = ((IndiSwitchVectorMember*)member)->getValue() ? ISState::ISS_ON : ISState::ISS_OFF;
			IUFillSwitch(switches + (pos++), member->name.c_str(), member->label.c_str(), value);
		}
		IUFillSwitchVector(newVector, switches, count, target->getDeviceName(), vector->name.c_str(), vector->label.c_str(), "badgroup", perm, ISRule::ISR_1OFMANY, 60, state);

		return newVector;
    }


    IndiVectorImage * getCurrentVectorImage(uint8_t uid)
    {
    	auto ptr = propsByUid.find(uid);
    	if (ptr == propsByUid.end()) return 0;
    	return ptr->second;
    }

    IndiVectorImage * getCurrentVectorImage(const std::string & uid)
    {
    	auto ptr = propsByName.find(uid);
    	if (ptr == propsByName.end()) return 0;
    	return ptr->second;
    }

    void destroy(IndiVectorImage * image)
    {
    	auto ptr = propsByUid.find(image->vector->uid);
    	if (ptr != propsByUid.end()) {
    		propsByUid.erase(ptr);
    	}
    	auto nptr = propsByName.find(image->vector->name);
    	if (nptr != propsByName.end()) {
    		propsByName.erase(nptr);
    	}

    	delete(image);
    }

    // Called by indiProtocol after a vector has been announced
    virtual void announced(IndiVector * vector) {
    	// If a vector with same name/uid/type exists, keep it

    	// Check that no vector with the same uid exists (or drop it)
    	// Check that not vector with the same name exists (or drop it)
    	IndiVectorImage * current = getCurrentVectorImage(vector->uid);
    	if (current && current->vector->name == vector->name && current->vector->kind().uid == vector->kind().uid) {
    		// They are the same, we are reconnecting. juste mutate it
    		current->announced = true;
    		mutated(vector);
    		return;
    	}
    	if (current) {
    		// current with another name, discard it
    		destroy(current);
    	}
    	current = getCurrentVectorImage(vector->name);
    	if (current) {
    		destroy(current);
    	}

    	current = new IndiVectorImage(vector);
    	current->announced = true;
    	propsByName[current->vector->name] = current;
    	propsByUid[current->vector->uid] = current;

    	switch(vector->kind().uid)
    	{
    	case IndiNumberVectorKindUid:
			{
				INumberVectorProperty * newVector = buildFromVector((IndiNumberVector*)vector);
				current->numberVectorProperty = newVector;
				IDDefNumber(newVector, 0);

				break;
			}
    	case IndiTextVectorKindUid:
    		{
    			ITextVectorProperty * newVector = buildFromVector((IndiTextVector*)vector);
    			current->textVectorProperty = newVector;
    			IDDefText(newVector, 0);

    			break;
    		}
    	case IndiSwitchVectorKindUid:
    		{
    			ISwitchVectorProperty * newVector = buildFromVector((IndiSwitchVector*)vector);
    			current->switchVectorProperty = newVector;
    			IDDefSwitch(newVector, 0);
    			break;
    		}
    	default:
    		std::cerr << "Unsupported vector kind for announce: " << ((int)vector->kind().uid) << "\n";
    	}
    }

    // Called by indiProtocol after a vector has been mutated (any change except type/name)
    virtual void mutated(IndiVector * vector)
    {
    	// Check that the vector exists and still has the same name
    	IndiVectorImage * current = getCurrentVectorImage(vector->uid);
    	if (current == nullptr || current->vector->name != vector->name) {
    		// FIXME: disconnect ???
    		std::cerr << "Packet mismatch for mutated packet\n";
    		return;
    	}
    	if (current->vector->kind().uid != vector->kind().uid) {
    		std::cerr << "Type mutation not supported\n";
    		return;
    	}

    	switch(vector->kind().uid)
		{
		case IndiNumberVectorKindUid:
			{
				INumberVectorProperty * newVector = buildFromVector((IndiNumberVector*)vector);
				current->cleanVectorProperty();
				current->numberVectorProperty = newVector;
				IDSetNumber(newVector, nullptr);

				break;
			}
		case IndiTextVectorKindUid:
			{
				ITextVectorProperty * newVector = buildFromVector((IndiTextVector*)vector);
				current->cleanVectorProperty();
				current->textVectorProperty = newVector;
				IDSetText(newVector, nullptr);

				break;
			}
		case IndiSwitchVectorKindUid:
			{
				ISwitchVectorProperty * newVector = buildFromVector((IndiSwitchVector*)vector);
				current->cleanVectorProperty();
				current->switchVectorProperty = newVector;
				IDSetSwitch(newVector, nullptr);

				break;
			}
		default:
			std::cerr << "Unsupported vector kind for mutate: " << ((int)vector->kind().uid) << "\n";
		}
    }

    // Called by indiProtocol after a vector has been updated
    virtual void updated(IndiVector * vector)
    {
    	std::cerr << "updated: " << ((long)vector) << "\n";
    	IndiVectorImage * current = getCurrentVectorImage(vector->uid);
    	std::cerr << "current is " << ((long)current) << "\n";
    	switch(vector->kind().uid)
		{
		case IndiNumberVectorKindUid:
			{
				int pos = 0;
				for(IndiVectorMember * member = vector->first; member ; member = member->next)
				{
					std::cerr << "updating: " << member->name.c_str() << "\n";

					current->numberVectorProperty->np[pos++].value = ((IndiNumberVectorMember*)member)->getDoubleValue();
				}
				std::cerr << "setNumber\n";
				IDSetNumber(current->numberVectorProperty, nullptr);
				break;
			}
		case IndiTextVectorKindUid:
			{
				int pos = 0;
				for(IndiVectorMember * member = vector->first; member ; member = member->next)
				{
					std::cerr << "updating: " << member->name.c_str() << "\n";

					IUSaveText(&current->textVectorProperty->tp[pos++], ((IndiTextVectorMember*)member)->getTextValue());
				}
				std::cerr << "setText\n";
				IDSetText(current->textVectorProperty, nullptr);
				break;
			}
		case IndiSwitchVectorKindUid:
			{
				int pos = 0;
				for(IndiVectorMember * member = vector->first; member ; member = member->next)
				{
					std::cerr << "updating: " << member->name.c_str() << "\n";

					current->switchVectorProperty->sp[pos++].s = ((IndiSwitchVectorMember*)member)->getValue() ? ISState::ISS_ON : ISState::ISS_OFF;
				}
				std::cerr << "setSwitch\n";
				IDSetSwitch(current->switchVectorProperty, nullptr);
				break;
			}
		default:
			std::cerr << "Unsupported vector kind for update: " << ((int)vector->kind().uid) << "\n";
		}
    }

    // FIXME: who is responsible for delete of vector
    virtual void deleted(IndiVector * vector)
    {
    	IndiVectorImage * current = getCurrentVectorImage(vector->uid);
    	IDDelete(target->getDeviceName(), current->vector->name.c_str(), nullptr);
    	destroy(current);
    }

    bool reqNewSomething(int vectorKindUid, const char *name, void * values, int valSize, char *names[], int n)
    {
    	auto item = propsByName.find(std::string(name));
    	if (item == propsByName.end()) {
    		return false;
    	}

    	// FIXME: check that no other request is currently in process.
    	// Peer cannot enqueue messages. We must wait until requestPacketSize is idle
    	if (requestPacketSize) {
    		std::cerr << "Write buffer overflow. FIFO needed";
    		return false;
    	}

    	// check type of vector is number. Otherwise, ignore
    	IndiVector * vector = item->second->vector;
    	if (vector->kind().uid != vectorKindUid) {
    		std::cerr << "Ignore request with type mismatch\n";
    		return false;
    	}

    	BinSerialWriteBuffer req(requestPacket, REQUEST_PACKET_MAX_SIZE);
    	req.appendPacketControl(PACKET_REQUEST);
    	req.appendUid(vector->uid);
    	int memId = 0;
    	int prevMem = -1;
    	for(IndiVectorMember * mem = vector->first; mem; mem = mem->next)
    	{
    		int memPres = -1;
    		for(int i = 0 ; i < n; ++i) {
    			if (mem->name == names[i]) {
    				memPres = i;
    				break;
    			}
    		}

    		if (memPres != -1) {
    			// Add a skip
    			int skip = (memId - prevMem - 1);
    			while(skip >= 127) {
    				req.appendUint7(skip);
    				skip -= 127;
    			}
    			req.appendUint7(skip);

    			// Add the value
    			mem->writeUpdateValue(req, (void*)(((char*)values) + valSize * memPres));
    			prevMem = memId;
    		}
    		memId++;
    	}

    	if (req.finish()) {
    		req.debug();
    		requestPacketSize = req.size();
    		::write(wakeFd, (char*)&wakeFd, 1);
			std::cerr << "request packet queued\n";
    		return true;
    	} else {
    		std::cerr << "Packet overflow !\n";
    	}

    	return false;

    }

    // Handle a request from a client to update a number vector
    bool reqNewNumber(const char *name, double values[], char *names[], int n)
    {
    	return reqNewSomething(IndiNumberVectorKindUid, name, (void*)values, sizeof(double), names, n);
    }

    bool reqNewText(const char * name, char ** values, char * names[], int n)
    {
    	return reqNewSomething(IndiTextVectorKindUid, name, (void*)values, sizeof(char*), names, n);
    }

    bool reqNewSwitch(const char * name, ISState values [], char * names[], int n)
    {
    	bool boolValues[n];
    	for(int i = 0; i < n; ++i) {
    		boolValues[i] = values[i] != ISState::ISS_OFF;
    	}
    	return reqNewSomething(IndiSwitchVectorKindUid, name, (void*)boolValues, sizeof(bool), names, n);
    }
};

void SimpleDevice::beforeDisconnect()
{
	std::cerr << "Stopping processing thread\n";
	Lock l(&sharingMutex);
	l.lock();
	killBackgroundProcessor(l);
}

void SimpleDevice::killBackgroundProcessor(Lock & held)
{
	if (!backgroundProcessorStarted) {
		return;
	}
	terminateBackgroundProcessorThread = true;

	while(!backgroundProcessorThreadDone) {
		write(protocolPipe[1], (char*)&protocolPipe, 1);
		pthread_cond_wait(&doneCond, &sharingMutex);
	}
	for(auto it = currentIndiProtocol->propsByUid.begin(); it !=currentIndiProtocol->propsByUid.end(); ++it)
	{
		currentIndiProtocol->deleted(it->second->vector);
	}

	delete currentIndiProtocol;
	delete currentIndiDevice;
	close(protocolPipe[0]);
	close(protocolPipe[1]);

	backgroundProcessorStarted = false;
}

bool SimpleDevice::Handshake()
{
    int PortFD = serialConnection->getPortFD();

    /* Drop RTS */
    int i = 0;
    i |= TIOCM_RTS;
    if (ioctl(PortFD, TIOCMBIC, &i) != 0)
    {
        DEBUGF(INDI::Logger::DBG_ERROR, "IOCTL error %s.", strerror(errno));
        return false;
    }
    tcflush(PortFD, TCIOFLUSH);

    Lock lock(&sharingMutex);

    lock.lock();

    // join previous thread
    killBackgroundProcessor(lock);


    if (pipe(protocolPipe) != 0) {
    	DEBUGF(INDI::Logger::DBG_ERROR, "pipe error %s.", strerror(errno));
		return false;
    }
    for(int i = 0; i < 2; ++i) {
    	if (fcntl(protocolPipe[i], F_SETFL, O_NONBLOCK) == -1) {
    		perror("fcntl");
    		close(protocolPipe[0]);
    		close(protocolPipe[1]);
    		return false;
    	}
    }

    currentIndiDevice = new IndiDevice(255);
    currentIndiProtocol = new CustomIndiProtocol(currentIndiDevice, this, this->protocolPipe[1]);

    backgroundProcessorStarted = true;
    terminateBackgroundProcessorThread = false;
    backgroundProcessorThreadDone = false;

    BackgroundProcessorContext * context = new BackgroundProcessorContext();
    context->device = this;
    context->port = PortFD;

    pthread_create(&backgroundProcessorThread, nullptr, &SimpleDevice::backgroundProcessorStarter, context);

    DEBUGF(INDI::Logger::DBG_DEBUG, "Connected successfuly to simulated %s. Retrieving startup data...", getDeviceName());
    return true;
}

bool SimpleDevice::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	if (INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n)) {
		return true;
	}
	Lock lock(&sharingMutex);

	lock.lock();

	std::cerr << "new number\n";
	if (currentIndiProtocol == nullptr) {

		return false;
	}
	return currentIndiProtocol->reqNewNumber(name, values, names, n);
}

bool SimpleDevice::ISNewText(const char *dev, const char *name, char * values[], char *names[], int n)
{
	if (INDI::DefaultDevice::ISNewText(dev, name, values, names, n)) {
		return true;
	}
	Lock lock(&sharingMutex);

	lock.lock();

	std::cerr << "new text\n";
	if (currentIndiProtocol == nullptr) {
		return false;
	}
	return currentIndiProtocol->reqNewText(name, values, names, n);
}

bool SimpleDevice::ISNewSwitch(const char *dev, const char *name, ISState states[], char *names[], int n)
{
	if (INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n)) {
		return true;
	}
	Lock lock(&sharingMutex);

	lock.lock();

	std::cerr << "new text\n";
	if (currentIndiProtocol == nullptr) {
		return false;
	}
	return currentIndiProtocol->reqNewSwitch(name, states, names, n);
}

static int max(int a, int b)
{
	return a > b ? a : b;
}

bool SimpleDevice::checkBackgroundProcessorThreadEnd() {
	if (terminateBackgroundProcessorThread) {
		backgroundProcessorThreadDone = true;
		pthread_cond_broadcast(&doneCond);
		return true;
	}
	return false;
}


void SimpleDevice::backgroundProcessor(int fd)
{
    // thread protection. The mutex is released only during select
	// (a pipe is used when this select must continue)
	Lock lock(&sharingMutex);

	lock.lock();

    uint8_t packet[1];
  
  
    fd_set readset;
    fd_set writeset;
    while(true) {
        bool canRead = currentIndiProtocol->canRead();
        bool canWrite = currentIndiProtocol->canWrite();

        int pipeFd = this->protocolPipe[0];

        if (checkBackgroundProcessorThreadEnd()) break;

        lock.release();

        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(pipeFd, &readset);
        if (canRead) {
            FD_SET(fd, &readset);
        }
        if (canWrite) {
            FD_SET(fd, &writeset);
        }

        int result = select(max(fd, pipeFd) + 1, &readset, canWrite ? &writeset : NULL, NULL, NULL);
        if (result == -1) {
        	perror("select");
        	break;
        }
        lock.lock();

        if (checkBackgroundProcessorThreadEnd()) break;

        if (FD_ISSET(pipeFd, &readset)) {
        	char buffer[256];
        	read(pipeFd, buffer, 256);
        }


        if (canRead && FD_ISSET(fd, &readset)) {
            int rd = read(fd, &packet, 1);
            if (rd == 1) {

            	currentIndiProtocol->received(packet[0]);
            } else if (rd == 0) {
                break;
            } else {
                if (errno != EAGAIN) {
                    perror("read");
                    break;
                }
            }
        }
        if (canWrite && FD_ISSET(fd, &writeset) && currentIndiProtocol->canWrite()) {
            packet[0] = currentIndiProtocol->write();
            int wr = write(fd, &packet, 1);
            if (wr == 1) {
            } else if (wr == 0) {
                break;
            } else {
                if (errno != EAGAIN) {
                    perror("write");
                    break;
                }
            }
        }
        currentIndiProtocol->flushIncomingMessages();
    }
    
    // FIXME: If close is not in progress, start it now
    backgroundProcessorThreadDone = true;
	pthread_cond_broadcast(&doneCond);
}

void * SimpleDevice::backgroundProcessorStarter(void*rawContext)
{
    BackgroundProcessorContext * context = (static_cast<BackgroundProcessorContext *>(rawContext));
    context->device->backgroundProcessor(context->port);
    delete context;
    return nullptr;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *SimpleDevice::getDefaultName()
{
    return "Simple Device";
}
