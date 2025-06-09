#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 98;

const int photoSensorPin = A2;
const int radioPin = A1;
const int magnetPin = A0;
const int leftMotorPin = 8;
const int leftMotorDirPin = 9;
const int rightMotorPin = 11;
const int rightMotorDirPin = 12;

// Motor speed constants
const int FULL_SPEED = 255;
const int DIAGONAL_SPEED = 50; // Slightly slower for diagonal movement
const int TURN_SPEED = 0;      // Slower for turning

// Ultrasonic var
char charIn;
String name = "";

WiFiWebServer server(80);

void setMotorSpeed(int leftSpeed, int rightSpeed, bool leftForward, bool rightForward)
{
  analogWrite(leftMotorPin, leftSpeed);
  analogWrite(rightMotorPin, rightSpeed);
  digitalWrite(leftMotorDirPin, leftForward ? HIGH : LOW);
  digitalWrite(rightMotorDirPin, rightForward ? HIGH : LOW);
}

void root()
{
  Serial.println(F("[INFO] Root accessed"));
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Connected"));
}

void moveForward()
{
  Serial.println(F("[INFO] Command: Move Forward"));
  setMotorSpeed(FULL_SPEED, FULL_SPEED, true, true);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Forward"));
}

void moveBack()
{
  Serial.println(F("[INFO] Command: Move Backward"));
  setMotorSpeed(FULL_SPEED, FULL_SPEED, false, false);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Backwards"));
}

void moveLeft()
{
  Serial.println(F("[INFO] Command: Move Left"));
  setMotorSpeed(TURN_SPEED, FULL_SPEED, true, true);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Left"));
}

void moveRight()
{
  Serial.println(F("[INFO] Command: Move Right"));
  setMotorSpeed(FULL_SPEED, TURN_SPEED, true, true);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Right"));
}

void moveForwardLeft()
{
  Serial.println(F("[INFO] Command: Move Forward-Left"));
  setMotorSpeed(DIAGONAL_SPEED, FULL_SPEED, true, true);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Forward-Left"));
}

void moveForwardRight()
{
  Serial.println(F("[INFO] Command: Move Forward-Right"));
  setMotorSpeed(FULL_SPEED, DIAGONAL_SPEED, true, true);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Forward-Right"));
}

void moveBackLeft()
{
  Serial.println(F("[INFO] Command: Move Back-Left"));
  setMotorSpeed(DIAGONAL_SPEED, FULL_SPEED, false, false);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Back-Left"));
}

void moveBackRight()
{
  Serial.println(F("[INFO] Command: Move Back-Right"));
  setMotorSpeed(FULL_SPEED, DIAGONAL_SPEED, false, false);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Moving Back-Right"));
}

void moveStop()
{
  Serial.println(F("[INFO] Command: Stop"));
  setMotorSpeed(0, 0, true, true); // Set speed to 0 but maintain direction
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), F("Stopping"));
}

void handleNotFound()
{
  String message = F("ERROR: File Not Found\n\n");
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

  unsigned long highTime = pulseIn(photoSensorPin, HIGH, 100000); // Timeout 50ms
  unsigned long lowTime = pulseIn(photoSensorPin, LOW, 100000);   // Timeout 50ms
  Serial.println(highTime);
  Serial.println(lowTime);

  if (highTime == 0 || lowTime == 0)
  {
    Serial.println(F("[WARN] IR sensor pulse timeout or no signal"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, F("application/json"), F("{\"status\":\"error\",\"message\":\"No IR signal detected or timeout\"}"));
    return;
  }

  unsigned long period = highTime + lowTime;
  if (period > 0)
  {
    float frequency = 1000000.0 / period;
    Serial.print(F("[INFO] Frequency measured: "));
    Serial.println(frequency);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), String(frequency) + "Hz");
  }
  else
  {
    Serial.println(F("[ERROR] Invalid IR pulse period"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, F("text/plain"), F("ERROR: Invalid pulse timing"));
  }
}

