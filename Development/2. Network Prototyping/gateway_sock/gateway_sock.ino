#include <LoRa.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

// Sets the GPIO pin numbers for the LoRa Transceiver connection
#define ss 5
#define rst 14
#define dio0 2

// Sets Network Credentials for connecting to WiFi
const char* ssid = "WiFiIsCool";
const char* password = "CoolCastl3s";

// Sets the URL for the central chat server
const char* server_uri = "ws://192.168.0.15:80/ws";

using namespace websockets;
WebsocketsClient client;

// Handles WebSocket messages from the chat server
// Recieves the message and relays it to endpoints over LoRa
void onMessageCallback(WebsocketsMessage message) {
    LoRa.beginPacket();
    LoRa.print(message.data());
    LoRa.endPacket(true);
    delay(100);
    Serial.println("Sent Packet: \n" + message.data());
}

// Handles WebSocket connection events with the chat server
void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    }
}

// Program initialisation. Runs on boot
void setup() {
  Serial.begin(115200);

  // Configures the LoRa transceiver connection
  LoRa.setPins(ss, rst, dio0);

  // Connects to the WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Waits for the LoRa transceiver and WiFi connections to be complete
  while ((WiFi.status() != WL_CONNECTED) || !LoRa.begin(866E6)) {
    Serial.print(".");
    delay(500);
  }
  
  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Sets sync word for the LoRa library
  // This must match with the sync word used by the Endpoints
  LoRa.setSyncWord(0xFF);
  Serial.println("LoRa Initialising OK!");

  // Registers callback functions on the WebSocket connection
  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  // Starts WebSocket connection to the chat server
  client.connect(server_uri);
}

// Recieves LoRa messages from endpoints and relays them to the chat server over the WebSocket connection
void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Recieves a LoRa message from an Endpoint
    Serial.print("Recieved Packet: '");
    String LoRaData;
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
    }
    Serial.print(LoRaData);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    // Sends the message to the chat server
    client.send(LoRaData);
  }
  // Ensures the WebSocket connection is active
  client.poll();
}
