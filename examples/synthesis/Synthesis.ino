/*
  This is a simple Synthesis example for the Teensy3.1 and its Audio shield.

  It requires the audio shield:
    http://www.pjrc.com/store/teensy3_audio.html
  and the Audio Library
    http://www.pjrc.com/teensy/td_libs_Audio.html
  and the Bounce Library for button debouncing
    https://github.com/thomasfredericks/Bounce2

  Please reference this: http://www.pjrc.com/teensy/gui/ for function arguments and explinations

  This patch is a simple Amplitude Modulation Synthesis system.
  There are two oscillators being multiplied together and getting fed through an envelope generator and a low pass filter. 

 */


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h>


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=61.090911865234375,117.09091186523438
AudioSynthWaveform       waveform1;      //xy=67.09091186523438,79.09091186523438
AudioEffectMultiply      multiply1;      //xy=234.09091186523438,137.09091186523438
AudioEffectEnvelope      envelope1;      //xy=388.0909118652344,146.09091186523438
AudioFilterStateVariable filter1;        //xy=532.0909423828125,152.09091186523438
AudioOutputI2S           i2s1;           //xy=584.0909729003906,251.09091186523438
AudioConnection          patchCord1(sine1, 0, multiply1, 1);
AudioConnection          patchCord2(waveform1, 0, multiply1, 0);
AudioConnection          patchCord3(multiply1, envelope1);
AudioConnection          patchCord4(envelope1, 0, filter1, 0);
AudioConnection          patchCord5(filter1, 1, i2s1, 1);
AudioConnection          patchCord6(filter1, 2, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=91.09091186523438,413.0909118652344
// GUItool: end automatically generated code

//initialize each button object
Bounce button1 = Bounce();
Bounce button2 = Bounce();

//setup variables for analog inputs
int pressureSensor1Pin = 2;
int pressureSensor2Pin = 3;
float press1 = 0;
float press2 = 0;

void setup() {
    // Configure the pushbutton pins
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  //attach debouncer to buttons
  button1.attach(0);
  button2.attach(1);
  //set debounce intervals
  button1.interval(8);
  button2.interval(8);

  Serial.begin(9600);
  AudioMemory(128); //see http://www.pjrc.com/teensy/td_libs_AudioConnection.html for reference
  sgtl5000_1.enable(); //enable audio card
  sgtl5000_1.volume(1); //set volume
  waveform1.begin(0.4, 220, WAVEFORM_TRIANGLE); //begin waveform
  sine1.frequency(10); //set initial frequency for SINE wave
  //setup Envelope (values in milliSeconds)
  envelope1.attack(500);
  envelope1.decay(50);
  envelope1.release(1000);
    //setup filter (see function documentation for explinations of these values)
  filter1.resonance(4);
  filter1.octaveControl(5);
}

void loop() {
  //read all sensor and button values
  button1.update();
  button2.update();
  press1 = analogRead(pressureSensor1Pin);
  press2 = analogRead(pressureSensor2Pin);
  //set OSC 1 frequency and trigger note
  if(button1.fell()) { //when button is pressed
    waveform1.frequency(midicps(random(30,80))); //sets to a random note.
    envelope1.noteOn(); //turn note ON
  } 
  if(button1.rose()) { //when button is released
    envelope1.noteOff(); //turn note OFF
  }
  if(button2.fell()) sine1.frequency(midicps(random(1, 50))); //if presed, set OSC 2 to a random note

  press1 = abs(press1-1023.0)/1023; //invert and map analog input values (0-1023) to a usable range  
  //set Filter variables
  filter1.resonance(press1*15); //set filter resonance
  filter1.frequency(press2); //set filter frequency
}

//converts MIDI notes into Cycles/Second
float midicps (float note) {
  return 440.0 * pow(2, (note - 69) / 12);
}