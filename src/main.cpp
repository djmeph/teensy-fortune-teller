#include <config.h>

void pitch() {
  if (credits.unused >= credits.price) {
    credits.unused -= credits.price;
    EEPROM_writeAnything(0, credits);
    stop();
    stage = CTA;
  } else if (approach.distance <= approach.maxDistance && !playWav.isPlaying()) {
    StaticJsonDocument<512> doc = deserializeJson();
    int randomNumber = Entropy.random(0, doc["pitch"].size());
    char* selectPitch = doc["pitch"][randomNumber];
    char pitchCopy[strlen(selectPitch) + 1] = {};
    strcpy(pitchCopy, selectPitch);
    play(pitchCopy);
  }
}

void cta() {
  StaticJsonDocument<512> doc = deserializeJson();
  int randomNumber = Entropy.random(0, doc["cta"].size());
  char* selectCta = doc["cta"][randomNumber];
  char ctaCopy[strlen(selectCta) + 1] = {};
  switch (ctaState) {
    case CTA_INACTIVE:
      strcpy(ctaCopy, selectCta);
      play(ctaCopy);
      ctaState = CTA_PLAY_SCRIPT;
      break;
    case CTA_PLAY_SCRIPT:
      if (!playWav.isPlaying()) ctaState = CTA_LED_ON;
      break;
    case CTA_LED_ON:
      digitalWrite(buttonLed, HIGH);
      button.press = false;
      ctaState = CTA_WAIT_FOR_BUTTON;
      break;
    case CTA_WAIT_FOR_BUTTON:
      if (button.press) {
        digitalWrite(buttonLed, LOW);
        ctaState = CTA_INACTIVE;
        button.press = false;
        stage = DISPENSE;
      }
      break;
  }
}

void dispense() {
  StaticJsonDocument<512> doc = deserializeJson();
  int randomNumber = Entropy.random(0, doc["dispense"].size());
  char* selectDispense = doc["dispense"][randomNumber];
  char dispenseCopy[strlen(selectDispense) + 1] = {};
  switch(dispenseState) {
    case DISPENSE_INACTIVE:
      strcpy(dispenseCopy, selectDispense);
      play(dispenseCopy);
      dispenseState = DISPENSE_PLAY_SCRIPT;
      break;
    case DISPENSE_PLAY_SCRIPT:
      if (!playWav.isPlaying()) {
        Serial.println("Dispensing card ...");
        dispenser.start = millis();
        digitalWrite(dispenseButton, HIGH);
        dispenseState = DISPENSE_CARD;
      }
      break;
    case DISPENSE_CARD:
      if ((millis() - dispenser.start) >= 100) {
        digitalWrite(dispenseButton, LOW);
        dispenser.timer = millis();
        dispenseState = DISPENSE_PAUSE;
      }
      break;
    case DISPENSE_PAUSE:
      if (millis() - dispenser.timer >= (10 * 1000)) {
        stop();
        dispenseState = DISPENSE_INACTIVE;
        Serial.println("Game Over");
        stage = PITCH;
      }
      break;
  }
}

void outOfCards() {
  StaticJsonDocument<512> doc = deserializeJson();
  int randomNumber = Entropy.random(0, doc["outOfCards"].size());
  char* selectOutOfCards = doc["outOfCards"][randomNumber];
  char outOfCardsCopy[strlen(selectOutOfCards) + 1] = {};

  switch (outOfCardsState) {
    case OUT_INACTIVE:
      stop();
      digitalWrite(buttonLed, LOW);
      strcpy(outOfCardsCopy, selectOutOfCards);
      play(outOfCardsCopy);
      outOfCardsState = OUT_PLAY_SCRIPT;
      break;
    case OUT_PLAY_SCRIPT:
      if (!playWav.isPlaying()) {
        outPause.start = millis();
        outOfCardsState = OUT_PAUSE;
      }
      break;
    case OUT_PAUSE:
      if ((millis() - outPause.start) > outPause.time) outOfCardsState = OUT_FINISHED;
      break;
    case OUT_FINISHED:
      outOfCardsState = OUT_INACTIVE;
      break;
  }
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
  approach.duration = pulseIn(echoPin, HIGH);
  approach.distance = approach.duration * 0.034 / 2;
}

void readButton() {
  if (digitalRead(buttonPin) == LOW) {
    if (!button.state) button.press = true;
    button.state = true;
    button.readTime = millis();
  } else if (millis() - button.readTime > 50) {
    button.state = false;
    credits.unused++;
    credits.total++;
    Serial.printf(
      "Total %u\t Unused: %u\n",
      credits.total,
      credits.unused
    );
  }
}

void readCoin() {
  if (digitalRead(coinPin) == LOW) {
    if (!coin.state) coin.press = true;
    coin.state = true;
    coin.readTime = millis();
  } else if (millis() - coin.readTime > 50) {
    coin.state = false;
  }
}

void monitor() {
  if (approach.distance <= approach.maxDistance) {
    Serial.printf("Distance: %d cm\n", approach.distance);
  }
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

void outOfCardsRead() {
  int reading = digitalRead(loadedInput);
  if (reading != dispenser.loaded.lastState) dispenser.loaded.lastDebounceTime = millis();

  if ((millis() - dispenser.loaded.lastDebounceTime) > dispenser.loaded.debounceDelay) {
    if (reading != dispenser.loaded.state) {
      dispenser.loaded.state = reading;
      stage = dispenser.loaded.state ? PITCH : OUT_OF_CARDS;
      if (dispenser.loaded.state) {
        stop();
        outOfCardsState = OUT_INACTIVE;
      }
    }
  }

  dispenser.loaded.lastState = reading;
}

void setup() {
  Serial.begin(9600);
  // while (!Serial) continue;

  pinMode(led, OUTPUT);
  pinMode(buttonLed, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(coinPin, INPUT_PULLUP);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(dispenseButton, OUTPUT);
  pinMode(loadedInput, INPUT_PULLUP);

  scheduler.addTask(readInputTask);
  scheduler.addTask(stageRouterTask);
  scheduler.addTask(monitorTask);

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    errorBlink();
    return;
  }

  readConfig();

  StaticJsonDocument<512> doc = deserializeJson();

  approach.maxDistance = doc["maxDistance"].as<int>();
  gain.animatronics = doc["animatronicsGain"].as<float>();
  gain.speaker = doc["speakerGain"].as<float>();
  credits.price = doc["credits.price"].as<int>();
  outPause.time = doc["outPause.time"].as<unsigned int>();

  int clearMemory = doc["clearMemory"].as<boolean>();

  AudioMemory(512);
  amp0.gain(gain.animatronics);
  amp1.gain(gain.speaker);

  if (clearMemory) clearMemoryTask();

  Serial.printf("Max user distance before pitch starts: %ucm \n", approach.maxDistance);
  Serial.printf("Animatronics Gain: %0.2f\n", gain.animatronics);
  Serial.printf("Speaker Gain: %0.2f\n", gain.speaker);
  Serial.printf("Cost to play: %u\n", credits.price);

  EEPROM_readAnything(0, credits);

  Serial.printf(
    "Total %u\t Unused: %u\n",
    credits.total,
    credits.unused
  );

  readInputTask.enable();
  stageRouterTask.enable();
  monitorTask.enable();

  Entropy.Initialize();

  Serial.println("Setup complete");
}

void loop() {
  scheduler.execute();
}
