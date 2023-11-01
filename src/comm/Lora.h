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
#include <SPI.h>
#include "../../AppLog.h"
#include "../../pins.h"
#include "../../RfConfig.h"
#include "../base/message/MessageQueue.h"

#define RADIOLIB_GODMODE
#define RADIOLIB_STATIC_ONLY
#include <RadioLib.h>

#if CONFIG_IDF_TARGET_ESP32
#define LORA_SPI SPI
#elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
static SPIClass SPI2(VSPI);
#define LORA_SPI SPI2
#endif

class Lora : public LLCC68, MessageQueue
{
public:
    enum TriggerSource : int16_t
    {
        IsrDio1,
        RxDone,
        TxDone
    };

    Lora(QueueHandle_t eventQueue,
         const int16_t eventValue) : MessageQueue(eventQueue),
                                     eventValue(eventValue),
                                     module(PIN_RF_NSS, PIN_RF_DIO1, PIN_RF_RST, PIN_RF_BUSY, LORA_SPI),
                                     LLCC68(&module)
    {
        _instance = this;
    }

    bool init(const RfConfig &rf)
    {
#if CONFIG_IDF_TARGET_ESP32
        LORA_SPI.begin();
#elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
        // LORA_SPI.begin();
        LORA_SPI.begin(PIN_VSCK, PIN_VMISO, PIN_VMOSI, PIN_RF_NSS); // SCLK, MISO, MOSI, SS
#endif
        int16_t result = reset();
        if (result != RADIOLIB_ERR_NONE)
        {
            LOG_ERROR("reset() returns ", result);
            return false;
        }

        LOG_TRACE("rfConfig.freq=", rf.freq, ", bw=", rf.bw, ", sf=", rf.sf, ", cr=", rf.cr,
                  ", syncWord=", rf.syncWord, ", power=", rf.power, ", preambleLength=", rf.preambleLength,
                  ", tcxoVoltage=", rf.tcxoVoltage, ", useLDO=", rf.useLDO);
        result = begin(rf.freq, rf.bw, rf.sf, rf.cr,
                       rf.syncWord, rf.power, rf.preambleLength,
                       rf.tcxoVoltage, rf.useLDO);
        if (result != RADIOLIB_ERR_NONE)
        {
            LOG_ERROR("begin() returns ", result);
            return false;
        }
        switch (result)
        {
        case RADIOLIB_ERR_NONE:
            // success => move forward
            break;
        case RADIOLIB_ERR_UNKNOWN:
            LOG_ERROR("RADIOLIB_ERR_UNKNOWN (", RADIOLIB_ERR_UNKNOWN, ")");
            return false;
        case RADIOLIB_ERR_CHIP_NOT_FOUND:
            LOG_ERROR("RADIOLIB_ERR_CHIP_NOT_FOUND (", RADIOLIB_ERR_CHIP_NOT_FOUND, ")");
            return false;
        case RADIOLIB_ERR_INVALID_BANDWIDTH:
            LOG_ERROR("RADIOLIB_ERR_INVALID_BANDWIDTH (", RADIOLIB_ERR_INVALID_BANDWIDTH, ")");
            return false;
        case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
            LOG_ERROR("RADIOLIB_ERR_INVALID_SPREADING_FACTOR (", RADIOLIB_ERR_INVALID_SPREADING_FACTOR, ")");
            return false;
        case RADIOLIB_ERR_INVALID_CODING_RATE:
            LOG_ERROR("RADIOLIB_ERR_INVALID_CODING_RATE (", RADIOLIB_ERR_INVALID_CODING_RATE, ")");
            return false;
        case RADIOLIB_ERR_INVALID_BIT_RANGE:
            LOG_ERROR("RADIOLIB_ERR_INVALID_BIT_RANGE (", RADIOLIB_ERR_INVALID_BIT_RANGE, ")");
            return false;
        case RADIOLIB_ERR_INVALID_FREQUENCY:
            LOG_ERROR("RADIOLIB_ERR_INVALID_FREQUENCY (", RADIOLIB_ERR_INVALID_FREQUENCY, ")");
            return false;
        case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
            LOG_ERROR("RADIOLIB_ERR_INVALID_OUTPUT_POWER (", RADIOLIB_ERR_INVALID_OUTPUT_POWER, ")");
            return false;
        default:
            LOG_ERROR("unsupported begin() return: ", result);
            return false;
        }

        result = setDio2AsRfSwitch(true);
        if (result != RADIOLIB_ERR_NONE)
        {
            LOG_ERROR("setDio2AsRfSwitch() returns ", result);
            return false;
        }

        // set the function that will be called when new packet is received
        setDio1Action([_instance]()
                      {
                          // LOG_TRACE("DIO1 interrupt");
                          if (_instance)
                          {
                              _instance->sendMessageFromIsrToTask(_instance->eventValue, IsrDio1);
                          } });

        result = startReceive();
        if (result != RADIOLIB_ERR_NONE)
        {
            LOG_ERROR("startReceive() returns ", result);
            return false;
        }

        return true;
    }

private:
    static Lora *_instance;

    const int16_t eventValue;

    Module module;
};
