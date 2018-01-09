#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h"
#include "SSD1306.h"
#include "SSD1306Spi.h"

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D1 -> RES
// D2 -> DC
// D8 -> CS
SSD1306Spi        display(D1, D2, D8);

int sensor_pin = A0; 
int output_value ;

// Adafruit IO 
#define AIO_SERVER      "io.adafruit.com" 
#define AIO_SERVERPORT  1883 
#define AIO_USERNAME    "natalietrinh102" 
#define AIO_KEY         "37d3a2261c7f497cbd52a1069b7e247a" 
WiFiClient client; 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY); 

// Setup feed 
Adafruit_MQTT_Publish moisture = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/anduinoMoisture"); 

// connect to adafruit io via MQTT 
void connect() { 
 Serial.print(F("Connecting to Adafruit IO... ")); 
 int8_t ret; 
 while ((ret = mqtt.connect()) != 0) { 
   switch (ret) { 
     case 1: Serial.println(F("Wrong protocol")); break; 
     case 2: Serial.println(F("ID rejected")); break; 
     case 3: Serial.println(F("Server unavail")); break; 
     case 4: Serial.println(F("Bad user/pass")); break; 
     case 5: Serial.println(F("Not authed")); break; 
     case 6: Serial.println(F("Failed to subscribe")); break; 
     default: Serial.println(F("Connection failed")); break; 
   } 
   if(ret >= 0) 
     mqtt.disconnect(); 
   Serial.println(F("Retrying connection...")); 
   delay(5000); 
 } 
 Serial.println(F("Adafruit IO Connected!")); 
} 

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("Nighthawk", "~trinhwireless626");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  // connect to adafruit io 
 connect(); 
  
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

}

void loop() {
     // ping adafruit io a few times to make sure we remain connected 
 if(! mqtt.ping(3)) { 
   // reconnect to adafruit io 
   if(! mqtt.connected()) 
     connect(); 
    } 
      
  // Read Moisture sensor & Normalize
  output_value= analogRead(sensor_pin);
  output_value = map(output_value,550,0,0,100);
  
   //convert int output_value to char array 
   char b[4]; 
   String str; 
   str=String(output_value); 
   for(int i=0; i<str.length(); i++) 
   { 
     b[i]=str.charAt(i); 
   } 
   b[(str.length())+1]=0; 

   // Publish data 
   if (!moisture.publish((char*)b)) { 
     Serial.println(F("Failed to publish moisture")); 
   } else { 
     Serial.print(F("Moisture published: ")); 
     Serial.println(output_value); 
   } 
   // Check if any reads failed and exit early (to try again). 
   if (isnan(output_value)) { 
     Serial.println("Failed to read from sensor!"); 
     return; 
   } 
   Serial.print("Moisture: "); 
   Serial.print(output_value); 

  // clear the display
  display.clear();

  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Moistue:");
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, str);
  
  // write the buffer to the display
  display.display();

  // Put ESP8266 into Deep Sleep
ESP.deepSleep(sleepTimeS * 1000000);

  //repeat every 1min 
   delay(60000); 
}


