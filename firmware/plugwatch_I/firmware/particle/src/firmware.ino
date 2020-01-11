// Native
#include <Particle.h>

// Third party libraries
#include <APNHelperRK.h>
#include <CellularHelper.h>

// Our code
#include "board.h"
#include "lib/CellStatus.h"
#include "lib/PowerCheck.h"
#include "lib/AB1815.h"
#include "lib/ChargeState.h"
#include "lib/FileLog.h"
#include "lib/Gps.h"
#include "lib/Imu.h"
#include "lib/SDCard.h"
#include "lib/Timesync.h"
#include "lib/Serialnumber.h"
#include "lib/led.h"
#include "product_id.h"

//***********************************
//* Critical System Config
//***********************************
//Starting with versions greater than 26 this is a SEMANTIC version in two digit decimal
//The first two digits are major revision, we won't use those for now
//The second two digit group is minor revision. We are using this to indicate fundamental hardware changes
//The last revision indicates small changes
//By this scheme we currently have deployed 000026
//Plugwatch F is new firmware with a new data format and would be 000100
//This make s it easier to make minor changes to firmware of different versions

#ifdef PRODUCT
int product_id = PRODUCT;
PRODUCT_ID(PRODUCT);
#else
int product_id = 8379;
PRODUCT_ID(8379);
#endif

int version_int = 112; 
PRODUCT_VERSION(112);

SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* Watchdogs and reset functions
//***********************************
const int HARDWARE_WATCHDOG_TIMEOUT_MS = 100000;
ApplicationWatchdog wd(HARDWARE_WATCHDOG_TIMEOUT_MS, soft_reset);

int soft_reset_helper(String cmd) {
  soft_reset();
  return 0;
}

void soft_reset() {
  // Soft reset the particle using system sleep
  Serial.println("Resetting by sleeping temporarily");
  System.sleep(SLEEP_MODE_DEEP, 10);
}

int hard_reset(String cmd) {
  // Hard reset the particle using the reset chip
  pinMode(PARTICLE_RST, OUTPUT);
  digitalWrite(PARTICLE_RST, HIGH);
  return 0;
}

//***********************************
//* SD Card
//***********************************
SDCard SD;

//***********************************
//* Timesync
//***********************************
Timesync timeSync;
AB1815 rtc;

//***********************************
//* CellStatus
//***********************************
CellStatus cellStatus;

//***********************************
//* PowerCheck
//***********************************
PowerCheck powercheck;

//***********************************
//* Charge state
//***********************************
ChargeState chargeState;

//***********************************
//* IMU
//***********************************
Imu imu; 

//***********************************
//* GPS
//***********************************
Gps gps;

//***********************************
//* Serialnumber
//***********************************
Serialnumber serialNumber;

//***********************************
//* APNs
//***********************************
const APNHelperAPN apns[7] = {
  {"8901260", "wireless.twilio.com"},
  {"8923301", "http://mtnplay.com.gh"},
  {"8991101", "airtelgprs.com"},
  {"8958021", "gprsweb.digitel.ve"},
  {"8958021", "internet.digitel.ve"},
  {"8923400", "9mobile"},
  {"8918500", "iot-eu.aer.net"}
};
APNHelper apnHelper(apns, sizeof(apns)/sizeof(apns[0]));

//***********************************
//* System Data storage
//***********************************
// The SD queue is the queue for data t obe sent to the SD cardk
std::queue<String> SDQueue;

// The cloud queue is the queue for data to be send to the cloud
// or put on the data backlog
std::queue<String> CloudQueue;

//The Data log writes data to the SD card
auto DataLog = FileLog(SD, "data_log.txt");
//The data dequeue is the backlog for sending to the cloud
auto DataDequeue = FileLog(SD, "data_dequeue.txt");

void setup() {
  //setup the APN credentials
  apnHelper.setCredentials();

  //Register reset functions with the particle cloud
  Particle.function("hard_reset", hard_reset);
  Particle.function("soft_reset", soft_reset_helper);

  // Set up debugging UART
  Serial.begin(9600);
  Serial.println("Initial Setup.");

  // Set up I2C
  Wire.begin();

  // Setup SD card first so that other setups can log
  SD.setup();

  //setup the other subsystems
  chargeState.setup();
  imu.setup();
  FuelGauge().quickStart();

  //Setup the watchdog toggle pin
  pinMode(WDI, OUTPUT);

  gps.setup();

  //Setup the AC enable pin
  pinMode(AC_PWR_EN, OUTPUT);

  //Run initial timesync
  timeSync.setup();

  //Setup the particle keepalive
  Particle.keepAlive(23*60); // send a ping every 30 seconds

  Serial.println("Setup complete.");
}

