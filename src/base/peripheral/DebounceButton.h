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
#include <FunctionalInterrupt.h>
#include "../../../AppEvent.h"
#include "../message/MessageQueue.h"
#include "../util/Tool.h"
#include "./DebounceDef.h"
#include "./DebounceTimer.h"

class DebounceTimer;

class DebounceButton : public MessageQueue
{
public:
    DebounceButton(uint8_t pin,
                   uint8_t activeState,
                   uint8_t ioMode,
                   QueueHandle_t queue) : PIN(pin),
                                          pinStateActive(activeState),
                                          isIntrEnable(false),
                                          debounceTimer(nullptr),
                                          eventValue(EventNull),
                                          buttonClick(EventNull),
                                          buttonDoubleClick(EventNull),
                                          buttonLongPress(EventNull),
                                          MessageQueue(queue)
    {
        debounceCount = 0;
        debounceActive = false;

        clickCount = 0;

        pinMode(PIN, ioMode);
    }

    ~DebounceButton()
    {
        disableInterrupt();
    }

    bool init(int16_t evValue, int16_t clickValue, int16_t doubleClickValue, int16_t longPressValue)
    {
        eventValue = evValue;
        buttonClick = clickValue;
        buttonDoubleClick = doubleClickValue;
        buttonLongPress = longPressValue;
        return true;
    }

    void enableInterrupt(uint8_t intrMode)
    {
        if (!isIntrEnable)
        {
            attachInterrupt(digitalPinToInterrupt(PIN), std::bind(&DebounceButton::isr, this), intrMode);
            isIntrEnable = true;
        }
    }

    void disableInterrupt(void)
    {
        if (isIntrEnable)
        {
            detachInterrupt(digitalPinToInterrupt(PIN));
            isIntrEnable = false;
        }
    }

    void setDebounceActive(bool active)
    {
        debounceCount = 0;
        debounceActive = active;
    }

    bool isDebounceActive(void)
    {
        return debounceActive;
    }

    void onEventIsr(uint8_t value, uint32_t ms)
    {
        if (value == getActiveState())
        {
            timeBegin = ms;
            if (clickCount == 0)
            {
                setDebounceActive(true);
                if (debounceTimer)
                {
                    debounceTimer->start();
                }
            }
        }
        else if (isDebounceActive())
        {
            timeEnd = ms;
            if (Tool::deltaTime(ms, timeBegin) > DebounceDuration)
            {
                clickCount++;
            }
        }
    }

    void onEventTimer(void)
    {
        if (!isDebounceActive())
        {
            return;
        }

        if (digitalRead(PIN) == pinStateActive)
        {
            if (debounceCount >= LongPressDuration)
            {
                clickCount = 0;
                setDebounceActive(false);
                sendMessageToTask(eventValue, buttonLongPress, PIN);
            }
            else
            {
                debounceCount += DebounceTimerInterval;
            }
        }
        else if (debounceCount >= DoubleClickDuration)
        {
            int16_t event;
            if (clickCount == 1)
            {
                sendMessageToTask(eventValue, buttonClick, PIN);
            }
            else if (clickCount == 2)
            {
                sendMessageToTask(eventValue, buttonDoubleClick, PIN);
            }

            clickCount = 0;
            setDebounceActive(false);
        }
        else
        {
            debounceCount += DebounceTimerInterval;
            // setDebounceActive(false);
        }
    }

    uint8_t getPin(void)
    {
        return PIN;
    }
    uint8_t getActiveState(void)
    {
        return pinStateActive;
    }

protected:
    uint32_t debounceCount;
    bool debounceActive;
    uint8_t pinStateActive;
    bool isIntrEnable;

private:
    void isr(void)
    {
        sendMessageFromIsrToTask(EventGpioISR, PIN, digitalRead(PIN), millis());
    }

    int16_t eventValue;
    int16_t buttonClick;
    int16_t buttonDoubleClick;
    int16_t buttonLongPress;

    friend DebounceTimer;
    DebounceTimer *debounceTimer;

    uint16_t clickCount;
    uint32_t timeBegin, timeEnd;

    const uint8_t PIN;
    QueueHandle_t queue;
};
