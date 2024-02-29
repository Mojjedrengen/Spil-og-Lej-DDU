#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* SSID = "Next-Guest";
const char* WIFI_PASS = "";

static const char* fingerprint PROGMEM = "DF 12 7C 16 41 3D 8C 87 0D 8A 40 DE FD 08 2E 7F 8D 3D 08 3B";

const char* MQTT_SERVER = "mqtt.nextservices.dk";
const uint16_t MQTT_PORT = 8883; 
const char* MQTT_CLIENT_ID = "FAMSSortHvid";

WiFiClientSecure wifiClient;
PubSubClient pubSubClient(wifiClient);

void setup_wifi() {
  Serial.printf("Connecting to %s", SSID);
  WiFi.begin(SSID, WIFI_PASS);

  // Wait until WiFi is connected.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.printf("\nSuccessfully connected to %s!\n", SSID);
  wifiClient.setInsecure();
}

void setup_mqtt() {
  pubSubClient.setServer(MQTT_SERVER, MQTT_PORT);
  pubSubClient.setCallback(mqtt_callback);
}

void reconnect_mqtt() {
  while (!pubSubClient.connected()) {
    Serial.printf("Attempting to reconnect to MQTT Broker: %s\n", MQTT_SERVER);
    if (pubSubClient.connect(MQTT_CLIENT_ID)) {
      Serial.printf("Connected!\n");
      pubSubClient.publish("DDU4/FAMS", "Connected!");
      pubSubClient.subscribe("DDU4/FAMS/SpilOgLej");
    } else {
      Serial.printf("failed to connect, rc=%d\n", pubSubClient.state());
      delay(5000);
    }
  }
}
#define sort 0
#define hvid 1
const int code[6] = {hvid, hvid, sort, sort, sort, hvid};
bool isGame = false;
int codeIndex = 0;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLUE);

  setup_wifi();
  setup_mqtt();

  pinMode(26, INPUT_PULLDOWN);
  pinMode(25, INPUT_PULLDOWN);
  pinMode(36, INPUT_PULLDOWN);
}

void loop() {
  if (!pubSubClient.connected()) {
    Serial.println("Connection lost to MQTT Broker!");
    reconnect_mqtt();
  }
  pubSubClient.loop();
  M5.update();

  if (isGame){
    gameLoop();
  }
  M5.Lcd.fillScreen(BLUE);
  delay(150);
}

void gameLoop(){
  if (digitalRead(26)) {
    if (code[codeIndex] == hvid) {
      codeIndex++;
      M5.Lcd.fillScreen(GREEN);
    } else if(code[codeIndex] != hvid){
      codeIndex = 0;
      M5.Lcd.fillScreen(RED);
    }
  }
  if (digitalRead(36)) {
    if (code[codeIndex] == sort) {
      codeIndex++;
      M5.Lcd.fillScreen(GREEN);
    } else if(code[codeIndex] != sort){
      codeIndex = 0;
      M5.Lcd.fillScreen(RED);
    }
  }

  Serial.printf("CodeIndex: %d\n", codeIndex);
  if (codeIndex >= 6){
    pubSubClient.publish("DDU4/FAMS/SpilOgLej", "SortHvidKodeIsDone");
    isGame = false;
    M5.Lcd.fillScreen(PURPLE);
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("Message arrived [%s]:", topic);
    // String str = byteArrayToString(payload, sizeof(payload));
    //Serial.println(str);
    char* data = new char[length + 1];
    memcpy(data, payload, length);
    data[length] = '\0';
    Serial.printf("byte: %s\n", data);
    for (int i = 0; i < length; i++) {
        //M5.Lcd.print((char)payload[i]);
    }
    if (strcmp(data, "start") == 0) {
      isGame = true;
      codeIndex = 0;
    }
}
