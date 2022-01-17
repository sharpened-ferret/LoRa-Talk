#include <LoRa.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#define ss 5
#define rst 14
#define dio0 2

// Network Credentials
const char* ssid = "WiFiIsCool";
const char* password = "CoolCastl3s";

WebSocketsClient webSocket;



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
