#include "server.h"

/* Variables --------------------------------------------------------------------*/

// Server address
const char *serverAddress = "http://bus.abcsolutions.com.vn";

/* Task handles */
TaskHandle_t serverTaskHandle = NULL;

/* Functions --------------------------------------------------------------------*/

String urlEncode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += "%20";
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    delay(0);
  }
  return encodedString;
}

void requestBus(String stopCode, String routeCode)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = String(serverAddress) + "/api/BusStops/RequestBus?stopCode=" + urlEncode(stopCode) + "&routeCode=" + urlEncode(routeCode);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void updateRequestBus(String stopCode, String routeCode, String newStatus)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = String(serverAddress) + "/api/BusStops/UpdateRequestStatus?stopCode=" + urlEncode(stopCode) + "&routeCode=" + urlEncode(routeCode) + "&newStatus=" + urlEncode(newStatus);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

// Function to make the API request
void getValueData(String stationcode, String id, String line, String seq, String lat, String lng, String spd)
{
  if (WiFi.status() == WL_CONNECTED)
  {

    HTTPClient http;
    String url = String(serverAddress) + "/Value/GetData?stationcode=" + urlEncode(stationcode) +
                 "&data=ID," + urlEncode(id) +
                 ";LINE," + urlEncode(line) +
                 ";SEQ," + urlEncode(seq) +
                 ";LAT," + urlEncode(lat) +
                 ";LNG," + urlEncode(lng) +
                 ";SPD," + urlEncode(spd) + ";";

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void server_task(void *pvParameters)
{
  while (1)
  {
    if (isBus50Ready == 1)
    {
      if (bus50.busLat != 0 && bus50.busLong != 0)
      {
        getValueData("Q10 055", "10001", "50", String(bus50.messageID), String(bus50.busLat, 6), String(bus50.busLong, 6), String(bus50.busSpeed, 2));
        isBus50Ready = 0;
      }
      else{
        getValueData("Q10 055", "10001", "50", String(bus50.messageID), String(10.879954161919233, 6), String(106.80608269725226, 6), String(bus50.busSpeed, 2));
        isBus50Ready = 0;
      }
    }
    if (isBus08Ready == 1)
    {
      if (bus08.busLat != 0 && bus08.busLong != 0)
      {
        getValueData("QTD 253", "10004", "08", String(bus08.messageID), String(bus08.busLat, 6), String(bus08.busLong, 6), String(bus08.busSpeed, 2));
        isBus08Ready = 0;
      }
      else{
        getValueData("QTD 253", "10004", "08", String(bus50.messageID), String(10.879954161919233, 6), String(106.80608269725226, 6), String(bus50.busSpeed, 2));
        isBus50Ready = 0;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 10 seconds
  }
}

void server_init(void)
{
  // Create task to communicate between gateway and server
  xTaskCreate(server_task, "Server Task", 4096, NULL, configMAX_PRIORITIES - 2, &serverTaskHandle);
}