#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#include <FastLED.h>
#include <memory>

// PixelTheater includes
#include "PixelTheater/platform/fastled_platform.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/stage.h"
// Include the model definition - we'll use an include guard to prevent multiple definitions
#ifndef DODECARGBV2_MODEL_INCLUDED
#define DODECARGBV2_MODEL_INCLUDED
#include "models/DodecaRGBv2/model.h" // Include the generated model
#endif
#include "scenes/blob_scene.h" // Include our blob scene
#include "benchmark.h" // Include our benchmark utilities

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.0.1"
#endif

// LED configs
// Version is defined in VERSION file in root directory
#define VERSION PROJECT_VERSION
#define USER_BUTTON 2
// https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
// pins 19+18 are used to control two strips of 624 LEDs, for a total of 1248 LEDs
#define LED_CHANNEL_1_PIN 19  // Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define LED_CHANNEL_2_PIN 18  // Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define ANALOG_PIN_A 24
#define ANALOG_PIN_B 25
#define ON_BOARD_LED 13

// Teensy I2C on pins 17/16 (SDA,SCL) is mapped to Wire1 in Arduinio framework
// https://www.pjrc.com/teensy/td_libs_Wire.html
// ex: Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire1);

#define BRIGHTNESS  15      // global brightness, should be used by all animations
#define USE_IMU true        // enable orientation sensor (currently: LSM6DSOX)

// model settings (replace with generated model params)
#define NUM_LEDS 1248
#define NUM_SIDES 12
#define LEDS_PER_SIDE 104

#define NUM_SCENES 1  // We'll increase this as we add more scenes

CRGB leds[NUM_LEDS];

// PixelTheater components
using DodecaModel = PixelTheater::Models::DodecaRGBv2;
std::unique_ptr<PixelTheater::FastLEDPlatform> platform;
std::unique_ptr<PixelTheater::Model<DodecaModel>> model;
std::unique_ptr<PixelTheater::Stage<DodecaModel>> stage;
Scenes::BlobScene<DodecaModel>* blob_scene = nullptr;

long random_seed = 0;
int seed1,seed2 = 0;
int mode = 0;

float calculate_power_usage() {
  // todo
  return 0.0f;
}

void timerStatusMessage(){
  uint8_t scene_number = 0;
  String scene_name = "BlobScene";
  
  if (stage && blob_scene) {
    scene_name = blob_scene->status().c_str();
  }
  
  Serial.printf("--> mode:%d (%s) @ %d FPS <--\n", 
    scene_number,
    scene_name.c_str(),
    FastLED.getFPS() 
  );

  String status = "running"; // TODO: get status from running scene
  Serial.printf("%s\n", status.c_str());

  Serial.printf("Est Power: %0.1f W (%.1f%% brightness)\n", 
    calculate_power_usage()/1000.0,
    (BRIGHTNESS/ 255.0f) * 100);
    
  // Include benchmark report in status message
  BENCHMARK_REPORT(FastLED.getFPS());
}

void fadeInSide(int side, int start_led, int end_led, int duration_ms) {
    for (int brightness = 0; brightness <= 120; brightness += 30) {
        for (int led = start_led; led <= end_led; ++led) {
            int index = side * LEDS_PER_SIDE + led;
            CRGB side_color = ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
            leds[index] = side_color;
            leds[index].fadeToBlackBy(255 - brightness);
        }
        FastLED.show();
        delay(duration_ms);  // Adjust delay to complete fade-in within the specified duration
    }
}