//This structure is what all of the drivers will return. It will
//be packetized and send to the cloud in the sendPacket state
#define RESULT_LEN 100
struct ResultStruct {
  char chargeStateResult[RESULT_LEN];
  char mpuResult[RESULT_LEN];
  char wifiResult[RESULT_LEN];
  char cellResult[RESULT_LEN];
  char sdStatusResult[RESULT_LEN];
  char gpsResult[RESULT_LEN];
  char systemStat[RESULT_LEN];
  char SDstat[RESULT_LEN];
};

// A function to clear all the fields of a resultStruct
void clearResults(ResultStruct* r) {
  r->chargeStateResult[0] = 0;
  r->mpuResult[0] = 0;
  r->wifiResult[0] = 0;
  r->cellResult[0] = 0;
  r->sdStatusResult[0] = 0;
  r->gpsResult[0] = 0;
  r->systemStat[0] = 0;
  r->SDstat[0] = 0;
}

// A function to take all of the resutl strings and concatenate them together
String stringifyResults(ResultStruct r) {
  String result = "";
  result += String(Time.now());
  result += MINOR_DLIM;
  result += String(millis());
  result += MAJOR_DLIM;
  result += String(r.chargeStateResult);
  result += MAJOR_DLIM;
  result += String(r.mpuResult);
  result += MAJOR_DLIM;
  result += String(r.wifiResult);
  result += MAJOR_DLIM;
  result += String(r.cellResult);
  result += MAJOR_DLIM;
  result += String(r.sdStatusResult);
  result += MAJOR_DLIM;
  result += String(r.gpsResult);
  result += MAJOR_DLIM;
  result += String(r.systemStat);
  result += MAJOR_DLIM;
  result += String(r.SDstat);
  return result;
}

// retain this so that on the next iteration we still get results on hang
ResultStruct sensingResults;

/* LOOP State enums and their global delcarations*/
// When we wake up we always want to start in sleep 
// Because we might just want to tickle the watchdog and sleep again
SystemState state = Sleep;

CellularState cellularState = InitiateParticleConnection;

WatchdogState watchdogState = WatchdogLow;

CollectionState collectionState = WaitForCollection;

SleepState sleepState = SleepToAwakeCheck;
/* END Loop delcaration variables*/

/* Configration variables and times*/
retained uint32_t last_collection_time;
//one hour
const uint32_t  COLLECTION_INTERVAL_SECONDS = 120;
//twelve hours
const uint32_t  SLEEP_COLLECTION_INTERVAL_SECONDS = 3600;
/* END Configration variables and times*/

/* Variables that track whether or not we should use the watchdog */


