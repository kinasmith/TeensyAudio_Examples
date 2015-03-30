//INIT Libraries
#include <Bounce.h>
#include <Encoder.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

//Initialize Audio Library Objects
AudioSynthWaveformSine   mod1_sine;
AudioSynthWaveform       mod1_saw;
AudioMixer4              mod1_mix;
AudioEffectMultiply      mod1_mult;
AudioSynthWaveformDc     mod1_dc;
AudioSynthWaveformSineModulated car1_sine;  
AudioEffectMultiply      car1_mult;
AudioSynthWaveformDc     car1_dc;

AudioSynthWaveformSine   mod2_sine;
AudioSynthWaveform       mod2_square;
AudioMixer4              mod2_mix;
AudioEffectMultiply      mod2_mult;
AudioSynthWaveformDc     mod2_dc;
AudioSynthWaveformSineModulated car2_sine; 
AudioEffectMultiply      car2_mult;
AudioSynthWaveformDc     car2_dc;


AudioMixer4              mixer;
AudioOutputI2S           out;
AudioControlSGTL5000     audioShield;

//Set Audio Routes
//---------- Oscillator 1 ----------------//
AudioConnection          osc1_p0(mod1_sine,0, mod1_mix,0); //modulators to mix
AudioConnection          osc1_p1(mod1_saw,0, mod1_mix,1); 
AudioConnection          osc1_p2(mod1_mix,0, mod1_mult,0); //mix to mult
AudioConnection          osc1_p3(mod1_dc,0, mod1_mult,1); //dc to mult (ctrl)
AudioConnection          osc1_p4(mod1_mult,0, car1_sine,0); //mod to carrier
AudioConnection          osc1_p5(car1_sine,0, car1_mult,0); //carrier to mult
AudioConnection          osc1_p6(car1_dc,0, car1_mult,1);  //dc to mult (ctrl)
AudioConnection          osc1_p7(car1_mult,0, mixer,0);  //carrier to main mix
//---------- Oscillator 2 ----------------//
AudioConnection          osc2_p0(mod2_sine,0, mod2_mix,0);
AudioConnection          osc2_p1(mod2_square,0, mod2_mix,1);
AudioConnection          osc2_p2(mod2_mix,0, mod2_mult,0);
AudioConnection          osc2_p3(mod2_dc,0, mod2_mult,1);
AudioConnection          osc2_p4(mod2_mult,0, car2_sine,0);
AudioConnection          osc2_p5(car2_sine,0, car2_mult,0);
AudioConnection          osc2_p6(car2_dc,0, car2_mult,1);
AudioConnection          osc2_p7(car2_mult,0, mixer,1);

AudioConnection          out_p1(mixer, 0, out, 0);
AudioConnection          out_p2(mixer, 0, out, 1);

//Declare encoders and position holder variables
Encoder enc0(28, 29);
long pos0 = 0; 
long _pos0 = 0;
int pos0_counter = 0;
int pos0_state = 0;
int pos0_last_state = 0;
int switch0Pin = 5;
Bounce switch0 = Bounce(switch0Pin, 5); 

Encoder enc1(27, 30);
long pos1 = 0;
long _pos1 = 0;
int pos1_counter = 0;
int pos1_state = 0;
int pos1_last_state;
int switch1Pin = 4;
Bounce switch1 = Bounce(switch1Pin, 5); 

Encoder enc2(26, 31);
long pos2 = 0;
long _pos2 = 0;
int pos2_counter = 0;
int pos2_state = 0;
int pos2_last_state;
int switch2Pin = 3;
Bounce switch2 = Bounce(switch2Pin, 5); 

Encoder enc3(7, 6);
long pos3 = 0;
long _pos3 = 0;
int pos3_counter = 0;
int pos3_state = 0;
int pos3_last_state;
int switch3Pin = 2;
Bounce switch3 = Bounce(switch3Pin, 5); 
int rot = 2.5; //number of rotations on control encoders. 96 (ppr) * 2.5 = 240

Encoder encFreq(25,24);
long posFreq = 0;
long oldPosFreq = 0;

//Global Synthesis Variables
float master_volume = 0.5;

