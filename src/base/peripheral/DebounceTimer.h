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
#include "../../../AppDef.h"
#include "../model/Message.h"
#include "../message/MessageQueue.h"
#include "./SoftwareTimer.h"
#include "./DebounceDef.h"

#define ButtonListSize 10

class DebounceButton;

class DebounceTimer : public SoftwareTimer, MessageQueue
{
public:
    DebounceTimer(QueueHandle_t queue,
                  int eventValue, int paramValue) : MessageQueue(queue),
                                                    eventValue(eventValue), paramValue(paramValue)
    {
        instance = this;
        memset(buttonList, 0, sizeof(buttonList));

        timerHandle = xTimerCreate(
            "Debounce Timer",
            DebounceTimerInterval,
            pdTRUE, // The timers will auto-reload themselves when they expire.
            nullptr,
            [](TimerHandle_t xTimer)
            // [instance](TimerHandle_t xTimer)
            {
                if (instance != nullptr)
                {
                    instance->isr(xTimer);
                }
            });
        configASSERT(timerHandle != NULL);
    }

    bool attachButton(DebounceButton *button);
    bool detachButton(DebounceButton *button);

    virtual void onEventTimer(void);

protected:
    virtual void isr(TimerHandle_t xTimer)
    {
        sendMessageFromIsrToTask(eventValue, paramValue, 0, (uint32_t)xTimer);
    }

private:
    static DebounceTimer *instance;
    DebounceButton *buttonList[ButtonListSize];

    int eventValue;
    int paramValue;
};
