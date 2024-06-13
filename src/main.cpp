#include "button.h"
#include "lora.h"
#include "ota.h"
#include "led.h"
#include "rs485.h"
#include "station.h"
#include "server.h"
#include "main.h"

void setup()
{
    Serial.begin(115200);

    wifi_init();
    server_init();
    lora_init();

    stationInit();
    rs485_init();

    AP_init();
    // dashboard_init();
}

void loop()
{
}