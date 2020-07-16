//
//   Rivet Game
//  Collect up higher-level data and write it out over serial.


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Arduino.h>
#include <FastLED.h>


#define NUM_LEDS_PER_ROW 18
#define NUM_LEDS 144
#define DATA_PIN 10
#define CLOCK_PIN 11

CRGB leds[NUM_LEDS];
int led_states[NUM_LEDS];

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
Adafruit_BNO055 bno_a = Adafruit_BNO055(55, 0x28);
float x_a, y_a, z_a = 0;
int rivets_a = 0;
double points_a = 0;
int combo_a = 0;
int rivet_attempts_a = 0;

// Player B Inputs
//const int trigger_b = A10;  // trigger button
const int trigger_b = 19;  // trigger button
boolean trigger_b_ready = false;
static unsigned long last_interrupt_time_b = 0;
//const int motor_b = 9;    // Vibration motor to indicate rivet event
const int photo_b = A5;   // Photodiode input pin
const int photo_b_out = 8;   // Photodiode 5v pin
Adafruit_BNO055 bno_b = Adafruit_BNO055(55, 0x29);
float x_b, y_b, z_b = 0;
int rivets_b = 0;
double points_b = 0;
int combo_b = 0;
int rivet_attempts_b = 0;

// Orientation windows
//int y_holster = -20;
//int z_holster = 60;
//int y_rivet = 0;
int z_rivet = 0;
float gyro_thresh = 20.0;

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
long state_timer = 0;
unsigned long last_time = 0;
boolean training_complete = false;


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
  while (!bno_a.begin())
  {
    serial_update("E", 0); // Error #0 - Gun A not connected
    delay(1000);
  }


  while (!bno_b.begin())
  {
    serial_update("E", 1); // Error #1 - Gun B not connected
    delay(1000);
  }
  /* Use external crystal for better accuracy */
  bno_a.setExtCrystalUse(true);
  bno_b.setExtCrystalUse(true);

  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  Serial.println("SETUP");

}




// == Main Loop ===

// Do diagnostic/cleanup steps that are common to all states and then drop into state-specific loops (next section)


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
        high_score();
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




// === State Loops ===

// Treat these like you normally would the main loop() function. But they'll only run in their active state.



void demo()
{

  if (counter % 1000 == 0) {
//    serial_update("A", int(z_a + 180.0) % 360);
//    serial_update("a", int(z_b + 180.0) % 360);

    if (
      (z_a > 0 && z_a < 60) || (z_b > 0 && z_b < 60) || 
      (abs(z_a) > 180 && abs(z_a) < 240) || (abs(z_b) > 180 && abs(z_b) < 240)
      ) {
      to_training();
    }
  }

  demo_lights();

  FastLED.show();
}


void training()
{
  if (counter % 1000 == 0) {
    serial_update("A", int(z_a + 180.0) % 360);
    serial_update("a", int(z_b + 180.0) % 360);
    serial_update("R", 650 - analogRead(photo_a));
    serial_update("r", 650 - analogRead(photo_b));

    if (!training_complete) {
      serial_update("T", -1);
      if ((is_rivet(z_a) && analogRead(photo_a) < 600) ||
          (is_rivet(z_b) && analogRead(photo_b) < 600)) {
        // Someone completed training
        state_timer = millis();
        training_complete = true;
      }

      // if we have been sitting in training mode for 2 minutes return to demo mode
      if(millis() - state_timer > 180000) {
        to_demo();
      }
    } else {
      // Let the second person have a couple seconds to keep trying
      int seconds_left = 10 - int((millis() - state_timer) / 1000);
      serial_update("T", seconds_left);

      if (seconds_left <= 0) {
        // wait 15 seconds after training done to transition
        to_training_complete();
      }
    }
  }

  // training loop
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 200);
  }
  FastLED.show();
}



void game()
{
  // game loop
  int remaining_seconds = 45 - ((millis() - state_timer) / 1000);

  if (remaining_seconds <= 0) {
    to_winner();
  }

  if (last_time != remaining_seconds) {
    serial_update("T", remaining_seconds);
    last_time = remaining_seconds;
  }
}

void winner() {
  demo_lights();

  // after 45 seconds change mode automatically
  if (millis() - state_timer > 45000) {
    to_high_score();
  }
}

void high_score() {
  demo_lights();

  if (millis() - state_timer > 45000) {
    to_demo();
  }
}

//// Trigger events /////

void on_trigger_a() {
  //  serial_update("Z", 1);
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 300ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time_a < 300) {
    return;
  }
  last_interrupt_time_a = interrupt_time;

  switch (state) {
    case DEMO:
      //      to_training();
      break;
    case TRAINING:
      //      to_training_complete();
      break;
    case TRAINING_COMPLETE:
      to_game();
      break;
    case GAME:
      trigger_a_ready = true;
      break;
    case WINNER:
      // Wait at least 15 seconds before leaving screen
      if (millis() - state_timer > 15000) {
        to_high_score();
      }
      break;
    case HIGH_SCORE:
      // Wait at least 15 seconds before leaving screen
      if (millis() - state_timer > 15000) {
        to_demo();
      }
      break;
  }
}

