#include<SoftwareSerial.h>
SoftwareSerial SUART(D2, D1); //SRX = D2, STX = D1

void setup() {
  Serial.begin(9600);
  SUART.begin(9600);
}

void loop() {
  byte n = SUART.available();
  if (n != 0) {
    char y = SUART.read();
    Serial.print(y);
  }
}
