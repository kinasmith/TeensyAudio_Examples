/*
  This is a simple WAV file Player example for the Teensy3.1 and its Audio shield.

  It requires the audio shield:
    http://www.pjrc.com/store/teensy3_audio.html
  and the Audio Library
    http://www.pjrc.com/teensy/td_libs_Audio.html
  and an SD card (see above link for recommendations).

  The data files to put on your SD card are in the /samples folder
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
/*--------------SETUP SYSTEM CONNECTIONS--------------*/
// GUItool: begin automatically generated code
AudioPlaySdWav           playWav1;       //xy=154,78
AudioOutputI2S           i2s1;           //xy=334,89
AudioConnection          patchCord1(playWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=240,153
// GUItool: end automatically generated code

void setup() {
  Serial.begin(9600);
  AudioMemory(5); //see http://www.pjrc.com/teensy/td_libs_AudioConnection.html for reference
  sgtl5000_1.enable(); //enables audio shield
  sgtl5000_1.volume(0.5); //sets output volume
  //set SPI pins
  SPI.setMOSI(7);
  SPI.setSCK(14);
  //Setup SD card
  if (!(SD.begin(10))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void loop() {
  //Play files
  playFile("test01.WAV");
  delay(500);
  playFile("test02.WAV");
  delay(500);
  playFile("test03.WAV");
  delay(500);
  playFile("test04.WAV");
  delay(1500);
}

void playFile(const char *filename) {
  Serial.print("Playing file: ");
  Serial.println(filename);
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);
  // A brief delay for the library read WAV info
  delay(5);
  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
  }
}


