#pragma once

#include "Arduino.h"
#include "rfid.h"
#include "SerialManager.h"

class Logic {
public:
  Logic();
  SerialManager serial;
  Rfid rfid;

  void setup();
  void handle();
  void status();

private:
};

