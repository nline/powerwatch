// Native
#include <Particle.h>

// Third party libraries
#include <APNHelperRK.h>
#include <CellularHelper.h>

// Our code
#include "CellStatus.h"
#include "AB1815.h"
#include "ChargeState.h"
#include "Cloud.h"
#include "FileLog.h"
#include "Gps.h"
#include "Imu.h"
#include "SDCard.h"
#include "Subsystem.h"
#include "Timesync.h"
#include "uCommand.h"
#include "firmware.h"
#include "BatteryCheck.h"
#include "led.h"
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
bool handshake_flag = false;
String id(void);
String shield_id = "";

//***********************************
//* Watchdogs
//***********************************
const int HARDWARE_WATCHDOG_TIMEOUT_MS = 1000 * 60;
ApplicationWatchdog wd(HARDWARE_WATCHDOG_TIMEOUT_MS, soft_watchdog_reset);
void soft_watchdog_reset() {
  Serial.println("Resetting by sleeping temporarily");
  System.sleep(SLEEP_MODE_DEEP, 60);
}

unsigned long system_cnt = 0;
retained unsigned long reboot_cnt = 0;
retained unsigned long sd_cnt = 0;

//***********************************
//* SD Card
//***********************************
SDCard SD;

//***********************************
//* Timesync
//***********************************
auto timeSyncSubsystem = Timesync();
AB1815 rtc;

//***********************************
//* CellStatus
//***********************************
auto cellStatus = CellStatus();

//***********************************
//* Charge state
//***********************************
auto chargeStateSubsystem = ChargeState();

//***********************************
//* IMU
//***********************************
auto imuSubsystem = Imu();

//***********************************
//* GPS
//***********************************
auto gpsSubsystem = Gps();

//***********************************
//* System Events
//***********************************
auto EventLog = FileLog(SD, "event_log.txt");
std::queue<String> EventQueue;
std::queue<String> CloudQueue;
std::queue<String> DataQueue;

unsigned long last_cloud_event = 0;

//***********************************
//* System Data
//***********************************
retained char data_log_name[50];
auto DataLog = FileLog(SD, "data_log.txt");
auto DataDequeue = FileLog(SD, "data_dequeue.txt");
unsigned long last_logging_event  = 0;


// String SYSTEM_EVENT = "s";
retained int system_event_count = 0;
retained String last_system_event_time = "";
retained int last_system_event_type = -999;
retained int num_reboots = 0;
retained int num_manual_reboots = 0;

volatile int network_state = 0;
volatile int cloud_state = 0;

