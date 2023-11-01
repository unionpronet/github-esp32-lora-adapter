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

#include "../model/Message.h"

class MessageQueue
{
public:
    MessageQueue() : _queue(nullptr){};
    MessageQueue(QueueHandle_t queue) : _queue(queue){};
    ~MessageQueue() { _queue = nullptr; };

    void sendMessageToTask(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        if (_queue == nullptr)
        {
            return;
        }

        Message msg;
        msg.event = event;
        msg.iParam = iParam;
        msg.uParam = uParam;
        msg.lParam = lParam;
        if (xQueueSend(_queue, &msg, 0) != pdTRUE)
        {
            // DBGLOG(Debug, "%s - sendMessageToTask failed!", TAG);
        }
    }

    void sendMessageFromIsrToTask(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        if (_queue == nullptr)
        {
            return;
        }

        Message msg;
        msg.event = event;
        msg.iParam = iParam;
        msg.uParam = uParam;
        msg.lParam = lParam;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(_queue, &msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR();
    }

    inline QueueHandle_t queue(void)
    {
        return _queue;
    }

protected:
    QueueHandle_t _queue;
};
