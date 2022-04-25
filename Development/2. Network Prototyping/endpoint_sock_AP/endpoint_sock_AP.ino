#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Sets the GPIO pin numbers for the Transceiver connection
#define ss 5
#define rst 14
#define dio0 2

// Sets Network Credentials for the hosted WiFi network
const char* ssid = "LoRaChat Endpoint";
// Note: *Minimum password length of eight characters required*
const char* password = "password";

// Sets server port and address path
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Sends a WebSocket message to all connected clients
void notifyClients(String message) {
  ws.textAll(message);
}

// Transmits message over LoRa, to message gateway nodes
void loraSend(String message) {
  while (!LoRa.beginPacket()) {
    Serial.print(".");
  }
  // "00" added to the message as a packet direction indicator
  LoRa.print("00"+message);
  if (LoRa.endPacket()) {
    Serial.println("Sent Packet: " + message);  
  }
}

// Callback for recieving a WebSocket message from a client
// Sends message contents over LoRa
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    loraSend(message);
  }
}

// Handles WebSocket connections
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

// Initialises WebSocket and registers our handler
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Defines the HTML file to be served for HTTP web connections
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
  <form action="" method="post">
    <label for="message">Message:</label>
    <input type="text" id="message" name="message">
    <button type="submit">Send</button>
  </form>
</body>
</html>
)rawliteral";

// Program initialisation. Runs on boot
void setup() {
  Serial.begin(115200);

  // Starts LoRa transceiver with configured settings. 
  LoRa.setPins(ss, rst, dio0);
  while(!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xFF);
  Serial.println("LoRa Initialising OK!");

  // Starts WiFi access point and outputs details on Serial Port
  Serial.println("Setting AP)…");
  Serial.printf("SSID: %s Password: %s\n", ssid, password);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endpoint IP address: ");
  Serial.println(IP);

  // Calls to configure the WebSocket Server
  initWebSocket();

  // Serves HTML file in response to client GET requests
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  // Relays client HTTP post data to gateway nodes over LoRa
  server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    String message;
    int params = request->params();
    for (int i = 0; i < params; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost()) {
            Serial.printf("%s: %s \n", p->name().c_str(), p->value().c_str());
            loraSend("{\"username\":\"Guest\", \"timestamp\":0, \"message\": \"" + p->value() + "\"}");    
        }
        else {
            Serial.println("Info: Recieved Invalid Post Content");
        }
    }
    request->send(200, "text/html", index_html);
    });

  // Starts the WebSocket and HTTP server
  server.begin();
}

// Recieves LoRa packets and relays them to clients over WebSocket connections
void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Recieved Packet: '");
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      notifyClients(LoRaData);
    }
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
