#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY_PIN 7
#define BUZZER_PIN 6
#define LED_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);

// Replace these hex bytes with your authorized RFID Tag's UID
byte authorizedUID[4] = {0xDE, 0xAD, 0xBE, 0xEF};

bool ignitionState = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Default state: Relay OFF (High-level or Low-level trigger depend on relay module)
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("RFID Bike Ignition System Ready.");
}

void loop() {
  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Check if scanned card matches the authorized UID
  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
    ignitionState = !ignitionState; // Toggle ignition ON/OFF
    
    if (ignitionState) {
      Serial.println("Access Granted: Ignition ON");
      digitalWrite(RELAY_PIN, LOW);  // Turn relay ON (Active LOW relays)
      digitalWrite(LED_PIN, HIGH);
      beepSuccess();
    } else {
      Serial.println("Access Granted: Ignition OFF");
      digitalWrite(RELAY_PIN, HIGH); // Turn relay OFF
      digitalWrite(LED_PIN, LOW);
      beepOff();
    }
  } else {
    Serial.println("Access Denied: Unauthorized Key");
    beepDenied();
  }

  // Halt PICC and stop encryption on PCD
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1500); // Debounce delay between scans
}

// Function to compare card UID with authorized UID
bool isAuthorized(byte *scannedUID, byte size) {
  if (size != 4) return false;
  for (byte i = 0; i < size; i++) {
    if (scannedUID[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

// Feedback Audio Patterns
void beepSuccess() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepOff() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepDenied() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
  }
}
