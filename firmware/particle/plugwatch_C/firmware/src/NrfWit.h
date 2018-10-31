#pragma once

#include <Particle.h>

#include "Subsystem.h"

class NrfWit: public Subsystem {
  typedef Subsystem super;

  String result;

public:
  void reset();
  void setup();
  LoopStatus loop();
  String getResult();
};