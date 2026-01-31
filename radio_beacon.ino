#include <RCSwitch.h>
RCSwitch Transmitter;

#define BUTTON_PIN 2

void setup() {
  Transmitter.enableTransmit(10);
  Transmitter.setProtocol(1);
  Transmitter.setPulseLength(350);
  Transmitter.setRepeatTransmit(5);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  byte typeId = 0b01;
  byte tableId = 2;
  if (digitalRead(BUTTON_PIN) == LOW) {
    byte taskId  = 2;
    unsigned long packet = (typeId << 8) | (tableId << 2) | taskId;
    Transmitter.send(packet, 10);
    delay(200);
  }

  delay(100);
}
