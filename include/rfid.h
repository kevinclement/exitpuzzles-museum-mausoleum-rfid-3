#pragma once

#include "Arduino.h"
#include <MFRC522.h>
#include <PN532.h>

class Logic;

#define NR_OF_READERS   2
enum RFID_STATE {INCORRECT, CORRECT, MISSING, UNKNOWN};

class Rfid {
  public:
    Rfid(Logic &logic);
    void setup();
    void handle();
    RFID_STATE state[NR_OF_READERS] = { UNKNOWN, UNKNOWN };

  private:
    Logic &_logic;
    bool compareTags(uint8_t index);
    RFID_STATE checkForTagMFR(uint8_t index, MFRC522 mfr);
    RFID_STATE checkForTagHSU(uint8_t index, PN532 nfc);
    String prettyState(uint8_t);

    bool tag_present_prev[NR_OF_READERS] = { false, false };
    bool tag_present[NR_OF_READERS] = { false, false };
    int error_counter[NR_OF_READERS] = { 0, 0 };
    bool tag_found[NR_OF_READERS] = { false, false };
    byte readCards[NR_OF_READERS][4];
};