// Import required libraries
#include "dashboard.h"
/* Variables --------------------------------------------------------------------*/
// SSID and password for STA Mode
const char *sta_ssid = DEFAULT_STA_SSID;
const char *sta_password = DEFAULT_STA_PASSWORD;

const char *sta_ssid_2 = "Hehee";
const char *sta_password_2 = "12344321";

const char *sta_ssid_3 = "BKIT_TTHUONG";
const char *sta_password_3 = "cselabc5c6";

// Replace with your computer's local network IP
const char *websocket_server_1 = "ws://192.168.1.53:3000/path";
const char *websocket_server_2 = "ws://192.168.1.28:3000/path";
String websocket_server_test = "ws://192.168.112.177:3000/path";
String websocket_server_test_2 = "ws://192.168.1.8:3000/path";

// using namespace websockets;

// WebsocketsClient client;

/* Task handles */
TaskHandle_t dashboardTaskHandle = NULL;
TaskHandle_t busSendInfoTaskHandle = NULL;
TaskHandle_t reconnectTaskHandle = NULL;

/* Functions -----------------------------------------------------------------*/
// void dashboard_task(void *pvParameters)
// {
//   while (1)
//   {
//     client.poll();
//     vTaskDelay(pdMS_TO_TICKS(50));
//   }
// }

// void reconnect_task(void *pvParameters)
// {
//   const int retryDelay = 10000;   // Delay between retries in milliseconds
//   const int checkInterval = 5000; // Interval to check the connection status in milliseconds

//   while (1)
//   {
//     if (!client.available())
//     {
//       client.close();
//       Serial.println("Reconnecting to WebSocket Server...");

//       if (isConnect == 1)
//       {
//         websocket_server_test = second_ip;

//         if (client.connect(websocket_server_test))
//         {
//           // Reconnected successfully
//           Serial.println("Reconnected to WebSocket Server.");
//         }
//         else
//         {
//           Serial.println("Connection Failed!");
//         }

//         // Wait a bit before retrying to avoid flooding with connection requests
//         vTaskDelay(pdMS_TO_TICKS(retryDelay)); // Wait 10 seconds before retrying
//       }
//     }

//     // Sleep a while before checking the connection status again
//     vTaskDelay(pdMS_TO_TICKS(checkInterval)); // Check every 5 seconds
//   }
// }

// void busSendInfo_task(void *pvParameters)
// {
//   while (1)
//   {
//     if (client.available())
//     {
//       String info = getBusInfo();
//       client.send(info);
//     }
//     vTaskDelay(pdMS_TO_TICKS(3000)); // Delay for 3 seconds
//   }
// }

// // Json Variable to Hold Bus Info
// JSONVar busInfo;

// uint16_t messageID = 0;

// // Get Bus Info
// String getBusInfo()
// {
//   busInfo["message_id"] = String(messageID);
//   busInfo["bus_id"] = String(BUS_50);
//   busInfo["bus_route"] = String(myBus.busRoute);
//   busInfo["bus_driver_name"] = String(myBus.busDriverName);
//   busInfo["bus_location_lat"] = String(myBus.busLat, 6);
//   busInfo["bus_location_lng"] = String(myBus.busLong, 6);
//   busInfo["bus_speed"] = String(myBus.busSpeed);
//   busInfo["bus_distance"] = String(myBus.busDistance);
//   // busInfo["bus_direction"] = String(direction[myBus.busDirection].dirName);
//   // busInfo["bus_now_stop"] = String(STATIONS_50[myBus.nowBusStop].stationName);

//   String jsonString = JSON.stringify(busInfo);
//   return jsonString;
// }

// Initialize WiFi
void wifi_init()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true);

  // Connect to the STA
  WiFi.begin(sta_ssid_2, sta_password_2);
  Serial.print("Connecting WiFi..");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// void reconnectWebServer()
// {
//   if (!client.available())
//   {
//     client.close();
//     Serial.println("Reconnecting to WebSocket Server...");

//     if (isConnect == 1)
//     {
//       isConnect = 0;
//       websocket_server_test = second_ip;

//       int retryCount = 0;
//       const int maxRetries = 5;
//       const int retryDelay = 10000; // 10 seconds

//       while (retryCount < maxRetries)
//       {
//         if (client.connect(websocket_server_test))
//         {
//           // Reconnected successfully
//           Serial.println("Reconnected to WebSocket Server.");
//           return;
//         }
//         else
//         {
//           Serial.println("Connection Failed! Retrying...");
//           retryCount++;
//           vTaskDelay(pdMS_TO_TICKS(retryDelay)); // Wait before retrying
//         }
//       }

//       Serial.println("Max retries reached. Will try again later.");
//     }
//     else
//     {
//       // No new connection info available, wait before retrying
//       vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before retrying
//     }
//   }
//   vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before rechecking
// }

// void webSocketEvent(WebsocketsEvent event, String data)
// {
//   switch (event)
//   {
//   case WebsocketsEvent::ConnectionOpened:
//     Serial.println("Connection Opened");
//     break;
//   case WebsocketsEvent::ConnectionClosed:
//     Serial.println("WebSocket connection closed");
//     break;
//     // Handle other events as needed...
//   }
// }

// void websocket_init()
// {
//   client.onEvent(webSocketEvent);
//   client.connect(websocket_server_test_2);
// }

// void dashboard_init(void)
// {
//   websocket_init();

//   // Start a periodic timer to send bus info every 10 seconds
//   xTaskCreate(busSendInfo_task, "Bus Send Info", 4096, NULL, configMAX_PRIORITIES, &busSendInfoTaskHandle);

//   // Create the dashboard task
//   xTaskCreate(dashboard_task, "Dashboard Task", 4096, NULL, 3, &dashboardTaskHandle);

//   // Create task to check connection between bus and server
//   xTaskCreate(reconnect_task, "Reconnect Task", 4096, NULL, 3, &reconnectTaskHandle);
// }