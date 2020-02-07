#this Makefile should work with the mbed-cli to build the program,
#then it can use JLink to flash

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#you must define a valid ARM MBed Target
TARGET = NUCLEO_L432KC

FLASH_START_ADDRESS = 0x8010000

#you must define a vliad ARM Mbed toolchain (GCC_ARM or IAR)
TOOLCHAIN = GCC_ARM

include ../../support/AppMakefileMbed.mk
