/*
   Rivet Game

  State-machine for Rivet game with 4 states - demo, training, game, and winner.

  In game mode, write out 8-bit ids to LED's and scan them bit by bit with photodiode output.

  Collect up higher-level data and write it out over serial.

*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Arduino.h>
#include <FastLED.h>


#define NUM_LEDS 64
#define NUM_LEDS_PER_ROW 8
#define DATA_PIN 10
#define CLOCK_PIN 11

CRGB leds[NUM_LEDS];
int led_states[NUM_LEDS];

bool is_testing = false;
bool do_print_diagnostics = false;

// Inputs
const int training_btn = A10;  // Start training
const int play_btn = 3;  // Start playing

// Player A Inputs
//const int trigger_a = A11;  // trigger button
const int trigger_a = 18;  // trigger button
boolean trigger_a_ready = false;
static unsigned long last_interrupt_time_a = 0;
//const int motor_a = 5;    // Vibration motor to indicate rivet event
const int photo_a = A0;   // Photodiode input pin
const int photo_a_out = 12;   // Photodiode 5v pin
uint8_t photothresh_a = 60; // Threshold value for photodiode reading
Adafruit_BNO055 bno_a = Adafruit_BNO055(55, 0x28);
float x_a, y_a, z_a = 0;
int rivets_a = 0;
double points_a = 0;
int combo_a = 0;
int max_combo_a = 0;

// Player B Inputs
//const int trigger_b = A10;  // trigger button
const int trigger_b = 19;  // trigger button
boolean trigger_b_ready = false;
static unsigned long last_interrupt_time_b = 0;
//const int motor_b = 9;    // Vibration motor to indicate rivet event
const int photo_b = A5;   // Photodiode input pin
const int photo_b_out = 8;   // Photodiode 5v pin
uint8_t photothresh_b = 60; // Threshold value for photodiode reading
Adafruit_BNO055 bno_b = Adafruit_BNO055(55, 0x29);
float x_b, y_b, z_b = 0;
int rivets_b = 0;
double points_b = 0;
int combo_b = 0;
int max_combo_b = 0;

// Orientation windows
int y_holster = -20;
int z_holster = 60;
int y_rivet = 0;
int z_rivet = 90;
float gyro_thresh = 10.0;

// States
#define DEMO       0
#define TRAINING   1
#define TRAINING_COMPLETE   2
#define GAME       3
#define WINNER     4
#define HIGH_SCORE     5
int state = DEMO;

// Counters
unsigned long counter = 0;
//unsigned long state_timer = 0;
//unsigned long countdown = 0;
//unsigned long motor_a_countdown = 1000; // Limit how long the motor will vibrate the gun for a trigger pull (millis)
//unsigned long motor_b_countdown = 1000; // Limit how long the motor will vibrate the gun for a trigger pull (millis)


void setup() {

  //  // Set the idle timeout
  //  state_timer = millis();

  pinMode(training_btn, INPUT);
  pinMode(play_btn, INPUT);
  // use input for analof current sense input
  //  pinMode(trigger_a, INPUT);
  //  pinMode(trigger_b, INPUT);
  // Use input pullup to
  pinMode(trigger_a, INPUT_PULLUP);
  pinMode(trigger_b, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(trigger_a), on_trigger_a, CHANGE);
  attachInterrupt(digitalPinToInterrupt(trigger_b), on_trigger_b, CHANGE);
  pinMode(photo_a, INPUT);
  pinMode(photo_b, INPUT);
  pinMode(photo_a_out, OUTPUT);
  pinMode(photo_b_out, OUTPUT);
  digitalWrite(photo_a_out, HIGH);
  digitalWrite(photo_b_out, HIGH);

  // initialize serial communications at 115200 bps for status updates
  Serial.begin(115200);
  delay(1000);

  /* Initialise the sensor */
  if (!bno_a.begin())
  {
    serial_update("E", 0); // Error #0 - Gun A not connected
    //    while(1); // TODO - Uncomment to prevent the game from advancing after error
  }


  if (!bno_b.begin())
  {
    serial_update("E", 1); // Error #1 - Gun B not connected
    //      while(1);
  }
  /* Use external crystal for better accuracy */
  bno_a.setExtCrystalUse(true);
  bno_b.setExtCrystalUse(true);

  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  Serial.println("SETUP");

}


