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
#include "../base/message/MessageQueue.h"

#if CONFIG_IDF_TARGET_ESP32
#include <BluetoothSerial.h>

#define BtRxPacketListSize 8

class BtSerial : public BluetoothSerial, MessageQueue
{
public:
    enum TriggerSource : int16_t
    {
        Connected,
        Disconnected,
        RxDone,
        TxDone
    };

    BtSerial(QueueHandle_t eventQueue, const int16_t eventValue);
    bool start(const char *name);

private:
    static BtSerial *_instance;

    const int16_t eventValue;

    void onSppCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

    void onDataCallback(const uint8_t *buffer, size_t size);
};

#endif // CONFIG_IDF_TARGET_ESP32