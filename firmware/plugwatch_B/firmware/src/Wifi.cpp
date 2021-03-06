#include <Particle.h>

#include "Cloud.h"
#include "Wifi.h"
#include "FileLog.h"

void Wifi::construct_ssid_list() {
  size_t place = 0;
  Serial.println();
  while (place < response->length() and place >= 0) {
    size_t first = response->indexOf('"', place);
    size_t second = response->indexOf('"', first+1);
    String ssid = response->substring(first+1, second);
    ssid_set.insert(ssid);
    place = response->indexOf("CWLAP", second);
  }
}

void Wifi::send(bool force) {
    String message = String(ssid_set.size());
    for (auto i = ssid_set.begin(); i != ssid_set.end(); ++i) {
      message += "|";
      message += *i;
    }
    if (force) {
      message = "FORCE|" + message;
    }
    log.append(message);
    Cloud::Publish(WIFI_SCAN_EVENT, message);
}

void Wifi::periodic(bool force) {
  force = force;
  ssid_set.clear();
  log.append("WIFI| Began scan!");
  scan_start_time = millis();
  esp8266.beginScan();
}

int Wifi::cloudCommand(String command) {
  return super::cloudCommand(command);
}

void Wifi::loop() {
  if (*done) {
    *done = false;
    // construct list, send/log success
    construct_ssid_list();
    log.append("Wifi Scan! Count: " + String(ssid_set.size()));
    send(force);
  }
  super::loop();
}
