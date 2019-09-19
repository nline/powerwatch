Plugwatch G
===========

The next major revision of plugwatch.

## Features/changes
- New method of performing voltage sampling
  - Voltage sampling coprocessor
  - New voltage sensing amplifier 
  - High and low voltage gains on the amplifier for expected and higher-than-expected voltages
  - A precision reference to bias all of the amplifiers at the same voltage and run the analog domain of the voltage coprocessor
- Voltage transient detection circuits for spikes over 800V
  - Isolation (in the form of a 1ohm resistor) to prevent the transient suppression varistor from impacting transient sensing
- Add an explicit reset pin from particle that powers down the external particle reset (without putting it in a reset loop)
- Add a circuit so that an RTC time can wake up the particle using the WKP pin
- Add tricolor LED and I2C controllable PWM IC to change it brightness even with the particle asleep
- remove screw terminals in favor of soldered wires for AC connection

## Fixes
- Fix the resistor divider so that the watchdog/supervisor chip keeps the particle on when plugged in despite battery level
- Fix resistor divider on WKP pin so that power on actually wakes up the devices

