#include <LoRa.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#define ss 5
#define rst 14
#define dio0 2

// Network Credentials
const char* ssid = "WiFiIsCool";
const char* password = "CoolCastl3s";
const char* server_uri = "ws://192.168.0.15:80/ws";

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
    LoRa.beginPacket();
    LoRa.print(message.data());
    LoRa.endPacket();  
    Serial.println("Sent Packet: \n" + message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    }
}

WebsocketsClient client;



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

  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  client.connect(server_uri);
////  client.onMessage([](WebsocketsMessage msg)){
//    
//  };
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Recieved Packet: '");
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      client.send(LoRaData);
    }
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
  client.poll();
}
