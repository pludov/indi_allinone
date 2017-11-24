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

#include "indicom.h"
#include "indiapi.h"
#include "indidevapi.h"
#include "connectionplugins/connectionserial.h"

#include "WriteBuffer.h"
#include "IndiDevice.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiNumberVectorMember.h"
#include "IndiProtocol.h"
#include "IndiDeviceMutator.h"
#include "BinSerialProtocol.h"
#include "BinSerialReadBuffer.h"
#include <unistd.h>
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

bool SimpleDevice::initProperties()
{
    INDI::DefaultDevice::initProperties();

    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    registerConnection(serialConnection);
    return true;
}

struct BackgroundProcessorContext {
    SimpleDevice * device;
    int port;
};

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
    
    // FIXME: join previous thread
    BackgroundProcessorContext * context = new BackgroundProcessorContext();
    context->device = this;
    context->port = PortFD;

    pthread_create(&backgroundProcessorThread, nullptr, &SimpleDevice::backgroundProcessorStarter, context);

    DEBUGF(INDI::Logger::DBG_DEBUG, "Connected successfuly to simulated %s. Retrieving startup data...", getDeviceName());
    return true;
}

struct IndiVectorImage {
	bool announced;
	IndiVector * vector;
	INumberVectorProperty * numberVectorProperty;

	IndiVectorImage(IndiVector * vector)
	{
		this->announced = false;
		this->vector = vector;
		this->numberVectorProperty = nullptr;
	}

	~IndiVectorImage()
	{
		if (vector) {
			delete vector;
		}
		if (numberVectorProperty) {
			delete(numberVectorProperty);
		}
	}
};


class CustomIndiProtocol : public IndiProtocol, public IndiDeviceMutator
{
    IndiDevice * device;
    SimpleDevice * target;
public:
    CustomIndiProtocol(IndiDevice * device, SimpleDevice * target) : IndiDeviceMutator(){
        this->device = device;
        this->target = target;
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
    	}
    }

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
				delete(current->numberVectorProperty);
				current->numberVectorProperty = newVector;
				IDSetNumber(newVector, nullptr);

				break;
			}
		}
    }

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
		}
    }
};

void SimpleDevice::backgroundProcessor(int fd)
{
    IndiDevice * dev = new IndiDevice(255);
    CustomIndiProtocol * proto = new CustomIndiProtocol(dev, this);
    uint8_t packet[1];
  
  
    fd_set readset;
    fd_set writeset;
    
    while(true) {
        bool canRead = proto->canRead();
        bool canWrite = proto->canWrite();
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        
        if (canRead) {
            FD_SET(fd, &readset);
        }
        if (canWrite) {
            FD_SET(fd, &writeset);
        }
        int result = select(fd + 1, canRead ? &readset : NULL, canWrite ? &writeset : NULL, NULL, NULL);
        if (canRead && FD_ISSET(fd, &readset)) {
            int rd = read(fd, &packet, 1);
            if (rd == 1) {
                proto->received(packet[0]);
            } else if (rd == 0) {
                break;
            } else {
                if (errno != EAGAIN) {
                    perror("read");
                    break;
                }
            }
        }
        if (canWrite && FD_ISSET(fd, &writeset) && proto->canWrite()) {
            packet[0] = proto->write();
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
        proto->flushIncomingMessages();
    }
    
/*    int rd;
    while((rd = read(fd, &packet, 1)) == 1)
    {
        proto->received(packet[0]);
        proto->flushIncomingMessages();
    }*/
/*    uint8_t packet[4096];
    uint16_t packetSize = 0;
    uint16_t bytesInBuff = 0;
    bool inPacket = false;
    int rd;
    
    // FIXME: add a timeout of about 1s
    while((rd = read(fd, packet + packetSize, 1)) == 1)
    {
        proto->received(packet[0]);
        uint8_t v = packet[packetSize];

        if (v >= MIN_PACKET_START && v <= MAX_PACKET_START) {
            if (packetSize != 0) {
                DEBUGF(INDI::Logger::DBG_DEBUG, "Packet interrupted after %d", packetSize);
                packet[0] = v;
            }
            packetSize = 1;
            inPacket = true;
        } else if (v == PACKET_END) {
            if (!inPacket) {
               DEBUGF(INDI::Logger::DBG_DEBUG, "End of unknown packet after %d", packetSize);
               packetSize = 0;
               inPacket = false;
            } else {
               DEBUGF(INDI::Logger::DBG_DEBUG, "Receveid packt of size %d", packetSize);
               BinSerialReadBuffer reader(packet, packetSize);
               reader.readAndApply(*dev, );
               packetSize = 0;
               inPacket = false;
            }
        } else {
            if (inPacket) {
               packetSize++;
               if (packetSize == 4096) {
                   DEBUG(INDI::Logger::DBG_DEBUG, "Packet overflow");
               }
            }
        }
    }*/
/*    if (rd == -1) {            
        DEBUGF(INDI::Logger::DBG_ERROR, "IOCTL error %s.", strerror(errno));
    } else {
        DEBUG(INDI::Logger::DBG_ERROR, "Channel closed.");
    }*/
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
