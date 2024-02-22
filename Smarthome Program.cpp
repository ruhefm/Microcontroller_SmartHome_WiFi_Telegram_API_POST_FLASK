#include <WiFi.h>

#include <SPI.h>

#include <MFRC522.h>

#include <PubSubClient.h>

#include <ThingsBoard.h>

#include <DHT.h>

#include <HTTPClient.h>

#include <ArduinoJson.h>

#define SS_PIN 5 // ESP32 pin GIOP5
#define RST_PIN 27 // ESP32 pin GIOP27

unsigned long sekarang = 0;
unsigned long interval = 10000;

// WIFI CLIENT HTTPS

const String server = "http://192.168.1.16/post.php/";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
String serverReal = "https://teleapigps.pythonanywhere.com/";

// WIFI CLIENT HTTPS

#define WIFI_AP "wwe" // ganti nama AP
#define WIFI_PASSWORD "123456elis" // ganti password AP
#define TOKEN "Usndp9QvG72Q7i7Zat9O" // Things Board Token

#define DHTPIN 4
#define DHTTYPE DHT11

char thingsboardServer[] = "192.168.1.16";

int status = WL_IDLE_STATUS;

const int buttonPin = 13; // the number of the pushbutton pin
const int buttonPin2 = 12; // limit switch
const int relay1 = 22; // the number of the DL pin

// variables will change:
int buttonState = 0; // variable for reading the pushbutton status
int buttonState2 = 0;

constexpr uint32_t MAX_MESSAGE_SIZE = 1024 U;

WiFiClient wifiClient;

MFRC522 mfrc522(SS_PIN, RST_PIN);
String UID;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(200);

  dht.begin();

  InitWiFi();

  SPI.begin(); // init SPI bus
  mfrc522.PCD_Init(); // init MFRC522

  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID"));

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
}
void bacarfid() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  UID = "";
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UID += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ") + String(mfrc522.uid.uidByte[i], HEX) + +(i != (mfrc522.uid.size - 1) ? ":" : "");
  }

  UID.toUpperCase();
  Serial.print(UID);
  mfrc522.PICC_HaltA();
  delay(10);

  if (UID.substring(1) == "93: 81: 03: 80" || UID.substring(1) == "03: EA: 91: A5") {
    Serial.println(" || akses oke");
    digitalWrite(relay1, HIGH);
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin("https://teleapigps.pythonanywhere.com/key");
      http.addHeader("Content-Type", "application/json");

      // Assuming humi, tempF, and tempC are defined and have valid values
      StaticJsonDocument < 200 > doc;
      doc["key"] = 1;
      doc["uid"] = UID.substring(1);

      String jsonString;
      serializeJsonPretty(doc, jsonString);
      Serial.println(jsonString); // Print the JSON string for debugging

      int httpResponseCode = http.POST(jsonString);
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      // wait a 2 seconds between readings
      delay(2000);
    }
  }
}

void bacadhtsensor() {

  float humi = dht.readHumidity();

  float tempC = dht.readTemperature();

  float tempF = dht.readTemperature(true);

 
  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {

    if (millis() - sekarang >= interval) {
      sekarang = millis();
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        http.begin("https://teleapigps.pythonanywhere.com/temp");
        http.addHeader("Content-Type", "application/json");

        
        StaticJsonDocument < 200 > doc;
        doc["humi"] = humi;
        doc["tempf"] = tempF;
        doc["tempc"] = tempC;

        String jsonString;
        serializeJsonPretty(doc, jsonString);
        Serial.println(jsonString); // Print the JSON string for debugging

        int httpResponseCode = http.POST(jsonString);
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String response = http.getString();
          Serial.println(response);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        http.end();
        
        delay(2000);
      }
    }
  }
}
void loop() {
  reconnect();
  bacarfid();
  bacadhtsensor();
  buttonState = digitalRead(buttonPin);
  buttonState2 = digitalRead(buttonPin2);
  if (buttonState2 == LOW) {
    digitalWrite(relay1, HIGH);
    Serial.println(" || tutup");
  } else if (buttonState == LOW) {
 
    Serial.println(" || akses oke");
    digitalWrite(relay1, LOW);

  }
}

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  status = WiFi.status();
  if (status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}