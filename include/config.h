#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>

const char configFileName[13] = "CONFJSON.TXT";

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