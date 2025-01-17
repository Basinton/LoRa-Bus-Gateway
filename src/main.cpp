#include "button.h"
#include "lora.h"
#include "ota.h"
#include "led.h"
#include "rs485.h"
#include "station.h"
#include "main.h"

void setup()
{
    Serial.begin(115200);

    wifi_init();

    lora_init();
    led_init();
    button_init();

    dashboard_init();

    stationInit();
    rs485_init();
}

void loop()
{
}