#include <config.h>

void pitch() {
  if (persistent.unusedCredits >= creditsToPlay) {
    persistent.unusedCredits -= creditsToPlay;
    EEPROM_writeAnything(0, persistent);
    stop();
    stage = CTA;
  } else if (distance <= maxDistance && !playWav.isPlaying()) {
    StaticJsonDocument<512> config = deserializeJson();
    int randomNumber = Entropy.random(0, config["pitch"].size());
    char* selectPitch = config["pitch"][randomNumber];
    char pitchCopy[strlen(selectPitch) + 1] = {};
    strcpy(pitchCopy, selectPitch);
    play(pitchCopy);
  }
}

void cta() {
  StaticJsonDocument<512> config = deserializeJson();
  int randomNumber = Entropy.random(0, config["cta"].size());
  char* selectCta = config["cta"][randomNumber];
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
      buttonPress = false;
      ctaState = CTA_WAIT_FOR_BUTTON;
      break;
    case CTA_WAIT_FOR_BUTTON:
      if (buttonPress) {
        digitalWrite(buttonLed, LOW);
        ctaState = CTA_INACTIVE;
        buttonPress = false;
        stage = DISPENSE;
      }
      break;
  }
}

void dispense() {
  StaticJsonDocument<512> config = deserializeJson();
  int randomNumber = Entropy.random(0, config["dispense"].size());
  char* selectDispense = config["dispense"][randomNumber];
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
        dispenseStart = millis();
        digitalWrite(dispenseButton, HIGH);
        dispenseState = DISPENSE_CARD;
      }
      break;
    case DISPENSE_CARD:
      if ((millis() - dispenseStart) >= 100) {
        digitalWrite(dispenseButton, LOW);
        dispenseTimer = millis();
        dispenseState = DISPENSE_PAUSE;
      }
      break;
    case DISPENSE_PAUSE:
      if (millis() - dispenseTimer >= (10 * 1000)) {
        stop();
        dispenseState = DISPENSE_INACTIVE;
        Serial.println("Game Over");
        stage = PITCH;
      }
      break;
  }
}

void outOfCards() {
  StaticJsonDocument<512> config = deserializeJson();
  int randomNumber = Entropy.random(0, config["outOfCards"].size());
  char* selectOutOfCards = config["outOfCards"][randomNumber];
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
        outPauseStart = millis();
        outOfCardsState = OUT_PAUSE;
      }
      break;
    case OUT_PAUSE:
      if ((millis() - outPauseStart) > outPauseTime) outOfCardsState = OUT_FINISHED;
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
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

void coinCounter() {
  switch (coin) {
    case START_DROP:
      if (millis() - nowInterrupt <= 200) {
        coin = COUNTING_DROP;
      }
      break;
    case COUNTING_DROP:
      if (millis() - nowInterrupt > 200) {
        Serial.printf("Total credits inserted: %u\n", creditsCounter);
        persistent.unusedCredits += creditsCounter;
        persistent.creditsTotal += creditsCounter;
        EEPROM_writeAnything(0, persistent);
        Serial.printf(
          "Total %u\t Unused: %u\n",
          persistent.creditsTotal,
          persistent.unusedCredits
        );
        coin = END_DROP;
      }
      break;
    case END_DROP:
      if (creditsCounter > 0) coin = START_DROP;
      break;
  }
}

void readButton() {
  if (digitalRead(button) == LOW) {
    if (!buttonState) buttonPress = true;
    buttonState = true;
    buttonReadTime = millis();
  } else if (millis() - buttonReadTime > 50) {
    buttonState = false;
  }
}

void monitor() {
  if (distance <= maxDistance) {
    Serial.printf("Distance: %d cm\n", distance);
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
  if (reading != lastLoadedState) lastLoadedDebounceTime = millis();

  if ((millis() - lastLoadedDebounceTime) > loadedDebounceDelay) {
    if (reading != loaded) {
      loaded = reading;
      stage = loaded ? PITCH : OUT_OF_CARDS;
      if (loaded) {
        stop();
        outOfCardsState = OUT_INACTIVE;
      }
    }
  }

  lastLoadedState = reading;
}

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(coinPin), coinInterrupt, RISING);
  // while (!Serial) continue;

  pinMode(led, OUTPUT);
  pinMode(buttonLed, OUTPUT);
  pinMode(button, INPUT_PULLUP);
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

  StaticJsonDocument<512> config = deserializeJson();

  maxDistance = config["maxDistance"].as<int>();
  amp0gain = config["animatronicsGain"].as<float>();
  amp1gain = config["speakerGain"].as<float>();
  creditsToPlay = config["creditsToPlay"].as<int>();
  outPauseTime = config["outPauseTime"].as<unsigned int>();

  int clearMemory = config["clearMemory"].as<boolean>();

  AudioMemory(512);
  amp0.gain(amp0gain);
  amp1.gain(amp1gain);

  if (clearMemory) clearMemoryTask();

  Serial.printf("Max user distance before pitch starts: %ucm \n", maxDistance);
  Serial.printf("Animatronics Gain: %0.2f\n", amp0gain);
  Serial.printf("Speaker Gain: %0.2f\n", amp1gain);
  Serial.printf("Cost to play: %u\n", creditsToPlay);

  EEPROM_readAnything(0, persistent);

  Serial.printf(
    "Total %u\t Unused: %u\n",
    persistent.creditsTotal,
    persistent.unusedCredits
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
