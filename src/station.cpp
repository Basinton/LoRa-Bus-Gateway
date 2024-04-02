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

uint8_t stationLoraSendMessage[50] = {0};
uint8_t responseToBoard[128]       = {0};
uint16_t responseToBoardCRC        = 0;

uint8_t isThereRequest   = 0;
uint8_t isThereBusCancel = 0;
uint8_t isBusAccept      = 0;
uint8_t isBusPass        = 0;

uint8_t isBusReAckBusPass   = 0;
uint8_t isBusReAckBusCancel = 0;

uint8_t isBoardReAckStationAccept = 0;
uint8_t isNotifyBusAcceptAck      = 0;
uint8_t isNotifyBusPassAck        = 0;
uint8_t isNotifyBusCancelAck      = 0;

uint8_t isBoardReAckPassengerCancel = 0;
uint8_t isPassengerCancel           = 0;
uint8_t isPassengerCancelAck        = 0;
uint8_t isBusReAckPassengerCancel   = 0;

SYSTEM_STATE busHandleState = INIT;
int busHandleTimeout        = 0;
BUS_RESPONSE busResponse    = {0};

uint8_t requestID = 0;

/* Functions -----------------------------------------------------------------*/
void stationRequestToBus(void);
void stationCancleToBus(void);
void stationAckToBus(SYSTEM_STATE state);
void stationAckToBoard(SYSTEM_STATE state);

void stationFsmResetState(SYSTEM_STATE state);
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
    xTaskCreate(stationTask, "Main process of Station", 8192, NULL, configMAX_PRIORITIES - 1, &stationTaskHandle);

    Serial.println("sta: \t [init]");
}

void stationFsmResetState(SYSTEM_STATE state)
{
    switch (state)
    {
        case WAITING:
            isThereRequest   = 0;
            isThereBusCancel = 0;
            isBusAccept      = 0;
            isBusPass        = 0;

            isBusReAckBusPass   = 0;
            isBusReAckBusCancel = 0;

            isBoardReAckStationAccept = 0;
            isNotifyBusAcceptAck      = 0;
            isNotifyBusCancelAck      = 0;

            Serial.println("sta: \t [fsm] waiting");
            break;

        case STATION_NOTIFY_ACCEPT_TO_BOARD:
            stationAckToBoard(STATION_NOTIFY_ACCEPT_TO_BOARD);

            Serial.println("sta: \t [fsm] notify station accept");
            break;

        case REQUEST_TO_BUS:
            stationRequestToBus();
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] request to bus");
            break;

        case STATION_NOTIFY_BUS_ACCEPT_TO_BOARD:
            stationAckToBoard(STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] notify bus accept");
            break;

        case BUS_ACCEPT:
            // Serial.printf("sta: \t [fsm] bus accept (request id=%d, hi=%d, lo=%d)\n", busResponse.id, busResponse.addressHI, busResponse.addressLO);
            Serial.println("sta: \t [fsm] bus accept");
            break;

        case STATION_NOTIFY_BUS_PASS_TO_BOARD:
            stationAckToBoard(STATION_NOTIFY_BUS_PASS_TO_BOARD);
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] notify bus pass");
            break;

        case BUS_PASS:
            stationAckToBus(BUS_PASS);
            Serial.println("sta: \t [fsm] bus pass");
            break;

        case STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD:
            stationAckToBoard(STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] notify bus cancel");
            break;

        case DRIVER_CANCEL:
            stationAckToBus(DRIVER_CANCEL);
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] bus cancel");
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            stationAckToBoard(BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);

            Serial.println("sta: \t [fsm] notify passenger cancel");
            break;

        case PASSENGER_CANCEL:
            stationAckToBus(PASSENGER_CANCEL);
            busHandleTimeout = 40;

            Serial.println("sta: \t [fsm] passenger cancel");
            break;

        case FINISHED:
            requestID = (requestID + 1) % 256;

            Serial.println("sta: \t [fsm] finished");
            break;

        default:
            break;
    }
}

