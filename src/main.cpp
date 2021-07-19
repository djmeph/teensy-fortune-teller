#include <config.h>

void pitch() {
  if (stage != PITCH) return;
  if (distance <= maxDistance && !playWav.isPlaying()) {
    StaticJsonDocument<512> config = deserializeJson();
    uint32_t randomNumber = Entropy.random(0, config["pitch"].size());
    char* selectPitch = config["pitch"][randomNumber];
    char pitchCopy[strlen(selectPitch) + 1] = {};
    strcpy(pitchCopy, selectPitch);
    play(pitchCopy);
  }
}

void cta() {
  if (stage != CTA) return;
}

void dispense() {
  if (stage != DISPENSE) return;
}

void play(char* filename) {
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
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  AudioMemory(512);
  amp0.gain(amp0gain);
  amp1.gain(amp1gain);

  scheduler.addTask(userDistanceTask);
  scheduler.addTask(monitorTask);
  scheduler.addTask(pitchTask);
  scheduler.addTask(ctaTask);
  scheduler.addTask(dispenseTask);

  Entropy.Initialize();

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    errorBlink();
    return;
  }

  readConfig();

  StaticJsonDocument<512> config = deserializeJson();

  maxDistance = config["maxDistance"].as<uint8_t>();
  amp0gain = config["animatronicsGain"].as<float>();
  amp1gain = config["speakerGain"].as<float>();

  Serial.printf("Max user distance before pitch starts: %ucm \n", maxDistance);
  Serial.printf("Animatronics Gain: %0.2f\n", amp0gain);
  Serial.printf("Speaker Gain: %0.2f\n", amp1gain);

  userDistanceTask.enable();
  monitorTask.enable();
  pitchTask.enable();
  ctaTask.enable();
  dispenseTask.enable();

  Serial.println("Setup complete");
}

void loop() {
  scheduler.execute();
}
