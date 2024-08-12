#include <OneWire.h>
#include <DallasTemperature.h>
#include "networking.h"

void checkIfEnoughTimeHasPassed();

#define ONE_WIRE_BUS D1
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

// timer stuff
const int minimumTimeForFirebaseWrite = 500;  // .5 SECONDS
long setTimeForDispatch = millis();
bool hasEnoughTimePassed = false;

// Number of temperature devices found
int numberOfDevices;

// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;

// networking class
Networking network;

void setup(void)
{
  // start serial port
  Serial.begin(9600);

  // Start up the libraries
  sensors.begin();
  network.setup(10000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  checkIfEnoughTimeHasPassed();
  if (hasEnoughTimePassed == true) {
    hasEnoughTimePassed = false;
    sensors.requestTemperatures();  // Send the command to get temperatures
    float temps[3] = { 0 };
    // Loop through each device, print out temperature data
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
      // Search the wire for address
      if (sensors.getAddress(tempDeviceAddress, i)) {
        float tempC = sensors.getTempC(tempDeviceAddress);
        Serial.print(" Temp F: ");
        Serial.print(i);
        Serial.print(": ");
        float tempF = DallasTemperature::toFahrenheit(tempC);
        Serial.println(tempF);  // Converts tempC to Fahrenheit
        temps[i] = tempF;
      }
    }
    String data = String(temps[0]) + ";" + String(temps[1]) + ";" + String(temps[2]);
    network.writeDataToThingSpeak(data);
  }
}

void checkIfEnoughTimeHasPassed() {
  int elapsedTime = millis() - setTimeForDispatch;
  if (elapsedTime > minimumTimeForFirebaseWrite && hasEnoughTimePassed == false) {
    setTimeForDispatch = millis();
    hasEnoughTimePassed = true;
  }
}