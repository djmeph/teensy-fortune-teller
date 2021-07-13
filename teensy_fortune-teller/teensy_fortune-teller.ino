#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>

uint8_t led = 13; // Built-in LED
const char configFileName[13] = "CONFJSON.TXT";
static char jsonDoc[1024] = {};

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;        //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
AudioOutputAnalog        dac;            //xy=1313,421
AudioConnection          patchCord1(playWav, 0, amp0, 0);
AudioConnection          patchCord2(playWav, 1, amp1, 0);
AudioConnection          patchCord3(amp0, 0, dac, 0);
AudioConnection          patchCord4(amp1, 0, dac, 1);
// GUItool: end automatically generated code

void setup() {
  Serial.begin(9600);
  // while (!Serial) continue;

  pinMode(led, OUTPUT);
  AudioMemory(128);
  amp0.gain(1);
  amp1.gain(1);

  Serial.print("Initializing SD card...");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  if (SD.exists(configFileName)) {
    File file = SD.open(configFileName);

    if (file) {
      uint8_t jsonLen = file.size();

      Serial.printf("Reading %s:\n", configFileName);

      for (uint8_t i = 0; i < jsonLen; i++) jsonDoc[i] = file.read();
      file.close();

      for (uint8_t i = 0; i < jsonLen; i++) Serial.write((char)jsonDoc[i]);
      Serial.println("");
    } else {
      Serial.printf("error opening %s\n", configFileName);
    }
  } else {
    Serial.printf("file %s doesn't exist\n", configFileName);
  }
}

void loop() {
  StaticJsonDocument<1024> config;
  DeserializationError error = deserializeJson(config, jsonDoc);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    delay(5000);
    return;
  }

  uint8_t pitchCount = config["pitch"].size();
  uint8_t insertCoinCount = config["insertCoin"].size();
  uint8_t pressButtonCount = config["pressButton"].size();
  uint8_t outOfCardsCount = config["outOfCards"].size();

  Serial.printf("Size of pitches array: %u\n", pitchCount);
  Serial.printf("Size of insertCoin array: %u\n", insertCoinCount);
  Serial.printf("Size of pressButton array: %u\n", pressButtonCount);
  Serial.printf("Size of outOfCards array: %u\n", outOfCardsCount);

  JsonVariantConst variant = config["pitch"][0];
  const char* selectPitch = variant;

  /**
   * If I pass `selectPitch` directly to playFile() the name of the file prints (line 66)
   * And returns true that the file exists (line 67), but the AudioPlaySdWav object will
   * not play it (line 68). If I make a copy of the char array and pass the copy to
   * playFile(), everything works. Feels like I'm doing something wrong here.
   */

  char selectionCopy[strlen(selectPitch)] = {};
  strcpy(selectionCopy, selectPitch);

  playFile(selectionCopy);
  delay(5000);
}

void playFile(char *filename) {
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
    errorBlink();
    delay(1000);
  }
}

void errorBlink() {
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
