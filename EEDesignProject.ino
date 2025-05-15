/**************************************************************************************************************************************
  Based on AdvancedWebServer.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc

  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Forked and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Forked and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license

  Copyright (c) 2015, Majenko Technologies
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of Majenko Technologies nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************************************************************************/
#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 14; // Set your group number to make the IP address constant - only do this on the EEERover network

const int photoSensorPin = A0;
const int leftMotorPin = 0;
const int leftMotorDirPin = 1;
const int rightMotorPin = 11;
const int rightMotorDirPin = 12;
const int motorSpeed = 128; // PWM value (0-255)

WiFiWebServer server(80);

void root()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Connected"));
}

void moveForward()
{
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Forward"));
}

void moveBack()
{
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, LOW);
  digitalWrite(rightMotorDirPin, LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Backwards"));
}

void moveLeft()
{
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Left"));
}

void moveRight()
{
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, LOW);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Right"));
}

void moveStop()
{
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Stopping"));
}

// Generate a 404 response with details of the failed request
void handleNotFound()
{
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, F("text/plain"), message);
}


// Global variables for frequency tracking
unsigned long lastEdgeTime = 0;
unsigned long lastPeriod = 0;
float frequency = 0;
bool lastState = LOW;
bool newCycle = false;

void handlePhotoSensor() {
  bool currentState = digitalRead(photoSensorPin);
  unsigned long now = micros();

  if (currentState != lastState) {
    // Detected an edge (rising or falling)
    if (newCycle) {
      lastPeriod = now - lastEdgeTime;
      frequency = 1000000.0 / lastPeriod;
      newCycle = false;
    } else {
      newCycle = true;
    }
    lastEdgeTime = now;
    lastState = currentState;
  }

  // Optional: Clear frequency if no new edge seen in a while
  if (now - lastEdgeTime > 500000) { // 0.5 seconds of silence
    frequency = 0;
  }
}


void setup()
{
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(photoSensorPin, INPUT);
  pinMode(leftMotorPin, OUTPUT);
  pinMode(rightMotorPin, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  Serial.begin(9600);

  // Wait 10s for the serial connection before proceeding
  // This ensures you can see messages from startup() on the monitor
  // Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000)
    ;

  Serial.println(F("\nStarting Web Server"));

  // Check WiFi shield is present
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("WiFi shield not present"));
    while (true)
      ;
  }

  // Configure the static IP address if group number is set
  if (groupNumber)
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  // attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  // Register the callbacks to respond to HTTP requests
  server.on(F("/"), root);
  server.on(F("/forward"), moveForward);
  server.on(F("/back"), moveBack);
  server.on(F("/left"), moveLeft);
  server.on(F("/right"), moveRight);
  server.on(F("/stop"), moveStop);

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.print(F("HTTP server started @ "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));
}

// Call the server polling function in the main loop
void loop()
{
  handlePhotoSensor();
  Serial.println(frequency);
  server.handleClient();
}