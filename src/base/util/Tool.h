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
#include <soc/rtc_wdt.h>
#include "../../../AppLog.h"

class Tool
{
public:
    static uint32_t deltaTime(uint32_t begin, uint32_t end)
    {
        unsigned long delta;
        if (end > begin)
        {
            delta = end - begin;
        }
        else
        {
            delta = begin - end;
        }
        return delta;
    }

    static bool isWakeupByDeepSleep(void)
    {
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
            return true;
        case ESP_SLEEP_WAKEUP_EXT1:
            return true;
        case ESP_SLEEP_WAKEUP_TIMER:
            return true;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return true;
        case ESP_SLEEP_WAKEUP_ULP:
            return true;
        default:
            return false;
        }
    }

    // Method to print the reason by which ESP32 has been awaken from sleep
    static void print_wakeup_reason(void)
    {
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            LOG_TRACE("Wakeup was not caused by exit from deep sleep");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            LOG_TRACE("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            LOG_TRACE("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            LOG_TRACE("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            LOG_TRACE("Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            LOG_TRACE("Wakeup caused by ULP program");
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            LOG_TRACE("Wakeup caused by GPIO");
            break;
        case ESP_SLEEP_WAKEUP_UART:
            LOG_TRACE("Wakeup caused by UART");
            break;
        case ESP_SLEEP_WAKEUP_WIFI:
            LOG_TRACE("Wakeup caused by WIFI");
            break;
        case ESP_SLEEP_WAKEUP_COCPU:
            LOG_TRACE("Wakeup caused by COCPU int");
            break;
        case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
            LOG_TRACE("Wakeup caused by COCPU crash");
            break;
        case ESP_SLEEP_WAKEUP_BT:
            LOG_TRACE("Wakeup caused by BT");
            break;
        default:
            LOG_TRACE("Wakeup was not caused by deep sleep: ", wakeup_reason);
            break;
        }
    }

    static void print_wdt_timeout(void)
    {
        unsigned int timeout_ms;
        esp_err_t espResult;

        espResult = rtc_wdt_get_timeout(RTC_WDT_STAGE0, &timeout_ms);
        LOG_TRACE("rtc_wdt_get_timeout(RTC_WDT_STAGE0) returns ", espResult, ", timeout_ms=", timeout_ms);

        espResult = rtc_wdt_get_timeout(RTC_WDT_STAGE1, &timeout_ms);
        LOG_TRACE("rtc_wdt_get_timeout(RTC_WDT_STAGE1) returns ", espResult, ", timeout_ms=", timeout_ms);

        espResult = rtc_wdt_get_timeout(RTC_WDT_STAGE2, &timeout_ms);
        LOG_TRACE("rtc_wdt_get_timeout(RTC_WDT_STAGE2) returns ", espResult, ", timeout_ms=", timeout_ms);

        espResult = rtc_wdt_get_timeout(RTC_WDT_STAGE3, &timeout_ms);
        LOG_TRACE("rtc_wdt_get_timeout(RTC_WDT_STAGE3) returns ", espResult, ", timeout_ms=", timeout_ms);
    }
};
