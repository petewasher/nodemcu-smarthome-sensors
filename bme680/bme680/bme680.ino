/* 
   ESP8266 wifi connection which reports from BME680 
   sensor to InfluxDB
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid     = "something";
const char* password = "password";

// Uses https://github.com/tobiasschuerg/ESP8266_Influx_DB
#include <InfluxDb.h>
#define INFLUXDB_HOST "your-server-ip-address-here"
#define INFLUXDB_PORT "8086"
#define INFLUXDB_DATABASE "your-database-here"
#define INFLUXDB_USER "your-user-here"
#define INFLUXDB_PASS "your-pass-here"

Influxdb influx(INFLUXDB_HOST); // port defaults to 8086, use 9999 for v2

#include "Adafruit_BME680.h"
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; // Uses i2C


void connectWifi() {
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println(WiFi.localIP());
}


void connectBME680() {
  if (!bme.begin(0x76))
  {
    Serial.println("Couldn't find a valid sensor!");
    while(1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320,150);
}

void connectInfluxDB() {
  // set the target database
  influx.setDb(INFLUXDB_DATABASE);
  // or use a db with auth
  influx.setDbAuth(INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASS); // with authentication
}
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable or USB
  
  // Connect to WiFi network
  connectWifi();
  connectBME680();
  connectInfluxDB();
}

void publishReading() {
  // create a measurement object
  InfluxData measurement ("temperature");
  measurement.addTag("device", "garage-office");
  measurement.addTag("type", "bme680");
  measurement.addValue("deg_c", bme.temperature);
  measurement.addValue("humidity_percent", bme.humidity);
  measurement.addValue("pressure_hPa", bme.pressure);
  
  // write it into db
  influx.write(measurement);
}

void printFullReading() {
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
}

void loop(void)
{
  // put your main code here, to run repeatedly:
  if (!bme.performReading())
  {
    Serial.println("Failed to read");
    return;
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi reconnecting...");
    WiFi.reconnect();
    delay(2000);
  }

  printFullReading();
  publishReading();

  // Shut up for 20s
  delay(20000);
} 
