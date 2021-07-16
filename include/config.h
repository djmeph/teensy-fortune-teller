#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <TaskScheduler.h>

#define echoPin 41 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 40 //attach pin D3 Arduino to pin Trig of HC-SR04
#define led 13 // Built-in LED
#define configFileName "CONFJSON.TXT"

void readDistance();
void taskMonitor();
void errorBlink();

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

enum Stage { pitch, cta, dispense };

Scheduler scheduler;
Task userDistance(10, TASK_FOREVER, &readDistance);
Task monitor(1000, TASK_FOREVER, &taskMonitor);
Stage stage;

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

StaticJsonDocument<128> readConfig() {
  /**
   * When I put the contents of this function under setup() I can read the configuration
   * once on the first loop, but on subsequent loops the same values read as [null]. When
   * I moved it to this function, and call it on the first line of loop() it reads and
   * parses the JSON file every loop, but it works. Wondering if there's a way to move
   * this back to setup() and preserve the contents of the `config` JsonDocument in
   * memory so I don't have to do this.
   */

  StaticJsonDocument<128> config;

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
  return config;
}