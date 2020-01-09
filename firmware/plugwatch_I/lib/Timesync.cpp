#include "lib/Timesync.h"

void Timesync::setup() {
  rtc.init();
  timesyncState = unsynced;

  //it seems smart to aggressively rely on the RTC for time sync
  Serial.println("Syncing to RTC");
  uint32_t t = rtc.getTime();
  Serial.printlnf("Setting time to %lu from RTC", t);
  Time.setTime(t);
}

LoopStatus Timesync::loop() {
  switch(timesyncState) {
    case unsynced:
      if(Particle.connected()) {
        //sync time with particle cloud
        Serial.println("Particle connected, trying to sync");
        Particle.syncTime();
        sync_start_time = millis();
        state = syncingParticle;
      } else if (Cellular.ready()) {
        //try to get an update VIA NTP 
        sync_start_time = millis();
        state = sendNTP;
      } else {
        //we can periodically set particle time to RTC time
      }
    break;
    case syncingParticle:
      if(!Particle.syncTimePending() && Particle.timeSyncedLast() > sync_start_time) {
        state = updateRTC;
      } else if (millis() - sync_start_time > 60000) {
        //sync timed out, try to sync via NTP
        sync_start_time = millis();
        state = sendNTP;
      } else {
        //just let it try to sync
      }
    break;
    case sendNTP:
      //Try to time sync with NTP
      if(udp.begin(2390) != true) {
        //failed - update from RTC
        state = updateFromRTC;
      } else {
        uint8_t packet[48];
        packet[0] = 0x1B;
        sync_start_time = millis();

        if(udp.sendPacket(packet, 48, IPAddress(128,138,141,172), 123) < 0) {
          udp.stop();
          state = updateFromRTC;
        }

        state = waitNTP;
      }
    break;
    case waitNTP:
      if(udp.receivePacket(packet, 48) > 0) {
        Serial.printlnf("Received udp packet of size %d", size);
        udp.stop();

        unsigned long Receivedmillis = millis();
        if(packet[1] == 0) {
          Serial.println("Received kiss of death.");
          state = updateFromRTC;
        }
        unsigned long NTPtime = packet[40] << 24 | packet[41] << 16 | packet[42] << 8 | packet[43];
        unsigned long NTPfrac = packet[44] << 24 | packet[45] << 16 | packet[46] << 8 | packet[47];

        if(NTPtime == 0) {
          state = updateFromRTC;
        }

        unsigned long NTPmillis = (unsigned long)(((double)NTPfrac)  / 0xffffffff * 1000);
        NTPmillis += (Receivedmillis - Sentmillis)/2;
        if(NTPmillis >= 1000) {
          NTPmillis -= 1000;
          NTPtime += 1;
        }

        unsigned long t = NTPtime - 2208988800UL + 1;
        Serial.printlnf("Got time %lu from NTP", t);
        Time.setTime(t);
        state = updateRTC;
      } else if (millis() - sync_start_time > 20000) {
        udp.stop();
        state = updateFromRTC;
      } else  {
        //just wait
      }
    break;
    case updateRTC:
        rtc.setTime(Time.now());
        state = synced;
    break;
    case updateFromRTC:
      //set the time
      Serial.println("Syncing to RTC");
      uint32_t t = rtc.getTime();
      Serial.printlnf("Setting time to %lu from RTC", t);
      Time.setTime(t);
      state = synced;
    break;
    case synced:
      if ((millis() - Particle.timeSyncedLast()) > Timesync::TWELVE_HOURS) {
        state = unsynced;
      }
    break;
  }

  return FinishedSuccess;
}
