/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "CodalConfig.h"
#include "Trinket.h"
#include "CodalDmesg.h"
#include "Timer.h"
#include "neopixel.h"

using namespace codal;

#ifdef DEVICE_DBG
RawSerial *SERIAL_DEBUG;
#endif

void cplay_dmesg_flush();
Trinket* cplay_device_instance = NULL;

/**
  * Constructor.
  *
  * Create a representation of a GenuinoZero device, which includes member variables
  * that represent various device drivers used to control aspects of the micro:bit.
  */
Trinket::Trinket() :
    timer(TC4, TC4_IRQn),
    messageBus(),
    io(),
    sws(io.d2),
    jacdac(sws),
    protocol(jacdac)
{

    cplay_device_instance = this;
    // Clear our status
    status = 0;

    codal_dmesg_set_flush_fn(cplay_dmesg_flush);

    // Bring up fiber scheduler.
    scheduler_init(messageBus);

    messageBus.listen(DEVICE_ID_MESSAGE_BUS_LISTENER, DEVICE_EVT_ANY, this, &Trinket::onListenerRegisteredEvent);

    for(int i = 0; i < DEVICE_COMPONENT_COUNT; i++)
    {
        if(CodalComponent::components[i])
            CodalComponent::components[i]->init();
    }
}

/**
  * A listener to perform actions as a result of Message Bus reflection.
  *
  * In some cases we want to perform lazy instantiation of components, such as
  * the compass and the accelerometer, where we only want to add them to the idle
  * fiber when someone has the intention of using these components.
  */
void Trinket::onListenerRegisteredEvent(Event evt)
{
}

/**
 * A periodic callback invoked by the fiber scheduler idle thread.
* We use this for any low priority, backgrounf housekeeping.
*
*/
void Trinket::idleCallback()
{
    codal_dmesg_flush();
}

void cplay_dmesg_flush()
{
#if CONFIG_ENABLED(DMESG_SERIAL_DEBUG)
#if DEVICE_DMESG_BUFFER_SIZE > 0

    if (codalLogStore.ptr > 0 && cplay_device_instance)
    {
        for (uint32_t i=0; i<codalLogStore.ptr; i++)
            ((Trinket *)cplay_device_instance)->serial.putc(codalLogStore.buffer[i]);

        codalLogStore.ptr = 0;
    }
#endif
#endif
}

