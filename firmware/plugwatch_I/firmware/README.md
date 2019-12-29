Plugwatch I Firmware
====================

The plugwatch I firmware is new event based firmware that is designed to both
collect better information about the power state and to use much less data than previous
firmware versions. It also intends to sleep when it doesn't have power so that it can last
much longer on its battery. 

The goals of the firmware are as follows

## When powered
 - Get the timestamp of outages sub second, and have those outages reported as soon as possible
 - Get the angle of the accelerometer some time before the outage
 - get the angle of the accelerometer some time after the outage
 - Get whether the accelerometer movement tripped within 10s before the outage
 - At every outage get the voltage fall time
 - every hour a status update with following metrics:
  - device ID, shield ID, SIM ICCID
  - GPS location
  - the average line voltage every minute
  - the average line frequency every minute
  - The number of half waveforms above a certain voltage (or multiple voltages)
  - the number of half waveforms below a certain voltage (or multiple voltages)
  - a spot sample on the angle of the votlage waveform if the gps has a fix
  - SD card metrics (size of file)
  - accelerometer angle
  - List of outage times since last report
  - list of restore times since last report
  - battery state
  - current cellular quality

## When unpowered
  - Go to sleep
  - the subsecond timestamp of power restoration, reported as soon as possible
  - the angle of the accelerometer some time before and some time after restoration
  - the rise time of the voltage at every restoration
  - every 12-24 hours a status update with the same status update metrics as above (except the voltage metrics)
