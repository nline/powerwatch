#include <OneWire.h>
OneWire ds(B0);

SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  Cellular.on();
}

void loop() {
  id();
  delay(500);
}


void id() {
  byte i;
  boolean present;
  byte data[8];     // container for the data from device
  byte crc_calc;    //calculated CRC
  byte crc_byte;    //actual CRC as sent by DS2401
  present = ds.reset();
  if (present == TRUE)
  {
    ds.write(0x33);  //Send Read data command
    data[0] = ds.read();
    String myID = System.deviceID();
    Serial.print(myID);
    Serial.print(",");
    for (i = 1; i <= 6; i++)
    {
      data[i] = ds.read(); //store each byte in different position in array
      PrintTwoDigitHex (data[i], 0);
    }
    crc_byte = ds.read(); //read CRC, this is the last byte
    crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data
    if (crc_byte != crc_calc) {
      Serial.println("!");
    } else {
      Serial.print(",");
    }

    CellularDevice dev;
    cellular_device_info(&dev, NULL);
    Serial.print(dev.iccid);
    Serial.print("\n");
  }
}

void PrintTwoDigitHex (byte b, boolean newline)
{
  Serial.print(b/16, HEX);
  Serial.print(b%16, HEX);
  if (newline) Serial.println();
}
