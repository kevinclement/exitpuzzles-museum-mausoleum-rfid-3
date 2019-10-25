#pragma once

#include "rfid.h"

class Logic {
public:
  Logic();
  Rfid rfid;

  void setup();
  void handle();
  void solved();
  void unsolvable();
  void status();

private:
  bool _unsolvable = false;
};

