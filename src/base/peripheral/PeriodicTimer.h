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

class PeriodicTimer
{
public:
    PeriodicTimer(const char *pcTimerName, TickType_t xTimerPeriodInTicks,
                  BaseType_t xAutoReload, TimerCallbackFunction_t pxCallbackFunction)
    {
        hTimer = xTimerCreate(pcTimerName,
                              xTimerPeriodInTicks,
                              xAutoReload,
                              nullptr,
                              pxCallbackFunction);
    }

    ~PeriodicTimer()
    {
        xTimerDelete(hTimer, 0);
        hTimer = nullptr;
    }

    static PeriodicTimer *create(TimerCallbackFunction_t pxCallbackFunction, uint32_t interval_in_ms)
    {
        return new PeriodicTimer("TimerPeriodic",
                                 pdMS_TO_TICKS(interval_in_ms),
                                 pdTRUE, // auto-reload when expire.
                                 pxCallbackFunction);
    }

    void start(void)
    {
        if (xTimerIsTimerActive(hTimer) == pdFALSE)
        {
            xTimerStart(hTimer, 0);
        }
    }

    void stop(void)
    {
        if (xTimerIsTimerActive(hTimer) != pdFALSE)
        {
            xTimerStop(hTimer, 0);
        }
    }

    TimerHandle_t timer(void)
    {
        return hTimer;
    }

private:
    TimerHandle_t hTimer;
};
