#include "Arduino.h"
#include "logic.h"
#include "rfid.h"
#include "version.h"

bool _solved = false;
Logic::Logic() : 
  rfid(*this)
{
}

void Logic::setup() {
  rfid.setup();
}

void Logic::handle() {
  rfid.handle();

  if (rfid.solved && !_solved) {
    if (_unsolvable) {
      Serial.println("WARN: device was solved, but unsolvable flag is on, so ignoring.");
    } else {
      solved();
    }
  }
}

void Logic::solved() {
  Serial.println("*** ALL IDOLS IN PLACE ***");
  _solved = true;
  status();
}

void Logic::unsolvable() {
  _unsolvable = !_unsolvable;
  status();
}

void Logic::status() {
  char cMsg[254];
  sprintf(cMsg, 
    "status="
      "version:%s,"
      "gitDate:%s,"
      "buildDate:%s,"

      "solved:%s,"
      "unsolvable:%s,"
      "idol_4:%s,"
      "idol_5:%s"

      "\r\n"
    , GIT_HASH,
      GIT_DATE,
      DATE_NOW,

      _solved ? "true" : "false",
      _unsolvable ? "true" : "false",
      rfid.state[0] == CORRECT ? "true" : "false",
      rfid.state[1] == CORRECT ? "true" : "false"
  );

  Serial.print(cMsg);
}
