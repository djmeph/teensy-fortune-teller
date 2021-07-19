#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <TaskScheduler.h>
#include <Entropy.h>

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

static long duration; // variable for the duration of sound wave travel
static int distance; // variable for the distance measurement
static uint8_t maxDistance = 10; // TODO: Move these values to JSON
static float amp0gain = 0.7; // TODO: Move these values to JSON
static float amp1gain = 1; // TODO: Move these values to JSON
static char json[512]; // Holds JSON string from SD card
static uint8_t jsonLen; // Length of JSON string

enum Stage { PITCH, CTA, DISPENSE };

Scheduler scheduler;
Task userDistanceTask(10, TASK_FOREVER, &userDistance);
Task monitorTask(500, TASK_FOREVER, &monitor);
Task pitchTask(100, TASK_FOREVER, &pitch);
Task ctaTask(100, TASK_FOREVER, &cta);
Task dispenseTask(100, TASK_FOREVER, &dispense);
Stage stage;

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

      for (uint8_t i = 0; i < jsonLen; i++) json[i] = file.read();
      file.close();

      for (uint8_t i = 0; i < jsonLen; i++) Serial.write((char)json[i]);
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
  for (uint8_t i = 0; i < jsonLen; i++) jsonCopy[i] = json[i];
  DeserializationError error = deserializeJson(config, jsonCopy);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  return config;
}