void on_trigger_b() {
  //  serial_update("Z", 2);
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 300ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time_b < 300) {
    return;
  }
  last_interrupt_time_b = interrupt_time;

  switch (state) {
    case DEMO:
      //      to_training();
      break;
    case TRAINING:
      //      to_training_complete();
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
  int led_index = -1;

  // Check gyro
  boolean gyro_correct;
  if (player == 1) {
    gyro_correct = is_rivet(z_a);
  }
  else {
    gyro_correct = is_rivet(z_b);
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

  if(player == 1) {
    rivet_attempts_a += 1;
    serial_update("M", rivet_attempts_a);
  } else {
    rivet_attempts_b += 1;
    serial_update("m", rivet_attempts_b);
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
  } else {
    rivets_b += 1;
    points_b += 1.0 + (double(combo_b) / 10.0);
    combo_b += 1;
  }

  // Tell arduino about success for sound
  serial_update("V", player);

  if (have_won()) {
    to_winner();
  }

  if (player == 1) {
    serial_update("C", combo_a);
    serial_update("P", points_a);
    serial_update("R", rivets_a);
  } else {
    serial_update("p", points_b);
    serial_update("c", combo_b);
    serial_update("r", rivets_b);
  }
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

  if (player == 1) {
    serial_update("C", combo_a);
  } else {
    serial_update("c", combo_b);
  }
}

boolean have_won() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (led_states[i] == 0) return false;
  }
  return true;
}



//  === State Transitions ===

//  Move to a new state and set/reset any necessary variables to get that state started.



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
  training_complete = false;
  state_timer = millis();
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
  points_a = 0;
  points_b = 0;
  combo_a = 0;
  rivet_attempts_a = 0;
  combo_b = 0;
  rivet_attempts_b = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    led_states[i] = 0;
  }

  // Winner loop animation timeout (milliseconds)
  state_timer = millis();

  serial_update("S", state);
  serial_update("R", rivets_a);
  serial_update("r", rivets_b);
  serial_update("P", points_a);
  serial_update("p", points_b);
  serial_update("C", combo_a);
  serial_update("c", combo_b);
  serial_update("M", rivet_attempts_a);
  serial_update("m", rivet_attempts_b);

  game();
}


void to_winner()
{
  // start winner loop
  state_timer = millis();
  state = WINNER;
  serial_update("S", state);
}

void to_high_score()
{
  // start high score loop
  state_timer = millis();
  state = HIGH_SCORE;
  serial_update("S", state);

}


/*

  === IMU Functions ===

  Parse the output from the gun positions and check that orientation is within different windows.

*/

void update_gun_positions() {
  /* Get a new sensor event */
  sensors_event_t event_a;
  bno_a.getEvent(&event_a);
  x_a = event_a.acceleration.x;
  y_a = event_a.acceleration.y;
  z_a = event_a.acceleration.z;

  sensors_event_t event_b;
  bno_b.getEvent(&event_b);
  x_b = event_b.acceleration.x;
  y_b = event_b.acceleration.y;
  z_b = event_b.acceleration.z;
}


bool is_rivet(float z) {
  int z_diff = min(min(min(abs(z - z_rivet), abs(180 + z - z_rivet)), abs(z - z_rivet - 180)), abs(z - z_rivet - 360));
  if (z_diff < gyro_thresh) {
    return true;
  }
  else {
    return false;
  }
}



/*
    ==== LEDS ===
*/


void demo_lights() {
  double phase = sin(double(counter) / 5000.0);
  double freq = sin(double(counter) / 50000.0);
  double amplitude = cos(double(counter) / 20000.0) / 2.0;
  // demo loop color wave
  for (int LEDSerial = 0; LEDSerial < NUM_LEDS; LEDSerial++ ) {
    int row = LEDSerial / NUM_LEDS_PER_ROW;
    double col = (LEDSerial % NUM_LEDS_PER_ROW);
    if (row % 2 == 0) {
      col = NUM_LEDS_PER_ROW  - col;
    }
    col = col  / double(NUM_LEDS_PER_ROW);
    double val_raw = ((row * freq) * (amplitude + 0.3)) + phase;
    double val = val_raw - floor(val_raw);
    double diff = min(min(abs(val - col), abs(val - col - 1)), abs(val - col + 1));
    if (diff < 0.1) {
      leds[LEDSerial] = CRGB(150, 5, 0);
    }
    else if (diff < 0.2) {
      leds[LEDSerial] = CRGB(180, 180, 180);
    }
    else if (diff < 0.3) {
      leds[LEDSerial] = CRGB(150, 150, 0);
    }
    else {
      leds[LEDSerial] = CRGB(0, 0, 0);
    }
  }

  FastLED.show();
}


void demo_lights_2() {
  double phase = sin(double(counter) / 5000.0);
  double freq = sin(double(counter) / 50000.0);
  double amplitude = cos(double(counter) / 20000.0) / 2.0;
  // demo loop color wave
  for (int LEDSerial = 0; LEDSerial < NUM_LEDS; LEDSerial++ ) {
    int row = LEDSerial / NUM_LEDS_PER_ROW;
    double col = (LEDSerial % NUM_LEDS_PER_ROW);
    int r = int(abs(255 * sin((double(counter) / 5000.0) + (row * sin((double(counter) / 50000.0)) / 50))));
    int g = int(abs(155 * cos((double(counter) / 17000.0) + (row * col * sin((double(counter) / 20000.0)) / 20))));
    int b = int(abs(155 * sin((double(counter) / 4000.0) + (col * sin((double(counter) / 10000.0)) / 10))));
    leds[LEDSerial] = CRGB(b, g, r);
  }

  FastLED.show();
}

byte read_led(int diode_pin) {

  byte result = 0;

  for (int serialByte = 7; serialByte >= 0; serialByte-- ) {
    for (int LEDSerial = 0; LEDSerial < NUM_LEDS; LEDSerial++ ) {
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
    for (int LEDSerial = 0; LEDSerial < NUM_LEDS; LEDSerial++ ) {
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
      leds[i] = CRGB(0, 127, 200);
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
