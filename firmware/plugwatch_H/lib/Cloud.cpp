#include <Particle.h>

#include "lib/Cloud.h"

bool Cloud::Publish(String tag, String message) {
  bool success;
  success = Particle.publish(tag, message, PRIVATE);
  return success;
}
