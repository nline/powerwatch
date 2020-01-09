#pragma once
#include <OneWire.h>

uint8_t getID(uint8_t* data) {
  OneWire ds(B0);
  byte i;
  boolean present;
  byte data[8];     // container for the data from device
  char temp[4];
  byte crc_calc;    //calculated CRC
  byte crc_byte;    //actual CRC as sent by DS2401
  //1-Wire bus reset, needed to start operation on the bus,
  //returns a 1/TRUE if presence pulse detected
  present = ds.reset();
  if (present == TRUE)
  {
    ds.write(0x33);  //Send Read data command
    data[0] = ds.read();
    for (i = 1; i <= 6; i++)
    {
      data[i] = ds.read(); //store each byte in different position in array
    }
    crc_byte = ds.read(); //read CRC, this is the last byte
    crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

    if(crc_calc == crc_byte) {
      return 0;
    } else {
      return 1;
    }
  }
  else //Nothing is connected in the bus
  {
    return 1;
  }
}
