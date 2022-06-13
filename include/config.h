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
#include <Bounce.h>

#define echoPin 35
#define trigPin 37
#define led 13 // Built-in LED
#define buttonLed 33
#define buttonPin 26
#define coinPin 28
#define loadedPin 14
#define dispenseButton 15
#define configFileName "CONFJSON.TXT"

struct sonic_range_finder_t {
  long duration;
  int distance;
  int maxDistance;
};

struct json_document_t {
  char payload[512];
  int len;
};

struct loaded_state_t {
  int state;
  int lastState = HIGH;
  unsigned lastDebounceTime = 0;
  unsigned long debounceDelay = 200;
};

struct card_dispenser_t {
  unsigned long start;
  long timer = millis();
  loaded_state_t loaded;
};

struct count_t {
  long total = 0; // Total credits added to this machine since entropy
  int unused = 0; // Number of unused credits paid
  int counter = 0; // Individual coin credit counter
  int price; // Number of credits required to get one fortune
};

struct gains_t {
  float animatronics;
  float speaker;
  float volume;
};

struct pause_t {
  unsigned int start;
  unsigned int time;
};

static sonic_range_finder_t approach;
static json_document_t config;
static card_dispenser_t dispenser;
static count_t credits;
static gains_t gain;
static pause_t outPause;
static pause_t pitchPause;

static bool buttonPress;
static bool firstCoinRead = true;

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
void readDistance();
void readButton();
void readCoin();
void outOfCardsRead();

enum Stage { PITCH, CTA, DISPENSE, OUT_OF_CARDS };
enum Coin { START_DROP, COUNTING_DROP, END_DROP };
enum CtaState { CTA_INACTIVE, CTA_PLAY_SCRIPT, CTA_LED_ON, CTA_WAIT_FOR_BUTTON };
enum DispenseState { DISPENSE_INACTIVE, DISPENSE_PLAY_SCRIPT, DISPENSE_CARD, DISPENSE_PAUSE };
enum OutOfCardsState { OUT_INACTIVE, OUT_PLAY_SCRIPT, OUT_PAUSE, OUT_FINISHED };
enum PitchPlayer { PITCH_READY, PITCH_PLAYING, PITCH_PAUSED };

Scheduler scheduler;
Task readInputTask(1, TASK_FOREVER, &readInput);
Task readDistanceTask(1000, TASK_FOREVER, &readDistance);
Task stageRouterTask(100, TASK_FOREVER, &stageRouter);
Stage stage = PITCH;
CtaState ctaState = CTA_INACTIVE;
DispenseState dispenseState = DISPENSE_INACTIVE;
OutOfCardsState outOfCardsState = OUT_INACTIVE;
PitchPlayer pitchPlayer = PITCH_READY;
Bounce coinTrigger = Bounce(coinPin, 10);
Bounce buttonTrigger = Bounce(buttonPin, 10);
Bounce loadedTrigger = Bounce(loadedPin, 10);


// GUItool: begin automatically generated code
AudioPlaySdWav           playWav;        //xy=261,297
AudioAmplifier           amp1;           //xy=499,304
AudioAmplifier           amp2;           //xy=511,353
AudioAmplifier           amp0;           //xy=516,253
AudioAmplifier           amp3;           //xy=548,408
AudioOutputMQS           mqs;           //xy=773,263
AudioOutputPT8211        pt8211_1;       //xy=810,354
AudioConnection          patchCord1(playWav, 0, amp0, 0);
AudioConnection          patchCord2(playWav, 0, amp2, 0);
AudioConnection          patchCord3(playWav, 1, amp1, 0);
AudioConnection          patchCord4(playWav, 1, amp3, 0);
AudioConnection          patchCord5(amp1, 0, mqs, 1);
AudioConnection          patchCord6(amp2, 0, pt8211_1, 0);
AudioConnection          patchCord7(amp0, 0, mqs, 0);
AudioConnection          patchCord8(amp3, 0, pt8211_1, 1);
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
  readCoin();
  readButton();
  outOfCardsRead();
}

void readDistance() {
  userDistance();
  monitor();
}
