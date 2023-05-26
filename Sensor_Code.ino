#include "DHT.h"
#include <RTClib.h>
#include <Wire.h>

#define DHTPIN 4          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11     // DHT 11
#define TRIGPIN 5         // Ultrasonic sensor trigger pin
#define ECHOPIN 18        // Ultrasonic sensor echo pin
#define HEIGHT_OF_TUBE 10 // Height of the water tank in cm
#define SOUND_SPEED 0.034 // Speed of sound in cm/microsecond
#define RELAY1 19         // Relay pin for controlling the pump
#define RELAY2 13         // Relay pin for controlling the LED
#define SOIL 1            // Soil moisture sensor analog pin
#define WET 210           // Moisture reading for wet soil
#define DRY 510           // Moisture reading for dry soil

// Create a DHT object.
DHT dht(DHTPIN, DHTTYPE);
long duration;
float distanceCm;

RTC_DS3231 rtc;
char t[32];

float t; // Temperature reading from DHT sensor
float wetThreshold = 100.0; // Initial moisture threshold for wet soil
float dryThreshold = 0.0;   // Initial moisture threshold for dry soil

void setup() {
  Serial.begin(115200);
  dht.begin();          // Initialize the DHT sensor
  pinMode(TRIGPIN, OUTPUT); // Set the trigPin as an Output
  pinMode(ECHOPIN, INPUT);  // Set the echoPin as an Input
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT); // Set the LED pin as an Output

  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Adjust RTC to compile time

  // rtc.adjust(DateTime(2019, 1, 21, 5, 0, 0)); // Manually adjust RTC date and time
}

void loop() {
  // DHT sensor
  t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Print the temperature to the serial port
  Serial.printf("Temperature: %.2f\n", t);

  // Adjust moisture threshold based on temperature
  if (t >= 30) {
    wetThreshold = 100.0; // Adjust wet threshold for high temperatures
    dryThreshold = 30.0;  // Adjust dry threshold for high temperatures
  } else if (t >= 20 && t < 30) {
    wetThreshold = 90.0; // Adjust wet threshold for moderate temperatures
    dryThreshold = 20.0; // Adjust dry threshold for moderate temperatures
  } else {
    wetThreshold = 80.0; // Adjust wet threshold for low temperatures
    dryThreshold = 10.0; // Adjust dry threshold for low temperatures
  }

  // Soil Moisture
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(SOIL);
  moisture_percentage = map(sensor_analog, WET, DRY, 100, 0);
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");

  // Pump control based on moisture percentage
  if (moisture_percentage < dryThreshold) {  // Pump water if moisture percentage is below threshold
    digitalWrite(RELAY1, LOW);
  } else if (moisture_percentage > wetThreshold) { // Stop the pump when moisture reaches desired threshold
    digitalWrite(RELAY1, HIGH);
  }

  // Ultrasonic sensor
  // Clears the trigPin
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHOPIN, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  if (distanceCm > HEIGHT_OF_TUBE) {
    Serial.print("Water Level Low!!");
  }

  DateTime now = rtc.now();
  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
  Serial.print("Date/Time: ");
  Serial.println(t);

  if (now.hour() == 18) {
    digitalWrite(RELAY2, HIGH);  // Turn on LED lights at 6pm every day
  } else if (now.hour() == 7) {
    digitalWrite(RELAY2, LOW);   // Turn off LED lights at 7am
  }

  delay(2000); // Obtain DHT sensor reading every 2 seconds (limit is 1kHz)
}
