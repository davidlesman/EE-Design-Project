#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 14;

const int photoSensorPin = A0;
const int radioPin = A1;
const int leftMotorPin = 0;
const int leftMotorDirPin = 1;
const int rightMotorPin = 11;
const int rightMotorDirPin = 12;
const int motorSpeed = 128;

WiFiWebServer server(80);

void root()
{
  Serial.println(F("[INFO] Root accessed"));
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Connected"));
}

void moveForward()
{
  Serial.println(F("[INFO] Command: Move Forward"));
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Forward"));
}

void moveBack()
{
  Serial.println(F("[INFO] Command: Move Backward"));
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, LOW);
  digitalWrite(rightMotorDirPin, LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Backwards"));
}

void moveLeft()
{
  Serial.println(F("[INFO] Command: Move Left"));
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, HIGH);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Left"));
}

void moveRight()
{
  Serial.println(F("[INFO] Command: Move Right"));
  digitalWrite(leftMotorPin, HIGH);
  digitalWrite(rightMotorPin, LOW);
  digitalWrite(leftMotorDirPin, HIGH);
  digitalWrite(rightMotorDirPin, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Right"));
}

void moveStop()
{
  Serial.println(F("[INFO] Command: Stop"));
  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Stopping"));
}

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

  Serial.println(F("[WARN] 404 Not Found:"));
  Serial.println(message);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, F("text/plain"), message);
}

void handlePhotoSensor()
{
  Serial.println(F("[INFO] Handling IR sensor request"));

  unsigned long highTime = pulseIn(photoSensorPin, HIGH, 50000); // Timeout 50ms
  unsigned long lowTime = pulseIn(photoSensorPin, LOW, 50000);   // Timeout 50ms

  if (highTime == 0 || lowTime == 0)
  {
    Serial.println(F("[WARN] IR sensor pulse timeout or no signal"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), F("No signal detected or timeout."));
    return;
  }

  unsigned long period = highTime + lowTime;
  if (period > 0)
  {
    float frequency = 1000000.0 / period;
    Serial.print(F("[INFO] Frequency measured: "));
    Serial.println(frequency);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), String(frequency));
  }
  else
  {
    Serial.println(F("[ERROR] Invalid IR pulse period"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), F("Invalid pulse timing."));
  }
}

void handleRadio()
{
  Serial.println(F("[INFO] Reading AM envelope signal"));
  int total = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    total += analogRead(radioPin);
    delay(2); // Sample over ~200ms total
  }

  float avg = total / float(samples);
  Serial.print(F("[INFO] Average envelope: "));
  Serial.println(avg);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Average envelope value: " + String(avg));
}

void setup()
{
  Serial.begin(9600);
  while (!Serial && millis() < 10000)
    ;

  Serial.println(F("\n[INFO] Starting Web Server Setup"));

  pinMode(photoSensorPin, INPUT);
  pinMode(leftMotorPin, OUTPUT);
  pinMode(rightMotorPin, OUTPUT);
  pinMode(radioPin, INPUT);
  pinMode(leftMotorDirPin, OUTPUT);
  pinMode(rightMotorDirPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(leftMotorPin, LOW);
  digitalWrite(rightMotorPin, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("[ERROR] WiFi shield not present"));
    while (true)
      delay(1000); // Avoid hard crash, slow loop
  }

  if (groupNumber)
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  Serial.print(F("[INFO] Connecting to SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }

  Serial.println(F("\n[INFO] Connected to WiFi"));
  Serial.print(F("[INFO] IP Address: "));
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));

  server.on(F("/"), root);
  server.on(F("/forward"), moveForward);
  server.on(F("/back"), moveBack);
  server.on(F("/left"), moveLeft);
  server.on(F("/right"), moveRight);
  server.on(F("/stop"), moveStop);
  server.on(F("/IR"), handlePhotoSensor);
  server.onNotFound(handleNotFound);

  server.on(F("/radio"), handleRadio);
  server.begin();
  Serial.println(F("[INFO] HTTP server started"));
}

void loop()
{
  server.handleClient();
}
