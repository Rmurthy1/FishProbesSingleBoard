#include "networking.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secureConfig.h"
#include <WiFiClientSecure.h>

// gotta turn off the led since its not plugged in.

JsonDocument doc;
    
ESP8266WiFiMulti WiFiMulti;
WiFiClientSecure  client;

char ssid[] = WIFI_SSID;   // your network SSID (name) 
char pass[] = WIFI_PASSWORD;   // your network password

// todo: expose this to the user of the library
bool lightBlink = false;
const byte wifiLED = 5; // d1

unsigned long time_now = 0;
int period = 1000; // time between blinks

// this fingerprint stuff is not used at the moment, we send data insecurely.
// Fingerprint check, make sure that the certificate has not expired.
const char * fingerprint = SECRET_SHA1_FINGERPRINT; // use SECRET_SHA1_FINGERPRINT for fingerprint check

bool sendData = true;
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = THINGSPEAK_API_WRITE;

int number = 0;
int delayTime = 10000 * 2; // delay between writes to ThingSpeak in ms

// Function declarations
void sendDataToThingSpeak(String data);
void blinkLight(bool keepBlinking);
void thingSpeakWriteREST(String data);
void prepareJSON(String message);
String getValue(String data, char separator, int index);
void wifiStatusLED();
void updateRate(int rate);

// update the delay time between writes to ThingSpeak, in ms. default is 10000
void updateRate(int rate) {
  delayTime = rate;
}

// called from user of library, sends data to ThingSpeak every delayTime ms
void sendDataToThingSpeak(String data) {
  
  //blinkLight(lightBlink);
  // flips between 0 and 1 every 60 seconds. (0 to min 1 = 0, 1 to 2 = 1, 2 to 3 = 0, etc) (sends data every two minutes, when 60000)
  // every (2 * 60000) seconds
  if (((millis() / delayTime) % 2)) {
   
    if (sendData == true) {
      //wifiStatusLED();
      WiFiMulti.run();
      Serial.println("data sent to thinkspeak: " + (data));
      thingSpeakWriteREST(data);
      sendData = false;
    }
  } else {
    sendData = true;
  }
}

// constants won't change. Used here to set a pin number:
const int ledPin = LED_BUILTIN;  // the number of the LED pin

// Variables will change:
int ledState = LOW;  // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)

// called from loop, just blinks the light.
void blinkLight(bool keepBlinking) {
  
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(ledPin, ledState);
    }
 
}

// write to thingspeak using a global api key and parameter data
void thingSpeakWriteREST(String data) {

  // prepare the json file then make it into a string
  prepareJSON(data);
  String payload;
  serializeJson(doc, payload);
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  Serial.println("writing to thingspeak");
  HTTPClient https;
  String endpoint = "https://api.thingspeak.com/update.json";

  if (https.begin(*client, endpoint)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(payload);
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String serverUpdate = https.getString();
        Serial.println(serverUpdate);
        //success = true; // this should be passed in as a reference
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();

  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  
}

// total tokens will
void prepareJSON(String message) {
  doc["api_key"] = THINGSPEAK_API_WRITE;
  // tokenize the string, count the tokens, prepare the data.
  String totalTokens = getValue(message, ';', 0);
  int totalTokensCount = totalTokens.toInt();
  Serial.println("total tokens: " + totalTokensCount);
  for (int i = 0; i <= totalTokensCount; i++) {
    String field = "field"+String(i+1);
    String dataValue = getValue(message, ';', i);
    doc[field] = dataValue.toDouble();
  }
  
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// is this a required function? i might need the wifimulti.run() function.
void wifiStatusLED() {
  Serial.println("wifiStatusLED");
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    digitalWrite(wifiLED, HIGH);
  } else {
    digitalWrite(wifiLED, LOW);
  }
}


void Networking::writeDataToThingSpeak(String data) {
    sendDataToThingSpeak(data);
}

void Networking::setup(int rate) {
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(wifiLED, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  WiFiMulti.run();
  //this->updateRate(rate);
}

void Networking::updateRate(int rate) {
  updateRate(rate);
}