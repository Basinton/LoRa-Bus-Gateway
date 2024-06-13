/* Includes ------------------------------------------------------------------*/
#include "lora.h"
#include "station.h"
// #include "server.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint8_t lora_receive[1000] = {0};
uint8_t locationBuffer[8] = {0};
uint32_t lora_receive_cnt = 0;

bool isBus50Ready = 0;
bool isBus08Ready = 0;

LoRa_E32 e32ttl100(&Serial2, PIN_AUX, PIN_M0, PIN_M1, UART_BPS_RATE_9600);

TaskHandle_t loraTaskHandle = NULL;

volatile bool isBusDataReceive = 0;

/* Functions -----------------------------------------------------------------*/
float bytesToFloat(uint8_t *bytes, int start)
{
    int32_t temp = (bytes[start] << 24) | (bytes[start + 1] << 16) | (bytes[start + 2] << 8) | bytes[start + 3];
    return temp / 1000000.0; // Assuming the same scale factor used during transmission
}

float distanceToFloat(uint8_t *bytes, int start)
{
    int16_t temp = (bytes[start] << 8) | bytes[start + 1];
    return temp / 10.0; // Assuming the same scale factor used during transmission
}

uint16_t bytesToMessageID(uint8_t *bytes, int start)
{
    int16_t temp = (bytes[start] << 8) | bytes[start + 1];
    return temp; // Assuming the same scale factor used during transmission
}

