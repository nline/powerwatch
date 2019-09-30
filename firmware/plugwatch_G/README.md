Plugwatch G Firmware
====================

## Testing
- Sensing
  - [ ] GPS can get fix - gps_test
  - [ ] GPS PPS line pulses and can be read by particle and STM32 - 1/2 gps_test
  - [ ] Can still set accel Interrupt and read accel angle of board - accel_test
  - [ ] STM32 reads correct voltage out of HV amplifiers
  - [ ] STM32 reads correct voltage out of LV amplifiers
  - [ ] Particle reads correct voltage out of HV amplifiers
  - [ ] Particle reads correct voltage out of LV amplifiers
  - [ ] When HV is applied (i.e. 320VAC) HV amplifiers still work with resolution
  - [ ] When 800V voltage transient is applied detector triggers
  - [ ] STM32 can read voltage angle using PPS
- Communication
  - [ ] Can connect and comm to particle cloud
  - [ ] STM32 and particle can communicate
  - [x] Can read ID chip - id_chip_test
- Power
  - [ ] 5V supplied when plugged into wall
  - [ ] Battery charges in all states when wall powered
  - [ ] When battery drops below voltage thresh and no wall power particle in RST
  - [ ] When wall power particle never in reset (battery normal or low voltage)
  - [ ] Particle can power on/off SD
  - [ ] Particle can power on/off AC sensing + STM32
  - [ ] Particle can power on/off GPS - gps_test
  - [ ] Particle wakes up when wall power is applied
- Storage
  - [ ] Can write to SD Card
  - [ ] Can read from SD Card
- Time
  - [ ] Write time to RTC
  - [ ] Read time from RTC
  - [ ] RTC maintains time after power off and on
  - [ ] Can set timer on RTC that wakes up Particle (using particle wakeup circuit)
- Programming
  - [ ] Can Program STM32 and STM32 boots
  - [ ] STM reset button works
- Other
  - [x] Particle can control RGB LED - led_test
  - [x] Particle can set brightness of RGB LED through brightness IC - led_test
  - [x] LEDs are same brightness when powered on - led_test
    - They are at least close? Maybe blue could be brighter?
  - [ ] ~Particle can deterministically hard reset itself (not in a loop) - particle_rst_test~
    - This does not WORK! Must switch to PIN D2 because there is a pull up on D3
  - [ ] Watchdog triggers when not tickled

## Bugs and things to fix
 - Switch pins D2 and D3
