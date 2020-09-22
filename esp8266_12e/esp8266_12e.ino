#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <ArduinoJson.h>

const char *ssid = "";
const char *password = "";
const char *domain = "esp8266";
//const int wifi_port = 8080;
const int web_port = 80;
String json = "";
ESP8266WebServer web_server(web_port);
//WiFiServer wifi_server(wifi_port);
//WiFiClient wifi_client;


boolean wifiConnected = false;
boolean localDomainStarted = false;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 60000;

String data;

void setup(void) {
  Serial.begin(115200);
  Wire.begin(D5, D6); /* join i2c bus with SDA=D5 and SCL=D6 of NodeMCU */

  // Initialize wifi connection
  wifiConnected = connectWifi();
  if (wifiConnected) {
    Serial.println("done connecting wifi");
  } else {
    Serial.println("failed connecting wifi");
  }

  // Initialize local domain
  localDomainStarted = startMDNSResponder();
  if (localDomainStarted) {
    Serial.print("Local Domain: ");
    Serial.print(domain);
    Serial.println(".local");
  } else {
    Serial.println("Fail to start local domain");
  }

  // Initialize web server
  startWebServer();

  //initialize TCPIP connection
  //startWifiServer();


}

void loop(void) {
  web_server.handleClient();
  MDNS.update();
  /*wifi_client = wifi_server.available();
    if (wifi_client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (wifi_client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (wifi_client.available()) {             // if there's bytes to read from the client,
        char c = wifi_client.read();             // read a byte, then
        data += c;
        if (c == '\n') {                    // if the byte is a newline character
          Serial.write(data.c_str());
          transmit2Arduino(data.c_str());
          data = "";
        }
      }
    }
    }*/
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi() {
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10) {
      state = false;
      break;
    }
    i++;
  }

  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

boolean startMDNSResponder() {
  return MDNS.begin(domain);
}

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  web_server.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += web_server.uri();
  message += "\nMethod: ";
  message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += web_server.args();
  message += "\n";

  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += " " + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
  }

  web_server.send(404, "text/plain", message);
}

void handleRedlight() {
  if (web_server.hasArg("plain") == false) { //Check if body received
    web_server.send(200, "text/plain", "Body not received");
    return;
  }
  else
  {
    json = web_server.arg("plain");
    Serial.println(json);
    DynamicJsonDocument jsonDoc(1024);
    auto error = deserializeJson(jsonDoc, json);
    if (error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return;
    }
    else
    {
      const char* redlight_status = jsonDoc["redlight"];
      web_server.send(200, "text/plain", redlight_status);
      if (strcmp(redlight_status, "on") == 0)
      {
        Wire.beginTransmission(8);
        Wire.write("0-1");
        Wire.endTransmission();
      }
      else if (strcmp(redlight_status, "off") == 0)
      {
        Wire.beginTransmission(8);
        Wire.write("0-0");
        Wire.endTransmission();
      }
    }
  }
}

void handleTimer()
{
  if (web_server.hasArg("plain") == false) { //Check if body received
    web_server.send(200, "text/plain", "Body not received");
    return;
  }
  else
  {
    json = web_server.arg("plain");
    Serial.println(json);
    DynamicJsonDocument jsonDoc(1024);
    auto error = deserializeJson(jsonDoc, json);
    if (error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return;
    }
    else
    {
      String duraionInSeconds = jsonDoc["timer"];
      char buffers[duraionInSeconds.length() + 3];
      strcpy(buffers, "1-");
      strcat(buffers, duraionInSeconds.c_str());
      web_server.send(200, "text/plain", duraionInSeconds);
      Wire.beginTransmission(8);
      Wire.write(buffers);
      Wire.endTransmission();
    }
  }
}

void startWebServer() {
  web_server.on("/", handleRoot);
  web_server.onNotFound(handleNotFound);
  web_server.on("/redlight", HTTP_POST, handleRedlight);
  web_server.on("/timer", HTTP_POST, handleTimer);
  web_server.begin();
  Serial.print("HTTP server started at port ");
  Serial.println(web_port);
}

/*void startWifiServer() {
  wifi_server.begin();
  }*/
