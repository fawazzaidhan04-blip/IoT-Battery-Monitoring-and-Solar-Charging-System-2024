#define BLYNK_TEMPLATE_ID "TMPL6gSyAqs85"
#define BLYNK_TEMPLATE_NAME "Battery Monitoring"
#define BLYNK_AUTH_TOKEN "JYruIr7gkqXRRRSLmMJ6dMnIEFfJIRKX"

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// Blynk credentials
char auth[] = "JYruIr7gkqXRRRSLmMJ6dMnIEFfJIRKX"; // Replace with your Blynk Auth Token
char ssid[] = "S23 FE";                      // Replace with your WiFi SSID
char pass[] = "12345678";                     // Replace with your WiFi Password

// INA219 sensor for battery monitoring
Adafruit_INA219 ina219;

// LCD setup
LiquidCrystal_I2C lcd(0x3f, 16, 2); // Pastikan alamat ini sesuai dengan modul Anda

// Servo motor setup
Servo servoHorizontal;
Servo servoVertical;

// LDR pins
const int ldrTopRight = 35;
const int ldrTopLeft = 34;
const int ldrBottomRight = 33;
const int ldrBottomLeft = 32;

// Servo positions
int posHorizontal = 90; // Start at middle position
int posVertical = 90;   // Start at middle position

// Battery specifications
const float BATTERY_MAX_VOLTAGE = 4.2;  // Tegangan penuh
const float BATTERY_MIN_VOLTAGE = 3.0;  // Tegangan habis
const int BATTERY_CAPACITY_MAH = 1500;  // Kapasitas mAh

// Function to map voltage to battery percentage
float mapVoltageToPercentage(float voltage) {
  if (voltage >= BATTERY_MAX_VOLTAGE) {
    return 100.0; // Baterai penuh
  } else if (voltage <= BATTERY_MIN_VOLTAGE) {
    return 0.0; // Baterai habis
  } else {
    // Mapping linear
    return (voltage - BATTERY_MIN_VOLTAGE) / 
           (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0;
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);

  // Initialize INA219 sensor
  if (!ina219.begin()) {
    Serial.println("INA219 not detected. Check wiring!");
    while (1) { delay(10); }
  } else {
    Serial.println("INA219 detected successfully.");
  }

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Attach servos to pins
  servoHorizontal.attach(18); // Horizontal servo pin
  servoVertical.attach(19);   // Vertical servo pin

  // Set initial servo positions
  servoHorizontal.write(posHorizontal);
  servoVertical.write(posVertical);

  // Print initial message on LCD
  lcd.setCursor(0, 0);
  lcd.print("  Solar Tracker");
  lcd.setCursor(0, 1);
  lcd.print(" Battery Monitor");
  delay(2000); // Show initial message for 2 seconds
}

void loop() {
  // Update Blynk
  Blynk.run();

  // Read LDR values
  int topRight = analogRead(ldrTopRight);
  int topLeft = analogRead(ldrTopLeft);
  int bottomRight = analogRead(ldrBottomRight);
  int bottomLeft = analogRead(ldrBottomLeft);

  // Calculate average light levels
  int avgLeft = (topLeft + bottomLeft) / 2;
  int avgRight = (topRight + bottomRight) / 2;
  int avgTop = (topLeft + topRight) / 2;
  int avgBottom = (bottomLeft + bottomRight) / 2;

  // Adjust horizontal servo position
  if (avgRight > avgLeft + 50) {
    posHorizontal = constrain(posHorizontal + 1, 0, 180);
  } else if (avgLeft > avgRight + 50) {
    posHorizontal = constrain(posHorizontal - 1, 0, 180);
  }

  // Adjust vertical servo position
  if (avgTop > avgBottom + 50) {
    posVertical = constrain(posVertical - 1, 0, 180);
  } else if (avgBottom > avgTop + 50) {
    posVertical = constrain(posVertical + 1, 0, 180);
  }

  // Move servos to new positions
  servoHorizontal.write(posHorizontal);
  servoVertical.write(posVertical);

  // Debugging servo positions
  Serial.print("Horizontal Servo: ");
  Serial.println(posHorizontal);
  Serial.print("Vertical Servo: ");
  Serial.println(posVertical);

  // Read battery data from INA219
  float busVoltage = ina219.getBusVoltage_V();    // Tegangan (V)
  float current_mA = ina219.getCurrent_mA();      // Arus (mA)
  float power_mW = ina219.getPower_mW();          // Daya (mW)

  // Calculate battery percentage
  float batteryPercentage = mapVoltageToPercentage(busVoltage);

  // Debugging output
  Serial.print("Bus Voltage: ");
  Serial.print(busVoltage);
  Serial.print(" V, Current: ");
  Serial.print(current_mA);
  Serial.print(" mA, Power: ");
  Serial.print(power_mW);
  Serial.print(" mW, Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println(" %");

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volt: ");
  lcd.print(busVoltage, 2);
  lcd.print("V");

  lcd.setCursor(0, 1);
  lcd.print("Batt: ");
  lcd.print(batteryPercentage, 0);
  lcd.print("%");

  // Send data to Blynk app
  Blynk.virtualWrite(V0, busVoltage);      // Voltage
  Blynk.virtualWrite(V1, current_mA);     // Current
  Blynk.virtualWrite(V2, power_mW);       // Power
  Blynk.virtualWrite(V3, batteryPercentage); // Battery percentage

  // Delay for stability
  delay(500);
}
