#include <config.h>

void pitch() {
  if (stage != PITCH) return;
  if (distance <= 10 && !playWav.isPlaying()) {
    StaticJsonDocument<512> config;
    char jsonCopy[jsonLen + 1];
    for (uint8_t i = 0; i < jsonLen; i++) jsonCopy[i] = json[i];
    DeserializationError error = deserializeJson(config, jsonCopy);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }

    uint8_t pitchCount = config["pitch"].size();
    uint32_t randomNumber = Entropy.random(0, pitchCount);
    const char* selectPitch = config["pitch"][randomNumber];
    char selectionCopy[strlen(selectPitch) + 1] = {};
    strcpy(selectionCopy, selectPitch);
    play(selectionCopy);
  }
}

void play(char filename[]) {
  Serial.printf("Playing file: %s\n", filename);
  if (SD.exists(filename)) {
    playWav.play(filename);
    delay(5);
  } else {
    Serial.printf("Error: File (%s) not found ...\n", filename);
    errorBlink();
  }
}

void stop() {
  if (playWav.isPlaying()) {
    playWav.stop();
  }
}

void userDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

void monitor() {
  Serial.printf("Distance: %d cm\n", distance);
  if (playWav.isPlaying()) {
    Serial.printf("Elapsed milliseconds: %d\n", playWav.positionMillis());
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

void setup() {
  Serial.begin(9600);
  while (!Serial) continue;

  pinMode(led, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT

  AudioMemory(512);
  amp0.gain(1);
  amp1.gain(1);

  scheduler.addTask(userDistanceTask);
  scheduler.addTask(monitorTask);
  scheduler.addTask(pitchTask);

  Entropy.Initialize();

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    errorBlink();
    return;
  }

  readConfig();

  userDistanceTask.enable();
  monitorTask.enable();
  pitchTask.enable();

  Serial.println("Setup complete");
}

void loop() {
  scheduler.execute();
}
