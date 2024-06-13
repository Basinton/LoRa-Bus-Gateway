/* Includes ------------------------------------------------------------------*/
#include "station.h"
#include "button.h"

#include "lora.h"
#include "rs485.h"
#include "crc16.h"

#include <Arduino.h>

/* Define --------------------------------------------------------------------*/
int32_t stationLAT = STATION_LAT * 10000000;
int32_t stationLNG = STATION_LNG * 10000000;

/* Variables -----------------------------------------------------------------*/
TaskHandle_t stationTaskHandle = NULL;

char debug_buffer[50];

BUS_ID busID = BUS_UNKNOWN;

uint8_t stationLoraSendMessage[10] = {0};
uint8_t responseToBoard[128] = {0};
uint16_t responseToBoardCRC = 0;

uint8_t errorCount[BUS_COUNT];

uint8_t isThereRequest[BUS_COUNT] = {0};
uint8_t isThereBusCancel[BUS_COUNT] = {0};
uint8_t isBusAccept[BUS_COUNT] = {0};
uint8_t isBusPass[BUS_COUNT] = {0};

uint8_t isBusReAckBusPass[BUS_COUNT] = {0};
uint8_t isBusReAckBusCancel[BUS_COUNT] = {0};

uint8_t isBoardReAckStationAccept[BUS_COUNT] = {0};
uint8_t isNotifyBusAcceptAck[BUS_COUNT] = {0};
uint8_t isNotifyBusPassAck[BUS_COUNT] = {0};
uint8_t isNotifyBusCancelAck[BUS_COUNT] = {0};

uint8_t isBoardReAckPassengerCancel[BUS_COUNT] = {0};
uint8_t isPassengerCancel[BUS_COUNT] = {0};
uint8_t isPassengerCancelAck[BUS_COUNT] = {0};
uint8_t isBusReAckPassengerCancel[BUS_COUNT] = {0};

SYSTEM_STATE busHandleState[BUS_COUNT] = {INIT};
uint8_t busHandleTimeout[BUS_COUNT] = {0};
BUS_RESPONSE busResponse[BUS_COUNT] = {0};

uint8_t requestID[BUS_COUNT] = {0};

/* Functions -----------------------------------------------------------------*/
void stationRequestToBus(BUS_ID busID);
void stationCancleToBus(void);
void stationAckToBus(BUS_ID busID, SYSTEM_STATE state);
void stationAckToBoard(BUS_ID busID, SYSTEM_STATE state);

void stationFsmResetState(BUS_ID busID, SYSTEM_STATE state);
void stationFsm(void);

void stationAckDebuger(void);
void boardAckDebuger(void);

void cancelProcess(void);

