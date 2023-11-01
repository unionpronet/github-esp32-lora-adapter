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
#include <MemStream.h>
#include "../../AppEvent.h"
#include "../../AppLog.h"
#include "../../RfConfig.h"
#include "../lora/Packet.h"
#include "../base/model/JsonMessage.h"
#include "ThreadLora.h"

// #define SIMULATE_LORA // define it to simulate receiving Lora message regularly

///////////////////////////////////////////////////////////////////////////////
static const RfConfig rfConfig = {
    915.0, // float freq;
    125.0, // float bw;
    9,     // uint8_t sf;
    7,     // uint8_t cr;
    18,    // uint8_t syncWord;
    -9,    // int8_t power;
    8,     // uint16_t preambleLength;
    0,     // float tcxoVoltage;
    false  // bool useLDO;
};
///////////////////////////////////////////////////////////////////////////////
// Thread
///////////////////////////////////////////////////////////////////////////////
#define RUNNING_CORE ARDUINO_RUNNING_CORE

#define TASK_NAME "ThreadLora"
#define TASK_STACK_SIZE 4096
#define TASK_PRIORITY 5
#define TASK_QUEUE_SIZE 128 // message queue size for app task

static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

static StackType_t xStack[TASK_STACK_SIZE];
static StaticTask_t xTaskBuffer;

///////////////////////////////////////////////////////////////////////////////
ThreadLora *ThreadLora::_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////
ThreadLora::ThreadLora() : ThreadBase(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                           lora(queue(), EventLora),
                           timer1Hz("Timer 1Hz",
                                    pdMS_TO_TICKS(1000),
                                    pdTRUE, // auto-reload when expire.
                                    [](TimerHandle_t xTimer)
                                    {
                                        if (_instance)
                                        {
                                            _instance->postEvent(EventSystem, SysSoftwareTimer, 0, (uint32_t)xTimer);
                                        }
                                    }),
                           handlerMap()
{
    _instance = this;

    handlerMap = {
        __EVENT_MAP(ThreadLora, EventLora),
        __EVENT_MAP(ThreadLora, EventSystem),
        __EVENT_MAP(ThreadLora, EventNull), // {EventNull, &ThreadLora::handlerEventNull},
    };
}

///////////////////////////////////////////////////////////////////////////////
// Event handlers
///////////////////////////////////////////////////////////////////////////////
__EVENT_FUNC_DEFINITION(ThreadLora, EventLora, msg) // void ThreadLora::handlerEventLora(const Message &msg)
{
    LOG_TRACE("EventLora(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
    Lora::TriggerSource src = static_cast<Lora::TriggerSource>(msg.iParam);
    switch (src)
    {
    case Lora::TriggerSource::IsrDio1:
    {
        LOG_TRACE("Lora::TriggerSource::IsrDio1(", src, ")");
        handlerLoraDio1();
        break;
    }

    default:
        LOG_TRACE("unsupported src=", src);
        break;
    }
}

__EVENT_FUNC_DEFINITION(ThreadLora, EventSystem, msg) // void ThreadLora::handlerEventSystem(const Message &msg)
{
    // LOG_TRACE("EventSystem(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
    enum SystemTriggerSource src = static_cast<SystemTriggerSource>(msg.iParam);
    switch (src)
    {
    case SysSoftwareTimer:
        handlerSoftwareTimer((TimerHandle_t)(msg.lParam));
        break;
    case SysButtonClick:
        handlerButtonClick(msg);
        break;
    default:
        LOG_TRACE("unsupported SystemTriggerSource=", src);
        break;
    }
}

__EVENT_FUNC_DEFINITION(ThreadLora, EventNull, msg) // void ThreadLora::handlerEventNull(const Message &msg)
{
    LOG_TRACE("EventNull(", msg.event, "), iParam = ", msg.iParam, ", uParam = ", msg.uParam, ", lParam = ", msg.lParam);
}

///////////////////////////////////////////////////////////////////////////////
void ThreadLora::onMessage(const Message &msg)
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

void ThreadLora::start(void *ctx)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
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

void ThreadLora::setup(void)
{
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());

    // do not call ThreadBase::setup() if delayInit() requires large stack size,
    ThreadBase::setup();
}

void ThreadLora::run(void)
{
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    ThreadBase::run();
}

void ThreadLora::delayInit(void)
{
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    timer1Hz.start();

    bool initRst = lora.init(rfConfig);
    LOG_TRACE("initRst=", initRst);
}
void ThreadLora::handlerLoraDio1(void)
{
    uint16_t irqStatus = lora.getIrqStatus();
    lora.clearIrqStatus(RADIOLIB_SX126X_IRQ_ALL);

    if (irqStatus & RADIOLIB_SX126X_IRQ_TX_DONE)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_TX_DONE (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_TX_DONE, ")");

        // put module back to listen mode
        lora.startReceive();
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_RX_DONE)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_RX_DONE (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_RX_DONE, ")");
        handlerRxDone();
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_RADIOLIB_PREAMBLE_DETECTED)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_RADIOLIB_PREAMBLE_DETECTED (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_RADIOLIB_PREAMBLE_DETECTED, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_SYNC_WORD_VALID)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_SYNC_WORD_VALID (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_SYNC_WORD_VALID, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_HEADER_VALID)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_HEADER_VALID (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_HEADER_VALID, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_HEADER_ERR)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_HEADER_ERR (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_HEADER_ERR), ")";
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_CRC_ERR)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_CRC_ERR (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_CRC_ERR, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_CAD_DONE)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_CAD_DONE (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_CAD_DONE, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_CAD_DETECTED)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_CAD_DETECTED (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_CAD_DETECTED, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_TIMEOUT)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_TIMEOUT (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_TIMEOUT, ")");
    }
    if (irqStatus & RADIOLIB_SX126X_IRQ_LR_FHSS_HOP)
    {
        LOG_TRACE("RADIOLIB_SX126X_IRQ_LR_FHSS_HOP (0x", DebugLogBase::HEX, RADIOLIB_SX126X_IRQ_LR_FHSS_HOP, ")");
    }

    // // put module back to listen mode
    // lora.startReceive();
}
void ThreadLora::handlerRxDone(void)
{
    static rf::Packet packet;
    memset(&packet, 0, sizeof(packet));
    uint8_t length = lora.getPacketLength();
    uint8_t *ptrPayload = (uint8_t *)(void *)(&packet.payload.data);
    int state = lora.readData(ptrPayload, length);
    LOG_TRACE("length=", length, ", dataBuf=", DebugLogBase::HEX, LOG_AS_ARR(packet.payload.data, length));

    switch (state)
    {
    case RADIOLIB_ERR_NONE:
    {
        packet.payload.length = length;
        packet.rssi = lora.getRSSI();
        packet.snr = lora.getSNR();
        LOG_TRACE("rssi=", packet.rssi, ", snr=", packet.snr);

        processPayloadData(ptrPayload);
        break;
    }
    case RADIOLIB_ERR_CRC_MISMATCH:
        LOG_TRACE("CRC error!");
        break;
    default:
        LOG_TRACE("Unsupported state=", state);
        break;
    }
}

