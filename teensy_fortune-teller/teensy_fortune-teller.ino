#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>

uint8_t led = 13; // Built-in LED
const char configFileName[13] = "CONFJSON.TXT";

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;     //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
AudioOutputAnalog        dac;            //xy=1313,421
AudioConnection          patchCord1(playWav, 0, amp0, 0);
AudioConnection          patchCord2(playWav, 1, amp1, 0);
AudioConnection          patchCord3(amp0, 0, dac, 0);
AudioConnection          patchCord4(amp1, 0, dac, 1);
// GUItool: end automatically generated code

StaticJsonDocument<128> config;

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
}

void loop() {
  readConfig();
  Serial.printf("Size of pitches array: %u\n", config["pitch"].size());
  Serial.printf("Size of insertCoin array: %u\n", config["insertCoin"].size());
  Serial.printf("Size of pressButton array: %u\n", config["pressButton"].size());
  Serial.printf("Size of outOfCards array: %u\n", config["outOfCards"].size());\

  const JsonVariantConst variant = config["pitch"][0];
  const char* selectPitch = variant;
  const int pitchLength = strlen(selectPitch);

  // If I pass `selectPitch` directly to playFile() the name of the file prints (line 62)
  // And returns true that the file exists, but the AudioPlaySdWav object will not play
  // it. If I make a copy of the array and pass the copy, everything works. Feels like I'm
  // Doing something wrong here.

  char selectionCopy[pitchLength] = {};
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

void readConfig() {
  /**
   * When I put the contents of this function under setup() I can read the configuration
   * once on the first loop, but on subsequent loops the same values read as [null]. When
   * I moved it to this function, and call it on the first line of loop() it reads and
   * parses the JSON file every loop, but it works. Wondering if there's a way to move
   * this back to setup() and preserve the contents of the `config` JsonDocument in
   * memory so I don't have to do this.
   */

  if (SD.exists(configFileName)) {
    File file = SD.open(configFileName);

    if (file) {
      Serial.printf("Reading %s:\n", configFileName);

      uint8_t jsonLen = file.size();
      char json[jsonLen];

      for (uint8_t i = 0; i < jsonLen; i++) json[i] = file.read();
      file.close();

      for (uint8_t i = 0; i < jsonLen; i++) Serial.write((char)json[i]);
      Serial.println("");

      DeserializationError error = deserializeJson(config, json);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
    } else {
      Serial.printf("error opening %s\n", configFileName);
    }
  } else {
    Serial.printf("file %s doesn't exist\n", configFileName);
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
