/*
   EGR Mitsubishi L200 as AP
*/

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D7

#ifndef APSSID
#define APSSID "EGR_Mitsu"
#define APPSK  "pass"
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
// FOR EGR
int OutPin = A0;
int OutValue = 128;

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

void setup() {
  delay(100);
  Serial.begin(115200);
  // FOR DALLAS
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  
  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
  
  // FOR EGR
  pinMode(D5, INPUT_PULLUP); //INPUT_PULLUP
  pinMode(D6, INPUT_PULLUP);
  analogWrite(OutPin, OutValue);
  delay(10);
  
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

// function to print the temperature for a device
float printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
  
  return(tempC);
}

int getOutValue() {
  if (!(digitalRead(D5)))
  {
    if (OutValue < 255) OutValue += 1;
  }
  if (!(digitalRead(D6)))
  {
    if (OutValue > 0) OutValue -= 1;
  }
  analogWrite(OutPin, OutValue);
//  digitalWrite(OutPin, OutValue);
  Serial.print("OutValue: ");
  Serial.println(OutValue);
  delay(50);  
  
  return(OutValue);
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.*/
void handleRoot() {
// webPage += "<p>Тёплый пол <a href=\"teplpolON\"><button>ON</button></a>&nbsp;<a href=\"teplpolOFF\"><button>OFF</button></a></p>" 
    server.send(200, "text/html", webPage());
//  server.send(200, "text/html", "<h1>You are connected to </h1>");
//  server.send(200, "text/html", "<h1>Mitsu L200 EGR</h1>");
  
//  server.send(200, "text/html", "<h3>EGR OutValue -> </h3>");  
//  String myStrVal = String(getOutValue());
//  server.send(200, "text/html", myStrVal);
//  
//  server.send(200, "text/html", "<h3>Dallas engine T'C -> </h3>");
//  String myStrTC = String(printTemperature(insideThermometer));
//  server.send(200, "text/html", myStrTC);
  
}


void loop() {

  // FOR DALLAS
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  // It responds almost immediately. Let's print out the data
//  printTemperature(insideThermometer); // Use a simple function to print out the data
//  getOutValue();
  
  
  server.handleClient();
  server.send(200, "text/html", webPage());  
  delay(100);
  
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

String webPage()
{
  String web;
  String myStrVal = String(getOutValue());
  String myStrTC = String(printTemperature(insideThermometer));
  web += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/> <meta charset=\"utf-8\"><title>ESP 8266</title><style>button{color:red;padding: 10px 27px;}</style></head>";
  
  web += "<h1 style=\"text-align: center;font-family: Open sans;font-weight: 100;font-size: 20px;\">EGR OutValue -> " + myStrVal;
  web +="</h1><div>";
  
  web += "<h1 style=\"text-align: center;font-family: Open sans;font-weight: 100;font-size: 20px;\">Dallas engine T'C -> " + myStrTC;
  web += "</h1><div>"; 

  // ========REFRESH=============
  web += "<div style=\"text-align:center;margin-top: 20px;\"><a href=\"/\"><button style=\"width:158px;\">REFRESH</button></a></div>";
  // ========REFRESH=============
  web += "</div>";
  return(web);
}
