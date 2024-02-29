#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* SSID = "Next-Guest";
const char* WIFI_PASS = "";

static const char* fingerprint PROGMEM = "DF 12 7C 16 41 3D 8C 87 0D 8A 40 DE FD 08 2E 7F 8D 3D 08 3B";

const char* MQTT_SERVER = "mqtt.nextservices.dk";
const uint16_t MQTT_PORT = 8883; 
const char* MQTT_CLIENT_ID = "FAMSMorse";

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


const int code[8] = {1,0,1, 0, 1,0,1,1};
// KEY
int codeindex = 0;

bool isGame = false;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  //M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLUE);

  pinMode(26, INPUT_PULLDOWN);

  setup_wifi();
  setup_mqtt();

  pinMode(26, INPUT_PULLDOWN);
  pinMode(25, INPUT_PULLDOWN);
  pinMode(36, INPUT_PULLDOWN);

  //M5.Lcd.fillScreen(BLUE);
  M5.update();
  //display();
}

void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
  M5.Lcd.fillScreen(BLUE);

  if (!pubSubClient.connected()) {
    Serial.println("Connection lost to MQTT Broker!");
    reconnect_mqtt();
  }
  pubSubClient.loop();

  if (isGame) {
     checkbutton();
  }
 
  delay(250);
}

void checkbutton(){
  if (digitalRead(26)) {
    if (code[codeindex] == 0) {
      codeindex++;
    } else if (code[codeindex] == 1) {
      codeindex = 0;
      M5.Lcd.fillScreen(RED);
    }
  } else if (digitalRead(36)) {
    if (code[codeindex] == 1) {
      codeindex++;
    } else if (code[codeindex] == 0) {
      codeindex = 0;
      M5.Lcd.fillScreen(RED);
    }
  }
  Serial.printf("CodeIndex: %d\n", codeindex);
  if (codeindex >= 8){
    M5.Lcd.fillScreen(GREEN);
    pubSubClient.publish("DDU4/FAMS/SpilOgLej", "MorseKodeTheKeyIsKeyIsDone");
    isGame = false;
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
      codeindex = 0;
    }
}


// Burde være færdig. Manlger MQTT og snakke med de andre M5'er
