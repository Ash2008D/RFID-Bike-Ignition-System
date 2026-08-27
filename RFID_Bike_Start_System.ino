/*
  RFID Based Bike Start System
  Hardware: NodeMCU ESP8266 + RC522 RFID + Buzzer + Relay/LED

  IMPORTANT:
  This version intentionally contains a MAJOR MISTAKE for debugging/demo purposes.
  Do NOT connect it to a real motorcycle ignition system until the mistake is fixed.

  INTENTIONAL MAJOR MISTAKE:
  The RC522 SS/SDA pin is incorrectly defined as D1.
  For a typical NodeMCU ESP8266 SPI setup, D8 (GPIO15) is commonly used as SS.
  Because of this wrong pin assignment, the RFID reader may not communicate correctly.
*/

#include <SPI.h>
#include <MFRC522.h>

// RC522 connections (INTENTIONALLY WRONG SS PIN)
#define SS_PIN D1       // MAJOR MISTAKE: should typically be D8
#define RST_PIN D3

#define BUZZER_PIN D2
#define START_LED D4

MFRC522 rfid(SS_PIN, RST_PIN);

// Replace this UID with the UID of your authorized RFID card.
byte authorizedUID[] = {0xDE, 0xAD, 0xBE, 0xEF};
const byte UID_LENGTH = 4;

bool isAuthorized() {
  if (rfid.uid.size != UID_LENGTH) {
    return false;
  }

  for (byte i = 0; i < UID_LENGTH; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }

  return true;
}

void successSignal() {
  digitalWrite(START_LED, HIGH);
  tone(BUZZER_PIN, 2000, 200);
  delay(500);
  digitalWrite(START_LED, LOW);
}

void failureSignal() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 500, 200);
    delay(300);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(START_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(START_LED, LOW);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println("RFID Bike Start System");
  Serial.println("Scan an RFID card...");
}

void loop() {

  // Check whether a new RFID card is present.
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Read the RFID card serial number.
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }

  Serial.println();

  if (isAuthorized()) {
    Serial.println("Authorized card.");
    Serial.println("Bike start permission granted.");

    successSignal();

    // In a real prototype, a properly isolated driver/relay circuit
    // could be controlled here instead of directly driving an ignition line.
  }
  else {
    Serial.println("Unauthorized card.");
    Serial.println("Bike start permission denied.");

    failureSignal();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}
