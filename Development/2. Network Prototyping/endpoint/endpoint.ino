#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define ss 5
#define rst 14
#define dio0 2

// Network Credentials
const char* ssid = "WiFiIsCool";
const char* password = "CoolCastl3s";

// Set web server port number to 80
AsyncWebServer server(80);

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
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while ((WiFi.status() != WL_CONNECTED) || !LoRa.begin(866E6)) {
    Serial.print(".");
    delay(500);
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  LoRa.setSyncWord(0xFF);
  Serial.println("LoRa Initialising OK!");


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