//---------- Oscillator 1 ----------------//
float osc1_freq; //Master OSC frequency
float osc1_midi; //Master OSC MIDI
float mod1_dc_val; //Modulator Amplitude (controlled by pS2
float mod1_amplitude = 3;
float mod1_freq; //Modulator Frequency (a function of osc1Freq)
float mod1_mix_pos = 0.5;

//---------- Oscillator 2 ----------------//
float osc2_freq = 110.0; //Osc2 Frequency
float osc2_midi = 20; //Osc2 MIDI
float mod2_dc_val; //Modulator Amplitude (controlled by pS2
float mod2_amplitude = 3;
float mod2_freq; //Modulator Frequency (a function of osc1Freq)
float mod2_mix_pos = 0.5;

float osc2_offset = 0; //offset between OSC1 and OSC2 when in FollowMode
int osc2_follow = 1; //boolean for pitch Following

//---------- OSC1 Control Inputs ----------------//
int osc1_amp_pin = A13; //amplitude for OSC1
float osc1_amp_val; //amp1Read
float _osc1_amp_val; //oldAmp1
float osc1_amp; //amp1
int pS1_pin = A6; //main pressure strip Pin
float pS1; //Pressure Strips
float _pS1;

//---------- OSC2 Control Inputs ----------------//
int osc2_amp_pin = A12; //amplitude for OSC2
float osc2_amp_val;
float osc2_amp; 
float _osc2_amp_val;
int pS2_pin = A7; //main pressure strip Pin
float pS2; //Pressure Strips
float _pS2;

//---------- LED Control ----------------//
char colors[] = {
  'r', 'g', 'b'};

int e0r_val, e0g_val, e0b_val,
e1r_val, e1g_val, e1b_val,
e2r_val, e2g_val, e2b_val,
e3r_val, e3g_val, e3b_val;

int presetNumber = 0;
int presetWritten = 0;
long switch0_timer = 0;


void setup(void) {
  Serial1.begin(9600);
  //Setup Switch Pin Modes
  pinMode(switch0Pin, INPUT);
  pinMode(switch1Pin, INPUT);
  pinMode(switch2Pin, INPUT);
  pinMode(switch3Pin, INPUT);

  AudioMemory(120);
  audioShield.enable();
  audioShield.volume(0.6);

  //---------- Setup OSC1 ----------------//
  mod1_sine.amplitude(0.7);
  mod1_saw.begin(0.7, 0, WAVEFORM_SAWTOOTH);
  mixer.gain(0, 0.5);

  //---------- Setup OSC2 ----------------//  
  mod2_sine.amplitude(0.7);
  mod2_square.begin(0.7, 0, WAVEFORM_SQUARE);
  car2_sine.amplitude(0.5);
  mixer.gain(1, 0.5);

  //---------- Load-up Preset Values ----------------// 
  recallPreset(presetNumber);
}

