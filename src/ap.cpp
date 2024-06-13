#include "ap.h"
#include <WebServer.h>

// SSID and password for AP Mode
const char *ap_ssid = "LORA_TESTING";
const char *ap_password = "12345678";

TaskHandle_t APTaskHandle = NULL;

WebServer server(80);

const char *htmlForm = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP32 WiFi Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
    <h1>ESP32 WiFi Configuration</h1>
    <form action="/connect" method="POST">
      <label for="ssid">SSID:</label>
      <input type="text" id="ssid" name="ssid"><br><br>
      <label for="password">Password:</label>
      <input type="password" id="password" name="password"><br><br>
      <input type="submit" value="Connect">
    </form>
  </body>
</html>
)rawliteral";

void handleRoot()
{
  server.send(200, "text/html", htmlForm);
}

void handleConnect()
{
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  second_ip = "ws://" + server.arg("server_ip") + ":3000/path";
  Serial.println(second_ip);

  // Attempt to connect to the provided WiFi network
  WiFi.begin(ssid.c_str(), password.c_str());
  server.send(200, "text/html", "<html><body><h1>Connecting to WiFi...</h1></body></html>");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected!");
    server.send(200, "text/html", "<html><body><h1>Connected to WiFi!</h1></body></html>");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    isConnect = 1;
  }
  else
  {
    Serial.println("Failed to connect.");
    server.send(200, "text/html", "<html><body><h1>Failed to connect to WiFi.</h1></body></html>");
  }
}

void AP_task(void *pvParameters)
{
  while (1)
  {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void AP_init()
{
  // Start the AP
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);

  // Set up the web server
  server.on("/", handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.begin();
  Serial.println("HTTP server started");

  xTaskCreate(AP_task, "AP Task", 4096, NULL, configMAX_PRIORITIES - 4, &APTaskHandle);
}
