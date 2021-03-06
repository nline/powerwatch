#include "Particle.h"


/**
 * Simple class to monitor for has power (USB or VIN), has a battery, and is charging
 *
 * Just instantiate one of these as a global variable and call setup() out of setup()
 * to initialize it. Then use the getHasPower(), getHasBattery() and getIsCharging()
 * methods as desired.
 */

#define AC_PWR_EN C3

#define AC_L_HV_OUT A0
#define AC_N_HV_OUT B4

#define AC_L_LV_OUT B3
#define AC_N_LV_OUT B5


class PowerCheck {
public:
	/**
	 * You must call this out of setup() to initialize the interrupt handler!
	 */
	void setup();

	/**
	 * Returns true if the Electron has power, either a USB host (computer), USB charger, or VIN power.
	 *
	 * Not interrupt or timer safe; call only from the main loop as it uses I2C to query the PMIC.
	 */
	bool getHasPower();

	/**
	 * Returns true if the Electron has a battery.
	 */
	bool getHasBattery();

	/**
	 * Returns true if the Electron is currently charging (red light on)
	 *
	 * Not interrupt or timer safe; call only from the main loop as it uses I2C to query the PMIC.
	 */
	bool getIsCharging();

	bool enableCharging();

	void lowerChargeVoltage();
	void setChargeCurrent();

	int getChargeCurrent();
	float getVoltage();
	float getHVVoltage();
	int getLCycles();
	int getNCycles();
	int getPeriod();
	uint16_t* getbuf();

	volatile unsigned long lastUnplugMillis = 0;
	volatile int lastUnplugTime = 0;
	volatile unsigned long lastPlugMillis = 0;
	volatile int lastPlugTime = 0;
	int periodMicros = 0;

private:
	void interruptHandler();

	PMIC pmic;
	volatile bool hasBattery = true;
	volatile bool hasPower = true;
	volatile unsigned long lastChange;
};