void stationTask(void *pvParameters)
{
    while (1)
    {
        cancelProcess();
        stationAckDebuger();
        boardAckDebuger();
        stationFsm();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void stationInit(void)
{
    xTaskCreate(stationTask, "Main process of Station", 4096, NULL, configMAX_PRIORITIES, &stationTaskHandle);

    Serial.println("sta: \t [init]");
}

void stationFsmResetState(BUS_ID busID = BUS_UNKNOWN, SYSTEM_STATE state = INIT)
{
    switch (state)
    {
    case INIT:
        memset(isThereRequest, 0, sizeof(isThereRequest));
        memset(isThereBusCancel, 0, sizeof(isThereBusCancel));
        memset(isBusPass, 0, sizeof(isBusPass));

        memset(isBusReAckBusPass, 0, sizeof(isBusReAckBusPass));
        memset(isBusReAckBusCancel, 0, sizeof(isBusReAckBusCancel));

        memset(isBoardReAckStationAccept, 0, sizeof(isBoardReAckStationAccept));
        memset(isNotifyBusAcceptAck, 0, sizeof(isNotifyBusAcceptAck));
        memset(isNotifyBusCancelAck, 0, sizeof(isNotifyBusCancelAck));
        break;
    case WAITING:

        Serial.println("sta: \t [fsm] waiting");
        break;

    case STATION_NOTIFY_ACCEPT_TO_BOARD:
        stationAckToBoard(busID, STATION_NOTIFY_ACCEPT_TO_BOARD);

        sprintf(debug_buffer, "sta: notify board accept bus %d", busID);
        Serial.println(debug_buffer);
        break;

    case REQUEST_TO_BUS:
        if (WiFi.status() == WL_CONNECTED)
        {
            if (busID == 0)
            {
                requestBus("Q10 055", "50");
            }
            else if (busID == 4)
            {
                requestBus("QTD 253", "08");
            }
        }
        stationRequestToBus(busID);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: request to bus %d", busID);
        Serial.println(debug_buffer);
        break;

    case STATION_NOTIFY_BUS_ACCEPT_TO_BOARD:
        stationAckToBoard(busID, STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: \t [fsm] notify bus %d accept", busID);
        Serial.println(debug_buffer);
        break;

    case BUS_ACCEPT:
        if (WiFi.status() == WL_CONNECTED)
        {
            if (busID == BUS_50)
            {
                updateRequestBus("Q10 055", "50", "2");
            }
            else if (busID == BUS_08)
            {
                updateRequestBus("QTD 253", "08", "2");
            }
        }
        // Serial.printf("sta: \t [fsm] bus accept (request id=%d, hi=%d, lo=%d)\n", busResponse.id, busResponse.addressHI, busResponse.addressLO);
        sprintf(debug_buffer, "sta: bus %d accept", busID);
        Serial.println(debug_buffer);
        break;

    case STATION_NOTIFY_BUS_PASS_TO_BOARD:
        stationAckToBoard(busID, STATION_NOTIFY_BUS_PASS_TO_BOARD);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: notify bus %d pass", busID);
        Serial.println(debug_buffer);
        break;

    case BUS_PASS:
        if (WiFi.status() == WL_CONNECTED)
        {
            if (busID == BUS_50)
            {
                updateRequestBus("Q10 055", "50", "3");
            }
            else if (busID == BUS_08)
            {
                updateRequestBus("QTD 253", "08", "3");
            }
        }
        stationAckToBus(busID, BUS_PASS);

        sprintf(debug_buffer, "sta: bus %d pass", busID);
        Serial.println(debug_buffer);
        break;

    case STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD:
        stationAckToBoard(busID, STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: notify bus %d cancel", busID);
        Serial.println(debug_buffer);
        break;

    case DRIVER_CANCEL:
        if (WiFi.status() == WL_CONNECTED)
        {
            if (busID == BUS_50)
            {
                updateRequestBus("Q10 055", "50", "4");
            }
            else if (busID == BUS_08)
            {
                updateRequestBus("QTD 253", "08", "4");
            }
        }
        stationAckToBus(busID, DRIVER_CANCEL);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: bus %d cancel", busID);
        Serial.println(debug_buffer);
        break;

    case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
        stationAckToBoard(busID, BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);

        sprintf(debug_buffer, "sta: notify passenger %d cancel", busID);
        Serial.println(debug_buffer);
        break;

    case PASSENGER_CANCEL:
        if (WiFi.status() == WL_CONNECTED)
        {
            if (busID == BUS_50)
            {
                updateRequestBus("Q10 055", "50", "5");
            }
            else if (busID == BUS_08)
            {
                updateRequestBus("QTD 253", "08", "5");
            }
        }
        stationAckToBus(busID, PASSENGER_CANCEL);
        busHandleTimeout[busID] = 40;

        sprintf(debug_buffer, "sta: passenger %d cancel", busID);
        Serial.println(debug_buffer);
        break;

    case FINISHED:
        requestID[busID] = (requestID[busID] + 1) % 256;

        sprintf(debug_buffer, "sta: bus %d finished", busID);
        Serial.println(debug_buffer);
        break;

    case ERROR_TIMEOUT:
        stationAckToBoard(busID, ERROR_TIMEOUT);
        sprintf(debug_buffer, "sta: bus %d request error - TIMEOUT", busID);
        Serial.println(debug_buffer);
        break;

    default:
        break;
    }
}

void stationFsm(void)
{
    for (int bus_i = BUS_50; bus_i <= BUS_08; bus_i++)
    {
        busID = (BUS_ID)bus_i; // Cast integer back to BUS_ID for usage
        switch (busHandleState[busID])
        {
        case INIT:
            requestID[busID] = 0;

            stationFsmResetState(busID, INIT);
            busHandleState[busID] = WAITING;
            break;

        case WAITING:

            if (isThereRequest[busID])
            {
                isThereRequest[busID] = 0;

                stationFsmResetState(busID, STATION_NOTIFY_ACCEPT_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_ACCEPT_TO_BOARD;
            }
            break;

        case STATION_NOTIFY_ACCEPT_TO_BOARD:
            stationFsmResetState(busID, REQUEST_TO_BUS);
            busHandleState[busID] = REQUEST_TO_BUS;
            break;

        case REQUEST_TO_BUS:
            if (isBusAccept[busID])
            {
                isBusAccept[busID] = 0;

                stationFsmResetState(busID, STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_BUS_ACCEPT_TO_BOARD;
            }
            else if (--busHandleTimeout[busID] == 0)
            {
                if (errorCount[busID] >= 20)
                {
                    errorCount[busID] = 0;
                    stationFsmResetState(busID, ERROR_TIMEOUT);
                    busHandleState[busID] = ERROR_TIMEOUT;
                    break;
                }
                errorCount[busID]++;
                stationFsmResetState(busID, REQUEST_TO_BUS);
                busHandleState[busID] = REQUEST_TO_BUS;
            }
            break;

        case STATION_NOTIFY_BUS_ACCEPT_TO_BOARD:

            if (isNotifyBusAcceptAck[busID])
            {
                isNotifyBusAcceptAck[busID] = 0;

                stationFsmResetState(busID, BUS_ACCEPT);
                busHandleState[busID] = BUS_ACCEPT;
            }
            else if (--busHandleTimeout[busID] == 0)
            {
                if (errorCount[busID] >= 20)
                {
                    errorCount[busID] = 0;
                    stationFsmResetState(busID, ERROR_TIMEOUT);
                    busHandleState[busID] = ERROR_TIMEOUT;
                    break;
                }
                errorCount[busID]++;
                stationFsmResetState(busID, STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_BUS_ACCEPT_TO_BOARD;
            }

            break;

        case BUS_ACCEPT:

            if (isBusPass[busID])
            {
                isBusPass[busID] = 0;

                stationFsmResetState(busID, STATION_NOTIFY_BUS_PASS_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_BUS_PASS_TO_BOARD;
            }
            else if (isThereBusCancel[busID])
            {
                isThereBusCancel[busID] = 0;

                stationFsmResetState(busID, STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD;
            }

            break;

        case STATION_NOTIFY_BUS_PASS_TO_BOARD:

            if (isNotifyBusPassAck[busID])
            {
                isNotifyBusPassAck[busID] = 0;

                stationFsmResetState(busID, BUS_PASS);
                busHandleState[busID] = BUS_PASS;
            }
            else if (--busHandleTimeout[busID] == 0)
            {
                if (errorCount[busID] >= 20)
                {
                    errorCount[busID] = 0;
                    stationFsmResetState(busID, ERROR_TIMEOUT);
                    busHandleState[busID] = ERROR_TIMEOUT;
                    break;
                }
                errorCount[busID]++;
                stationFsmResetState(busID, STATION_NOTIFY_BUS_PASS_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_BUS_PASS_TO_BOARD;
            }

            break;

        case BUS_PASS:
            stationFsmResetState(busID, FINISHED);
            busHandleState[busID] = FINISHED;
            break;

        case STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD:

            if (isNotifyBusCancelAck[busID])
            {
                isNotifyBusCancelAck[busID] = 0;

                stationFsmResetState(busID, DRIVER_CANCEL);
                busHandleState[busID] = DRIVER_CANCEL;
            }
            else if (--busHandleTimeout[busID] == 0)
            {
                if (errorCount[busID] >= 20)
                {
                    errorCount[busID] = 0;
                    stationFsmResetState(busID, ERROR_TIMEOUT);
                    busHandleState[busID] = ERROR_TIMEOUT;
                    break;
                }
                errorCount[busID]++;
                stationFsmResetState(busID, STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
                busHandleState[busID] = STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD;
            }

            break;

        case DRIVER_CANCEL:
            stationFsmResetState(busID, FINISHED);
            busHandleState[busID] = FINISHED;
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            stationFsmResetState(busID, PASSENGER_CANCEL);
            busHandleState[busID] = PASSENGER_CANCEL;
            break;

        case PASSENGER_CANCEL:

            if (isPassengerCancelAck[busID])
            {
                isPassengerCancelAck[busID] = 0;

                stationFsmResetState(busID, FINISHED);
                busHandleState[busID] = FINISHED;
            }
            else if (--busHandleTimeout[busID] == 0)
            {
                if (errorCount[busID] >= 20)
                {
                    errorCount[busID] = 0;
                    stationFsmResetState(busID, ERROR_TIMEOUT);
                    busHandleState[busID] = ERROR_TIMEOUT;
                    break;
                }
                errorCount[busID]++;
                stationFsmResetState(busID, PASSENGER_CANCEL);
                busHandleState[busID] = PASSENGER_CANCEL;
            }

            break;

        case FINISHED:
            stationFsmResetState(busID, WAITING);
            busHandleState[busID] = WAITING;
            break;

        case ERROR_TIMEOUT:
            stationFsmResetState(busID, WAITING);
            busHandleState[busID] = WAITING;
            break;

        default:
            busHandleState[busID] = INIT;
            break;
        }
    }
}

void stationAckDebuger(void)
{
    for (int bus_i = BUS_50; bus_i <= BUS_08; bus_i++)
    {
        busID = (BUS_ID)bus_i; // Cast integer back to BUS_ID for usage

        if (isBusReAckBusPass[busID])
        {
            isBusReAckBusPass[busID] = 0;

            stationAckToBus(busID, BUS_PASS);
            Serial.println("sta: \t [ReAck to bus] finish");
        }

        if (isBusReAckBusCancel[busID])
        {
            isBusReAckBusCancel[busID] = 0;

            stationAckToBus(busID, DRIVER_CANCEL);
            Serial.println("sta: \t [ReAck to bus] cancel");
        }
    }
}

void boardAckDebuger(void)
{
    for (int bus_i = BUS_50; bus_i <= BUS_08; bus_i++)
    {
        busID = (BUS_ID)bus_i; // Cast integer back to BUS_ID for usage

        if (isBoardReAckStationAccept[busID])
        {
            isBoardReAckStationAccept[busID] = 0;

            stationAckToBoard(busID, STATION_NOTIFY_ACCEPT_TO_BOARD);
            Serial.println("sta: \t [ReAck to board] station accept");
        }

        if (isBoardReAckPassengerCancel[busID])
        {
            isBoardReAckPassengerCancel[busID] = 0;

            stationAckToBoard(busID, PASSENGER_CANCEL);
            Serial.println("sta: \t [ReAck to board] passenger cancel");
        }
    }
}

void cancelProcess(void)
{
    for (int bus_i = BUS_50; bus_i <= BUS_08; bus_i++)
    {
        busID = (BUS_ID)bus_i; // Cast integer back to BUS_ID for usage

        if (isPassengerCancel[busID])
        {
            isPassengerCancel[busID] = 0;

            stationFsmResetState(busID, BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
            busHandleState[busID] = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
        }
    }
}

void stationRequestToBus(BUS_ID busID)
{
    stationLoraSendMessage[0] = 0xFF;

    stationLoraSendMessage[1] = requestID[busID];

    stationLoraSendMessage[2] = (GATEWAY_ADDRESS >> 8) & 0xFF; // HIGH
    stationLoraSendMessage[3] = GATEWAY_ADDRESS & 0xFF;        // LOW

    stationLoraSendMessage[4] = REQUEST_TO_BUS;
    stationLoraSendMessage[5] = busID;
    stationLoraSendMessage[6] = REQUESTED_BUS_DIRECTION;
    stationLoraSendMessage[7] = STATION_ID;

    stationLoraSendMessage[8] = checkSum(stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND);

    e32ttl100.sendBroadcastFixedMessage(BUS_CHANEL[busID], stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
    // e32ttl100.sendFixedMessage(busResponse.addressHI, busResponse.addressLO, BUS_50_CHANEL, stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
}

void stationAckToBus(BUS_ID busID, SYSTEM_STATE state)
{
    stationLoraSendMessage[0] = 0xFF;

    stationLoraSendMessage[1] = requestID[busID];

    stationLoraSendMessage[2] = (GATEWAY_ADDRESS >> 8) & 0xFF; // HIGH
    stationLoraSendMessage[3] = GATEWAY_ADDRESS & 0xFF;        // LOW

    stationLoraSendMessage[4] = state;
    stationLoraSendMessage[5] = busID; // Bus number
    stationLoraSendMessage[6] = 0;
    stationLoraSendMessage[7] = 0;

    stationLoraSendMessage[8] = checkSum(stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND);

    // e32ttl100.sendBroadcastFixedMessage(BUS_50_CHANEL, stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
    e32ttl100.sendFixedMessage(busResponse[busID].addressHI, busResponse[busID].addressLO, BUS_CHANEL[busID], stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
}

void stationAckToBoard(BUS_ID busID, SYSTEM_STATE state)
{
    responseToBoard[0] = busID;
    responseToBoard[1] = state;
    responseToBoardCRC = CRC16((char *)responseToBoard, 2);
    responseToBoard[2] = responseToBoardCRC & 0xFF;
    responseToBoard[3] = responseToBoardCRC >> 8;
    responseToBoard[4] = '\0';
    rs485_setmode(RS485_TRANSMIT);
    RS485_Serial.write((const uint8_t *)responseToBoard, 5);
    rs485_setmode(RS485_RECEIVE);
}