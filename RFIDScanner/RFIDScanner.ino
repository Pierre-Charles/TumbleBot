#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 21
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

void setup() {
  Serial.begin(115200);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Show UID on serial monitor
  Serial.print("UID: ");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  if (content.substring(1) == "82 BB B9 67") {
    Serial.println("Hello Pierre!");
    delay(3000);
  } else if (content.substring(1) == "12 1A AD 67") {
    Serial.println("Hello Nicky");
    delay(3000);
  } else if (content.substring(1) == "D2 B1 BE 67") {
    Serial.println("Hello, Saif");
    delay(3000);
  } else if (content.substring(1) == "49 93 05 4F") {
    Serial.println("Hello, Pierre!");
    delay(3000);
  }
}
