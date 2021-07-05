#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=730,980
AudioOutputPT8211        pt8211_1;       //xy=1065,982
AudioConnection          patchCord1(playSdWav1, 0, pt8211_1, 0);
AudioConnection          patchCord2(playSdWav1, 1, pt8211_1, 1);
// GUItool: end automatically generated code

void setup() {
  // playSdWav1.play('moo.wav');
}

void loop() {
  // put your main code here, to run repeatedly:

}