void loop() {
  //---------- Switch 0 Functions ----------------// 
  switch0.update();
  pos0_state = switch0.read();
  if(pos0_state != pos0_last_state) {
    if(pos0_state == HIGH) {
      switch0_timer = millis();
    }
  }
  if(pos0_state == HIGH && millis() - switch0_timer > 1000) {
    if(!presetWritten) {
      writePreset(presetNumber);
      presetWritten = 1;
    }
  }
  if(pos0_state == LOW && pos0_last_state == HIGH) {
    if(!presetWritten) {
      recallPreset(presetNumber);
    }
    presetWritten = 0;
  }
  pos0_last_state = pos0_state;

  //---------- Switch 1 Functions ----------------// 
  //set values on switches
  switch1.update();
  pos1_state = switch1.read();
  if(pos1_state != pos1_last_state) {
    if(pos1_state == HIGH) {
      pos1_counter++;
      pos1_counter %= 3;
      switch(pos1_counter) {
      case 0:
        enc1.write(e1r_val);
        ledSetEnc(1, map(e1r_val, 0, 96*rot, 0, 4095), 0, 0);
        break;
      case 1:
        enc1.write(e1g_val);
        ledSetEnc(1, 0, map(e1g_val, 0, 96*rot, 0, 4095), 0);
        break;
      case 2:
        enc1.write(e1b_val);
        ledSetEnc(1, 0, 0, map(e1b_val, 0, 96*rot, 0, 4095));
        break;
      }
    }
  }
  pos1_last_state = pos1_state;

  //---------- Switch 2 Functions ----------------// 
  switch2.update();
  pos2_state = switch2.read();
  if(pos2_state != pos2_last_state) {
    if(pos2_state == HIGH) {
      pos2_counter++;
      pos2_counter %= 3;
      switch(pos2_counter) {
      case 0:
        enc2.write(e2r_val);
        ledSetEnc(2, map(e2r_val, 0, 96*rot, 0, 4095), 0, 0);
        break;
      case 1:
        enc2.write(e2g_val);
        ledSetEnc(2, 0, map(e2g_val, 0, 96*rot, 0, 4095), 0);
        break;
      case 2:
        enc2.write(e2b_val);
        ledSetEnc(2, 0, 0, map(e2b_val, 0, 96*rot, 0, 4095));
        break;
      }
    }
  }
  pos2_last_state = pos2_state;

  //---------- Switch 3 Functions ----------------// 
  switch3.update();
  pos3_state = switch3.read();
  if(pos3_state != pos3_last_state) {
    if(pos3_state == HIGH){
      pos3_counter++;
      pos3_counter %= 2;
      osc2_follow = pos3_counter;
      switch (pos3_counter) {
      case 0:
        enc3.write(e3r_val);
        ledSetEnc(3, map(e3r_val, 0, 96*rot, 0, 4095), abs(map(e3r_val, 0, 96*rot, 0, 4095)-4095), 0);
        break;
      case 1:
        enc3.write(e3b_val);
        ledSetEnc(3, 0, abs(map(e3b_val, 0, 96*rot, 0, 4095)-4095), map(e3b_val, 0, 96*rot, 0, 4095));
        break;
      }

    }
  }
  pos3_last_state = pos3_state;

  //set root Pitch
  if(switch1.read() && switch2.read()) {
    encFreq.write(0); 
  }

  //---------- Pressure Strip Inputs ----------------// 
  pS1 = analogRead(pS1_pin); 
  if(pS1 != _pS1) {
    _pS1 = pS1;
    mod1_dc_val = linexp(pS1, 100, 900, 0.2, 3);
  }
  pS2 = analogRead(pS2_pin); 
  if(pS2 != _pS2) {
    _pS2 = pS2;
    mod2_dc_val = linexp(pS2, 100, 900, 0.2, 3);
  }
  //---------- Amplitude Control Inputs ----------------// 
  osc1_amp_val = analogRead(osc1_amp_pin);
  if(osc1_amp_val != _osc1_amp_val) {
    _osc1_amp_val = osc1_amp_val;
    osc1_amp = constrain((linexp(osc1_amp_val, 40.0, 700, 0.1, 1)-0.1), 0, 1);
  }

  osc2_amp_val = analogRead(osc2_amp_pin);
  if(osc2_amp_val != _osc2_amp_val) {
    _osc2_amp_val = osc2_amp_val;
    osc2_amp = constrain((linexp(osc2_amp_val, 40.0, 700, 0.1, 1)-0.1), 0, 1);
  }
  //---------- Encoder 0 Functions ----------------// 
  pos0 = enc0.read();
  if(pos0 < 0) enc0.write(0);
  if(pos0 > 64) enc0.write(64); 
  if(pos0 != _pos0) {
    _pos0 = pos0;
    e0r_val = pos0;
    presetNumber = int(e0r_val/16);
    switch(presetNumber) {
    case 0:
      ledSetEnc(0, 4095, 0, 0);
      break;
    case 1:
      ledSetEnc(0, 0, 4095, 0);
      break;
    case 2:
      ledSetEnc(0, 0, 0, 4095);
      break;
    case 3:
      ledSetEnc(0, 4095, 4095, 4095);
      break;
    case 4:
      ledSetEnc(0, 0, 4095, 4095);
      break;
    case 5:
      ledSetEnc(0, 4095, 4095, 4095);
      break;
    case 6:
      ledSetEnc(0, 0, 0, 4095);
      break;
    case 7:
      ledSetEnc(0, 4095, 0, 4095);
      break;
    case 8:
      ledSetEnc(0, 4095, 4095, 4095);
      break;
    }
  }
  //---------- Encoder 1 Functions ----------------// 
  //red is crossfade values from 0 to 1
  //green is modulator frequency values from 0 to 2
  //blue is modulator amplitude values from 0 to 3
  pos1 = enc1.read();
  if(pos1 < 0) enc1.write(0);
  if(pos1 > 96*rot) enc1.write(96*rot);
  if(pos1 != _pos1) {
    _pos1 = pos1;
    switch (pos1_counter) {
    case 0:
      e1r_val = pos1;
      //Set crossfade for OSC1_Modulation
      mod1_mix_pos = linlin(e1r_val, 0, 96*rot, 0, 1); //map values
      mod1_mix.gain(0, mod1_mix_pos*mod1_amplitude); //set values
      mod1_mix.gain(1, abs(mod1_mix_pos-1)*mod1_amplitude);
      ledSetEnc(1, map(e1r_val, 0, 96*rot, 0, 4095), 0, 0);
      break;
    case 1:
      e1g_val = pos1;
      mod1_freq = linlin(e1g_val, 0, 96*rot, 0, 2);
      ledSetEnc(1, 0, map(e1g_val, 0, 96*rot, 0, 4095), 0);
      break;
    case 2:
      e1b_val = pos1;
      mod1_amplitude = linlin(e1b_val, 0, 96*rot, 0, 3);
      mod1_mix.gain(0, mod1_mix_pos*mod1_amplitude); //set values
      mod1_mix.gain(1, abs(mod1_mix_pos-1)*mod1_amplitude);
      ledSetEnc(1, 0, 0, map(e1b_val, 0, 96*rot, 0, 4095));
      break;
    }
  }
  //---------- Encoder 2 Functions ----------------// 
  pos2 = enc2.read();
  if(pos2 < 0) enc2.write(0);
  if(pos2 > 96*rot) enc2.write(96*rot);
  if(pos2 != _pos2) {
    _pos2 = pos2;
    switch (pos2_counter) {
    case 0:
      e2r_val = pos2;
      mod2_mix_pos = linlin(e2r_val, 0, 96*rot, 0, 1);
      mod2_mix.gain(0, mod2_mix_pos*mod2_amplitude);
      mod2_mix.gain(1, abs(mod2_mix_pos-1)*mod2_amplitude);
      ledSetEnc(2, map(e2r_val, 0, 96*rot, 0, 4095), 0, 0);
      break;
    case 1:
      e2g_val = pos2;
      mod2_freq = linlin(e2g_val, 0, 96*rot, 0, 2);
      ledSetEnc(2, 0, map(e2g_val, 0, 96*rot, 0, 4095), 0);
      break;
    case 2:
      e2b_val = pos2;
      mod2_amplitude = linlin(e2b_val, 0, 96*rot, 0, 1.5);
      mod2_mix.gain(0, mod2_mix_pos*mod2_amplitude);
      mod2_mix.gain(1, abs(mod2_mix_pos-1)*mod2_amplitude);
      ledSetEnc(2, 0, 0, map(e2b_val, 0, 96*rot, 0, 4095));
      break;

    }

  }
  //---------- Encoder 3 Functions ----------------// 
  pos3 = enc3.read();
  if(pos3 < 0) enc3.write(0);
  if(pos3 > 96*rot) enc3.write(96*rot);
  if(pos3 != _pos3) {
    _pos3 = pos3;
    switch (pos3_counter) {
    case 0:
      e3r_val = pos3;
      osc2_midi = linlin(e3r_val, 0, 96*rot, 20, 100);
      ledSetEnc(3, map(e3r_val, 0, 96*rot, 0, 4095),abs(map(e3r_val, 0, 96*rot, 0, 4095)-4095), 0);
      break;
    case 1:
      e3b_val = pos3;
      osc2_offset = linlin(e3b_val, 0, 96*rot, 0, 24) - 12;
      ledSetEnc(3, 0, abs(map(e3b_val, 0, 96*rot, 0, 4095)-4095), map(e3b_val, 0, 96*rot, 0, 4095));
      break;
    }
  }



  //---------- Encoder FREQUENCY Functions ----------------// 
  posFreq = encFreq.read();
  osc1_midi = linlin(posFreq, 0, 4096*12, 48, 100); //CHANGE: was 36, 100
  osc1_freq = midicps(osc1_midi);


  if(osc2_follow) {
    osc2_midi = osc1_midi+osc2_offset;
    osc2_freq = midicps(osc2_midi);
  } 
  else {
    osc2_freq = midicps(osc2_midi);
  }

  //---------- Synthesis ----------------//
  car1_sine.frequency(osc1_freq);
  mod1_sine.frequency(osc1_freq*mod1_freq);
  mod1_saw.frequency(osc1_freq*mod1_freq);
  mod1_dc.amplitude((osc1_amp*mod1_dc_val), 1);
  car1_dc.amplitude(osc1_amp, 1);

  car2_sine.frequency(osc2_freq);
  mod2_sine.frequency(osc2_freq*mod2_freq);
  mod2_square.frequency(osc2_freq*mod2_freq);
  mod2_dc.amplitude(osc2_amp*mod2_dc_val, 1);
  car2_dc.amplitude(osc2_amp, 1);

}

