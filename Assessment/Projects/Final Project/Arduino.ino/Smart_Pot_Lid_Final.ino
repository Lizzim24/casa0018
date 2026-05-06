/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <Smart_Pot_Lid_Timer_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define LED_COUNT   2
#define BUZZER_PIN  9
#define CONVERT_G_TO_MS2    9.80665f

#define DEFAULT_MINUTES     5
#define INCREMENT_MINUTES   1   
#define WARNING_MINUTES     1   

enum TimerState { IDLE, COUNTING, WARNING, ALARM };
TimerState state = IDLE;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
unsigned long timerEndTime   = 0;   
unsigned long lastBlinkTime  = 0;   
bool          blinkOn        = false;
static bool   debug_nn       = false;

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, strip.Color(r, g, b));
  strip.show();
}

void ledsOff() { setLED(0, 0, 0); }

void beep(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}

void triggerActionFeedback(uint32_t color, int freq) {
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, color);
  strip.show();
  beep(freq, 150);
  delay(150); 
}

void startTimer(unsigned long minutes) {
  timerEndTime = millis() + (minutes * 60000UL);
  state = COUNTING;
  triggerActionFeedback(strip.Color(255, 255, 255), 1000); 
  Serial.println("Timer Started");
}

void addTime(int minutes) {
  if (state == IDLE || state == ALARM) return;
  long addMs = (long)minutes * 60000L;
  
  if (timerEndTime + addMs < millis() + 10000L) {
     timerEndTime = millis() + 10000L;
  } else {
     timerEndTime += addMs;
  }

  unsigned long now = millis();
  long remaining = (long)(timerEndTime - now);

  if (remaining > (long)WARNING_MINUTES * 60000L) {
    state = COUNTING; 
  } else if (remaining > 0) {
    state = WARNING;
  }

  if (minutes > 0) {
    triggerActionFeedback(strip.Color(0, 255, 100), 1500); 
    Serial.print("+1 Min. Total remaining: "); Serial.println(remaining / 1000);
  } else {
    triggerActionFeedback(strip.Color(255, 200, 0), 600); 
    Serial.print("-1 Min. Total remaining: "); Serial.println(remaining / 1000);
  }
}

void stopAlarm() {
  noTone(BUZZER_PIN);
  ledsOff();
  state = IDLE;
  Serial.println("Alarm Stopped. Reset.");
}

void handleGesture(const char* label) {
  if (strcmp(label, "place_down") == 0) {
    if (state == IDLE) startTimer(DEFAULT_MINUTES);
  }
  else if (strcmp(label, "lift_up") == 0) {
    if (state == ALARM) stopAlarm();
    else if (state == COUNTING || state == WARNING) {
        state = IDLE;
        ledsOff();
        beep(400, 300); 
        Serial.println("Timer Cancelled");
    }
  }
  else if (strcmp(label, "rotate_cw") == 0) {
    addTime(-INCREMENT_MINUTES); 
  }
  else if (strcmp(label, "rotate_ccw") == 0) {
    addTime(INCREMENT_MINUTES);  
  }
}

void setup() {
  Serial.begin(115200);

  if (!IMU.begin()) Serial.println("IMU Fail");
  
  pinMode(BUZZER_PIN, OUTPUT);
  strip.begin();
  strip.setBrightness(100);
  ledsOff();

  Serial.println("Smart Pot Lid Timer AI Integrated");
}

void loop() {
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 9) {
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    IMU.readAcceleration(buffer[ix], buffer[ix + 1], buffer[ix + 2]);
    IMU.readGyroscope(buffer[ix + 3], buffer[ix + 4], buffer[ix + 5]);
    IMU.readMagneticField(buffer[ix + 6], buffer[ix + 7], buffer[ix + 8]);
    for (int i = 0; i < 3; i++) buffer[ix + i] *= CONVERT_G_TO_MS2;
    int64_t wait_time = next_tick - micros();
    if(wait_time > 0) delayMicroseconds(wait_time);
  }

  signal_t signal;
  numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result = { 0 };
  if (run_classifier(&signal, &result, debug_nn) != EI_IMPULSE_OK) return;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > 0.80) { 
      handleGesture(result.classification[ix].label);
      break; 
    }
  }

  unsigned long now = millis();
  if (state == COUNTING) {
    setLED(0, 0, 255); 
    if ((long)(timerEndTime - now) <= (long)WARNING_MINUTES * 60000L) {
      state = WARNING;
    }
  } 
  else if (state == WARNING) {
    setLED(255, 100, 0); 
  }

  if (state == ALARM) {
    if (now - lastBlinkTime > 500) {
      lastBlinkTime = now;
      blinkOn = !blinkOn;
      if (blinkOn) { setLED(255, 0, 0); tone(BUZZER_PIN, 1000); }
      else { ledsOff(); noTone(BUZZER_PIN); }
    }
  }
  
  if (state == COUNTING || state == WARNING) {
    if ((long)(timerEndTime - now) <= 0) state = ALARM;
  }
}