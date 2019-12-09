#include <Particle.h>

#include "lib/Subsystem.h"

void Subsystem::setup() {
}

LoopStatus Subsystem::loop() {
  return FinishedError;
}

String Subsystem::getResult() {
  return "ERR";
}
