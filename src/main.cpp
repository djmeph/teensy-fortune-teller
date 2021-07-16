#include <config.h>

void playFile(char filename[]) {
  Serial.printf("Playing file: %s\n", filename);
  if (SD.exists(filename)) {
    playWav.play(filename);
    delay(5);

    while (playWav.isPlaying()) {
      Serial.printf("Elapsed milliseconds: %d\n", playWav.positionMillis());
      delay(500);
    }
  } else {
    Serial.printf("Error: File (%s) not found ...\n", filename);
    errorBlink();
  }
}

void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

void taskMonitor() {
  Serial.printf("Distance: %d cm\n", distance);
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
  AudioMemory(128);
  amp0.gain(1);
  amp1.gain(1);

  Serial.print("Initializing SD card...");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  scheduler.addTask(userDistance);
  scheduler.addTask(monitor);
  userDistance.enable();
  monitor.enable();
}

void loop() {
  scheduler.execute();
  // StaticJsonDocument<128> config = readConfig();

  // uint8_t pitchCount = config["pitch"].size();
  // uint8_t insertCoinCount = config["insertCoin"].size();
  // uint8_t pressButtonCount = config["pressButton"].size();
  // uint8_t outOfCardsCount = config["outOfCards"].size();

  // Serial.printf("Size of pitches array: %u\n", pitchCount);
  // Serial.printf("Size of insertCoin array: %u\n", insertCoinCount);
  // Serial.printf("Size of pressButton array: %u\n", pressButtonCount);
  // Serial.printf("Size of outOfCards array: %u\n", outOfCardsCount);

  // while (stage == pitch) {
  //   pitchFn();
  // }

  // const JsonVariantConst variant = config["pitch"][0];
  // const char* selectPitch = variant;

  /**
   * If I pass `selectPitch` directly to playFile() the name of the file prints (line 66)
   * And returns true that the file exists (line 67), but the AudioPlaySdWav object will
   * not play it (line 68). If I make a copy of the char array and pass the copy to
   * playFile(), everything works. Feels like I'm doing something wrong here.
   */

  // char selectionCopy[strlen(selectPitch)] = {};
  // strcpy(selectionCopy, selectPitch);

  // playFile(selectionCopy, led);
  // delay(5000);
}