/*

  === Main Loop ===

  Do diagnostic/cleanup steps that are common to all states and then drop into state-specific loops (next section)

*/

void loop() {
  counter = (counter + 1) % 1000000;

  if (trigger_a_ready) {
    trigger(1);
    trigger_a_ready = false;
  }


  if (trigger_b_ready) {
    trigger(2);
    trigger_b_ready = false;
  }

  // Update the gun position data
  if (counter % 1000 == 0) {
    update_gun_positions();
  }

  // Animate the next frame of the current game mode.
  if (counter % 100 == 0) {
    switch (state) {
      case DEMO:
        demo();
        break;
      case TRAINING:
        training();
        break;
      case TRAINING_COMPLETE:
        break;
      case GAME:
        game();
        break;
      case WINNER:
        winner();
        break;
      case HIGH_SCORE:
        break;
    }
  }

  if (do_print_diagnostics) {
    // print human-readable diagnostics over serial if specified.
    if (counter % 1000 == 0) {
      Serial.println(state);
      print_diagnostics();
    }
  }

  delayMicroseconds(10);
}


/*

  === State Loops ===

  Treat these like you normally would the main loop() function. But they'll only run in their active state.

*/

void demo()
{
  double phase = sin(counter / 1000);
  double freq = sin(counter / 10000);
  // demo loop color wave
  for (int LEDSerial = 0; LEDSerial < 64; LEDSerial++ ) {
    int row = LEDSerial / NUM_LEDS_PER_ROW;
    double col = (LEDSerial % NUM_LEDS_PER_ROW) / double(NUM_LEDS_PER_ROW);
    double val_raw = ((row * freq) * 0.3) + phase;
    double val = val_raw - floor(val_raw);
    double diff = abs(val - col);
    if (diff < 0.05) {
      leds[LEDSerial] = CRGB(53, 38, 213);
    }
    else if (diff < 0.1) {
      leds[LEDSerial] = CRGB(77, 112, 246);
    }
    else if (diff < 0.15) {
      leds[LEDSerial] = CRGB(50, 217, 203);
    }
    else if (diff < 0.2) {
      leds[LEDSerial] = CRGB(136, 236, 71);
    }
    else if (diff < 0.25) {
      leds[LEDSerial] = CRGB(240, 244, 81);
    }
    else {
      leds[LEDSerial] = CRGB(0, 0, 0);
    }
  }

  FastLED.show();
}


void training()
{
  // training loop
  for (int i = 0 ; i < 64 ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}


void game()
{
  // game loop
}


void winner()
{
  // winner loop
}


void on_trigger_a() {
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 300ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time_a < 300) {
    return;
  }
  last_interrupt_time_a = interrupt_time;

  switch (state) {
    case DEMO:
      to_training();
      break;
    case TRAINING:
      to_training_complete();
      break;
    case TRAINING_COMPLETE:
      to_game();
      break;
    case GAME:
      trigger_a_ready = true;
      break;
    case WINNER:
      to_high_score();
      break;
    case HIGH_SCORE:
      to_demo();
      break;
  }
}

void on_trigger_b() {
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 300ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time_a < 300) {
    return;
  }
  last_interrupt_time_a = interrupt_time;

  switch (state) {
    case DEMO:
      to_training();
      break;
    case TRAINING:
      to_training_complete();
      break;
    case TRAINING_COMPLETE:
      to_game();
      break;
    case GAME:
      trigger_b_ready = true;
      break;
    case WINNER:
      to_high_score();
      break;
    case HIGH_SCORE:
      to_demo();
      break;
  }
}

