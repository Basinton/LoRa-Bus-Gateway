/* Includes ------------------------------------------------------------------*/
#include "lora.h"
#include "station.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint8_t lora_receive[1000] = {0};
uint32_t lora_receive_cnt = 0;

LoRa_E32 e32ttl100(&Serial2, PIN_AUX, PIN_M0, PIN_M1, UART_BPS_RATE_9600);

TaskHandle_t loraTaskHandle = NULL;

/* Functions -----------------------------------------------------------------*/
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

    xTaskCreate(lora_task, "lora task", 8192, NULL, configMAX_PRIORITIES, &loraTaskHandle);

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

    uint32_t sizeOfData = lora_receive_cnt - LORA_PACKAGE_SIZE_RECIEVE;

    for (uint32_t i = 0; i < lora_receive_cnt; i++)
    {
        if (i >= sizeOfData)
        {
            break;
        }

        if (lora_receive[i] == 0xFF)
        {
            checksum = checkSum(&lora_receive[i], LORA_PACKAGE_SIZE_RECIEVE);
            if (checksum == lora_receive[i + LORA_PACKAGE_SIZE_RECIEVE])
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
                    Serial.println("Error: Default case entered!");
                    break;
                }

                startDataReach = i + LORA_PACKAGE_SIZE_RECIEVE + 1;
                i = i + LORA_PACKAGE_SIZE_RECIEVE;
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

    if (lora_receive_cnt > LORA_PACKAGE_SIZE_RECIEVE)
    {
        Serial.printf("lora: \t [%d] ", lora_receive_cnt);
        for (size_t i = 0; i < lora_receive_cnt; i++)
        {
            printf("%02X ", lora_receive[i]);
        }
        printf("\n");

        checkDataReceive();
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