void handle_all_system_events(system_event_t event, int param) {
  system_event_count++;
  // cast as per BDub post here: https://community.particle.io/t/system-events-param-problem/32071/14
  if((uint32_t)event == 64) {
    cloud_state = param;
  }

  if((uint32_t)event == 32) {
    network_state = param;
  }

  String event_type;
  String event_param;

  switch((uint32_t) event) {
  case setup_begin:
    event_type = "setup_begin";
  break;
  case setup_update:
    event_type = "setup_update";
  break;
  case setup_end:
    event_type = "setup_end";
  break;
  case network_credentials:
    event_type = "network_credentials";
    switch(param) {
    case network_credentials_added:
      event_param = "added";
    break;
    case network_credentials_cleared:
      event_param = "cleared";
    break;
    }
  break;
  case network_status:
    event_type = "network_status";
    switch(param) {
    case network_status_powering_on:
      event_param = "network_status_powering_on";
    break;
    case network_status_on:
      event_param = "on";
    break;
    case network_status_powering_off:
      event_param = "powering_off";
    break;
    case network_status_off:
      event_param = "off";
    break;
    case network_status_connecting:
      event_param = "connecting";
    break;
    case network_status_connected: 
      event_param = "connected";
    break;
    }
  break;
  case cloud_status:
    event_type = "cloud_status";
    switch(param) {
    case cloud_status_disconnecting:
      event_param = "disconnecting";
    break;
    case cloud_status_disconnected: 
      event_param = "disconnected";
    break;
    case cloud_status_connecting:
      event_param = "connecting";
    break;
    case cloud_status_connected: 
      event_param = "connected";
    break;
    }
  break;
  case button_status:
    event_type = "button_status";
  break;
  case firmware_update:
    event_type = "firmware_update";
    switch(param) {
    case firmware_update_begin:
      event_param = "begin";
    break;
    case firmware_update_progress: 
      event_param = "progress";
    break;
    case firmware_update_complete:
      event_param = "complete";
    break;
    case firmware_update_failed: 
      event_param = "failed";
    break;
    }
  break;
  case firmware_update_pending:
    event_type = "firmware_update_pending";
  break;
  case reset_pending:
    event_type = "reset_pending";
  break;
  case reset:
    event_type = "reset";
  break;
  case button_click:
    event_type = "button_click";
  break;
  case button_final_click:
    event_type = "button_final_click";
  break;
  case time_changed:
    event_type = "time_changed";
    switch(param) {
    case time_changed_manually:
      event_param = "manual";
    break;
    case time_changed_sync:
      event_param = "sync";
    break;
    }
  break;
  case low_battery:
    event_type = "low_battery";
  break;
  }


  Serial.printlnf("got event %s with param %s", event_type.c_str(), event_param.c_str());
  String system_event_str = String((int)event) + "|" + String(param);
  String time_str = String(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
  last_system_event_time = time_str;
  last_system_event_type = param;

  //Push this system event onto the queue to be logged in the error logging state
  if(EventQueue.size() < 200) {
    EventQueue.push(time_str + ": " + system_event_str);
  }
}

void handle_error(String error, bool cloud) {
  Serial.printlnf("Got error: %s", error.c_str());
  String time_str = String(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
  last_system_event_time = time_str;

  //Push this system event onto the queue to be logged in the error logging state
  if(cloud) {
    if(CloudQueue.size() < 200) {
      CloudQueue.push(time_str + ": " + error);
    }
  }

  if(EventQueue.size() < 200) {
    EventQueue.push(time_str + ": " + error);
  }
}

int force_handshake(String cmd) {
  handshake_flag = true;
  return 0;
}


SystemState nextState(SystemState s) {
    return static_cast<SystemState>(static_cast<int>(s) + 1);
}

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

void reset_helper() {
  reset_state("");
}

int reset_state(String cmd) {
  state = CheckCloudEvent;
  lastState = Wait;
  System.reset();
  return 0;
}

//***********************************
//* ye-old Arduino
//***********************************
void setup() {
  // Keep track of reboots
  reboot_cnt++;

  //setup the apns
  apnHelper.setCredentials();

  //This function tells the particle to force a reconnect with the cloud
  Particle.function("handshake", force_handshake);
  Particle.function("reset_state", reset_state);

  // Set up debugging UART
  Serial.begin(9600);
  Serial.println("Initial Setup.");

  // Set up I2C
  Wire.begin();

  // For now, just grab everything that happens and log about it
  // https://docs.particle.io/reference/firmware/photon/#system-events
  System.on(all_events, handle_all_system_events);

  // Setup SD card first so that other setups can log
  SD.setup();

  //setup the other subsystems
  chargeStateSubsystem.setup();
  imuSubsystem.setup();
  gpsSubsystem.setup();
  wifiSubsystem.setup();
  FuelGauge().quickStart();
  shield_id = id();

  //Setup the watchdog toggle pin
  pinMode(DAC, OUTPUT);
  digitalWrite(DAC, LOW);

  // GPS
  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);

  //Timesync
  timeSyncSubsystem.setup();

  Particle.keepAlive(23*60); // send a ping every 30 seconds

  Serial.println("Setup complete.");
}


//State Timer is reused to make sure a state doesn't loop for too long.
//If it loops for too long we just call a reset. This can get us out of
//liveness bugs in a specific driver
Timer stateTimer(60000,soft_watchdog_reset,true);

//Before calling each state's loop function the state should call this
//function with a period of the maximum time the state is allowed to take
void manageStateTimer(unsigned long period) {
  if(state != lastState) {
    Serial.printlnf("Transitioning to state %d from %d", state, lastState);
    stateTimer.stop();
    stateTimer.changePeriod(period);
    stateTimer.reset();
    stateTimer.start();
    lastState = state;
  }
}

//This structure is what all of the drivers will return. It will
//be packetized and send to the cloud in the sendPacket state
#define RESULT_LEN 100
struct ResultStruct {
  char chargeStateResult[RESULT_LEN];
  char mpuResult[RESULT_LEN];
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
retained ResultStruct sensingResults;

/* LOOP State enums and their global delcarations*/
enum SystemState {
  Sleep,
  CheckPowerState,
  MaintainCellularConnection,
  CheckTimeSync,
  LogPacket,
  SendPacket,
  CollectPeriodicInformation,
  ServiceWatchdog,
};

// When we wake up we always want to start in sleep 
// Because we might just want to tickle the watchdog and sleep again
SystemState state = Sleep;

enum CellularState {
  InitiateParticleConnection,
  ParticleConnecting,
  ParticleConnected,
  InitiateCellularConnection,
  CellularConnecting,
  CellularConnected
};

CellularState cellularState = InitiateParticleConnection;

enum WatchdogState {
  WatchdogHigh,
  WatchdogLow
};

WatchdogState watchdogState = WatchdogLow;

enum CollectionState {
  WaitForCollection,
  ReadIMU,
  ReadGPS,
  ReadSDMetrics,
  ReadPowerMetrics,
  ReadSystemMetrics,
  AddToQueues
};

CollectionState collectionState = WaitForCollection;

enum SleepState {
  SleepToAwakeCheck,
  PrepareForWake,
  AwakeToSleepCheck,
  PrepareForSleep,
};

SleepState sleepState = SleepToAwakeCheck;


/* END Loop delcaration variables*/

/* Configration variables and times*/
retained uint32_t last_collection_time;
//one hour
const uint32_t  COLLECTION_INTERVAL_SECONDS = 3600;
//twelve hours
const uint32_t  SLEEP_COLLECTION_INTERVAL_SECONDS = 3600*12;

/* END Configration variables and times*/



void loop() {
  // This is the only thing that will happen outside of the state machine!
  // Everything else, including reconnection attempts and cloud update Checks
  // Should happen in the cloud event state
  Particle.process();

  //The software watchdog checks to make sure we are still making it around the loop
  wd.checkin();

  switch(state) {

    /*Initializes the state machine and board. Decides when to sleep or wake*/
    case Sleep: {
      //Check if you should go to sleep. Manage waking up and sending periodic check ins
      switch(sleepState) {
        case SleepToAwakeCheck:
          //This is the entry state to the state machine
          //Everything is asleep/off
          //We can either decide to wake up (to send a periodic packet, or we have power now)
          //Or tickle the watchdog and go back to sleep
          //Stays in macros state sleep
          state = Sleep
        break;
        case PrepareForWake:
          //In this state we turn everything on then let the state machine progress
        break;
        case AwakeToSleepCheck:
          //Should we sleep? Are we unpowered? Have we cleared/tried to clear our recent queues?
        break;
        case PrepareForSleep:
          //In this state we turn everything off then go to sleep. Using sleep mode deep starts
          //Everything back from the beginning
        break;
      }
    break;
    }

    /*Checks for power state changes and generates events for them*/
    case CheckPowerState: {
      state = CollectPeriodicInformation;    
    break;
    }

    /*Manages connection to the particle cloud or cellular network*/
    case MaintainCellularConnection: {
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
          } else if(millis() - connection_start_time > 600000)
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
            cellularState = CellularConnecting
        break;
        case CellularConnecting:
          if(Cellular.ready()) {
            connection_start_time = millis();
            cellularState = CellularConnected;
          } else if(millis() - connection_start_time > 600000)
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
            cellularState = InitiateParticleConnection
          }
        break;
      }
      state = CheckTimeSync;
      break;

    /*Keeps the device time up to date*/
    case CheckTimeSync: {
      timeSyncSubsystem.loop();
      state = LogData;
      break;
    }

    /*Saves data to the SD card*/
    case LogPacket: {
      //just turn it on for good measure, although we should just leave it on
      SD.PowerOn();
      if(SDQueue.size() > 0) {
        if(DataLog.appendAndRotate(SDQueue.front(), Time.now())) {
          //Error writing data to the SD card - do something with it
          Serial.println("Error writing data to the SD card")
        } else {
          SDQueue.pop();
        }
      }

      state = SendPacket;
      break;
    }

    /*Sends data to the cloud either from the event queue or from the SD card based backlog*/
    case SendPacket: {
      SD.PowerOn();
      if(CloudQueue.size() > 0) {
        if(Particle.connected()) {
            if(!Cloud::Publish("g",CloudQueue.front())) {
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
          if(!Cloud::Publish("g",DataDequeue.getLastLine())) {
            //should handle this error
            Serial.println("Failed to send from dequeue");
          } else {
            Serial.println("Sent from queue successfully");
            DataDequeue.removeLastLine();
          }
        } 
      }
      state = CheckPowerState;
      break;
    }

    /*Periodically collects summary metrics from the device and generates those events*/
    case CollectPeriodicInformation: {
      switch(collectionState) {
        case WaitForCollection:
          uint32_t current_time = rtc.get_time();
          uint32_t last_collection_number = last_collection_time/COLLECTION_INTERVAL_SECONDS;
          uint32_t current_collection_number = current_time/COLLECTION_INTERVAL_SECONDS;
          if(last_collection_number != current_collection_number) {
            collectionState = ReadIMU;
          }
        break;
        case ReadIMU:
        break;
        case ReadGPS:
        break;
        case ReadSDMetrics:
        break;
        case ReadPowerMetrics:
        break;
        case ReadSystemMetrics:
        break;
        case AddToQueues:
        break;
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