void trigger(int player) {
  // Ignore holster shots
  if (player == 1) {
    if (is_holstered(x_a, y_a, z_a)) {
      if (do_print_diagnostics) {
        Serial.print("Holstered shot");
      }
      return;
    }
  }
  else {
    if (is_holstered(x_b, y_b, z_b)) {
      if (do_print_diagnostics) {
        Serial.print("Holstered shot");
      }
      return;
    }
  }

  int led_index = -1;

  // Check gyro
  boolean gyro_correct;
  if (player == 1) {
    gyro_correct = is_rivet(x_a, y_a, z_a);
  }
  else {
    gyro_correct = is_rivet(x_b, y_b, z_b);
  }

  if (gyro_correct) {
    // Fire pattern to get led location
    int photo_pin = photo_a;
    if (player == 2) {
      photo_pin = photo_b;
    }
    led_index = read_led(photo_pin) - 1;
    if (do_print_diagnostics) {
      Serial.println("GYRO correct");
      Serial.println(led_index);
    }
  }
  else if (do_print_diagnostics) {
    Serial.println("GYRO incorrect");
  }

  // If gun is not postioned right led index will be 255 or -1
  if (led_index >= 0 && led_index < NUM_LEDS) {
    fire_sucess(led_index, player);
  }
  else {
    fire_fail(player);
  }

  write_led_states();
}

void fire_sucess(int led_index, int player) {
  // Ignore already riveted holes
  if (led_states[led_index] != 0) {
    return;
  }

  led_states[led_index] = player;

  // Update scores
  if (player == 1) {
    rivets_a += 1;
    points_a += 1.0 + (double(combo_a) / 10.0);
    combo_a += 1;
    max_combo_a = max(combo_a, max_combo_a);
  }
  else {
    rivets_b += 1;
    points_b += 1.0 + (double(combo_b) / 10.0);
    combo_b += 1;
    max_combo_b = max(combo_b, max_combo_b);
  }

  // Tell arduino about success for sound
  serial_update("V", player);

  if (have_won()) {
    to_winner();
  }

  serial_update("R", rivets_a);
  serial_update("r", rivets_b);
  serial_update("P", points_a);
  serial_update("p", points_b);
  serial_update("C", combo_a);
  serial_update("c", combo_b);
  serial_update("M", max_combo_a);
  serial_update("m", max_combo_b);
}

void fire_fail(int player) {
  // Tell arduino about failure for sound
  serial_update("V", 3);

  // Update scores
  if (player == 1) {
    combo_a = 0;
  }
  else {
    combo_b = 0;
  }

  serial_update("R", rivets_a);
  serial_update("r", rivets_b);
  serial_update("P", points_a);
  serial_update("p", points_b);
  serial_update("C", combo_a);
  serial_update("c", combo_b);
  serial_update("M", max_combo_a);
  serial_update("m", max_combo_b);
}

boolean have_won() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (led_states[i] == 0) return false;
  }
  return true;
}

/*

  === State Transitions ===

  Move to a new state and set/reset any necessary variables to get that state started.

*/

void to_demo()
{
  // start demo loop
  state = DEMO;
  serial_update("S", state);
}


void to_training()
{
  // start training loop
  state = TRAINING;
  serial_update("S", state);
}

void to_training_complete()
{
  // start training loop
  state = TRAINING_COMPLETE;
  serial_update("S", state);
}


void to_game()
{
  // start game loop
  state = GAME;
  points_a = 1.30;
  points_b = 8.90;
  combo_a = 0;
  max_combo_a = 0;
  combo_b = 0;
  max_combo_b = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    led_states[i] = 0;
  }
  serial_update("S", state);
}


void to_winner()
{
  // start winner loop
  state = WINNER;
  serial_update("S", state);

  //  // Winner loop animation timeout (milliseconds)
  //  countdown = 10000;
  //  state_timer = millis();
}

void to_high_score()
{
  // start winner loop
  state = HIGH_SCORE;
  serial_update("S", state);

  //  // Winner loop animation timeout (milliseconds)
  //  countdown = 10000;
  //  state_timer = millis();
}