//---------- ---------- Utility Functions ---------------- ----------------// 
float linexp (float x, float a, float b, float c, float d)
//intput, input low, input high, output low, output high
{
  if (x <= a) return c;
  if (x >= b) return d;
  return pow(d/c, (x-a)/(b-a)) * c;
}

float linlin (float x, float a, float b, float c, float d)
{
  if (x <= a) return c;
  if (x >= b) return d;
  return (x - a) / (b - a) * (d - c) + c;
}

/*-------------------------------------------------------------------
 * midicps: Map from MIDI note to frequency
 *-------------------------------------------------------------------*/
float midicps (float note)
{
  return 440.0 * pow(2, (note - 69) / 12);
}

/*-------------------------------------------------------------------
 * cpsmidi: Map from frequency to MIDI note
 *-------------------------------------------------------------------*/
float cpsmidi (float freq)
{
  return (log(freq / 440.0) / log(2.0)) * 12 + 69;
}

void ledSetAll(int val) {
  val = abs(val - 4095);
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < sizeof colors; j++) {
      Serial1.print(i);
      Serial1.print(" ");
      Serial1.print(colors[j]);
      Serial1.print(" ");
      Serial1.print(val);
      Serial1.print(".");
    } 
  }
}

void ledSetAllColor(char col, int val){
  val = abs(val - 4095);
  for(int i = 0; i < 4; i++) {
    Serial1.print(i);
    Serial1.print(" ");
    Serial1.print(col);
    Serial1.print(" ");
    Serial1.print(val);
    Serial1.print(".");
  }
}

