#include "wt32setup.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <cstring>

// wifi credentials
const char* ssid = "wifi_6";
const char* password = "#BossRaLuchas03";
const char* mqtt_server = "35.91.221.3";
const char* temp_topic = "temp";
const char* humid_topic = "humid";
const char* p1 = "particle1";
const char* p2 = "particle2";
const char* p3 = "particle3";
const int mqtt_port = 1883;
String temp_value = "";
String humid_value = "";

LGFX tft;
#define bgcolor1 0x4A69
#define bgcolor2 0x632C
WiFiClient espClient;
PubSubClient client(espClient);

struct SpriteData {
  int x;
  int y;
  int width;
  int height;
  const char* label;
};
SpriteData buttonData[] = {
  { 35, 35, 36, 59, "Button 1" },             // Button 1 coordinates and size
  { 35, 35 + 97, 36, 59, "Button 2" },        // Button 2 coordinates and size
  { 35, 35 + (2 * 97), 36, 59, "Button 3" },  // Button 3 coordinates and size
};

void callback(char* topic, byte* payload, unsigned int length) {
  String payloadString;
  for (int i = 0; i < length; i++) {
    // Serial.print((char)payload[i]);
    payloadString += (char)payload[i];
  }
  if (strcmp(topic, "temp") == 0) {
    temp_value = payloadString;
  }
  if (strcmp(topic, "humid") == 0) {
    humid_value = payloadString;
  }
  draw();
}

void setup() {
  // in this mode esp32 connects to a wifi access point
  Serial.begin(921600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str())) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe(temp_topic);
  client.subscribe(humid_topic);
  client.subscribe(p1);
  client.subscribe(p2);
  client.subscribe(p3);
  Serial.println("");
  Serial.println(WiFi.localIP());
  tft.begin();
  tft.setSwapBytes(true);
  draw();
}

void draw() {
  tft.fillScreen(TFT_WHITE);
  tft.setColorDepth(16);
  tft.setRotation(1);
  tft.setTextSize(5);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(105, 53);
  tft.println("Group 2");
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(105, 130);
  tft.print("Temperature: " + temp_value);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(105, 150);
  tft.print("Humidity: " + humid_value);
}

void loop() {
  client.loop();
}