/*

  === IMU Functions ===

  Parse the output from the gun positions and check that orientation is within different windows.

*/

void update_gun_positions() {
  /* Get a new sensor event */
  sensors_event_t event_a;
  bno_a.getEvent(&event_a);
  x_a = event_a.orientation.x;
  y_a = event_a.orientation.y;
  z_a = event_a.orientation.z;

  sensors_event_t event_b;
  bno_b.getEvent(&event_b);
  x_b = event_b.orientation.x;
  y_b = event_b.orientation.y;
  z_b = event_b.orientation.z;
}

bool is_holstered(float x, float y, float z) {
  int y_diff = min(abs(y - y_holster), abs(360 + y - y_holster));
  int z_diff = min(abs(z - z_holster), abs(360 + z - z_holster));
  if (y_diff < gyro_thresh && z_diff < gyro_thresh) {
    return true;
  }
  else {
    return false;
  }
}

bool is_rivet(float x, float y, float z) {
  int y_diff = min(abs(y - y_rivet), abs(360 + y - y_rivet));
  int z_diff = min(abs(z - z_rivet), abs(360 + z - z_rivet));
  if (y_diff < gyro_thresh && z_diff < gyro_thresh) {
    return true;
  }
  else {
    return false;
  }
}



/*
    ==== LEDS ===
*/

byte read_led(int diode_pin) {

  byte result = 0;

  for (int serialByte = 7; serialByte >= 0; serialByte-- ) {
    for (int LEDSerial = 0; LEDSerial < 64; LEDSerial++ ) {
      if (bitRead(LEDSerial + 1, serialByte)) {
        leds[LEDSerial] = CRGB(255, 255, 255);
      }
      else {
        leds[LEDSerial] = CRGB(0, 0, 0);
      }

    }

    FastLED.show();

    //    delay(50);
    result = (result << 1) + byte(analogRead(diode_pin) < 600);
  }

  // Error correction
  byte result2 = 0;

  for (int serialByte = 7; serialByte >= 0; serialByte-- ) {
    for (int LEDSerial = 0; LEDSerial < 64; LEDSerial++ ) {
      if (bitRead(LEDSerial + 1, serialByte)) {
        leds[LEDSerial] = CRGB(255, 255, 255);
      }
      else {
        leds[LEDSerial] = CRGB(0, 0, 0);
      }

    }

    FastLED.show();

    //    delay(50);
    result2 = (result2 << 1) + byte(analogRead(diode_pin) < 600);
  }

  if (result != result2) {
    return 0;
  }

  return result;
}

void write_led_states() {
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    if (led_states[i] == 1) {
      leds[i] = CRGB(127, 127, 0);
    }
    else if (led_states[i] == 2) {
      leds[i] = CRGB(0, 127, 0);
    }
    else {
      leds[i] = CRGB(0, 0, 50);
    }
  }
  FastLED.show();
}

/*

  === Serial Functions ===

  Communucate over Serial to the Pi.
  This implements no mechanism to get messages from the Pi because that shouldn't happen.
  Control is cleaner if it is one-way, so the Pi is just a follower of the Arduino.

*/

void serial_update(char* key, double val) {
  // Standard update message to communicate with RasPi
  // {key: value} pair defines updates of specific variables
  Serial.print(key);
  Serial.println(val);
}

void serial_update(char* key, int val) {
  // Standard update message to communicate with RasPi
  // {key: value} pair defines updates of specific variables
  Serial.print(key);
  Serial.println(val);
}

void print_diagnostics() {
  Serial.print("x_a=");
  Serial.print(x_a);
  Serial.print(" y_a=");
  Serial.print(y_a);
  Serial.print(" z_a=");
  Serial.print(z_a);
  Serial.print(" x_b=");
  Serial.print(x_b);
  Serial.print(" y_b=");
  Serial.print(y_b);
  Serial.print(" z_b=");
  Serial.print(z_b);
  //  Serial.print(" =");
  //  Serial.print();
  Serial.println("");
}
