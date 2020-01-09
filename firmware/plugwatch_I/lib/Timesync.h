#pragma once

#include <Particle.h>
#include "lib/Subsystem.h"
#include "lib/AB1815.h"

//***********************************
//* Time Sync
//*
//* Particle synchronizes its clock when it first connects. Over time, it will
//* drift away from real time. This routine will re-sync local time.
//***********************************

class Timesync: public Subsystem {
  typedef Subsystem super;

  const int TWELVE_HOURS = 1000 * 60 * 60 * 12;

  AB1815 rtc;

  enum TimesyncState { 
    unsynced, 
    synced, 
    syncingParticle ,
    sendNTP,
    waitNTP,
    updateRTC,
    updateFromRTC,
  };

  TimesyncState timesyncState = unsynced;
  unsigned long sync_start_time;
  UDP udp;

public:
  void setup();
  LoopStatus loop();
};