void handleRadio()
{
  Serial.println(F("[INFO] Handling radio sensor request"));

  unsigned long highTime = pulseIn(radioPin, HIGH, 50000); // Timeout 50ms
  unsigned long lowTime = pulseIn(radioPin, LOW, 50000);   // Timeout 50ms
  Serial.println(highTime);
  Serial.println(lowTime);

  if (highTime == 0 || lowTime == 0)
  {
    Serial.println(F("[WARN] Radio sensor pulse timeout or no signal"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, F("text/plain"), F("ERROR: No radio signal detected or timeout"));
    return;
  }

  unsigned long period = highTime + lowTime;
  if (period > 0)
  {
    float frequency = 1000000.0 / period;
    Serial.print(F("[INFO] Frequency measured: "));
    Serial.println(frequency);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), String(frequency) + "Hz");
  }
  else
  {
    Serial.println(F("[ERROR] Invalid radio pulse period"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, F("text/plain"), F("ERROR: Invalid pulse timing"));
  }
}

void handleMagnetic()
{
  Serial.println(F("[INFO] Handling magnetic sensor request"));

  int rawValue = analogRead(magnetPin);
  float voltage = rawValue * (3.3 / 1023.0); // Convert to volts

  Serial.print(F("[INFO] Voltage measured: "));
  Serial.println(voltage);

  // DRV5053 Output Interpretation:
  if (voltage >= 2.05)
  { // Threshold for South pole (DOWN)
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), "SOUTH");
    Serial.println("[INFO] DOWN (South pole into surface) → Gribbit/Zapple");
  }
  else if (voltage <= 1.98)
  { // Threshold for North pole (UP)
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, F("text/plain"), "NORTH");
    Serial.println("[INFO] UP (North pole out of ground) → Wibbo/Snorkle");
  }
  else
  {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    Serial.println("[ERROR] No magnet detected");
    server.send(400, F("text/plain"), F("ERROR: Invalid magnet reading"));
  }
}

void handleUltra()
{
  Serial.println(F("[INFO] Command: Ultrasonic"));
  name = ""; // Clear any previous data

  while (Serial1.available())
    Serial1.read(); // Clear out any previous data

  unsigned long startTime = millis();

  // Wait for start character '#'
  bool foundStart = false;
  while (!foundStart && (millis() - startTime < 1000))
  {
    if (Serial1.available())
    {
      charIn = Serial1.read();
      Serial.print("[INFO] Checking char: ");
      Serial.println(charIn);
      if (charIn == '#')
      {
        name += charIn;
        foundStart = true;
        Serial.println(F("[INFO] Start char '#' found"));
      }
    }
    delay(10); // Small delay to prevent overwhelming the loop
  }

  if (!foundStart)
  {
    Serial.println(F("[ERROR] No start char received for ultrasonic"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, F("text/plain"), F("ERROR: Ultrasonic start signal not detected"));
    return;
  }

  // Read the remaining 3 characters
  while (name.length() < 4)
  {
    if (Serial1.available())
    {
      charIn = Serial1.read();
      name += charIn;
      Serial.print("[INFO] Char received: ");
      Serial.println(charIn);
    }
    delay(10); // Small delay for 600 baud
  }

  Serial.println("[INFO] Complete name: " + name);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, F("text/plain"), "Name: " + name);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial && millis() < 10000)
    ;
  Serial1.begin(600);

  Serial.println(F("\n[INFO] Starting Web Server Setup"));

  pinMode(photoSensorPin, INPUT);
  pinMode(leftMotorPin, OUTPUT);
  pinMode(rightMotorPin, OUTPUT);
  pinMode(radioPin, INPUT);
  pinMode(leftMotorDirPin, OUTPUT);
  pinMode(rightMotorDirPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize motors to stopped state
  setMotorSpeed(0, 0, true, true);
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
  server.on(F("/forward-left"), moveForwardLeft);
  server.on(F("/forward-right"), moveForwardRight);
  server.on(F("/back-left"), moveBackLeft);
  server.on(F("/back-right"), moveBackRight);
  server.on(F("/stop"), moveStop);
  server.on(F("/IR"), handlePhotoSensor);
  server.on(F("/radio"), handleRadio);
  server.on(F("/ultra"), handleUltra);
  server.on(F("/magnet"), handleMagnetic);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println(F("[INFO] HTTP server started"));
}

void loop()
{
  server.handleClient();
}
