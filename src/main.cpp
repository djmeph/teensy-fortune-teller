#include <Wire.h>
#include <config.h>
#include <play.h>

uint8_t led = 13; // Built-in LED

void setup() {
  Serial.begin(9600);
  while (!Serial) continue;

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
  StaticJsonDocument<128> config = readConfig();

  uint8_t pitchCount = config["pitch"].size();
  uint8_t insertCoinCount = config["insertCoin"].size();
  uint8_t pressButtonCount = config["pressButton"].size();
  uint8_t outOfCardsCount = config["outOfCards"].size();

  Serial.printf("Size of pitches array: %u\n", pitchCount);
  Serial.printf("Size of insertCoin array: %u\n", insertCoinCount);
  Serial.printf("Size of pressButton array: %u\n", pressButtonCount);
  Serial.printf("Size of outOfCards array: %u\n", outOfCardsCount);

  const JsonVariantConst variant = config["pitch"][0];
  const char* selectPitch = variant;

  /**
   * If I pass `selectPitch` directly to playFile() the name of the file prints (line 66)
   * And returns true that the file exists (line 67), but the AudioPlaySdWav object will
   * not play it (line 68). If I make a copy of the char array and pass the copy to
   * playFile(), everything works. Feels like I'm doing something wrong here.
   */

  char selectionCopy[strlen(selectPitch)] = {};
  strcpy(selectionCopy, selectPitch);

  playFile(selectionCopy, led);
  delay(5000);
}
