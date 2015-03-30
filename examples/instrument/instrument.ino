/*
	This is an example of combining Synthesis and Sample Playback for the Teensy3.1 and its Audio shield.

	It requires the audio shield:
		http://www.pjrc.com/store/teensy3_audio.html
	and the Audio Library
		http://www.pjrc.com/teensy/td_libs_Audio.html
	and an SD card (see above link for recommendations).
	and the Bounce Library for button debouncing
		https://github.com/thomasfredericks/Bounce2
	The data files to put on your SD card are in the /samples folder

	This example uses buttons to trigger samples (up to two at a time) and another button to step through a melody played on a simple oscillator.
	All of the sounds are fed through a Low Pass Filter controlled by two pressure sensors
 */

#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

/*--------------SETUP SYSTEM CONNECTIONS--------------*/
// GUItool: begin automatically generated code
AudioPlaySdWav           playWav1;       //xy=69.09091186523438,77.09091186523438
AudioPlaySdWav           playWav2;       //xy=87.90909364400817,148.09091186523438
AudioSynthWaveform       waveform1;      //xy=97.90909364400817,230.09091186523438
AudioEffectEnvelope      envelope1;      //xy=283.9090936440082,231.09091186523438
AudioMixer4              mixer1;         //xy=318.9090936440082,65.09091186523438
AudioMixer4              mixer3;         //xy=469.9091033935547,150.09091186523438
AudioFilterStateVariable filter1;        //xy=602.0910186767578,145.5
AudioOutputI2S           i2s1;           //xy=676.0910797119141,232.09091186523438
AudioControlSGTL5000     sgtl5000_1;
AudioConnection          patchCord1(playWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playWav1, 1, mixer1, 1);
AudioConnection          patchCord3(playWav2, 0, mixer1, 2);
AudioConnection          patchCord4(playWav2, 1, mixer1, 3);
AudioConnection          patchCord5(waveform1, envelope1);
AudioConnection          patchCord6(envelope1, 0, mixer3, 1);
AudioConnection          patchCord7(mixer1, 0, mixer3, 0);
AudioConnection          patchCord8(mixer3, 0, filter1, 0);
AudioConnection          patchCord9(filter1, 2, i2s1, 0);
AudioConnection          patchCord10(filter1, 2, i2s1, 1);
// GUItool: end automatically generated code

//initialize each button object
Bounce button1 = Bounce();
Bounce button2 = Bounce();
Bounce button3 = Bounce();
Bounce button4 = Bounce();
//setup variables for analog inputs
int pressureSensor1Pin = 2;
int pressureSensor2Pin = 3;
float press1 = 0;
float press2 = 0;
//setup Melody
int melody[] = {40,35,36,38,   36,35,33,33,   36,40,38,36,   35,36,38,40,    36,33,33};
int melodyCnt = 0;

void setup() {
	// Configure the pushbutton pins
	pinMode(0, INPUT_PULLUP);
	pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	//attach debouncer button stuff
	button1.attach(0);
	button2.attach(1);
	button3.attach(2);
	button4.attach(3);
	//set debounce intervals
	button1.interval(8);
	button2.interval(8);
	button3.interval(8);
	button4.interval(8);

	Serial.begin(9600); 
	AudioMemory(128); //see http://www.pjrc.com/teensy/td_libs_AudioConnection.html for reference
	sgtl5000_1.enable(); //enable audio card
	sgtl5000_1.volume(1); //set volume
	//set SPI pins
	SPI.setMOSI(7); 
	SPI.setSCK(14);
	//Check to make sure the SD card is there
	if (!(SD.begin(10))) {
		while (1) {
			Serial.println("Unable to access the SD card");
			delay(500);
		}
	}
	//set Mixer gains medium values (this could be optimized)
	for(int i = 0; i < 4; i++) mixer1.gain(i, 0.5);
	for(int i = 0; i < 4; i++) mixer3.gain(i, 0.5);

	waveform1.begin(0.4, 220, WAVEFORM_TRIANGLE); //begin oscillator1
	//setup Envelope
	envelope1.attack(50);
	envelope1.decay(50);
	envelope1.release(1000);
	//setup filter (see documentation for these values)
	filter1.resonance(4);
	filter1.octaveControl(5);
}

void loop() {
	//update button and analog values
	button1.update();
	button2.update();
	button3.update();
	button4.update();
	press1 = analogRead(pressureSensor1Pin);
	press2 = analogRead(pressureSensor2Pin);
	
	press1 = abs(press1-1023.0)/1023; //invert and map pressure sensor value to a usable range
	filter1.resonance(press1*15); //set filter resonance
	filter1.frequency(press2); //set filter frequency
	//play samples (only samples played from different WAV player objects can be played simultaneously) 
	if(button1.fell()) playWav2.play("stick2.WAV"); delay(5); //play sample 1 with wav player 2
	if(button4.fell()) playWav2.play("bass1.WAV"); delay(5); //play sample 2 with wav player 2
	if(button3.fell()) playWav1.play("bass2.WAV"); delay(5); //play sample 3 with wav player 1
	//if button2 is pressed, play next note in melody
	if(button2.fell()) { 
		 //step through melody
		 if(melodyCnt < 18) { 
			waveform1.frequency(midicps(melody[melodyCnt]+12));
			melodyCnt++;
		} else melodyCnt = 0; //reset to beginning of melody when the end is reached.
		envelope1.noteOn(); //trigger envelope ON
	}  
	if(button2.rose()) envelope1.noteOff();  //turn envelope OFF when button is lifted
}

//convert MIDI note number to Cycles/Second
float midicps (float note) {
  return 440.0 * pow(2, (note - 69) / 12);
}