void ledSet(int enc, char col, int val) {
  val = abs(val - 4095);
  Serial1.print(enc);
  Serial1.print(" ");
  Serial1.print(col);
  Serial1.print(" ");
  Serial1.print(val);
  Serial1.print(".");
}

void ledSetEnc(int enc, int r, int g, int b) {
  r = abs(r - 4095);
  g = abs(g - 4095);
  b = abs(b - 4095);
  Serial1.print(enc);
  Serial1.print(" ");
  Serial1.print("r");
  Serial1.print(" ");
  Serial1.print(r);
  Serial1.print(".");

  Serial1.print(enc);
  Serial1.print(" ");
  Serial1.print("g");
  Serial1.print(" ");
  Serial1.print(g);
  Serial1.print(".");

  Serial1.print(enc);
  Serial1.print(" ");
  Serial1.print("b");
  Serial1.print(" ");
  Serial1.print(b);
  Serial1.print(".");
}

void recallPreset(int presetNum) {
  ledSetEnc(0, 0, 0, 0);
  ledSetEnc(1, 0, 0, 0);
  ledSetEnc(2, 0, 0, 0);
  ledSetEnc(3, 0, 0, 0);
  int addr = 0;
  int addressOffset = presetNum * 16;
  addr += addressOffset; 
  //recall Values
  // e0r_val = EEPROM.read(addr);
  addr++;
  // e0g_val = EEPROM.read(addr);
  addr++;
  // e0b_val = EEPROM.read(addr);
  addr++;
  e1r_val = EEPROM.read(addr);
  addr++;
  e1g_val = EEPROM.read(addr);
  addr++;
  e1b_val = EEPROM.read(addr);
  addr++;
  e2r_val = EEPROM.read(addr);
  addr++;
  e2g_val = EEPROM.read(addr);
  addr++;
  e2b_val = EEPROM.read(addr);
  addr++;
  e3r_val = EEPROM.read(addr);
  addr++;
  e3g_val = EEPROM.read(addr);
  addr++;
  e3b_val = EEPROM.read(addr);
  addr++;
  pos0_counter = EEPROM.read(addr);
  addr++;
  pos1_counter = EEPROM.read(addr);
  addr++;
  pos2_counter = EEPROM.read(addr);
  addr++;
  pos3_counter = EEPROM.read(addr);
  addr++;
  osc2_follow = pos3_counter;

  //push Values
  //------------------ENCODER 0-----------------------
  switch(presetNum) {
  case 0:
    ledSetEnc(0, 4095, 0, 0);
    break;
  case 1:
    ledSetEnc(0, 0, 4095, 0);
    break;
  case 2:
    ledSetEnc(0, 0, 0, 4095);
    break;
  case 3:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  case 4:
    ledSetEnc(0, 0, 4095, 4095);
    break;
  case 5:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  case 6:
    ledSetEnc(0, 0, 0, 4095);
    break;
  case 7:
    ledSetEnc(0, 4095, 0, 4095);
    break;
  case 8:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  }
  //------------------ENCODER 1-----------------------
  mod1_mix_pos = linlin(e1r_val, 0, 96*rot, 0, 1); //map values
  mod1_mix.gain(0, mod1_mix_pos*mod1_amplitude); //set values
  mod1_mix.gain(1, abs(mod1_mix_pos-1)*mod1_amplitude);
  ledSetEnc(1, map(e1r_val, 0, 96*rot, 0, 4095), 0, 0);

  mod1_freq = linlin(e1g_val, 0, 96*rot, 0, 2);
  ledSetEnc(1, 0, map(e1g_val, 0, 96*rot, 0, 4095), 0);

  mod1_amplitude = linlin(e1b_val, 0, 96*rot, 0, 3);
  mod1_mix.gain(0, mod1_mix_pos*mod1_amplitude); //set values
  mod1_mix.gain(1, abs(mod1_mix_pos-1)*mod1_amplitude);
  ledSetEnc(1, 0, 0, map(e1b_val, 0, 96*rot, 0, 4095));

  switch (pos1_counter) {
  case 0:
    enc1.write(e1r_val);
    break;
  case 1:
    enc1.write(e1g_val);
    break;
  case 2:
    enc1.write(e1b_val);
    break;
  }


  //------------------ENCODER 2-----------------------
  mod2_mix_pos = linlin(e2r_val, 0, 96*rot, 0, 1);
  mod2_mix.gain(0, mod2_mix_pos*mod2_amplitude);
  mod2_mix.gain(1, abs(mod2_mix_pos-1)*mod2_amplitude);
  ledSetEnc(2, map(e2r_val, 0, 96*rot, 0, 4095), 0, 0);

  mod2_freq = linlin(e2g_val, 0, 96*rot, 0, 2);
  ledSetEnc(2, 0, map(e2g_val, 0, 96*rot, 0, 4095), 0);

  mod2_amplitude = linlin(e2b_val, 0, 96*rot, 0, 1.5);
  mod2_mix.gain(0, mod2_mix_pos*mod2_amplitude);
  mod2_mix.gain(1, abs(mod2_mix_pos-1)*mod2_amplitude);
  ledSetEnc(2, 0, 0, map(e2b_val, 0, 96*rot, 0, 4095));

  switch (pos2_counter) {
  case 0:
    enc2.write(e2r_val);
    break;
  case 1:
    enc2.write(e2g_val);
    break;
  case 2:   
    enc2.write(e2b_val);
    break;
  }

  //------------------ENCODER 3-----------------------
  osc2_midi = linlin(e3r_val, 0, 96*rot, 20, 100);
  ledSetEnc(3, map(e3r_val, 0, 96*rot, 0, 4095),abs(map(e3r_val, 0, 96*rot, 0, 4095)-4095), 0);

  osc2_offset = linlin(e3b_val, 0, 96*rot, 0, 24) - 12;
  ledSetEnc(3, 0, abs(map(e3b_val, 0, 96*rot, 0, 4095)-4095), map(e3b_val, 0, 96*rot, 0, 4095));

  switch (pos3_counter) {
  case 0:
    enc3.write(e3r_val);
    break;
  case 1:
    enc3.write(e3b_val);
    break;
  }
  //Write Encoder Positions
  //enc0.write(e0r_val);
  //enc1.write(e1r_val);
  // enc2.write(e2r_val);
  // enc3.write(e3r_val);

  Serial.print("enc1r: ");
  Serial.println(e1r_val);
  Serial.print("enc1g: ");
  Serial.println(e1g_val);
  Serial.print("enc1b: ");
  Serial.println(e1b_val);
  Serial.print("enc2r: ");
  Serial.println(e2r_val);
  Serial.print("enc2g: ");
  Serial.println(e2g_val);
  Serial.print("enc2b: ");
  Serial.println(e2b_val);
  Serial.print("enc3r: ");
  Serial.println(e3r_val);
  Serial.print("enc3g: ");
  Serial.println(e3g_val);
  Serial.print("enc3b: ");
  Serial.println(e3b_val);
  Serial.print("pos0cnt: ");
  Serial.println(pos0_counter);
  Serial.print("pos1cnt: ");
  Serial.println(pos1_counter);
  Serial.print("pos2cnt: ");
  Serial.println(pos2_counter);
  Serial.print("pos3cnt: ");
  Serial.println(pos3_counter);
  Serial.print("addr: ");
  Serial.println(addr);
  Serial.println("--------------------------------");
}