void lora_task(void *pvParameters)
{
    while (1)
    {
        lora_process();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void lora_init(void)
{
    e32ttl100.begin();
    setConfig(GATEWAY_ADDRESS, GATEWAY_CHANNEL, AIR_DATA_RATE_000_03, POWER_20);

    xTaskCreate(lora_task, "lora task", 2048, NULL, configMAX_PRIORITIES, &loraTaskHandle);

    Serial.println("lora: \t [init]");
}

uint8_t checkSum(uint8_t *message, int size)
{
    uint8_t checksum = 0;
    for (uint8_t i = 1; i < size; i++)
        checksum = checksum + message[i];
    return checksum;
}

void checkDataReceive(void)
{
    uint8_t status = 0;
    uint8_t checksum;
    uint32_t startDataReach = 0;

    uint32_t sizeOfData = lora_receive_cnt - LORA_PACKAGE_SIZE_RECEIVE;

    for (uint32_t i = 0; i < lora_receive_cnt; i++)
    {
        if (i >= sizeOfData)
        {
            break;
        }

        if (lora_receive[i] == 0xFF)
        {
            checksum = checkSum(&lora_receive[i], LORA_PACKAGE_SIZE_RECEIVE);
            if (checksum == lora_receive[i + LORA_PACKAGE_SIZE_RECEIVE])
            {
                busID = BUS_ID(lora_receive[i + BUS_NUMBER]);
                switch (lora_receive[i + STATE_INDEX])
                {
                case BUS_ACCEPT:
                    if (busHandleState[busID] == REQUEST_TO_BUS)
                    {
                        busResponse[busID].id = lora_receive[i + ID_INDEX];
                        busResponse[busID].addressHI = lora_receive[i + ADDRESS_HI_INDEX];
                        busResponse[busID].addressLO = lora_receive[i + ADDRESS_LO_INDEX];

                        isBusAccept[busID] = 1;
                    }
                    break;

                case BUS_PASS:
                    if (busHandleState[busID] == BUS_ACCEPT)
                    {
                        isBusPass[busID] = 1;
                    }
                    else
                    {
                        isBusReAckBusPass[busID] = 1;
                    }
                    break;

                case DRIVER_CANCEL:
                    if (busHandleState[busID] == BUS_ACCEPT)
                    {
                        isThereBusCancel[busID] = 1;
                    }
                    else
                    {
                        isBusReAckBusCancel[busID] = 1;
                    }
                    break;

                case PASSENGER_CANCEL:
                    if (busHandleState[busID] == PASSENGER_CANCEL)
                    {
                        isPassengerCancelAck[busID] = 1;
                    }

                    break;

                default:
                    break;
                }

                startDataReach = i + LORA_PACKAGE_SIZE_RECEIVE + 1;
                i = i + LORA_PACKAGE_SIZE_RECEIVE;
            }
            else
            {
                // Serial.println("lost");
                // startDataReach = i + LORA_PACKAGE_SIZE_RECIEVE + 1;
                // i              = i + LORA_PACKAGE_SIZE_RECIEVE;
            }
        }
    }

    if (lora_receive_cnt < startDataReach)
        return;

    for (uint32_t i = 0; i < lora_receive_cnt - startDataReach; i++)
    {
        if (i >= (lora_receive_cnt - startDataReach))
            break;
        lora_receive[i] = lora_receive[startDataReach + i];
    }
    lora_receive_cnt = lora_receive_cnt - startDataReach;

    memset(&lora_receive[lora_receive_cnt], 0x00, 1000 - lora_receive_cnt);
}

void checkLocationReceive(void)
{
    uint8_t status = 0;
    uint8_t checksum;
    uint32_t startDataReach = 0;

    uint32_t sizeOfData = lora_receive_cnt - LORA_LOCATION_SIZE_RECEIVE;

    for (uint32_t i = 0; i < lora_receive_cnt; i++)
    {
        if (i >= sizeOfData)
        {
            break;
        }

        if (lora_receive[i] == 0xAA)
        {
            checksum = checkSum(&lora_receive[i], LORA_LOCATION_SIZE_RECEIVE);
            if (checksum == lora_receive[i + LORA_LOCATION_SIZE_RECEIVE])
            {
                busID = BUS_ID(lora_receive[i + BUS_NUMBER]);
                if (busID == BUS_50)
                {
                    bus50.messageID = bytesToMessageID(lora_receive, i + MESSAGE_ID_0);

                    bus50.busLat = bytesToFloat(lora_receive, i + BUS_LAT_0);   // Convert first 4 bytes to float
                    bus50.busLong = bytesToFloat(lora_receive, i + BUS_LONG_0); // Convert next 4 bytes to float

                    bus50.busSpeed = double(lora_receive[i + BUS_SPEED] / 10);

                    bus50.busDistance = distanceToFloat(lora_receive, i + BUS_DISTANCE_0);
                    // bus50.busDirection = lora_receive[i + BUS_DIRECTION];
                    // bus50.nowBusStop = lora_receive[i + BUS_NOW_STOP];

                    isBus50Ready = 1;
                }
                else if (busID == BUS_08)
                {
                    bus08.messageID = bytesToMessageID(lora_receive, i + MESSAGE_ID_0);

                    bus08.busLat = bytesToFloat(lora_receive, i + BUS_LAT_0);   // Convert first 4 bytes to float
                    bus08.busLong = bytesToFloat(lora_receive, i + BUS_LONG_0); // Convert next 4 bytes to float

                    bus08.busSpeed = double(lora_receive[i + BUS_SPEED] / 10);

                    bus08.busDistance = distanceToFloat(lora_receive, i + BUS_DISTANCE_0);
                    // bus08.busDirection = lora_receive[i + BUS_DIRECTION];
                    // bus08.nowBusStop = lora_receive[i + BUS_NOW_STOP];

                    isBus08Ready = 1;
                }
            }
        }

        startDataReach = i + LORA_LOCATION_SIZE_RECEIVE + 1;
        i = i + LORA_LOCATION_SIZE_RECEIVE;
    }

    if (lora_receive_cnt < startDataReach)
        return;

    for (uint32_t i = 0; i < lora_receive_cnt - startDataReach; i++)
    {
        if (i >= (lora_receive_cnt - startDataReach))
            break;
        lora_receive[i] = lora_receive[startDataReach + i];
    }
    lora_receive_cnt = lora_receive_cnt - startDataReach;

    memset(&lora_receive[lora_receive_cnt], 0x00, 1000 - lora_receive_cnt);
}

void lora_process(void)
{
    static uint32_t _checkNoData = 0;

    if (e32ttl100.available() > 1)
    {
        _checkNoData = 0;
        ResponseContainer rs = e32ttl100.receiveMessage();

        for (uint32_t i = 0; i < rs.data.length(); i++)
        {
            lora_receive[lora_receive_cnt] = rs.data[i];
            lora_receive_cnt = (lora_receive_cnt + 1) % 1000;
        }
    }
    else
        _checkNoData = _checkNoData + 1;

    if (_checkNoData > 1)
    {
        lora_receive_cnt = 0;
        _checkNoData = 0;
    }

    if (lora_receive_cnt == LORA_PACKAGE_SIZE_RECEIVE + 1)
    {
        isBusDataReceive = 1;
        Serial.printf("lora: \t [%d] ", lora_receive_cnt);
        for (size_t i = 0; i < lora_receive_cnt; i++)
        {
            printf("%02X ", lora_receive[i]);
        }
        printf("\n");

        checkDataReceive();
        isBusDataReceive = 0;
    }
    if (lora_receive_cnt == LORA_LOCATION_SIZE_RECEIVE + 1 && isBusDataReceive == 0)
    {
        Serial.printf("lora: \t [%d] ", lora_receive_cnt);
        for (size_t i = 0; i < lora_receive_cnt; i++)
        {
            printf("%02X ", lora_receive[i]);
        }
        printf("\n");

        checkLocationReceive();
    }
}

void accessModeConfig(void)
{
    digitalWrite(PIN_M0, HIGH);
    digitalWrite(PIN_M1, HIGH);
}

void accessModeTransmit(void)
{
    digitalWrite(PIN_M0, LOW);
    digitalWrite(PIN_M1, LOW);
}

void setConfig(uint16_t address, uint16_t channel, uint8_t airRate, uint8_t power)
{
    accessModeConfig();
    delay(10);
    ResponseStructContainer c;
    c = e32ttl100.getConfiguration();
    // It's important get configuration pointer before all other operation
    Configuration configuration = *(Configuration *)c.data;

    // Serial.println(c.status.getResponseDescription());
    // Serial.println(c.status.code);
    // printParameters(configuration);

    // Configuration configuration;
    configuration.ADDL = address & 0xff;
    configuration.ADDH = (address >> 8) & 0xff;
    configuration.CHAN = channel;

    configuration.OPTION.fec = FEC_1_ON;
    configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
    configuration.OPTION.transmissionPower = power;
    configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;

    configuration.SPED.airDataRate = airRate;
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    // Set configuration changed and set to not hold the configuration
    ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);

    // Serial.println(rs.getResponseDescription());
    // Serial.println(rs.code);
    // printParameters(configuration);

    c.close();
    accessModeTransmit();
}

void printParameters(struct Configuration configuration)
{
    Serial.println("----------------------------------------");

    Serial.print(F("HEAD : "));
    Serial.print(configuration.HEAD, BIN);
    Serial.print(" ");
    Serial.print(configuration.HEAD, DEC);
    Serial.print(" ");
    Serial.println(configuration.HEAD, HEX);
    Serial.println(F(" "));
    Serial.print(F("AddH : "));
    Serial.println(configuration.ADDH, BIN);
    Serial.print(F("AddL : "));
    Serial.println(configuration.ADDL, BIN);
    Serial.print(F("Chan : "));
    Serial.print(configuration.CHAN, DEC);
    Serial.print(" -> ");
    Serial.println(configuration.getChannelDescription());
    Serial.println(F(" "));
    Serial.print(F("SpeedParityBit     : "));
    Serial.print(configuration.SPED.uartParity, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.SPED.getUARTParityDescription());
    Serial.print(F("SpeedUARTDatte  : "));
    Serial.print(configuration.SPED.uartBaudRate, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.SPED.getUARTBaudRate());
    Serial.print(F("SpeedAirDataRate   : "));
    Serial.print(configuration.SPED.airDataRate, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.SPED.getAirDataRate());

    Serial.print(F("OptionTrans        : "));
    Serial.print(configuration.OPTION.fixedTransmission, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.OPTION.getFixedTransmissionDescription());
    Serial.print(F("OptionPullup       : "));
    Serial.print(configuration.OPTION.ioDriveMode, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.OPTION.getIODroveModeDescription());
    Serial.print(F("OptionWakeup       : "));
    Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
    Serial.print(F("OptionFEC          : "));
    Serial.print(configuration.OPTION.fec, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.OPTION.getFECDescription());
    Serial.print(F("OptionPower        : "));
    Serial.print(configuration.OPTION.transmissionPower, BIN);
    Serial.print(" -> ");
    Serial.println(configuration.OPTION.getTransmissionPowerDescription());

    Serial.println("----------------------------------------");
}