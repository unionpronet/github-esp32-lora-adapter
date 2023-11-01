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
#include <Arduino.h>
#include "../base/message/MessageHandler.h"
#include "../../AppContext.h"

class ThreadBase : public MessageHandler
{
public:
    ThreadBase(uint16_t queueLength,
               uint8_t *pucQueueStorageBuffer = nullptr,
               StaticQueue_t *pxQueueBuffer = nullptr) : taskHandle(nullptr),
                                                         context(nullptr),
                                                         taskDone(false),
                                                         MessageHandler()
    {
        if (pucQueueStorageBuffer != nullptr && pxQueueBuffer != nullptr)
        {
            _queue = xQueueCreateStatic(queueLength, sizeof(Message), pucQueueStorageBuffer, pxQueueBuffer);
        }
        else
        {
            _queue = xQueueCreate(queueLength, sizeof(Message));
        }
        configASSERT(_queue != NULL);
    };

    void postEvent(ThreadBase *thread, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, TickType_t xTicksToWait = 0)
    {
        if (thread == nullptr)
        {
            return;
        }

        Message msg;
        msg.event = event;
        msg.iParam = iParam;
        msg.uParam = uParam;
        msg.lParam = lParam;

        postEvent(thread, msg, xTicksToWait);
    }
    inline void postEvent(ThreadBase *thread, const Message &msg, TickType_t xTicksToWait = 0)
    {
        if (thread == nullptr)
        {
            return;
        }

        if (xQueueSend(thread->_queue, &msg, xTicksToWait) != pdTRUE)
        {
            // LOG_ERROR("xQueueSend failed!");
        }
    }

    inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, TickType_t xTicksToWait = 0)
    {
        postEvent(this, event, iParam, uParam, lParam, xTicksToWait);
    }
    inline void postEvent(const Message &msg, TickType_t xTicksToWait = 0)
    {
        postEvent(this, msg, xTicksToWait);
    }

    virtual void messageLoop(TickType_t xTicksToWait = portMAX_DELAY)
    {
        Message msg;
        if (xQueueReceive(_queue, (void *)&msg, xTicksToWait) == pdTRUE)
        {
            onMessage(msg);
        }
        else
        {
            // LOG_TRACE("xQueueReceive() timeout");
        }
    }

    virtual void messageLoopForever(void)
    {
        while (!taskDone)
        {
            messageLoop();
        }
    }

    virtual void delayInit(void) {}

    virtual void start(void *) = 0;
    virtual void onMessage(const Message &msg) = 0;

    virtual void setup(void)
    {
        BaseType_t result = xTimerPendFunctionCall(
            [](void *param1, uint32_t param2)
            {
                // LOG_TRACE("xTimerPendFunctionCall()");
                static_cast<ThreadBase *>(param1)->delayInit();
            },
            this,        // param1
            (uint32_t)0, // param2
            pdMS_TO_TICKS(200));
        // LOG_TRACE("xTimerPendFunctionCall() returns ", result);
    }

    virtual void run(void)
    {
        setup();

        messageLoopForever();

        vTaskDelay(pdMS_TO_TICKS(100)); // delay 100ms
        vTaskDelete(taskHandle);
    }

protected:
    bool taskDone;
    TaskHandle_t taskHandle;
    AppContext *context;
};