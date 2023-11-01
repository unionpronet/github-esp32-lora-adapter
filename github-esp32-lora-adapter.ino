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
#include <WiFi.h>
#include "./AppContext.h"
#include "./AppLog.h"
#include "./Version.h"
#include "./src/thread/ThreadApp.h"
#include "./src/thread/ThreadLora.h"

///////////////////////////////////////////////////////////////////////////////
static AppContext appContext = {0};

///////////////////////////////////////////////////////////////////////////////
static void initGlobalVar(void)
{
    WiFi.macAddress(appContext.mac);
    LOG_TRACE("WiFi MAC address: ", DebugLogBase::HEX, appContext.mac[0], "-", appContext.mac[1], "-", appContext.mac[2], "-", appContext.mac[3], "-", appContext.mac[4], "-", appContext.mac[5]);
}

static void createTasks(void)
{
    static ThreadApp threadApp;
    static ThreadLora threadLora;

    appContext.threadApp = &threadApp;
    appContext.threadLora = &threadLora;

    threadApp.start(&appContext);
    threadLora.start(&appContext);
}

static void printAppInfo(void)
{
    PRINTLN("===============================================================================");
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    PRINTLN("Firmware version=", Version::getFirmwareVersionString(),
            "\r\nchipModel=", ESP.getChipModel(), ", chipRevision=", ESP.getChipRevision(), ", flash size=", ESP.getFlashChipSize(),
            "\r\nNumber of cores=", ESP.getChipCores(), ", SDK version=", ESP.getSdkVersion());
#elif PICO_RP2040
    PRINTLN("App version = ", getFirmwareVersionString(), ", rp2040_chip_version() = ", rp2040_chip_version(), ", rp2040_rom_version() = ", rp2040_rom_version());
#endif
    PRINTLN("===============================================================================");
}

void setup()
{
    /////////////////////////////////////////////////////////////////////////////
    // initial serial port to 115200bps
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }

    // set log output to serial port, and init log params such as log_level
    LOG_SET_LEVEL(DefaultLogLevel);
    LOG_SET_DELIMITER("");
    LOG_ATTACH_SERIAL(Serial);
    /////////////////////////////////////////////////////////////////////////////

    printAppInfo();

    initGlobalVar();
    createTasks();

    // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
    // LOG_TRACE("loop()");
    delay(1000);
}