void loop() {
  // This is the only thing that will happen outside of the state machine!
  // Everything else, including reconnection attempts and cloud update Checks
  // Should happen in the cloud event state
  Particle.process();

  //The software watchdog checks to make sure we are still making it around the loop
  wd.checkin();

  Serial.printlnf("State: %d", state);
  switch(state) {

    /*Initializes the state machine and board. Decides when to sleep or wake*/
    case Sleep: {
      //Check if you should go to sleep. Manage waking up and sending periodic check ins
      Serial.printlnf("Sleep State: %d", sleepState);
      switch(sleepState) {
        case SleepToAwakeCheck:
          // We are here either because 
          //  1) the program just started
          //        - If this is the case we are either powered or past our sleep collection interval
          //  2) the RTC woke us up to collect and send data 
          //        - this should be true if we are past our sleep collection interval
          //  3) a power restoration woke us up and we now should be awake and send data
          //        - If this is the case the power will be on
          //  4) The particle woke us and we should just tickle the watchdog and go back to sleep
          //        - This would be true if none of the above are true.

          if(powercheck.getHasPower()) {
            //We should just be awake
            state = Sleep;
            sleepState = PrepareForWake;
            Serial.println("Transitioning to PrepareForWake");
          } else if (last_collection_time/SLEEP_COLLECTION_INTERVAL_SECONDS != rtc.getTime()/SLEEP_COLLECTION_INTERVAL_SECONDS) {
            //have we rolled over to a new collection interval? truncating division shoul handle the modulus of the interval time.
            state = Sleep;
            sleepState = PrepareForWake;
          } else {
            //We just woke up to tickle the watchdog and sleep again
            state = Sleep;
            sleepState = PrepareForSleep;
          }
        break;
        case PrepareForWake:
          //In this state we turn everything on then let the state machine progress
          // Turn on the GPS
          Serial.println("Turning GPS on");
          gps.powerOn();

          // Turn on the STM and voltage sensing
          Serial.println("Turning AC sense on");
          digitalWrite(AC_PWR_EN, HIGH);

          // Turn on the SD card
          Serial.println("Turning SD card on");
          SD.PowerOn();

          //Proceed through both state machines
          Serial.println("Transitioning to AwakeToSleepCheck");
          sleepState = AwakeToSleepCheck;
          state = CheckPowerState;
        break;
        case AwakeToSleepCheck:
          //We should not sleep if
          // - we have power
          // - we are currently collecting data
          // - Data has just been collected and needs to be sent/queued
          if(powercheck.getHasPower()) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else if (collectionState != WaitForCollection) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else if (CloudQueue.size() != 0 || SDQueue.size() != 0) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else {
            sleepState = PrepareForSleep;
            Serial.println("Transitioning to PrepareForSleep");
            state = Sleep;
          }
        break;
        case PrepareForSleep:
          //In this state we turn everything off then go to sleep. Using sleep mode deep starts
          //Everything back from the beginning
          // Turn off the GPS
          digitalWrite(GPS_PWR_EN, LOW);
          // Turn off the STM and voltage sensing
          digitalWrite(AC_PWR_EN, LOW);
          // Turn off the SD card
          SD.PowerOff();

          // Toggle the watchdog to make sure that it doesn't trigger early
          digitalWrite(WDI, HIGH);
          delay(100);
          digitalWrite(WDI, LOW);

          //make sure the rtc interrupt is clear so that we wake up
          //we can do this by reinitializing the RTC
          timeSync.setup();
          Serial.println("Going to sleep");
          
          Serial.flush();
          Serial.end();

          // Set two timers
          // One timer on the RTC for the next time we want to collect data
          // Another timer for the particle so that we can tickle the watchdog
          
          //calculate the next time we want to collect data
          // Get the current time
          uint32_t current_time = rtc.getTime();
          // the division truncates this time to the last time we would have collected
          uint32_t collection_number = current_time/SLEEP_COLLECTION_INTERVAL_SECONDS;
          // calculate the next time we should collect
          uint32_t new_collection_number = collection_number + 1;
          uint32_t new_collection_time = new_collection_number*SLEEP_COLLECTION_INTERVAL_SECONDS;
          rtc.setTimer(new_collection_time);

          // Now go to sleep with the system sleep set to wakeup for the watchdog
          System.sleep(SLEEP_MODE_DEEP, 600);

          //If for some reason this fails try again?
          state = Sleep;
          sleepState = PrepareForSleep;
        break;
      }
    break;
    }

    /*Checks for power state changes and generates events for them*/
    case CheckPowerState: {
      state = MaintainCellularConnection;    
    break;
    }

    /*Manages connection to the particle cloud or cellular network*/
    case MaintainCellularConnection: {
      Serial.printlnf("Cellular State: %d", cellularState);
      static unsigned long connection_start_time;
      switch(cellularState) {
        case InitiateParticleConnection: 
          if(!Particle.connected()) {
            Particle.connect();
            connection_start_time = millis();
            cellularState = ParticleConnecting;
          } else {
            cellularState = ParticleConnected;
          }
        break;
        case ParticleConnecting:
          if(Particle.connected()) {
            cellularState = ParticleConnected;
          } else if(millis() - connection_start_time > 600000) {
            //try to connect to cellular network as a backup to get time
            //stop trying to connect to particle
            Particle.disconnect();
            cellularState = InitiateCellularConnection;
          } else {
            //do nothing - let it try to connect
          }
        break;
        case ParticleConnected:
          if(!Particle.connected()) {
            cellularState = InitiateParticleConnection;
          }
        break;
        case InitiateCellularConnection:
            Cellular.connect();
            connection_start_time = millis();
            cellularState = CellularConnecting;
        break;
        case CellularConnecting:
          if(Cellular.ready()) {
            connection_start_time = millis();
            cellularState = CellularConnected;
          } else if(millis() - connection_start_time > 600000) {
            //try to connect to particle again if we fail
            cellularState = InitiateParticleConnection;
          } else {
            //do nothing - let it try to connect
          }
        break;
        case CellularConnected:
          //If we disconnect from the cellular network or if we just haven't
          //retried to connect with particle in a while, try to connect to particle again
          if(!Cellular.ready() || millis() - connection_start_time > 3600000) {
            cellularState = InitiateParticleConnection;
          }
        break;
      }
      state = CheckTimeSync;
      break;
    }
    /*Keeps the device time up to date*/
    case CheckTimeSync: {
      timeSync.update(cellularState);
      state = LogData;
      break;
    }

    /*Saves data to the SD card*/
    case LogData: {
      //just turn it on for good measure, although we should just leave it on
      SD.PowerOn();
      if(SDQueue.size() > 0) {
        if(DataLog.appendAndRotate(SDQueue.front(), Time.now())) {
          //Error writing data to the SD card - do something with it
          Serial.println("Error writing data to the SD card");
        } else {
          SDQueue.pop();
        }
      }

      state = SendData;
      break;
    }

    /*Sends data to the cloud either from the event queue or from the SD card based backlog*/
    case SendData: {
      SD.PowerOn();
      if(CloudQueue.size() > 0) {
        if(Particle.connected()) {
            if(!Particle.publish("g",CloudQueue.front(), PRIVATE)) {
              //should handle this error
              Serial.println("Failed to send packet. Appending to dequeue.");
              if(DataDequeue.append(CloudQueue.front())) {
                //should handle this error
                Serial.println("Failed to append to dequeue");
              } else {
                Serial.println("Appended to dequeue successfully");
                CloudQueue.pop();
              }
            } else {
              Serial.println("Appended to dequeue successfully");
              CloudQueue.pop();
            }
        } else {
          if(DataDequeue.append(CloudQueue.front())) {
            //should handle this error
            Serial.println("Failed to append to dequeue");
          } else {
            Serial.println("Appended to dequeue successfully");
            CloudQueue.pop();
          }
        }
      } else if(DataDequeue.getFileSize() > 0) {
        if(Particle.connected()) {
          if(!Particle.publish("g",DataDequeue.getLastLine(),PRIVATE)) {
            //should handle this error
            Serial.println("Failed to send from dequeue");
          } else {
            Serial.println("Sent from queue successfully");
            DataDequeue.removeLastLine();
          }
        } 
      }
      state = CollectPeriodicInformation;
      break;
    }

    /*Periodically collects summary metrics from the device and generates those events*/
    case CollectPeriodicInformation: {
      Serial.printlnf("Collection State: %d", collectionState);
      switch(collectionState) {
        case WaitForCollection: {
          //Decide if we need to collect
          uint32_t current_time = rtc.getTime();
          uint32_t last_collection_number = last_collection_time/COLLECTION_INTERVAL_SECONDS;
          uint32_t current_collection_number = current_time/COLLECTION_INTERVAL_SECONDS;
          Serial.printlnf("Current time: %d, Last collection time: %d",current_time, last_collection_time);
          if(last_collection_number != current_collection_number) {
            collectionState = CollectResults;
          }
          
          //Also service the things that need to be serviced when we aren't
          //collectinga
          gps.update();
          imu.update();
          break;
        }
        case CollectResults: {
          strncpy(sensingResults.mpuResult,imu.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.gpsResult,gps.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.sdStatusResult,SD.getResult().c_str(), RESULT_LEN);
          strncpy(sensingResults.chargeStateResult,chargeState.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.cellResult,cellStatus.read().c_str(), RESULT_LEN);
          snprintf(sensingResults.systemStat, RESULT_LEN, "%lu|%s|%u", 0, serialNumber.read(),0);
          snprintf(sensingResults.SDstat,RESULT_LEN, "%u|%d",0,DataLog.getRotatedFileSize(Time.now()));
          String result = stringifyResults(sensingResults);
          SDQueue.push(result);
          CloudQueue.push(result);
          last_collection_time = rtc.getTime();
          collectionState = WaitForCollection;
          break;
        }
      }
      state = ServiceWatchdog;
    break;
    }

    /*manages the watchdog and light on the device*/
    case ServiceWatchdog: {
      static unsigned int lastWatchdog;
      static bool service = false;
      switch(watchdogState) {
        case WatchdogHigh:
          digitalWrite(DAC, HIGH);
          if(millis() - lastWatchdog > 1000) {
            lastWatchdog = millis();
            watchdogState = WatchdogLow;
          }
        break;
        case WatchdogLow:
          digitalWrite(DAC, LOW);
          if(millis() - lastWatchdog > 1000 && 1/*the watchdog conditions are met*/) {
            lastWatchdog = millis();
            watchdogState = WatchdogHigh;
          }
        break;
      }
      state = Sleep;
    break;
    }
    
    default: {
      state = Sleep;
      break;
    }
  }


}


