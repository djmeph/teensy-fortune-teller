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

#define echoPin 41
#define trigPin 40
#define led 13 // Built-in LED
#define buttonLed 12
#define button 24
#define configFileName "CONFJSON.TXT"

static const int coinPin = 11;
static long duration; // variable for the duration of sound wave travel
static int distance; // variable for the distance measurement
static int maxDistance; // Max distance until pitch trigger
static float amp0gain; // Animatronics input gain
static float amp1gain; // Speaker output gain
static char json[512]; // Holds JSON string from SD card
static int jsonLen; // Length of JSON string
static unsigned long nowInterrupt = millis(); // Last time we got an interrupt in ms
static int creditsCounter = 0; // Individual coin credit counter
static int creditsToPlay; // Number of credits required to get one fortune
static bool buttonState = false;
static bool buttonPress = false;
static unsigned long buttonReadTime = millis();
static long dispenseTimer = millis();

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
void readInput();
void readButton();

enum Stage { PITCH, CTA, DISPENSE };
enum Coin { START_DROP, COUNTING_DROP, END_DROP };
enum CtaState { CTA_INACTIVE, CTA_PLAY_SCRIPT, CTA_LED_ON, CTA_WAIT_FOR_BUTTON };
enum DispenseState { DISPENSE_INACTIVE, DISPENSE_PLAY_SCRIPT, DISPENSE_CARD, DISPENSE_PAUSE };

struct config_t {
  long creditsTotal = 0; // Total credits added to this machine since entropy
  int unusedCredits = 0; // Number of unused credits paid
} persistent;

Scheduler scheduler;
Task readInputTask(10, TASK_FOREVER, &readInput);
Task stageRouterTask(100, TASK_FOREVER, &stageRouter);
Task monitorTask(500, TASK_FOREVER, &monitor);
Stage stage = PITCH;
Coin coin = END_DROP;
CtaState ctaState = CTA_INACTIVE;
DispenseState dispenseState = DISPENSE_INACTIVE;

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
    creditsCounter = 1;
  } else if (elapsed > 50) {
    creditsCounter++;
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

void readInput() {
  userDistance();
  coinCounter();
  readButton();
}