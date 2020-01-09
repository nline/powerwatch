/**
 * SD Card related operations.
 */

#pragma once

#include <Particle.h>

#include <SdFat.h>

#include "lib/Subsystem.h"

class SDCard: public Subsystem {
	typedef Subsystem super;

	const uint8_t SD_INT_PIN = D6;
	const uint8_t SD_ENABLE_PIN = C2;
	const uint8_t SD_CHIP_SELECT = A2;

	#define SCK A3
	#define  MISO A4
	#define MOSI A5
	#define  SS A2
	// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)

	String result;

public:
	SdFat sd;
	void setup();
	LoopStatus loop();
	String getResult();

  /**
	 * Power cycles SD Card. Blocking call.
	 */
	void PowerOn();
	void PowerOff();
	
	String getLastLine(String filename);
	bool removeLastLine(String filename);
	bool readLastLine(String filename);
 	bool Write(String filename, String to_write);
	int getSize(String filename);

	String ReadLine(String filename, uint32_t position);
	String Read(String filename);
};
