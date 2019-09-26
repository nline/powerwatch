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


## List of things to functionally verify
- Sensing
  - [  ] GPS can get fix
  - [  ] GPS PPS line pulses and can be read by particle and  STM32
  - [  ] Can still set accel Interrupt and read accel angle of board
  - [  ] STM32 reads correct voltage out of HV amplifiers
  - [  ] STM32 reads correct voltage out of LV amplifiers
  - [  ] Particle reads correct voltage out of HV amplifiers
  - [  ] Particle reads correct voltage out of LV amplifiers
  - [  ] When HV is applied (i.e. 320VAC) HV amplifiers still work with resolution
  - [  ] When 800V voltage transient is applied detector triggers
  - [  ] STM32 can read voltage angle using PPS
- Communication
  - [  ] Can connect and comm to particle cloud
  - [  ] STM32 and particle can communicate
- Power
  - [  ] 5V supplied when plugged into wall
  - [  ] Battery charges in all states when wall powered
  - [  ] When battery drops below voltage thresh and no wall power particle in RST
  - [  ] When wall power particle never in reset (battery normal or low voltage)
  - [  ] Particle can power on/off SD
  - [  ] Particle can power on/off AC sensing + STM32
  - [  ] Particle can power on/off GPS
  - [  ] Particle wakes up when wall power is applied
- Storage
  - [  ] Can write to SD Card
  - [  ] Can read from SD Card
- Time
  - [  ] Write time to RTC
  - [  ] Read time from RTC
  - [  ] RTC maintains time after power off and on
  - [  ] Can set timer on RTC that wakes up Particle (using particle wakeup circuit)
- Programming
  - [  ] Can Program STM32 and STM32 boots
  - [  ] STM reset button works
- Other
  - [  ] Particle can control RGB LED
  - [  ] Particle can set brightness of RGB LED through brightness IC
  - [  ] LEDs are same brightness when powered on
  - [  ] Particle can deterministically hard reset itself (not in a loop)
  - [  ] Watchdog triggers when not tickled

## Potential issues 
- TX/RX output from STM32 for debugging? To particle?
- Paticle reset STM32 without a hard reset?