void writePreset(int presetNum) {
  ledSetEnc(0, 0, 0, 0);
  delay(100);
  int addressOffset = presetNum * 16;
  writeEEPROM(addressOffset);
  switch(presetNum) {
  case 0:
    ledSetEnc(0, 4095, 0, 0);
    break;
  case 1:
    ledSetEnc(0, 0, 4095, 0);
    break;
  case 2:
    ledSetEnc(0, 0, 0, 4095);
    break;
  case 3:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  case 4:
    ledSetEnc(0, 0, 4095, 4095);
    break;
  case 5:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  case 6:
    ledSetEnc(0, 0, 0, 4095);
    break;
  case 7:
    ledSetEnc(0, 4095, 0, 4095);
    break;
  case 8:
    ledSetEnc(0, 4095, 4095, 4095);
    break;
  }

}

void writeEEPROM(int addrOffset) {
  int addr = 0;
  addr += addrOffset; 
  // EEPROM.write(addr, e0r_val);
  addr++;
  // EEPROM.write(addr, e0g_val);
  addr++;
  // EEPROM.write(addr, e0b_val);
  addr++;
  EEPROM.write(addr, e1r_val);
  addr++;
  EEPROM.write(addr, e1g_val);
  addr++;
  EEPROM.write(addr, e1b_val);
  addr++;
  EEPROM.write(addr, e2r_val);
  addr++;
  EEPROM.write(addr, e2g_val);
  addr++;
  EEPROM.write(addr, e2b_val);
  addr++;
  EEPROM.write(addr, e3r_val);
  addr++;
  EEPROM.write(addr, e3g_val);
  addr++;
  EEPROM.write(addr, e3b_val);
  addr++;
  EEPROM.write(addr, pos0_counter);
  addr++;
  EEPROM.write(addr, pos1_counter);
  addr++;
  EEPROM.write(addr, pos2_counter);
  addr++;
  EEPROM.write(addr, pos3_counter);
  addr++;
  Serial.print("enc1r: ");
  Serial.println(e1r_val);
  Serial.print("enc1g: ");
  Serial.println(e1g_val);
  Serial.print("enc1b: ");
  Serial.println(e1b_val);
  Serial.print("enc2r: ");
  Serial.println(e2r_val);
  Serial.print("enc2g: ");
  Serial.println(e2g_val);
  Serial.print("enc2b: ");
  Serial.println(e2b_val);
  Serial.print("enc3r: ");
  Serial.println(e3r_val);
  Serial.print("enc3g: ");
  Serial.println(e3g_val);
  Serial.print("enc3b: ");
  Serial.println(e3b_val);
  Serial.print("pos0cnt: ");
  Serial.println(pos0_counter);
  Serial.print("pos1cnt: ");
  Serial.println(pos1_counter);
  Serial.print("pos2cnt: ");
  Serial.println(pos2_counter);
  Serial.print("pos3cnt: ");
  Serial.println(pos3_counter);
  Serial.print("addr: ");
  Serial.println(addr);
  Serial.println("--------------------------------");
}
























