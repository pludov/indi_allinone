/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.h
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.h
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#pragma once

#include "defaultdevice.h"

#include <pthread.h>


namespace Connection
{
class Serial;
}

class CustomIndiProtocol;

class SimpleDevice : public INDI::DefaultDevice
{

  public:
    SimpleDevice() = default;
    virtual bool initProperties();

    // FIXME: protect under mutex
    CustomIndiProtocol * currentIndiProtocol;
    // Use to wakeup the select thread
    int protocolPipe[2];
  protected:
    // Thread for data readings
    pthread_t backgroundProcessorThread;
    pthread_mutex_t lock;

    bool Handshake();
    const char *getDefaultName();

    void backgroundProcessor(int fd);

    static void * backgroundProcessorStarter(void * rawContext);

    Connection::Serial *serialConnection { nullptr };
  public:
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n);


};
