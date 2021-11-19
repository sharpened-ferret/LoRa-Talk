#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

int counter = 0;
int wait_counter;

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
  wait_counter = 0;
  
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  while (wait_counter < 5) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      while(LoRa.available()) {
        String LoRaData = LoRa.readString();
        Serial.println(LoRaData);
      }
      break;
    }
    delay(1000);
    wait_counter++;
  }

  counter++;

  delay(10000);
}
