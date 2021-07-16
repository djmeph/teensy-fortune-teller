#include <config.h>

void pitch() {
  if (stage != PITCH) return;
  if (distance <= 400 && !playWav.isPlaying()) {
    Serial.printf("Size of pitches array: %u\n", pitchCount);
    uint32_t randomNumber = Entropy.random(0, (pitchCount - 1));
    Serial.printf("Random pitch index: %u\n", randomNumber);
    // const JsonVariantConst variant = config["pitch"][randomNumber];
    const char* selectPitch = config["pitch"][randomNumber];
    Serial.printf("Pitch selected: %s\n", selectPitch);
    // char selectionCopy[strlen(selectPitch)] = {};
    // strcpy(selectionCopy, selectPitch);
    // Serial.printf("Pitch selected: %s\n", selectionCopy);
    // play(selectionCopy);
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

  AudioMemory(256);
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

  config = readConfig();

  userDistanceTask.enable();
  monitorTask.enable();
  pitchTask.enable();

  Serial.println("Setup complete");
}

void loop() {
  scheduler.execute();
}

void foo () {
  StaticJsonDocument<256> config = readConfig();

  uint8_t pitchCount = config["pitch"].size();
  uint8_t insertCoinCount = config["insertCoin"].size();
  uint8_t pressButtonCount = config["pressButton"].size();
  uint8_t outOfCardsCount = config["outOfCards"].size();

  Serial.printf("Size of pitches array: %u\n", pitchCount);
  Serial.printf("Size of insertCoin array: %u\n", insertCoinCount);
  Serial.printf("Size of pressButton array: %u\n", pressButtonCount);
  Serial.printf("Size of outOfCards array: %u\n", outOfCardsCount);

  const JsonVariantConst variant = config["pitch"][0];
  const char* selectPitch = variant;

  /**
   * If I pass `selectPitch` directly to play() the name of the file prints (line 66)
   * And returns true that the file exists (line 67), but the AudioPlaySdWav object will
   * not play it (line 68). If I make a copy of the char array and pass the copy to
   * play(), everything works. Feels like I'm doing something wrong here.
   */

  char selectionCopy[strlen(selectPitch)] = {};
  strcpy(selectionCopy, selectPitch);

  play(selectionCopy);
  delay(5000);
}