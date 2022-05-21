#include <Arduino.h>
#include <SoftwareSerial.h>

#define DUT_TX 12 // D6
#define DUT_RX 14 // D5

#define POW1 4 // D2
#define POW2 5 // D1

#define NRST 13 // D7
#define BOOT0 0 // D3

SoftwareSerial DutSerial;

inline void IRAM_ATTR DutPowerOn() {
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, ((1 << POW1) | (1 << POW2)));
}

inline void IRAM_ATTR DutPowerOff() {
  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ((1 << POW1) | (1 << POW2)));
}

void setup() {
  pinMode(POW1, OUTPUT);
  pinMode(POW2, OUTPUT);
  pinMode(BOOT0, OUTPUT);
  pinMode(NRST, INPUT);
  digitalWrite(BOOT0, 1);
  DutPowerOn();

  Serial.begin(9600);
  DutSerial.begin(9600, SWSERIAL_8N1, DUT_RX, DUT_TX, false);
  Serial.println("ESP ready");
}

boolean haveReadChar = false;
boolean alreadyGlitched = false;

void serialParsingLoop() {
  while(DutSerial.available() > 0) {
    Serial.write(DutSerial.read());
  }
  while(Serial.available() > 0) {
    haveReadChar = true;
    DutSerial.write(Serial.read());
  }
}

void glitch() {
  uint32_t startCycles = ESP.getCycleCount();
  DutPowerOff();
  while(digitalRead(NRST));
  DutPowerOn();
  uint32_t cycles = ESP.getCycleCount() - startCycles;
  Serial.printf("Glitched in %d cycles\n", cycles);
  delay(1000);
  digitalWrite(BOOT0, 0);
  pinMode(NRST, OUTPUT);
  digitalWrite(NRST, 0);
  delay(500);
  pinMode(NRST, INPUT);
}

void loop() {
  serialParsingLoop();
  if (alreadyGlitched || !haveReadChar) {
    return;
  }
  alreadyGlitched = true;
  glitch();
} 

