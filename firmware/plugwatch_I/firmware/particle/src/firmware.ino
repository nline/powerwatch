// Native
#include <Particle.h>

// Third party libraries
#include <APNHelperRK.h>
#include <CellularHelper.h>
#include <OneWire.h>

// Our code
#include "CellStatus.h"
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
#include "product_id.h"

//***********************************
//* TODO's
//***********************************

// Default Heartbeat Packet
// Heartbeat Stretch goals
//  SMS Heartbeat
//Tests
//  Device comes back from dead battery

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
OneWire ds(B0);
String id(void);
String shield_id = "";

//***********************************
//* Watchdogs
//***********************************
const int HARDWARE_WATCHDOG_TIMEOUT_MS = 1000 * 60;
ApplicationWatchdog wd(HARDWARE_WATCHDOG_TIMEOUT_MS, soft_watchdog_reset);
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

void system_reset_to_safemode() {
  num_manual_reboots++;
  // Commenting out this should be logged as a system event
  // Cloud::Publish(SYSTEM_EVENT, "manual reboot"); //TODO test if this hangs
  System.enterSafeMode();
}

int force_handshake(String cmd) {
  handshake_flag = true;
  return 0;
}

// The loop will act as a state machine. Certain particle calls are called every
// loop then states are executed in order. The following enumeration defines
// the states. Each state has a timeout. On timeout we reset the Particle
// and log that state as an error, moving on to the next state.
enum SystemState {
  CheckCloudEvent,
  CheckTimeSync,
  SenseChargeState,
  SenseIMU,
  SenseWiFi,
  SenseCell,
  SenseSDPresent,
  SenseGPS,
  UpdateSystemStat,
  LogPacket,
  SendPacket,
  LogError,
  SendError,
  Wait
};

SystemState nextState(SystemState s) {
    return static_cast<SystemState>(static_cast<int>(s) + 1);
}

enum ParticleCloudState {
  ParticleConnectionCheck,
  CellularConnectionCheck,
  HandshakeCheck
};

ParticleCloudState cloudState = ParticleConnectionCheck;

// Retained system states are used to diagnose restarts (error vs hard reset)
retained SystemState state = CheckCloudEvent;
retained SystemState lastState = Wait;

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

  LEDStatus status;
  status.off();

  // If our state and lastState is the same we got stuck in a
  // state and didn't transtition
  if(state == lastState) {
    String err_str(state);
    handle_error("Reset after stuck in state " + err_str, true);

    // If we hung on the last iteration, move on to the next state
    state = nextState(state);
  } else {
    // If we reset and the states aren't the same then we didn't get stuck
    // I don't know what state we're in but go back to start
    state = CheckCloudEvent;
    lastState = Wait;
  }

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
retained ResultStruct sensingResults;

void loop() {
  // This is the only thing that will happen outside of the state machine!
  // Everything else, including reconnection attempts and cloud update Checks
  // Should happen in the cloud event state
  Particle.process();

  switch(state) {
    case MaintainCellularConnection: {
      static unsigned long connection_start_time;
      switch(cellularState) {
        case InitiateParticleConnection: 
          if(!Particle.connected()) {
            Particle.connect();
            connection_start_time = millis();
            cellularState = ParticleConnecting;
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
    case CheckTimeSync: {
      timeSyncSubsystem.loop();
      state = LogData;
      break;
    }

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

    case CheckPowerState: {
    
    break;
    }

    case CollectPeriodicInformation: {
    
    break;
    }

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

      if(service) {
          //service the breathing of the light
      }
    break;
    }
    default: {
      state = CheckCloudEvent;
      lastState = Wait;
      break;
    }
  }

  //Call the automatic watchdog
  wd.checkin();

}

void soft_watchdog_reset() {
  //reset_flag = true; //let the reset subsystem shutdown gracefully
  //TODO change to system reset after a certain number of times called

  //This function won't work from an ISR??
  //System.reset();
  
  //Rick suggests this one
  Serial.println("Trying to reset");
  System.sleep(SLEEP_MODE_DEEP, 60);
}

String id() {
  byte i;
  boolean present;
  byte data[8];     // container for the data from device
  char temp[4];
  String id = "";
  byte crc_calc;    //calculated CRC
  byte crc_byte;    //actual CRC as sent by DS2401
  //1-Wire bus reset, needed to start operation on the bus,
  //returns a 1/TRUE if presence pulse detected
  present = ds.reset();
  if (present == TRUE)
  {
    ds.write(0x33);  //Send Read data command
    data[0] = ds.read();
    Serial.print("Family code: 0x");
    PrintTwoDigitHex (data[0], 1);
    snprintf(temp, 4, "%02X", data[0]);
    id.concat(String(temp));
    Serial.print("Hex ROM data: ");
    for (i = 1; i <= 6; i++)
    {
      data[i] = ds.read(); //store each byte in different position in array
      snprintf(temp, 4, "%02X", data[i]);
      id.concat(String(temp));
      PrintTwoDigitHex (data[i], 0);
      Serial.print(" ");
    }
    Serial.println();
    crc_byte = ds.read(); //read CRC, this is the last byte
    crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

    Serial.print("Calculated CRC: 0x");
    PrintTwoDigitHex (crc_calc, 1);
    Serial.print("Actual CRC: 0x");
    PrintTwoDigitHex (crc_byte, 1);

    if(crc_calc == crc_byte) {
      return id;
    } else {
      return "ERR";
    }
  }
  else //Nothing is connected in the bus
  {
    Serial.println("xxxxx Nothing connected xxxxx");
    return "ERR";
  }
}


void PrintTwoDigitHex (byte b, boolean newline)
{
  Serial.print(b/16, HEX);
  Serial.print(b%16, HEX);
  if (newline) Serial.println();
}
