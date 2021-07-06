#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
AudioOutputAnalog        dac;            //xy=1313,421
AudioConnection          patchCord1(playSdWav1, 0, amp0, 0);
AudioConnection          patchCord2(playSdWav1, 1, amp1, 0);
AudioConnection          patchCord3(amp0, 0, dac, 0);
AudioConnection          patchCord4(amp1, 0, dac, 1);
// GUItool: end automatically generated code

int led = 13;

void setup() {
  Serial.begin(9600);
   while (!Serial)
    ; // wait for serial port to connect.

  pinMode(led, OUTPUT);
  AudioMemory(100);
  amp0.gain(1);
  amp1.gain(1);

  Serial.print("Initializing SD card...");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void loop() {
  playFile("SOUND_2.WAV");
  delay(500);
}

void playFile(const char *filename) {
  Serial.printf("Playing file: %s\n", filename);
  if (SD.exists(filename)) {
    playSdWav1.play(filename);
    delay(5);

    // Simply wait for the file to finish playing.
    while (playSdWav1.isPlaying()) {
      Serial.printf("Elapsed milliseconds: %d\n", playSdWav1.positionMillis());
      delay(500);
    }
  } else {
    Serial.printf("Error: File (%s) not found ...\n", filename);
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(1000);
  }
}
