#include "Arduino.h"
#include <HX711.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<ArduinoJson.h>



// ESP
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

 
const char* ssid = "R&R";
const char* password = "randrboiz";
String s= "";
//Your Domain name with URL path or IP address with path
const char* serverName = "http://smartshelf16.herokuapp.com/weight";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

// OLED Reset protocol

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306  display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
 
void calibrate();
 
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
 
 
HX711 scale;
 
void setup() {
  Serial.begin(115200);


  
  //OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();


  
  Serial.println("HX711 Demo");
 
  Serial.println("Initializing the scale");
 
  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
 


  calibrate();
 
  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC
 
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC
 
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()
 
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale
 
  Serial.println("Readings:");
  
// ESP
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  
}
 
void loop() {

  
  /*
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);
*/
int indi = (scale.get_units()/490); // Number of individual product 

  display.clearDisplay();

  display.setTextSize(2.2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Quantity: ");
  display.setTextSize(3);
  display.setCursor(0,20);
  display.println(indi, 1);
  display.setTextSize(2);
  display.print("Nos");
  display.display();

 
 float x =scale.get_units();
 
 
//  scale.power_down();         // put the ADC in sleep mode
  delay(2000);
//  scale.power_up();
 
 
 
 
 
 
 if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      // Data to send with HTTP POST


StaticJsonDocument<200> doc;

doc["weight"]   = x ;


serializeJson(doc, s);
      int httpResponseCode = http.POST(s);
      
     Serial.println(s);
     doc.clear();
     s="";
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
 
void calibrate() {
  // Remove any calibration values and clear the scale
  scale.set_scale();   // Start scale
  scale.tare();        // Reset scale to zero
 
  // Prompt the user
  Serial.println("Add your known weight to the scale, enter the weight and press <Enter>");
  int userInput = -123;
  String inputString = "";
  // Loop until we receive an input (which will change the value of userInput
  while (userInput == -123) {
    // Read serial input:
    while (Serial.available() > 0) {
      int inputChar = Serial.read();
      if (isDigit(inputChar)) {
        // convert the incoming byte to a char and add it to the string:
        inputString += (char)inputChar;
      }
      // if you get a newline, print the string, then the string's value:
      if (inputChar == '\n') {
        userInput = inputString.toInt();
      }
   
    }
  }
 
  // Now get the reading from the scale
  float calReading = scale.get_units(10);
 
  Serial.print("Setting the cal to ");
  Serial.println(calReading / userInput);
 
  scale.set_scale(calReading / userInput);





// ESP
//Send an HTTP POST request every 5 seconds
 
}
 
  
