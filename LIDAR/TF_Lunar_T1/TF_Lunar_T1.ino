#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

uint8_t buf[9];

void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);
}

void loop() {
  if (mySerial.available()) {
    if (mySerial.read() == 0x59) {
      if (mySerial.read() == 0x59) {

        for (int i = 0; i < 7; i++) {
          buf[i] = mySerial.read();
        }

        int distance = buf[0] + buf[1] * 256;

        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
      }
    }
  }
}