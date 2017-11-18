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

#include "simpledevice.h"

#include "indicom.h"
#include "connectionplugins/connectionserial.h"

#include "WriteBuffer.h"
#include "IndiDevice.h"
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
    // FIXME: join previous thread
    BackgroundProcessorContext * context = new BackgroundProcessorContext();
    context->device = this;
    context->port = PortFD;

    pthread_create(&backgroundProcessorThread, nullptr, &SimpleDevice::backgroundProcessorStarter, context);

    DEBUGF(INDI::Logger::DBG_DEBUG, "Connected successfuly to simulated %s. Retrieving startup data...", getDeviceName());
    return true;
}

void SimpleDevice::backgroundProcessor(int fd)
{
    IndiDevice * dev = new IndiDevice(255);
    
    uint8_t packet[4096];
    uint16_t packetSize = 0;
    uint16_t bytesInBuff = 0;
    bool inPacket = false;
    int rd;
    
    // FIXME: add a timeout of about 1s
    while((rd = read(fd, packet + packetSize, 1)) == 1)
    {
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
               reader.readAndApply(*dev);
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
    }
    if (rd == -1) {            
        DEBUGF(INDI::Logger::DBG_ERROR, "IOCTL error %s.", strerror(errno));
    } else {
        DEBUG(INDI::Logger::DBG_ERROR, "Channel closed.");
    }
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
