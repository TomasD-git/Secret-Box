// Secret box

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <ESP32Servo.h>

#define PIN_SDA      8  
#define PIN_SCL      9 
#define PIN_SERVO1   4   
#define PIN_SERVO2   5  
#define PIN_MOSFET1  6  
#define PIN_MOSFET2  7   

#define SERVO1_HOME    90   
#define SERVO1_OPEN   180 
#define SERVO2_HOME    90   
#define SERVO2_OPEN     0    

#define POWER_SETTLE_MS   60    
#define SERVO_TRAVEL_MS  700    
#define DETACH_DELAY_MS   30   
#define SCAN_COOLDOWN_MS 2500   

uint8_t authorizedUID[]  = { 0x00, 0x00, 0x00, 0x00 };
uint8_t authorizedUIDLen = 4;   

Adafruit_PN532 nfc(-1, -1);   
Servo servo1;
Servo servo2;

bool isUnlocked        = false;
unsigned long lastScan = 0;

bool uidMatch(uint8_t *uid, uint8_t len) {
  if (len != authorizedUIDLen) return false;
  for (uint8_t i = 0; i < len; i++)
    if (uid[i] != authorizedUID[i]) return false;
  return true;
}
void setServoPower(bool on) {
  digitalWrite(PIN_MOSFET1, on ? HIGH : LOW);
  digitalWrite(PIN_MOSFET2, on ? HIGH : LOW);
  if (on) delay(POWER_SETTLE_MS);   
}
void moveServos(int angle1, int angle2) {
  setServoPower(true);
  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);
  servo1.write(angle1);
  servo2.write(angle2);
  delay(SERVO_TRAVEL_MS);         
  servo1.detach();
  servo2.detach();
  delay(DETACH_DELAY_MS);
  setServoPower(false);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(PIN_MOSFET1, OUTPUT);
  pinMode(PIN_MOSFET2, OUTPUT);
  digitalWrite(PIN_MOSFET1, LOW);
  digitalWrite(PIN_MOSFET2, LOW);
  Wire.begin(PIN_SDA, PIN_SCL);
  nfc.begin();
  uint32_t ver = nfc.getFirmwareVersion();
  Serial.printf("[OK]    PN532 found — FW: %d.%d\n",
                (ver >> 16) & 0xFF, (ver >> 8) & 0xFF);
  nfc.SAMConfig();   
  Serial.print("[CFG]   Authorized UID =");
  for (uint8_t i = 0; i < authorizedUIDLen; i++)
    Serial.printf(" 0x%02X", authorizedUID[i]);
  Serial.println();
  bool allZero = true;
  for (uint8_t i = 0; i < authorizedUIDLen; i++)
    if (authorizedUID[i] != 0) { allZero = false; break; }
  if (allZero) {
    Serial.println("[WARN]  authorizedUID is all zeros — tap your card to see its UID,");
    Serial.println("        then paste the bytes into the sketch and re-upload.");
  }
}

void loop() {
  if (millis() - lastScan < SCAN_COOLDOWN_MS) return;
  uint8_t uid[7];
  uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(
      PN532_MIFARE_ISO14443A, uid, &uidLen, 300);
  if (!found) return;
  Serial.print("[SCAN]  UID:");
  for (uint8_t i = 0; i < uidLen; i++) Serial.printf(" 0x%02X", uid[i]);
  Serial.println();
  lastScan = millis();
  if (!uidMatch(uid, uidLen)) {
    Serial.println("[DENY]  Card not authorised.\n");
    return;
  }
  if (!isUnlocked) {
    Serial.println("[AUTH]  Unlocking — Servo1 to 180 deg, Servo2 to 0 deg");
    moveServos(SERVO1_OPEN, SERVO2_OPEN);
    isUnlocked = true;
    Serial.println("[DONE]  Unlocked. Servo power OFF.\n");
  } else {
    Serial.println("[AUTH]  Locking — Servo1 to 90 deg, Servo2 to 90 deg");
    moveServos(SERVO1_HOME, SERVO2_HOME);
    isUnlocked = false;
    Serial.println("[DONE]  Locked. Servo power OFF.\n");
  }
}
