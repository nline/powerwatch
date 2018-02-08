#include "Serial5/Serial5.h"

#include "ESP8266.h"

const String endstring = "OK\r\n";

ESP8266::ESP8266(String* response, bool* done):
  response { response },
  done { done }
{
  *done = false;

  // Reset and set baud rate to 9600 if at 115200
  Serial5.begin(115200);
  Serial5.println("AT+RST");
  delay(1000);
  Serial5.println("AT+UART_CUR=9600,8,1,0,0");
  delay(500);

  // Set baud to 9600
  Serial5.end();
  Serial5.begin(9600);
  Serial5.println("AT");

  // Set mode to client
  Serial5.println("AT+CWMODE=1");
  delay(500);

  // Clear out response from buffer
  while(Serial5.available()) {
    Serial5.read();
  }

  *response = "";
  *done = true;
}

void ESP8266::beginScan() {
  Serial5.println("AT+CWLAP");
  *response = "";
  *done = false;
}

void ESP8266::updateResponse(String recv) {
  if (!done) {
    response->concat(recv);
  }
}

void ESP8266::loop() {
  if (!*done) {
    if (response->endsWith(endstring)) {
      *done = true;
    }
  }
}
