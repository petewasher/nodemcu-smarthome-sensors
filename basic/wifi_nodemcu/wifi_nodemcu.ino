/* 
   Basic ESP8266 wifi connection ready to use for other projects
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid     = "something";
const char* password = "password";
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable or USB
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rDo connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}
 
void loop(void)
{
  /* Your mainloop code goes here */
} 
