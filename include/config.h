#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <TaskScheduler.h>
#include <EEPROMAnything.h>
#include <Entropy.h>

#define echoPin 1
#define trigPin 3
#define led 13 // Built-in LED
#define buttonLed 12
#define buttonPin 24
#define coinPin 10
#define loadedInput 14
#define dispenseButton 15
#define configFileName "CONFJSON.TXT"

struct sonicRangeFinder_t {
  long duration;
  int distance;
  int maxDistance;
} static approach;

struct jsonDocument_t {
  char payload[512];
  int len;
} static config;

struct buttonInput_t {
  bool state = false;
  bool press = false;
  long readTime = millis();
} static button, coin;

struct loadedState_t {
  int state;
  int lastState = HIGH;
  unsigned lastDebounceTime = 0;
  unsigned long debounceDelay = 200;
};

struct cardDispenser_t {
  unsigned long start;
  long timer = millis();
  loadedState_t loaded;
} static dispenser;

struct count_t {
  long total = 0; // Total credits added to this machine since entropy
  int unused = 0; // Number of unused credits paid
  int counter = 0; // Individual coin credit counter
  int price; // Number of credits required to get one fortune
} static credits;

struct gains_t {
  float animatronics;
  float speaker;
} static gain;

struct pause_t {
  unsigned int start;
  unsigned int time;
} static outPause;

void userDistance();
void monitor();
void pitch();
void cta();
void dispense();
void outOfCards();
void errorBlink();
void play(char filename[]);
void stop();
void readMemory();
void stageRouter();
void clearMemoryTask();
void readInput();
void readButton();
void readCoin();
void outOfCardsRead();

enum Stage { PITCH, CTA, DISPENSE, OUT_OF_CARDS };
enum Coin { START_DROP, COUNTING_DROP, END_DROP };
enum CtaState { CTA_INACTIVE, CTA_PLAY_SCRIPT, CTA_LED_ON, CTA_WAIT_FOR_BUTTON };
enum DispenseState { DISPENSE_INACTIVE, DISPENSE_PLAY_SCRIPT, DISPENSE_CARD, DISPENSE_PAUSE };
enum OutOfCardsState { OUT_INACTIVE, OUT_PLAY_SCRIPT, OUT_PAUSE, OUT_FINISHED };

Scheduler scheduler;
Task readInputTask(10, TASK_FOREVER, &readInput);
Task stageRouterTask(100, TASK_FOREVER, &stageRouter);
Task monitorTask(500, TASK_FOREVER, &monitor);
Stage stage = PITCH;
CtaState ctaState = CTA_INACTIVE;
DispenseState dispenseState = DISPENSE_INACTIVE;
OutOfCardsState outOfCardsState = OUT_INACTIVE;

// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;     //xy=864,427
AudioAmplifier           amp0;           //xy=1081,400
AudioAmplifier           amp1;           //xy=1084,440
AudioOutputAnalogStereo  dac;          //xy=1277,417
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

      config.len = file.size();

      for (int i = 0; i < config.len; i++) config.payload[i] = file.read();
      file.close();

      for (int i = 0; i < config.len; i++) Serial.write((char)config.payload[i]);
      Serial.println("");
    } else {
      Serial.printf("error opening %s\n", configFileName);
    }
  } else {
    Serial.printf("file %s doesn't exist\n", configFileName);
  }
}

StaticJsonDocument<512> deserializeJson() {
  StaticJsonDocument<512> doc;
  char jsonCopy[config.len + 1];
  for (int i = 0; i < config.len; i++) jsonCopy[i] = config.payload[i];
  DeserializationError error = deserializeJson(doc, jsonCopy);

  if (error) {
    Serial.printf("deserializeJson() failed: %s\n", error.f_str());
  }
  return doc;
}

void stageRouter() {
  switch (stage) {
    case OUT_OF_CARDS:
      return outOfCards();
    case PITCH:
      return pitch();
    case CTA:
      return cta();
    case DISPENSE:
      return dispense();
  }
}

void clearMemoryTask() {
  EEPROM_writeAnything(0, credits);
  // Write something here to change the JSON clearMemory = false and save
}

void readInput() {
  outOfCardsRead();
  userDistance();
  readCoin();
  readButton();
}