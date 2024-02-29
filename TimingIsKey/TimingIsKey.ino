#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <FastLED.h>

#define Neopixel_PIN 32
#define NUM_LEDS     5

CRGB leds[NUM_LEDS];

const char* SSID = "Next-Guest";
const char* WIFI_PASS = "";

static const char* fingerprint PROGMEM = "DF 12 7C 16 41 3D 8C 87 0D 8A 40 DE FD 08 2E 7F 8D 3D 08 3B";

const char* MQTT_SERVER = "mqtt.nextservices.dk";
const uint16_t MQTT_PORT = 8883; 
const char* MQTT_CLIENT_ID = "FAMSTiming";

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

const int code[5] = {4, 2, 0, 3, 1}; //the code remberber that it has to be one lower
int codeIndex = 0;
bool isGame = false;


void setup() {
  M5.begin();
  //M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLUE);

  pinMode(26, INPUT_PULLDOWN);

  setup_wifi();
  setup_mqtt();
  M5.Lcd.setRotation(1); //Rotate the screen 90 degrees clockwise (1*90)
  M5.Lcd.setTextSize(13);
  M5.Lcd.setTextColor(BLACK, GREEN);
  M5.Lcd.setCursor(0, 0);

  //M5.Lcd.fillScreen(BLUE);
  M5.update();
  //display();

  // Neopixel initialization
    FastLED.addLeds<WS2811, Neopixel_PIN, GRB>(leds, NUM_LEDS)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(64);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
  FastLED.show();  // must be executed for neopixel becoming effectiv
  
}

void loop() {
  M5.update();
  if (!pubSubClient.connected()) {
    Serial.println("Connection lost to MQTT Broker!");
    Serial.printf("state: %d\n", pubSubClient.state());
    reconnect_mqtt();
  }

  pubSubClient.loop();
  

  if (isGame == true) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
      FastLED.show();
      M5.update();
      buttonCheck(i);
      pubSubClient.loop();
      delay(750);
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  }

  delay(250);
}

//function to check button input
//currently it does not work
void buttonCheck(int currLight) {
  if (digitalRead(26)){
    if (code[codeIndex] == currLight){
      codeIndex++;
      leds[currLight] = CRGB::Green;
      FastLED.show();
    } else if (code[codeIndex] != currLight){
      codeIndex = 0;
      leds[currLight] = CRGB::Red;
      FastLED.show();
    }
  }

  //M5.Lcd.print(codeIndex);

  if (codeIndex >= 5) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
    M5.Lcd.fillScreen(GREEN);
    isGame = false;
    M5.Lcd.setCursor(30, 50);
    M5.Lcd.print("112C"); //her vil den printe koden som bliver brugt i SetTheTempurtur
    pubSubClient.publish("DDU4/FAMS/SetTheTempurturCode", "start");
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
      M5.Lcd.fillScreen(BLUE);
    }
}


// Spillet er næsten færdig. Mangler bare at få det til at virke med de andre gåder.