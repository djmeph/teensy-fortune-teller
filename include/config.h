#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <TaskScheduler.h>
#include <Entropy.h>

#define echoPin 41 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 40 //attach pin D3 Arduino to pin Trig of HC-SR04
#define led 13 // Built-in LED
#define configFileName "CONFJSON.TXT"

void userDistance();
void monitor();
void pitch();
void errorBlink();
void play(char filename[]);
void stop();

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
uint8_t pitchCount;
uint8_t insertCoinCount;
uint8_t pressButtonCount;
uint8_t outOfCardsCount;

enum Stage { PITCH, CTA, DISPENSE };

Scheduler scheduler;
Task userDistanceTask(10, TASK_FOREVER, &userDistance);
Task monitorTask(1000, TASK_FOREVER, &monitor);
Task pitchTask(1000, TASK_FOREVER, &pitch);
Stage stage;
StaticJsonDocument<256> config;

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

StaticJsonDocument<256> readConfig() {
  /**
   * When I put the contents of this function under setup() I can read the configuration
   * once on the first loop, but on subsequent loops the same values read as [null]. When
   * I moved it to this function, and call it on the first line of loop() it reads and
   * parses the JSON file every loop, but it works. Wondering if there's a way to move
   * this back to setup() and preserve the contents of the `config` JsonDocument in
   * memory so I don't have to do this.
   */

  StaticJsonDocument<256> config;

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

      pitchCount = config["pitch"].size();
      insertCoinCount = config["insertCoin"].size();
      pressButtonCount = config["pressButton"].size();
      outOfCardsCount = config["outOfCards"].size();

      Serial.printf("Size of pitches array: %u\n", pitchCount);
      Serial.printf("Size of insertCoin array: %u\n", insertCoinCount);
      Serial.printf("Size of pressButton array: %u\n", pressButtonCount);
      Serial.printf("Size of outOfCards array: %u\n", outOfCardsCount);
    } else {
      Serial.printf("error opening %s\n", configFileName);
    }
  } else {
    Serial.printf("file %s doesn't exist\n", configFileName);
  }
  return config;
}