void ThreadLora::handlerSoftwareTimer(TimerHandle_t xTimer)
{
    if (xTimer == timer1Hz.timer())
    {
        // LOG_TRACE("timer1Hz");
        simulateLoraRx();
    }
    else
    {
        LOG_TRACE("unsupported timer handle=0x%04x", (uint32_t)(xTimer));
    }
}

void ThreadLora::handlerButtonClick(const Message &msg)
{
    // LOG_TRACE("handlerButtonClick");

    static rf::Packet packet;
    memset(&packet, 0, sizeof(packet));

    rf::Payload *ptrPayload = &packet.payload;
    ptrPayload->length = 0;

    int16_t co2;
    uint16_t temperature;
    uint32_t pressure;
    generateRandomSensorData(co2, temperature, pressure);

    static uint8_t memBuffer[MessageJsonDocSize];
    static MemStream memStream(memBuffer, sizeof(memBuffer));
    static JsonMessage<MessageJsonDocSize> jsonMessage(&memStream);

    memStream.flush();
    jsonMessage.clear();
    jsonMessage["event"] = "eventSensor";
    jsonMessage["arg0"] = co2;
    jsonMessage["arg1"] = temperature;
    jsonMessage["arg2"] = pressure;
    jsonMessage.serialize();

    const char *data = (const char *)memStream;
    size_t size = min(sizeof(rf::Payload::data), strlen(data));
    memcpy(ptrPayload->data, data, size);
    ptrPayload->length = size;

    lora.startTransmit(ptrPayload->data, ptrPayload->length);

    LOG_TRACE("ptrPayload->length=", ptrPayload->length, ", data=", (const char *)ptrPayload->data);
}

void ThreadLora::processPayloadData(const void *ptrPayload)
{
    static JsonMessage<MessageJsonDocSize> jsonMessage;

    jsonMessage.clear();
    DeserializationError error = jsonMessage.deserialize((const char *)ptrPayload);
    switch (error.code())
    {
    case DeserializationError::Ok:
    {
        // LOG_TRACE("DeserializationError::Ok");

        Message msg;
        msg.event = EventLoraRx;
        msg.iParam = jsonMessage["arg0"];
        msg.uParam = jsonMessage["arg1"];
        msg.lParam = jsonMessage["arg2"];
        LOG_TRACE("DeserializationError::Ok: msg.iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

        configASSERT(context && context->threadApp);
        postEvent(context->threadApp, msg);

        break;
    }
    default:
        LOG_TRACE("jsonMessage.deserialize() returns error (", error.code(), ")");
        break;
    }
}

void ThreadLora::generateRandomSensorData(int16_t &co2, uint16_t &temperature, uint32_t &pressure)
{
    co2 = rand() % 32000;
    temperature = rand() % 10000;
    pressure = (rand() % (1200 - 300)) + 300;
}

#define SIMULATE_LORA_RX_INTERVAL 5
void ThreadLora::simulateLoraRx()
{
#ifdef SIMULATE_LORA
    static int count = 0;
    if (++count > SIMULATE_LORA_RX_INTERVAL)
    {
        configASSERT(context && context->threadApp);
        count = 0;

        int16_t co2;
        uint16_t temperature;
        uint32_t pressure;
        generateRandomSensorData(co2, temperature, pressure);

        postEvent(context->threadApp, EventLoraRx, co2, temperature, pressure);
    }
#endif // SIMULATE_LORA
}
