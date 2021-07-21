#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <TaskScheduler.h>
#include <Entropy.h>
#include <EEPROMAnything.h>

#define echoPin 41
#define trigPin 40
#define led 13 // Built-in LED
#define configFileName "CONFJSON.TXT"

void userDistance();
void monitor();
void pitch();
void cta();
void dispense();
void errorBlink();
void play(char filename[]);
void stop();
void coinInterrupt();
void coinCounter();
void readMemory();
void stageRouter();
void clearMemoryTask();

static const int coinPin = 2;
static long duration; // variable for the duration of sound wave travel
static int distance; // variable for the distance measurement
static int maxDistance; // Max distance until pitch trigger
static float amp0gain; // Animitronics input gain
static float amp1gain; // Speaker output gain
static char json[512]; // Holds JSON string from SD card
static int jsonLen; // Length of JSON string
static unsigned long nowInterrupt = millis(); // Last time we got an interrupt in ms
static int centsCounter = 0; // Individual coin cent counter
static int centsToPlay; // Number of cents required to get one fortune

enum Stage { PITCH, CTA, DISPENSE };
enum Coin { START_DROP, COUNTING_DROP, END_DROP };

struct config_t {
  long centsTotal = 0; // Total cents added to this machine since entropy
  int unusedCents = 0; // Number of unused cents paid
} persistent;

Scheduler scheduler;
Task userDistanceTask(10, TASK_FOREVER, &userDistance);
Task coinCounterTask(10, TASK_FOREVER, &coinCounter);
Task monitorTask(500, TASK_FOREVER, &monitor);
Task stageRouterTask(100, TASK_FOREVER, &stageRouter);
Stage stage = PITCH;
Coin coin = END_DROP;

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;        //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
// AudioOutputAnalog        dac;            //xy=1313,421
// AudioOutputMQS           dac;           //xy=1258,427
// AudioOutputI2S           dac;           //xy=1267,419
AudioOutputPT8211        dac;       //xy=1285,416
AudioConnection          patchCord1(playWav, 0, amp0, 0);
AudioConnection          patchCord2(playWav, 1, amp1, 0);
AudioConnection          patchCord3(amp0, 0, dac, 0);
AudioConnection          patchCord4(amp1, 0, dac, 1);
// GUItool: end automatically generated code

void readConfig() {
  if (SD.exists(configFileName)) {
    File file = SD.open(configFileName);

    if (file) {
      Serial.printf("Reading %s:\n", configFileName);

      jsonLen = file.size();

      for (int i = 0; i < jsonLen; i++) json[i] = file.read();
      file.close();

      for (int i = 0; i < jsonLen; i++) Serial.write((char)json[i]);
      Serial.println("");
    } else {
      Serial.printf("error opening %s\n", configFileName);
    }
  } else {
    Serial.printf("file %s doesn't exist\n", configFileName);
  }
}

StaticJsonDocument<512> deserializeJson() {
  StaticJsonDocument<512> config;
  char jsonCopy[jsonLen + 1];
  for (int i = 0; i < jsonLen; i++) jsonCopy[i] = json[i];
  DeserializationError error = deserializeJson(config, jsonCopy);

  if (error) {
    Serial.printf("deserializeJson() failed: %s\n", error.f_str());
  }
  return config;
}

void coinInterrupt() {
  const unsigned long elapsed = millis() - nowInterrupt;
  nowInterrupt = millis();
  if (elapsed > 200) {
    centsCounter = 1;
    Serial.printf("Time elapsed: %u\tPulses counted: %u\n", elapsed, centsCounter);
  } else if (elapsed > 50) {
    centsCounter++;
    Serial.printf("Time elapsed: %u\tPulses counted: %u\n", elapsed, centsCounter);
  }
}

void stageRouter() {
  switch (stage) {
    case PITCH:
      pitch();
      break;
    case CTA:
      cta();
      break;
    case DISPENSE:
      dispense();
      break;
  }
}

void clearMemoryTask() {
  EEPROM_writeAnything(0, persistent);
  // Write something here to change the JSON clearMemory = false and save
}