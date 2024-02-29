#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <FastLED.h>
// select the input pin for the potentiometer
#define sensorPin 33
// last variable to store the value coming from the sensor
int cur_sensorValue = 0;

const char* SSID = "Next-Guest";
const char* WIFI_PASS = "";

static const char* fingerprint PROGMEM = "DF 12 7C 16 41 3D 8C 87 0D 8A 40 DE FD 08 2E 7F 8D 3D 08 3B";

const char* MQTT_SERVER = "mqtt.nextservices.dk";
const uint16_t MQTT_PORT = 8883; 
const char* MQTT_CLIENT_ID = "FAMSTemperatur";

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
      pubSubClient.subscribe("DDU4/FAMS/SetTheTempurturCode");
    } else {
      Serial.printf("failed to connect, rc=%d\n", pubSubClient.state());
      delay(5000);
    }
  }
}

const int code = 112; //Dette er den temperatur der skal sættes ind i spillet
bool isGame = false;
const int timer = 20; //denne burder være 3 sekunder med et delay på 150 millisekunder.
int counter = 0;

void setup() {
  M5.begin();
  setup_wifi();
  setup_mqtt();
  pinMode(sensorPin, INPUT);
  M5.Lcd.setRotation(1); //Rotate the screen 90 degrees clockwise (1*90)
  M5.Lcd.setTextSize(13);
  M5.Lcd.setCursor(0, 0);
}

void loop() {
  if (!pubSubClient.connected()) {
    Serial.println("Connection lost to MQTT Broker!");
    reconnect_mqtt();
  }
  pubSubClient.loop();

  if (isGame){
    gameLoop();
  }
 
  delay(150);
}

void gameLoop(){
   // read the value from the sensor:
  cur_sensorValue = 180 - (((float)analogRead(sensorPin))/4095.0)*180;
  //cur_sensorValue = analogRead(sensorPin);
  M5.Lcd.setCursor(30, 50);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.print(cur_sensorValue);

  if (cur_sensorValue >= code-1 && cur_sensorValue <= code+1) {
    counter++;
  } else if (cur_sensorValue != code) {
    counter = 0;
  }

  if (counter >= timer){
    isGame = false;
    pubSubClient.publish("DDU4/FAMS/SpilOgLej", "SetThetempurturIsDone");
    M5.Lcd.fillScreen(GREEN);
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
        M5.Lcd.print((char)payload[i]);
    }
    if (strcmp(data, "start") == 0) {
      isGame = true;
    }
}


// Spillet er fuldstænding komplet med Mqtt
// Har fikset mqtt brug mqtt_callback fra denne til de andre programmer
// Angle uniten virker mellem 0-180
// Skal huske at koden er 112 og er hard coded.