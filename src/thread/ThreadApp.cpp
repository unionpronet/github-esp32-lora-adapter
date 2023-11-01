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
#include <string.h>
#include "../../AppEvent.h"
#include "../../AppLog.h"
#include "ThreadApp.h"

////////////////////////////////////////////////////////////////////////////////////////////

static const char btNamePrefix[] = "ESP32";
static char btName[sizeof(btNamePrefix) + 1 + MAC_ADDRESS_SIZE * 2];

ThreadApp *ThreadApp::_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////
// Thread
////////////////////////////////////////////////////////////////////////////////////////////
#define RUNNING_CORE ARDUINO_RUNNING_CORE

#define TASK_NAME "ThreadApp"
#define TASK_STACK_SIZE 4096
#define TASK_PRIORITY 3
#define TASK_QUEUE_SIZE 128 // message queue size for app task

#define TASK_INIT_NAME "taskDelayInit"
#define TASK_INIT_STACK_SIZE 4096
#define TASK_INIT_PRIORITY 0

static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

static StackType_t xStack[TASK_STACK_SIZE];
static StaticTask_t xTaskBuffer;

////////////////////////////////////////////////////////////////////////////////////////////
ThreadApp::ThreadApp() : ThreadBase(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                         ledBluetooth(),
                         buttonBoot(queue()),
                         debounceTimer(queue(), EventSystem, SysSoftwareTimer),
                         isConnected(false),
#if CONFIG_IDF_TARGET_ESP32
                         btSerial(queue(), EventBt),
                         jsonMessage(&btSerial),
#endif
                         handlerMap()
{
    _instance = this;
    handlerMap = {
        __EVENT_MAP(ThreadApp, EventLoraRx),
        __EVENT_MAP(ThreadApp, EventBt),
        __EVENT_MAP(ThreadApp, EventSystem),
        __EVENT_MAP(ThreadApp, EventGpioISR),
        __EVENT_MAP(ThreadApp, EventNull), // {EventNull, &ThreadApp::handlerEventNull},
    };
}

///////////////////////////////////////////////////////////////////////
__EVENT_FUNC_DEFINITION(ThreadApp, EventBt, msg) // void ThreadApp::handlerEventBt(const Message &msg)
{
    // LOG_TRACE("EventBt(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

#if CONFIG_IDF_TARGET_ESP32
    BtSerial::TriggerSource src = static_cast<BtSerial::TriggerSource>(msg.iParam);
    switch (src)
    {
    case BtSerial::Connected:
        LOG_TRACE("BtSerial::TriggerSource::Connected, btSerial.hasClient()=", btSerial.hasClient());
        isConnected = true;
        ledBluetooth.on();
        break;
    case BtSerial::Disconnected:
        LOG_TRACE("BtSerial::TriggerSource::Disconnected, btSerial.hasClient()=", btSerial.hasClient());
        isConnected = false;
        ledBluetooth.off();
        break;
    default:
        LOG_TRACE("unsupported src=", src);
        break;
    }
#endif
}
__EVENT_FUNC_DEFINITION(ThreadApp, EventLoraRx, msg) // void ThreadApp::handlerEventLoraRx(const Message &msg)
{
    LOG_TRACE("EventLoraRx(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

    if (isConnected)
    {
        int16_t co2 = msg.iParam;
        double temperatureDegC = roundf((double)(msg.uParam)) / 100.0;
        double pressurePascal = roundf((double)(msg.lParam)) / 100.0;

        jsonMessage.clear();
        jsonMessage["event"] = "eventSensor";
        jsonMessage["arg0"] = co2;
        jsonMessage["arg1"] = temperatureDegC;
        jsonMessage["arg2"] = pressurePascal;
        jsonMessage.serialize();
    }
}
__EVENT_FUNC_DEFINITION(ThreadApp, EventSystem, msg) // void ThreadApp::handlerEventSystem(const Message &msg)
{
    // LOG_TRACE("EventSystem(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
    enum SystemTriggerSource src = static_cast<SystemTriggerSource>(msg.iParam);
    switch (src)
    {
    case SysSoftwareTimer:
        handlerSoftwareTimer((TimerHandle_t)(msg.lParam));
        break;

    case SysButtonClick:
    {
        int16_t pin = msg.uParam;
        if (pin == buttonBoot.getPin())
        {
            LOG_TRACE("ButtonClick: buttonBoot");
            configASSERT(context->threadLora);
            postEvent(context->threadLora, msg);
        }
        else
        {
            LOG_TRACE("SysButtonClick: unsupported pin=", pin);
        }
        break;
    }
    case SysButtonDoubleClick:
    {
        int16_t pin = msg.uParam;
        if (pin == buttonBoot.getPin())
        {
            LOG_TRACE("SysButtonDoubleClick: buttonBoot");
        }
        else
        {
            LOG_TRACE("SysButtonDoubleClick: unsupported pin=", pin);
        }
        break;
    }
    case SysButtonLongPress:
    {
        int16_t pin = msg.uParam;
        if (pin == buttonBoot.getPin())
        {
            LOG_TRACE("SysButtonLongPress: buttonBoot");
        }
        else
        {
            LOG_TRACE("SysButtonLongPress: unsupported pin=", pin);
        }
        break;
    }

    default:
        LOG_TRACE("unsupported SystemTriggerSource=", src);
        break;
    }
}
__EVENT_FUNC_DEFINITION(ThreadApp, EventGpioISR, msg) // void ThreadApp::handlerEventGpioISR(const Message &msg)
{
    LOG_TRACE("EventISR(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);

    uint8_t pin = msg.iParam;
    uint8_t value = msg.uParam;
    if (pin == buttonBoot.getPin())
    {
        uint32_t ms = msg.lParam;
        buttonBoot.onEventIsr(value, ms);
    }
    else
    {
        LOG_TRACE("unsupported button: GPIO", pin);
    }
}
__EVENT_FUNC_DEFINITION(ThreadApp, EventNull, msg) // void ThreadApp::handlerEventNull(const Message &msg)
{
    LOG_TRACE("EventNull(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
}

///////////////////////////////////////////////////////////////////////
void ThreadApp::onMessage(const Message &msg)
{
    // LOG_TRACE("event=", msg.event, ", iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    auto func = handlerMap[msg.event];
    if (func)
    {
        (this->*func)(msg);
    }
    else
    {
        LOG_TRACE("Unsupported event = ", msg.event, ", iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
    }
}

void ThreadApp::start(void *ctx)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    configASSERT(ctx);
    context = (AppContext *)ctx;

    taskHandle = xTaskCreateStaticPinnedToCore(
        [](void *instance)
        { static_cast<ThreadBase *>(instance)->run(); },
        TASK_NAME,
        TASK_STACK_SIZE, // This stack size can be checked & adjusted by reading the Stack Highwater
        this,
        TASK_PRIORITY, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        xStack,
        &xTaskBuffer,
        RUNNING_CORE);
}

void ThreadApp::setup(void)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());

    ledBluetooth.off();

    buttonBoot.init(EventSystem, SysButtonClick, SysButtonDoubleClick, SysButtonLongPress);
    debounceTimer.attachButton(&buttonBoot);

    // // do not call ThreadBase::setup() if delayInit() requires large stack size,
    // ThreadBase::setup();

    xTaskCreatePinnedToCore(
        [](void *_instance)
        {
            configASSERT(_instance);
            LOG_TRACE("taskDelayInit() on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
            ThreadApp *instance = static_cast<ThreadApp *>(_instance);
            instance->delayInit();
            // vTaskDelay(pdMS_TO_TICKS(100));        // delay 100ms
            vTaskDelete(instance->taskInitHandle); // init completed => delete itself
        },
        TASK_INIT_NAME,
        TASK_INIT_STACK_SIZE, // This stack size can be checked & adjusted by reading the Stack Highwater
        this,
        TASK_INIT_PRIORITY, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        &taskInitHandle,
        ARDUINO_RUNNING_CORE);
}

void ThreadApp::run(void)
{
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    ThreadBase::run();
}

// note: btSerial.start() requires large stack
void ThreadApp::delayInit(void)
{
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());

    const uint8_t *mac = context->mac;
    snprintf(btName, sizeof(btName), "%s-%02x%02x", btNamePrefix, mac[MAC_ADDRESS_SIZE - 2], mac[MAC_ADDRESS_SIZE - 1]);
    // LOG_TRACE("btName=", btName);

#if CONFIG_IDF_TARGET_ESP32
    btSerial.start(btName);
#endif
}

void ThreadApp::handlerSoftwareTimer(TimerHandle_t xTimer)
{
    if (xTimer == debounceTimer.timerHandle)
    {
        // LOG_TRACE("debounceTimer::timer()");
        debounceTimer.onEventTimer();
    }
    else
    {
        LOG_TRACE("unsupported timer handle=0x%04x", (uint32_t)(xTimer));
    }
}