void stationFsm(void)
{
    switch (busHandleState)
    {
        case INIT:
            requestID = 0;

            stationFsmResetState(WAITING);
            busHandleState = WAITING;
            break;

        case WAITING:
            if (isThereRequest || button_keycode() == 40)
            // if (isThereRequest)
            {
                isThereRequest = 0;

                stationFsmResetState(STATION_NOTIFY_ACCEPT_TO_BOARD);
                busHandleState = STATION_NOTIFY_ACCEPT_TO_BOARD;
            }
            break;

        case STATION_NOTIFY_ACCEPT_TO_BOARD:
            stationFsmResetState(REQUEST_TO_BUS);
            busHandleState = REQUEST_TO_BUS;
            break;

        case REQUEST_TO_BUS:
            if (isBusAccept)
            {
                isBusAccept = 0;

                stationFsmResetState(STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
                busHandleState = STATION_NOTIFY_BUS_ACCEPT_TO_BOARD;
            }
            else if (--busHandleTimeout == 0)
            {
                stationFsmResetState(REQUEST_TO_BUS);
                busHandleState = REQUEST_TO_BUS;
            }

            break;

        case STATION_NOTIFY_BUS_ACCEPT_TO_BOARD:
            if (isNotifyBusAcceptAck)
            {
                isNotifyBusAcceptAck = 0;

                stationFsmResetState(BUS_ACCEPT);
                busHandleState = BUS_ACCEPT;
            }
            else if (--busHandleTimeout == 0)
            {
                stationFsmResetState(STATION_NOTIFY_BUS_ACCEPT_TO_BOARD);
                busHandleState = STATION_NOTIFY_BUS_ACCEPT_TO_BOARD;
            }
            break;

        case BUS_ACCEPT:
            if (isBusPass)
            {
                isBusPass = 0;

                stationFsmResetState(STATION_NOTIFY_BUS_PASS_TO_BOARD);
                busHandleState = STATION_NOTIFY_BUS_PASS_TO_BOARD;
            }
            else if (isThereBusCancel)
            {
                isThereBusCancel = 0;

                stationFsmResetState(STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
                busHandleState = STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD;
            }
            break;

        case STATION_NOTIFY_BUS_PASS_TO_BOARD:
            if (isNotifyBusPassAck)
            {
                isNotifyBusPassAck = 0;

                stationFsmResetState(BUS_PASS);
                busHandleState = BUS_PASS;
            }
            else if (--busHandleTimeout == 0)
            {
                stationFsmResetState(STATION_NOTIFY_BUS_PASS_TO_BOARD);
                busHandleState = STATION_NOTIFY_BUS_PASS_TO_BOARD;
            }
            break;

        case BUS_PASS:
            stationFsmResetState(FINISHED);
            busHandleState = FINISHED;
            break;

        case STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD:
            if (isNotifyBusCancelAck)
            {
                isNotifyBusCancelAck = 0;

                stationFsmResetState(DRIVER_CANCEL);
                busHandleState = DRIVER_CANCEL;
            }
            else if (--busHandleTimeout == 0)
            {
                stationFsmResetState(STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD);
                busHandleState = STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD;
            }

            break;

        case DRIVER_CANCEL:
            stationFsmResetState(FINISHED);
            busHandleState = FINISHED;
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            stationFsmResetState(PASSENGER_CANCEL);
            busHandleState = PASSENGER_CANCEL;
            break;

        case PASSENGER_CANCEL:
            if (isPassengerCancelAck)
            {
                isPassengerCancelAck = 0;

                stationFsmResetState(FINISHED);
                busHandleState = FINISHED;
            }
            else if (--busHandleTimeout == 0)
            {
                stationFsmResetState(PASSENGER_CANCEL);
                busHandleState = PASSENGER_CANCEL;
            }

        case FINISHED:
            stationFsmResetState(WAITING);
            busHandleState = WAITING;

            break;

        default:
            busHandleState = INIT;
            break;
    }
}

void stationAckDebuger(void)
{
    if (isBusReAckBusPass)
    {
        isBusReAckBusPass = 0;

        stationAckToBus(BUS_PASS);
        Serial.println("sta: \t [ReAck to bus] finish");
    }

    if (isBusReAckBusCancel)
    {
        isBusReAckBusCancel = 0;

        stationAckToBus(DRIVER_CANCEL);
        Serial.println("sta: \t [ReAck to bus] cancel");
    }
}

void boardAckDebuger(void)
{
    if (isBoardReAckStationAccept)
    {
        isBoardReAckStationAccept = 0;

        stationAckToBoard(STATION_NOTIFY_ACCEPT_TO_BOARD);
        Serial.println("sta: \t [ReAck to board] station accept");
    }

    if (isBoardReAckPassengerCancel)
    {
        isBoardReAckPassengerCancel = 0;

        stationAckToBoard(PASSENGER_CANCEL);
        Serial.println("sta: \t [ReAck to board] passenger cancel");
    }
}

void cancelProcess(void)
{
    if (isPassengerCancel)
    {
        isPassengerCancel = 0;

        stationFsmResetState(BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
        busHandleState = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
    }
}

void stationRequestToBus(void)
{
    stationLoraSendMessage[0] = 0xff;

    stationLoraSendMessage[1] = requestID;

    stationLoraSendMessage[2] = (GATEWAY_ADDRESS >> 8) & 0xff; // HIGH
    stationLoraSendMessage[3] = GATEWAY_ADDRESS & 0xff;        // LOW

    stationLoraSendMessage[4] = REQUEST_TO_BUS;

    stationLoraSendMessage[5] = REQUESTED_BUS_NUMBER;
    stationLoraSendMessage[6] = REQUESTED_BUS_DIRECTION;
    stationLoraSendMessage[7] = STATION_ID;

    stationLoraSendMessage[8] = checkSum(stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND);

    e32ttl100.sendBroadcastFixedMessage(BUS_50_CHANEL, stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
}

void stationAckToBus(SYSTEM_STATE state)
{
    stationLoraSendMessage[0] = 0xff;

    stationLoraSendMessage[1] = requestID;

    stationLoraSendMessage[2] = (GATEWAY_ADDRESS >> 8) & 0xff; // HIGH
    stationLoraSendMessage[3] = GATEWAY_ADDRESS & 0xff;        // LOW

    stationLoraSendMessage[4] = state;
    stationLoraSendMessage[5] = 0;
    stationLoraSendMessage[6] = 0;
    stationLoraSendMessage[7] = 0;

    stationLoraSendMessage[8] = checkSum(stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND);

    // e32ttl100.sendBroadcastFixedMessage(BUS_50_CHANEL, stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
    e32ttl100.sendFixedMessage(busResponse.addressHI, busResponse.addressLO, BUS_50_CHANEL, stationLoraSendMessage, LORA_PACKAGE_SIZE_SEND + 1);
}

void stationAckToBoard(SYSTEM_STATE state)
{
    responseToBoard[0] = state;
    responseToBoardCRC = CRC16((char *)responseToBoard, 1);
    responseToBoard[1] = responseToBoardCRC & 0xFF;
    responseToBoard[2] = responseToBoardCRC >> 8;
    responseToBoard[3] = '\0';
    rs485_setmode(RS485_TRANSMIT);
    RS485_Serial.write((const uint8_t *)responseToBoard, 4);
    rs485_setmode(RS485_RECEIVE);
}