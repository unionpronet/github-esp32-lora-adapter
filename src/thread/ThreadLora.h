/* Copyright 2023 unionpro.net@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <map>
#include "../../AppDef.h"
#include "../../AppEvent.h"
#include "../base/peripheral/PeriodicTimer.h"
#include "../comm/Lora.h"
#include "./ThreadBase.h"

///////////////////////////////////////////////////////////////////////////////
class ThreadLora : public ThreadBase
{
public:
    ThreadLora();
    virtual void start(void *);

    static ThreadLora *getInstance(void)
    {
        return _instance;
    }

protected:
    typedef void (ThreadLora::*handlerFunc)(const Message &);
    std::map<int16_t, handlerFunc> handlerMap;

    virtual void onMessage(const Message &msg);

    virtual void run(void);

private:
    ///////////////////////////////////////////////////////////////////////////
    static ThreadLora *_instance;
    TaskHandle_t taskInitHandle;

    PeriodicTimer timer1Hz;
    Lora lora;

    ///////////////////////////////////////////////////////////////////////////
    virtual void
    setup(void);
    virtual void delayInit(void);

    void handlerSoftwareTimer(TimerHandle_t xTimer);
    void handlerButtonClick(const Message &msg);
    void handlerLoraDio1(void);
    void handlerRxDone(void);

    void processPayloadData(const void *ptrPayload);

    void generateRandomSensorData(int16_t &co2, uint16_t &temperature, uint32_t &pressure);
    void simulateLoraRx();

    ///////////////////////////////////////////////////////////////////////////
    // event handler
    ///////////////////////////////////////////////////////////////////////////
    __EVENT_FUNC_DECLARATION(EventLora)
    __EVENT_FUNC_DECLARATION(EventSystem)
    __EVENT_FUNC_DECLARATION(EventNull) // void handlerEventNull(const Message &msg);
};