void setup() {
  float temp = InternalTemperature.readTemperatureC();
  random_seed = (temp - int(temp)) * 100000; 
  random_seed += (analogRead(ANALOG_PIN_A) * analogRead(ANALOG_PIN_B));
  randomSeed(random_seed);
  random16_set_seed(random_seed);
  
  seed1 = random(random_seed) * 2 % 4000;
  seed2 = random(random_seed) * 3 % 5000;
  random16_add_entropy(seed1);
  random16_add_entropy(seed2);

  Serial.begin(115200);
  delay(300);
  Serial.printf("Start: DodecaRGBv2 firmware v%s\n", VERSION);
  Serial.printf("Teensy version: %d\n", TEENSYDUINO);
  // Parse FastLED version
  int major = FASTLED_VERSION / 1000000;
  int minor = (FASTLED_VERSION / 1000) % 1000;
  int patch = FASTLED_VERSION % 1000;
  Serial.printf("FastLED version: %d.%d.%d\n", major, minor, patch);
  Serial.printf("Compiled: %s %s\n", __DATE__, __TIME__);
  Serial.printf("CPU Temp: %f c\n", temp);
  Serial.printf("Num LEDs: %d\n", NUM_LEDS);
  Serial.printf("Random seed: %d\n", random_seed);
  Serial.println();

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(ANALOG_PIN_A, INPUT);
  pinMode(ANALOG_PIN_B, INPUT);
  pinMode(USER_BUTTON, INPUT_PULLUP);

  // set up fastled - two strips on two pins
  // see https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
  FastLED.addLeds<WS2812, LED_CHANNEL_1_PIN, GRB>(leds, NUM_LEDS/2);
  FastLED.addLeds<WS2812, LED_CHANNEL_2_PIN, GRB>(leds, NUM_LEDS/2, NUM_LEDS/2);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  FastLED.setMaxRefreshRate(90);
  FastLED.clear();
  FastLED.show();

  // Fade in each side in sequence
  int fade_duration = 1;  
  for (int side = 0; side < NUM_SIDES; ++side) {
    fadeInSide(side, 6, 15, fade_duration);
  }

  // Initialize PixelTheater components
  Serial.println("Initializing PixelTheater...");
  
  // Enable or disable benchmarking
  Benchmark::enabled = true;  // Set to false to disable benchmarking
  
  // Create platform with FastLED integration
  // Need to cast the leds array to PixelTheater::CRGB* to match the expected type
  platform = std::make_unique<PixelTheater::FastLEDPlatform>(
    reinterpret_cast<PixelTheater::CRGB*>(leds), 
    DodecaModel::LED_COUNT
  );
  
  // Create model based on our model definition
  model = std::make_unique<PixelTheater::Model<DodecaModel>>(DodecaModel{}, platform->getLEDs());
  
  // Create stage with platform and model
  stage = std::make_unique<PixelTheater::Stage<DodecaModel>>(std::move(platform), std::move(model));
  
  // Add and set initial scene
  blob_scene = stage->addScene<Scenes::BlobScene<DodecaModel>>(*stage);
  blob_scene->setup();
  stage->setScene(blob_scene);

  Serial.println("Init done");

  FastLED.clear();
  FastLED.show();

  delay(100);
}

long interval, last_interval = 0;
const long max_interval = 3000;

void updateOnboardLED() {
    static uint8_t led_brightness = 0;
    static uint32_t last_update = 0;
    const uint32_t update_interval = 16;  // ~60Hz updates
    
    interval = millis()/max_interval;
    if (millis() - last_update >= update_interval) {
        // Create smooth sine wave breathing (4 second cycle)
        float breath = (sin(millis() * PI / 2000.0) + 1.0) / 2.0;
        led_brightness = breath * 255;
        analogWrite(ON_BOARD_LED, led_brightness);
        last_update = millis();
    }
}

void loop() {  
  updateOnboardLED();  // Replace old LED code with this
  
  if (interval != last_interval){
    timerStatusMessage();
    last_interval = interval;
  }
  
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    // flash while button is still down
    while (digitalRead(USER_BUTTON) == LOW){
      CRGB c = CRGB::White;
      c.setHSV(millis()/500 % 255, 255, 64);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(20); 
    }
    Serial.println("Button released");
    mode = (mode + 1) % NUM_SCENES;
    Serial.printf("Button pressed, changed mode to %d\n", mode);
    // TODO: implement scene switching when we have more scenes
  }

  // Update the stage (calls current scene's tick() and updates LEDs)
  if (stage) {
    BENCHMARK_START("frame_total");
    stage->update();
    BENCHMARK_END();
  }
}
