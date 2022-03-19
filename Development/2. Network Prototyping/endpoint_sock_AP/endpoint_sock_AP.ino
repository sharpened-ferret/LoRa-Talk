#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define ss 5
#define rst 14
#define dio0 2

// Network Credentials
const char* ssid = "TestAP";
const char* password = "pass";

// Set web server port number to 80
//WiFiServer server(50);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients(String message) {
  ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    Serial.println(message);
    
    LoRa.beginPacket();
    LoRa.print("00"+message);
    LoRa.endPacket();
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Chat</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
  </style>
</head>
<body>
  <h2>ESP Web Chat</h2>
  <form action="">
    <label for="message">Message:</label>
    <input type="text" id="message" name="message">
    <button type="submit">Submit</button>
  </form>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  LoRa.setPins(ss, rst, dio0);

  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  LoRa.setSyncWord(0xFF);
  Serial.println("LoRa Initialising OK!");

  initWebSocket();
  
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam("message")) {
      inputMessage = request->getParam("message")->value();
      Serial.println("message: " + inputMessage);
      
      LoRa.beginPacket();
      LoRa.print(inputMessage);
      LoRa.endPacket();
    }
    request->send(200, "text/html", index_html);
  });
   
  server.begin();
  

  
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Recieved Packet: '");
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
