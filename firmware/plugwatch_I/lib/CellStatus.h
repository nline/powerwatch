#pragma once

#include <Particle.h>

#include "lib/Subsystem.h"
#include "lib/FileLog.h"

class CellStatus: public Subsystem {
  typedef Subsystem super;

  String result;
public:
  LoopStatus loop();
  String getResult();
};
