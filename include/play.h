#include <Audio.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;        //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
// AudioOutputAnalog        dac;            //xy=1313,421
AudioOutputMQS           dac;           //xy=1258,427
AudioConnection          patchCord1(playWav, 0, amp0, 0);
AudioConnection          patchCord2(playWav, 1, amp1, 0);
AudioConnection          patchCord3(amp0, 0, dac, 0);
AudioConnection          patchCord4(amp1, 0, dac, 1);
// GUItool: end automatically generated code

void errorBlink(uint8_t led);

void playFile(char filename[], uint8_t led) {
  Serial.printf("Playing file: %s\n", filename);
  if (SD.exists(filename)) {
    playWav.play(filename);
    delay(5);

    while (playWav.isPlaying()) {
      Serial.printf("Elapsed milliseconds: %d\n", playWav.positionMillis());
      delay(500);
    }
  } else {
    Serial.printf("Error: File (%s) not found ...\n", filename);
    errorBlink(led);
    delay(1000);
  }
}

void errorBlink(uint8_t led) {
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
}
