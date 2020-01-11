enum SystemState {
  Sleep,
  CheckPowerState,
  MaintainCellularConnection,
  CheckTimeSync,
  LogData,
  SendData,
  CollectPeriodicInformation,
  ServiceWatchdog,
};

enum PowerState {
  Unknown,
  Powered,
  Unpowered
};

enum CellularState {
  InitiateParticleConnection,
  ParticleConnecting,
  ParticleConnected,
  InitiateCellularConnection,
  CellularConnecting,
  CellularConnected
};

enum WatchdogState {
  WatchdogHigh,
  WatchdogLow
};

enum CollectionState {
  WaitForCollection,
  CollectResults
};

enum SleepState {
  SleepToAwakeCheck,
  PrepareForWake,
  AwakeToSleepCheck,
  PrepareForSleep,
};
