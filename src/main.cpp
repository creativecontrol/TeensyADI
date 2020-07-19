#include <Arduino.h>
#include <Adafruit_MCP3008.h>
#include <MCP23017_multi.h>
#include <ResponsiveAnalogRead.h>

#define DEBUG false

#define CS1_PIN 9
#define CS2_PIN 10
#define CLOCK_PIN 13
#define MOSI_PIN 11
#define MISO_PIN 12

#define SCL_A 19
#define SDA_A 18
#define DIGITAL1_SELA 20
#define DIGITAL1_SELB 21

#define SCL_B 16
#define SDA_B 17
#define DIGITAL2_SELA 22
#define DIGITAL2_SELB 23

#define MIDI_CHANNEL 3

// Starting positions for the MIDI controllers 
#define MIDICCChannel_A1 11
#define MIDICCChannel_A2 21
#define MIDICCChannel_A3 31

#define MIDICCChannel_D1 41  // 41 - 56
#define MIDICCChannel_D2 61  // 61 - 76

Adafruit_MCP3008 analog1;
Adafruit_MCP3008 analog2;
// Adafruit_MCP3008 analog3;  // This is reserverd for the next revision

MCP23017_multi digital1;
MCP23017_multi digital2;

#define numAnalog1 8
#define numAnalog2 8
// #define numAnalog3 8
#define numDigital1 16
#define numDigital2 16

ResponsiveAnalogRead analog1Vals[numAnalog1];
ResponsiveAnalogRead analog2Vals[numAnalog2];
// ResponsiveAnalogRead analog3Vals[numAnalog3];

// Analog scaling
#define HIGH_VAL 511
#define LOW_VAL 213

#define HIGH_TARGET 130
#define LOW_TARGET 0

int digital1Vals[numDigital1];
int digital2Vals[numDigital2];

int last;
int current;


void setup() {

  // Analog setup
  pinMode(CS1_PIN, OUTPUT);
  pinMode(CS2_PIN, OUTPUT);
  analog1.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS1_PIN);
  analog2.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS2_PIN);

  for(int i = 0; i<numAnalog1; i++) {
    analog1Vals[i].begin(0,true);
  }
  for(int i = 0; i<numAnalog2; i++) {
    analog2Vals[i].begin(0,true);
  }

  // Digital setup
  pinMode(DIGITAL1_SELA, OUTPUT);
  pinMode(DIGITAL1_SELB, OUTPUT);
  pinMode(DIGITAL2_SELA, OUTPUT);
  pinMode(DIGITAL2_SELB, OUTPUT);

  pinMode(SCL_B, INPUT_PULLUP);
  pinMode(SDA_B, INPUT_PULLUP);

  digital1.begin(0);
  digital2.begin(Wire1, 1, SCL_B, SDA_B);

  for (int i=0; i<numDigital1; i++) {
    digital1.pinMode(i, INPUT);
    digital1.pullUp(i, HIGH);
    digital1Vals[i] = 5;  // start with an invalid number to force the state of the Digital pins to be read and sent
  }
  for (int i=0; i<numDigital2; i++) {
    digital2.pinMode(i, INPUT);
    digital2.pullUp(i, HIGH);
    digital1Vals[i] = 5;  // start with an invalid number to force the state of the Digital pins to be read and sent
  }

  usbMIDI.begin();

}


int scaleReading(int reading) {
  return int(map(reading, LOW_VAL, HIGH_VAL, LOW_TARGET, HIGH_TARGET));
}

void loop() {

  // Analog 1
  for (int i=0; i<numAnalog1; i++) {
    int reading = scaleReading(analog1.readADC(i));
    
    if (DEBUG) {
      Serial.print("Analog 1 ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(reading);
    }
    analog1Vals[i].update(reading);
    usbMIDI.sendControlChange(MIDICCChannel_A1+i, analog1Vals[i].getValue(), MIDI_CHANNEL);
    delay(10);
  }

  // Analog 2
  for (int i=0; i<numAnalog2; i++) {
    int reading = scaleReading(analog2.readADC(i));
    
    if (DEBUG) {
      Serial.print("Analog 2 ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(reading);
    }
    analog2Vals[i].update(reading);
    usbMIDI.sendControlChange(MIDICCChannel_A2+i, analog2Vals[i].getValue(), MIDI_CHANNEL);
    delay(10);
  }

  // Digital 1

  for (int i=0; i<numDigital1; i++) {
    last = digital1Vals[i];
    current = abs(127-(digital1.digitalRead(i)*127));

    if (current != last){
      digital1Vals[i] = current; 
      if (DEBUG) {
        Serial.print("Digital 1 ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(digital1Vals[i]);
      }
      usbMIDI.sendControlChange(MIDICCChannel_D1+i, digital1Vals[i], MIDI_CHANNEL);
    }
  }

  for (int i=0; i<numDigital2; i++) {
      last = digital2Vals[i];
      current = abs(127-(digital2.digitalRead(i)*127));;

      if (current != last){
        digital2Vals[i] = current; 
        if (DEBUG) {
          Serial.print("Digital 1 ");
          Serial.print(i);
          Serial.print(" : ");
          Serial.println(digital2Vals[i]);
        }
        usbMIDI.sendControlChange(MIDICCChannel_D2+i, digital2Vals[i], MIDI_CHANNEL);
      }
    }
  
  while (usbMIDI.read()) {
  }
}
