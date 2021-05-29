#include "Arduino.h"
#include "rfid.h"
#include "logic.h"
#include <SPI.h>
#include <PN532_HSU.h>
#include <PN532.h>

#define OFFSET 4

PN532_HSU pn532hsu(Serial1);
PN532 nfc(pn532hsu);
MFRC522 mfr(23, 21);

byte tags [2][2][4] = {
  {
    { 0xF7, 0x6F, 0x8C, 0xF2 },
    { 0x87, 0x93, 0x8E, 0xF2 }
  },
  {
    { 0x27, 0x8F, 0x8E, 0xF2 },
    { 0xF7, 0x92, 0x8E, 0xF2 }
  }
};

Rfid::Rfid(Logic &logic)
: _logic(logic)
{
}

void Rfid::setup() {
  SPI.begin();

  mfr.PCD_Init();
  nfc.begin();

  delay(4);
  Serial.println();
  mfr.PCD_DumpVersionToSerial();

  // NFC device
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  Serial.print("Firmware Version (PN5"); Serial.print((versiondata>>24) & 0xFF, HEX); 
  Serial.print("): "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("\nReady to Scan...");
}

void Rfid::handle() {
  for (uint8_t i = 0; i < NR_OF_READERS; i++) {

    RFID_STATE st = UNKNOWN;
    if (i == 0) {
      st = checkForTagHSU(i, nfc);
    } else {
      st = checkForTagMFR(i, mfr);
    }

    if (st != state[i]) {
      Serial.print("state changed for ");
      Serial.print(i + OFFSET);
      Serial.print(" ");
      Serial.print(prettyState(state[i]));
      Serial.print(" => ");
      Serial.println(prettyState(st));

      state[i] = st;
      _logic.status();
    }
  }
}

RFID_STATE Rfid::checkForTagMFR(uint8_t index, MFRC522 mfr) {
  RFID_STATE st = state[index];
  tag_present_prev[index] = tag_present[index];

  error_counter[index] += 1;
  if(error_counter[index] > 2){
    tag_found[index] = false;
  }
  
  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfr.PCD_WriteRegister(mfr.TxModeReg, 0x00);
  mfr.PCD_WriteRegister(mfr.RxModeReg, 0x00);

  // Reset ModWidthReg
  mfr.PCD_WriteRegister(mfr.ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfr.PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfr.STATUS_OK){
    if ( ! mfr.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
      return st;
    }
    error_counter[index] = 0;
    tag_found[index] = true;

    for ( uint8_t i = 0; i < 4; i++) {
       readCards[index][i] = mfr.uid.uidByte[i];
    }
  }

  tag_present[index] = tag_found[index];

  // rising edge
  if (tag_present[index] && !tag_present_prev[index]){
    Serial.println("Tag found, checking...");
    st = compareTags(index) ? CORRECT : INCORRECT;
  }

  // falling edge
  if (!tag_present[index] && tag_present_prev[index]){
    Serial.println("Tag gone");
    st = MISSING;
  }

  return st;
}

RFID_STATE Rfid::checkForTagHSU(uint8_t index, PN532 nfc) {
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)  
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)) {
    for ( uint8_t i = 0; i < 4; i++) {
      readCards[index][i] = uid[i];
    }
    return compareTags(index) ? CORRECT : INCORRECT;
  } else {
    return MISSING;
  }
}

bool Rfid::compareTags(uint8_t index) {
  for ( uint8_t i = 0; i < 2; i++ ) {
    bool cardMatch = true;
    for ( uint8_t j = 0; j < 4; j++ ) {
        cardMatch = cardMatch && (readCards[index][j] == tags[index][i][j]);
    }

    if (cardMatch) {
      return true;
    }
  }

  return false;
}

String Rfid::prettyState(uint8_t state) {
  return 
    state == INCORRECT ? "Incorrect" : 
    state == CORRECT ? "Correct" : 
    state == MISSING ? "Missing" : 
    "Unknown";
}