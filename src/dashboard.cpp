// Import required libraries
#include "dashboard.h"
/* Variables --------------------------------------------------------------------*/
// SSID and password for AP Mode
const char *ap_ssid = "LORA_TESTING";
const char *ap_password = "12345678";

// SSID and password for STA Mode
const char *sta_ssid = DEFAULT_STA_SSID;
const char *sta_password = DEFAULT_STA_PASSWORD;

// const char *sta_ssid = "Basinton";
// const char *sta_password = "myling2850";

// Replace with your computer's local network IP
// const char *websocket_server = "ws://lorabus.ddns.net:8080/path";
const char *websocket_server_1 = "ws://192.168.1.53:3000/path";
const char *websocket_server_2 = "ws://192.168.1.28:3000/path";
using namespace websockets;

WebsocketsClient client;

/* Task handles */
TaskHandle_t dashboardTaskHandle = NULL;
TaskHandle_t busSendInfoTaskHandle = NULL;
TaskHandle_t reconnectTaskHandle = NULL;

/* Functions -----------------------------------------------------------------*/
void dashboard_task(void *pvParameters)
{
  while (1)
  {
    client.poll();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void reconnect_task(void *pvParameters)
{
  while (1)
  {
    reconnectWebServer();
    // Sleep a while before checking the connection status again
    vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
  }
}

void busSendInfo_task(void *pvParameters)
{
  while (1)
  {
    if (client.available())
    {
      String info = getBusInfo();
      client.send(info);
    }
    vTaskDelay(pdMS_TO_TICKS(3000)); // Delay for 3 seconds
  }
}

// Json Variable to Hold Bus Info
JSONVar busInfo;
BUS myBus = {50, "Nguyễn Văn A", 0, 0, 0, 0, 0, 0};
uint16_t messageID = 0;

// Get Bus Info
String getBusInfo()
{
  busInfo["message_id"] = String(messageID);
  Serial.println(messageID);
  busInfo["bus_id"] = String(BUS_00);
  busInfo["bus_route"] = String(myBus.busRoute);
  busInfo["bus_driver_name"] = String(myBus.busDriverName);
  busInfo["bus_location_lat"] = String(myBus.busLat, 6);
  busInfo["bus_location_lng"] = String(myBus.busLong, 6);
  busInfo["bus_speed"] = String(myBus.busSpeed);
  busInfo["bus_direction"] = String(direction[myBus.busDirection].dirName);
  busInfo["bus_now_stop"] = String(STATIONS_50[myBus.nowBusStop].stationName);

  String jsonString = JSON.stringify(busInfo);
  return jsonString;
}

// Initialize WiFi
void wifi_init()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  // // Start the AP
  // WiFi.softAP(ap_ssid, ap_password);
  // Serial.print("Access Point \"");
  // Serial.print(ap_ssid);
  // Serial.println("\" started");
  // IPAddress IP = WiFi.softAPIP();
  // Serial.println(IP);

  // Connect to the STA
  WiFi.begin(sta_ssid, sta_password);
  Serial.print("Connecting WiFi..");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectWebServer()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();

    // Wait for WiFi to reconnect
    while (WiFi.status() != WL_CONNECTED)
    {
      vTaskDelay(pdMS_TO_TICKS(500)); // Wait 500 ms
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi.");
  }

  if (!client.available())
  {
    client.close();
    Serial.println("Reconnecting to WebSocket Server...");
    if (!client.connect(websocket_server_2))
    {
      Serial.println("Connection Failed!");
      // Wait a bit before retrying to avoid flooding with connection requests
      vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before retrying
    }
    else
    {
      // Reconnected successfully
      Serial.println("Reconnected to WebSocket Server.");
    }
  }
}

void webSocketEvent(WebsocketsEvent event, String data)
{
  switch (event)
  {
  case WebsocketsEvent::ConnectionOpened:
    Serial.println("Connection Opened");
    break;
  case WebsocketsEvent::ConnectionClosed:
    Serial.println("WebSocket connection closed");
    break;
    // Handle other events as needed...
  }
}

void websocket_init()
{
  client.onEvent(webSocketEvent);
  client.connect(websocket_server_2);
}

void dashboard_init(void)
{
  websocket_init();

  // Start a periodic timer to send bus info every 10 seconds
  xTaskCreate(busSendInfo_task, "Bus Send Info", 2048, NULL, configMAX_PRIORITIES, &busSendInfoTaskHandle);

  // Create the dashboard task
  xTaskCreate(dashboard_task, "Dashboard Task", 2048, NULL, 3, &dashboardTaskHandle);

  // Create task to check connection between bus and server
  xTaskCreate(reconnect_task, "Reconnect Task", 4096, NULL, 3, &reconnectTaskHandle);
}