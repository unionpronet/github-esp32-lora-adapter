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
#include <Arduino.h>
#include <string.h>
#include "../../AppDef.h"
#include "../../AppEvent.h"
#include "../../AppLog.h"

#if CONFIG_IDF_TARGET_ESP32
#include "BtSerial.h"

BtSerial *BtSerial::_instance = nullptr;

BtSerial::BtSerial(QueueHandle_t eventQueue, const int16_t eventValue) : MessageQueue(eventQueue), eventValue(eventValue)
{
    configASSERT(eventQueue);
    _instance = this;
}

bool BtSerial::start(const char *name)
{
    LOG_TRACE("name=", name);

    this->begin(name); // Bluetooth device name
    this->register_callback(
        [_instance](esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
        {
            if (_instance)
            {
                _instance->onSppCallback(event, param);
            }
        });
    this->onData(
        [_instance](const uint8_t *buffer, size_t size)
        {
            if (_instance)
            {
                _instance->onDataCallback(buffer, size);
            }
        });

    return true;
}

void BtSerial::onSppCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT: // When SPP is inited, the event comes
        LOG_TRACE("event=ESP_SPP_INIT_EVT(", event, ")");
        break;
    case ESP_SPP_START_EVT: // When SPP server started, the event comes
        LOG_TRACE("event=ESP_SPP_START_EVT(", event, ")");
        break;
    case ESP_SPP_WRITE_EVT: // When SPP write operation completes, the event comes, only for ESP_SPP_MODE_CB
        LOG_TRACE("event=ESP_SPP_WRITE_EVT(", event, ")");
        break;
    case ESP_SPP_SRV_OPEN_EVT: // When SPP Server connection open, the event comes
        LOG_TRACE("event=ESP_SPP_SRV_OPEN_EVT(", event, ")");
        _instance->sendMessageToTask(eventValue, Connected);
        break;
    case ESP_SPP_CLOSE_EVT: //  When SPP connection closed, the event comes
        LOG_TRACE("event=ESP_SPP_CLOSE_EVT(", event, ")");
        _instance->sendMessageToTask(eventValue, Disconnected);
        break;
    default:
        LOG_TRACE("unsupported event=", event);
        break;
    }
}

void BtSerial::onDataCallback(const uint8_t *buffer, size_t size)
// void IRAM_ATTR BtSerial::onDataCallback(const uint8_t *buffer, size_t size)
{
    configASSERT(_instance);

    _instance->sendMessageToTask(eventValue, RxDone, 0, 0L);
}

#endif // CONFIG_IDF_TARGET_ESP32