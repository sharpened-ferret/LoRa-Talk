#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("LoRa Echo");

  LoRa.setPins(ss, rst, dio0);

  while(!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xFF);
  Serial.println("LoRa Initialising OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData;
    Serial.print("Recieved packet '");
    
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData); 

      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
    }

    LoRa.beginPacket();
    LoRa.print("Message Recieved: ");
    LoRa.print(LoRaData);
    LoRa.endPacket();

    Serial.println("Response Sent");
  }
}
