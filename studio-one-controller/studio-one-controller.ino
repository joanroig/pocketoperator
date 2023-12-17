/*

   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015

   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>



   http://www.arduino.cc/en/Tutorial/MidiDevice

*/

#include "Keyboard.h"
#include "MIDIUSB.h"
#include "PitchToNote.h"
#define NUM_BUTTONS  16

// See digital Arduino pinout here https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/hardware-overview-pro-micro
const int button1 = 1;
const int button2 = 2;
const int button3 = 3;
const int button4 = 4;
const int button5 = 5;
const int button6 = 6;
const int button7 = 7;
const int button8 = 8;
const int button9 = 9;
const int button10 = 10;
const int button11 = 18;
const int button12 = 19;
const int button13 = 20;
const int button14 = 14;
const int button15 = 15;
const int button16 = 16;

const int intensityPot = 0;  //A0 input

const int buttons[NUM_BUTTONS] = { button1, button2, button3, button4, button5, button6, button7, button8, button9, button10, button11, button12, button13, button14, button15, button16};

const byte notePitches[NUM_BUTTONS] = { C3, D3b, D3, E3b, E3, F3, G3b, G3, A3b, A3, B3b, B3, C4, D4b, D4, E4b};

int notesTime[NUM_BUTTONS];

int pressedButtons = 0x00;

int previousButtons = 0x00;

int intensity;

int sensorPin = PIN_A3;    // select the input pin for the analog button, IMPORTANT: use PIN_A3 as A3 is also defined in PitchToNote.h
int ledPin = 13;      // select the pin for the LED
int analogButton = 0;  // variable to store the value coming from the sensor

int playButton = 0;
int stopButton = 1;
int recordButton = 2;
int fnButton = 3;

int pressedTransport = -1; // set a default value
int previousTransport = 0;

char transportButtons[3] = {' ', ',', 221};

void setup() {
  for (int i = 0; i < NUM_BUTTONS; i++){
    pinMode(buttons[i], INPUT_PULLUP);
  }
}

void loop() {
  readButtons();
  readIntensity();
  playNotes();
}

void readButtons(){
  // Read analog
  analogButton = analogRead(sensorPin);
  delay(10);
//  Serial.println(analogButton);
  if (analogButton > 180 && analogButton < 220){
    pressedTransport = playButton;
  } else if (analogButton > 221 && analogButton < 300){
    pressedTransport = stopButton;
  } else if (analogButton > 301 && analogButton < 400){
    pressedTransport = recordButton;
  } else if (analogButton > 401 && analogButton < 600){
    pressedTransport = fnButton;
  } else{
    pressedTransport = -1;
  }

  // Read digital
  for (int i = 0; i < NUM_BUTTONS; i++){
    if (digitalRead(buttons[i]) == LOW){
      bitWrite(pressedButtons, i, 1);
      delay(10);
    }else 
      bitWrite(pressedButtons, i, 0);
  }
}

void readIntensity(){
  // int val = analogRead(intensityPot);
  // intensity = (uint8_t) (map(val, 0, 1023, 0, 127));
  intensity = 127;
}

void playNotes(){
  // Send transport MIDI
  if(previousTransport != pressedTransport){
    if(pressedTransport == fnButton){
      // undo:
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('z');
      Keyboard.releaseAll();
    } else if(pressedTransport != -1){
//      Serial.println(pressedTransport);
      Keyboard.write(transportButtons[pressedTransport]);
    }
    previousTransport = pressedTransport;
  }
  
  // Play MIDI notes
  for (int i = 0; i < NUM_BUTTONS; i++){
    if (bitRead(pressedButtons, i) != bitRead(previousButtons, i)){
      if (bitRead(pressedButtons, i)){
        bitWrite(previousButtons, i , 1);
        noteOn(0, notePitches[i], intensity);
        MidiUSB.flush();
      } 
      else{
        bitWrite(previousButtons, i , 0);
        noteOff(0, notePitches[i], 0);
        MidiUSB.flush();
      }
    }
  }
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
