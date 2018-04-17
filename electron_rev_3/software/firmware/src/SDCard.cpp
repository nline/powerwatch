#include <SdFat.h>

#include "Cloud.h"
#include "SDCard.h"

void SDCard::setup() {
  super::setup();

  pinMode(SD_ENABLE_PIN, OUTPUT);
  digitalWrite(SD_ENABLE_PIN, LOW);

  pinMode(SD_INT_PIN, INPUT);
}

void SDCard::loop() {
  super::loop();

  //TODO test
  if (digitalRead(SD_INT_PIN) == LOW) {
    log.debug("SD removed");
    Cloud::Publish(SD_ERROR_EVENT, "removed");
    removed_flag = true; //only publish 1 event
  } else {
    removed_flag = false;
  }

  if (power_cycle_flag) {
    power_cycle_flag = false;
    PowerCycle();

    // Power cycling is slow, so don't do anything else this loop, let others go
    return;
  }

  if (read_filename != "") {
    log.append("Read " + read_filename);
    String sd_res = Read(read_filename);
    read_filename = "";
  }

  if (query_filename != "") {
    log.append("Stat " + query_filename);
    String sd_res = Stat(query_filename);
    query_filename = "";
    Cloud::Publish(SD_QUERY_EVENT,sd_res);
  }

  if (delete_filename != "") {
    log.append("Delete " + delete_filename);
    bool sd_res = Delete(delete_filename);
    query_filename = "";
    Cloud::Publish(SD_DELETE_EVENT, String(sd_res));
  }
}

int SDCard::cloudCommand(String command) {
  if ((command == "cycle") || (command == "reboot")) {
    power_cycle_flag = true;
    return 0;
  }
  if (command.startsWith("read ")) {
    read_filename = command.substring(5); //TODO test
    return 0;
  }
  if (command.startsWith("?")) {
    query_filename = command.substring(5);
    return 0;
  }
  if (command.startsWith("DELETE")) {
    delete_filename = command.substring(5);
    return 0;
  }
  return -1;
}

//TODO test
bool SDCard::Delete(String filename) {
  bool deleted = sd.remove(filename);
  if (!deleted) {
    Cloud::Publish(SD_ERROR_EVENT, String(filename) + String(" delete"));
  } else {
    Cloud::Publish(SD_DELETE_EVENT, String(filename) + String(" delete success"));
  }
  return deleted;
}

//TODO think about the meta data we want
//file size
//last written
//TODO test this
String SDCard::Stat(String filename) {
  log.debug("query begin: " + filename);
  String res = "";
  if (filename == "SD") {
    uint32_t volFree = sd.vol()->freeClusterCount();
    float fs = 0.000512*volFree*sd.vol()->blocksPerCluster();
    res = res + "|" + String(volFree) + "|" + String(fs);
  } else {
    File myFile;
    if (!myFile.open(filename, O_READ)) {
      log.debug(String("opening ") + String(filename) + String(" for query failed"));
      Cloud::Publish(SD_ERROR_EVENT, String(filename) + String(" query"));
      return "string err";
    }
    dir_t d;
    if (!myFile.dirEntry(&d)) {
      log.debug(String("opening ") + String(filename) + String(" for dir entry failed"));
      Cloud::Publish(SD_ERROR_EVENT, "dir entry failed");
    }
    res = res + String("|") + String(myFile.fileSize());
    res = res + String("|") + String(d.creationDate) + String(":") + String(d.creationTime);
    res = res + String("|") + String(d.lastWriteDate) + String(":") + String(d.lastWriteTime);
    res = res + String("|") + String(myFile.fileSize());
    res += "\n";
    myFile.close();
  }
  return res;
}


void SDCard::PowerCycle() {
  log.debug("power cycle SD");
	digitalWrite(SD_ENABLE_PIN, HIGH);
	delay(1000);
	digitalWrite(SD_ENABLE_PIN, LOW);
	delay(1000);
  Cloud::Publish(SD_REBOOT_EVENT, String(" sd rebooted"));
}

void SDCard::Write(String filename, String to_write) {
  log.debug("write begin: " + filename);
	if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) { //TODO think about speed
		log.debug("CAN'T OPEN SD");
		Cloud::Publish(SD_ERROR_EVENT, "init");
    return;
	}
	File file_to_write;
	String time_str = String(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
	String final_to_write = time_str + String("|") + to_write;
	if (!file_to_write.open(filename, O_WRITE | O_CREAT | O_APPEND)) {
		log.debug(String("opening ") + String(filename) + String(" for write failed"));
		Cloud::Publish(SD_ERROR_EVENT, String(filename) + String(" write"));
		return;
	}
	file_to_write.println(final_to_write);
	file_to_write.close();
	log.debug(String("wrote : ") + String(filename) + String(":") + to_write);
}

String SDCard::Read(String filename) {
  log.debug("read begin: " + filename);
	File myFile;
	if (!myFile.open(filename, O_READ)) {
		//sd.errorHalt(String("opening ") + String(filename) + String(" for read failed"));
		log.debug(String("opening ") + String(filename) + String(" for read failed"));
		Cloud::Publish(SD_ERROR_EVENT, String(filename) + String(" read"));
		return "string err";
	}
	log.debug(String(filename) + String(" content:"));
	String res = "";
	res += String(myFile.fileSize());
	res += "\n";
	while (myFile.available()) {
		char cur = myFile.read();
		res += String(cur);
		Serial.write(cur);
	}
	myFile.close();

  sendDataOverTCP(res);
	return res;
}

bool SDCard::sendDataOverTCP(String data) {

}
