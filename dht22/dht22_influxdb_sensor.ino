// Required libraries
//
// Adafruit Unified Sensor
// DHT Sensor Library (Adafruit)
// ESP8266 Influxdb
//

// Wifi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Sensors
#include <DHT.h>

// Databases
#include "InfluxDb.h"

// Wifi settings
const char* ssid     = "something";
const char* password = "password";

// Influx DB settings
#define INFLUXDB_HOST "192.168.1.2"
#define INFLUXDB_PORT "8086"
#define INFLUXDB_DATABASE "smarthome"
#define INFLUXDB_USER "arduino"
#define INFLUXDB_PASS "arduino"

#define SENSOR_LOCATION "conservatory"

Influxdb influx(INFLUXDB_HOST);

// Constants
const int DATA_DELAY_MS = 60000; // Sixty seconds

// Sensor data
float humidity_percent = 0.0;
float temp_c = 0.0;

// Sensor readings timer - DHT22 produces new data at about 2s intervals.
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // Minimum interval at which to read sensor in ms

// Buffer space for writing float into
static char temp_buffer[15];

//--------------------------------------------------
#define DHTTYPE DHT22
#define DHTPIN 14
DHT dht(DHTPIN, DHTTYPE, 11);

void connectWifi() {  

  WiFi.begin(ssid, password);
  Serial.print("\n\rWifi will connect...");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(ssid);
  Serial.println(WiFi.localIP());
}

void connectInfluxDB()
{
  influx.setDbAuth("test", INFLUXDB_USER, INFLUXDB_PASS);
}

void setup() {
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  // Wait for serial to initialize.
  while(!Serial) { }
  
  // put your setup code here, to run once:
  connectWifi();  
  dht.begin();           // initialize temperature sensor
  connectInfluxDB();
}

void getTemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity_percent = dht.readHumidity();          // Read humidity (percent)
    temp_c = dht.readTemperature();     // Read temperature as deg_c
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity_percent) || isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Turn into a string:
    dtostrf(temp_c, 7, 3, temp_buffer);
    Serial.println(temp_buffer);
  }
}

void deliverTemperature() {
  InfluxData row("temperature");
  row.addTag("device", SENSOR_LOCATION);
  row.addTag("type", "dht22");
  row.addValue("deg_c", temp_c);
  row.addValue("humidity_percent", humidity_percent);

  influx.write(row);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi down. Reconnect...");
    WiFi.reconnect();
  }

  getTemperature();
  deliverTemperature();

  // Sleep before delivering the next reading
  delay(DATA_DELAY_MS);
}
