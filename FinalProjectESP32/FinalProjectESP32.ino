#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>

#define RXD2 17
#define TXD2 16
#define DHT_SENSOR_PIN 27
#define DHT_SENSOR_TYPE DHT22

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

const char* ssid = "wifi_6";
const char* password = "#BossRaLuchas03";
const char* mqtt_server = "35.91.221.3";
float currentTempC = 0.0;
float currentTempF = 0.0;
float currentHumidity = 0.0;
const char* temp_topic = "temp";
const char* humid_topic = "humid";
const char* p1 = "particle1";
const char* p2 = "particle2";
const char* p3 = "particle3";
WiFiClient espClient;
PubSubClient client(espClient);
void setup() {
  // our debugging output
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  client.setServer(mqtt_server, 1883);
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
  dht_sensor.begin();
  // Set up UART connection
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  currentTempC = dht_sensor.readTemperature();
  currentTempF = dht_sensor.readTemperature(true);
  currentHumidity = dht_sensor.readHumidity();
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;
char* convertIntToChar(int num) {
  char* str = new char[10];
  sprintf(str, "%d", num);
  return str;
}

char* convertFloatToChar(float num) {
  char* str = new char[10];
  sprintf(str, "%f", num);
  return str;
}

void loop() {
  float humi = dht_sensor.readHumidity();
  float tempC = dht_sensor.readTemperature();
  float tempF = dht_sensor.readTemperature(true);
  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    currentTempC = tempC;
    currentTempF = tempF;
    currentHumidity = humi;
    Serial.println(currentTempC);
    Serial.println(currentTempF);
    Serial.println(currentHumidity);
    client.publish(temp_topic, convertIntToChar(currentTempF));
    client.publish(humid_topic, convertIntToChar(currentHumidity));
    // client.publish(temp, std::to_string(currentTempC));
  }

  if (readPMSdata(&Serial1)) {
    // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_standard);
    client.publish(p1, convertIntToChar(data.pm10_standard));
    client.publish(p2, convertIntToChar(data.pm25_standard));
    client.publish(p3, convertIntToChar(data.pm100_standard));

    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: ");
    Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: ");
    Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air:");
    Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:");
    Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:");
    Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:");
    Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:");
    Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:");
    Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }
  sleep(10);
}

boolean readPMSdata(Stream* s) {
  if (!s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  /* debugging
    for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
  */

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void*)&data, (void*)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  
  // success!
  return true;
}
