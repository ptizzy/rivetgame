/*
 * Rivet Game
 * 
 * State-machine for Rivet game with 4 states - demo, training, game, and winner.
 * 
 * In game mode, write out 8-bit ids to LED's and scan them bit by bit with photodiode output.
 * 
 * Collect up higher-level data and write it out over serial.
 * 
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


bool is_testing = false;
bool do_print_diagnostics = false;

// Inputs
const int training_btn = 2;  // Start training
const int play_btn = 3;  // Start playing

// Player A Inputs
const int trigger_a = 4;  // trigger button
const int motor_a = 5;    // Vibration motor to indicate rivet event
const int photo_a = A0;   // Photodiode input pin
uint8_t photothresh_a = 60; // Threshold value for photodiode reading
uint8_t photoval_a = 0; // Parsed photodiode value
Adafruit_BNO055 bno_a = Adafruit_BNO055(55, 0x28);
float x_a, y_a, z_a = 0;
int points_a = 0;

// Player B Inputs
const int trigger_b = 8;  // trigger button
const int motor_b = 9;    // Vibration motor to indicate rivet event
const int photo_b = A4;   // Photodiode input pin
uint8_t photothresh_b = 60; // Threshold value for photodiode reading
uint8_t photoval_b = 0; // Parsed photodiode value
Adafruit_BNO055 bno_b = Adafruit_BNO055(55, 0x29);
float x_b, y_b, z_b = 0;
int points_b = 0;

// Orientation windows
int x_holster = 320;
int y_holster = -20;
int z_holster = 60;
int x_rivet = 170;
int y_rivet = -30;
int z_rivet = 15;

// States
#define DEMO       0
#define TRAINING   1
#define GAME       2
#define WINNER     3
int state = DEMO;

// Counters
unsigned long counter = 0;
unsigned long state_timer = 0;
unsigned long countdown = 0;
unsigned long motor_a_countdown = 1000; // Limit how long the motor will vibrate the gun for a trigger pull (millis)
unsigned long motor_b_countdown = 1000; // Limit how long the motor will vibrate the gun for a trigger pull (millis)


void setup() {
  
  // Set the idle timeout
  state_timer = millis();

  pinMode(training_btn, INPUT);
  pinMode(play_btn, INPUT);
  pinMode(trigger_a, INPUT);
  pinMode(trigger_b, INPUT);
  pinMode(photo_a, INPUT);
  pinMode(photo_b, INPUT);

  // initialize serial communications at 115200 bps for status updates
  Serial.begin(115200);
  delay(1000);
  
  /* Initialise the sensor */
  if(!bno_a.begin())
  {
    serial_update("E", 0); // Error #0 - Gun A not connected
//    while(1); // TODO - Uncomment to prevent the game from advancing after error
  }

// TODO - uncomment this once the addresses on the IMUs are set and both can be found.
//  if(!bno_b.begin())
//  {
//    serial_update("E", 1); // Error #1 - Gun B not connected
//    while(1);
//  }
  /* Use external crystal for better accuracy */
  bno_a.setExtCrystalUse(true);
  bno_b.setExtCrystalUse(true);

}


/*
 * 
 * === Main Loop ===
 * 
 * Do diagnostic/cleanup steps that are common to all states and then drop into state-specific loops (next section)
 * 
*/

void loop() {
  counter = (counter + 1) % 1000000;
  
  // Update the gun position data
  if (counter % 1000 == 0) {
    update_gun_positions();
  }
  // Update the photodiode state - TODO: move into LED animation logic.
  if (counter % 1000 == 0) {
    update_photodiodes();
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
      case GAME:
        game();
        break;
      case WINNER:
        winner();
        break;
    }
  }
  // Update the RasPi over serial on the current state.
  if (counter % 100 == 0 && do_print_diagnostics == false) {
    serial_update("S", state);
  }
  if (counter % 100 == 20 && do_print_diagnostics == false) {
    serial_update("P", points_a);
  }
  if (counter % 100 == 40 && do_print_diagnostics == false) {
    serial_update("p", points_b);
  }
  // OR print human-readable diagnostics over serial if specified.
  if (counter % 1000 == 0 && do_print_diagnostics == true) {
    print_diagnostics();
  }
  delayMicroseconds(10);

}


/*
 * 
 * === State Loops ===
 * 
 * Treat these like you normally would the main loop() function. But they'll only run in their active state.
 * 
*/

void demo()
{
  // demo loop

  // Check for trigger pull to exit demo mode
  if (digitalRead(trigger_a) == LOW || digitalRead(trigger_b) == LOW) {
    to_game();
  }
}


void training()
{
  // training loop
}


void game()
{
  // game loop (domo code just increments points)
  points_a += 1;
  points_b += 2;
}


void winner()
{
  // winner loop
}


/*
 * 
 * === State Transitions ===
 * 
 * Move to a new state and set/reset any necessary variables to get that state started.
 * 
*/

void to_demo()
{
  // start demo loop
  state = DEMO;
}


void to_training()
{
  // start training loop
  state = TRAINING;
}


void to_game()
{
  // start game loop
  state = GAME;
  points_a = 0;
  points_b = 0;
}


void to_winner()
{
  // start winner loop
  state = WINNER;

  // Winner loop animation timeout (milliseconds)
  countdown = 10000;
  state_timer = millis();
}


/*
 * 
 * === IMU Functions ===
 * 
 * Parse the output from the gun positions and check that orientation is within different windows.
 * 
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

bool is_holstered(float x, float y, float z, float thresh=20) {
  if (abs(x-x_holster) < thresh && abs(y-y_holster) < thresh && abs(z-z_holster) < thresh) {
    return true;
  } else {
    return false;
  }
}

bool is_rivet(float x, float y, float z, float thresh=20) {
  if (abs(x-x_rivet) < thresh && abs(y-y_rivet) < thresh && abs(z-z_rivet) < thresh) {
    return true;
  } else {
    return false;
  }
}


/*
 * 
 * === Photodiode Functions ===
 * 
 * Parse the photodiode readings.
 * 
*/

uint8_t get_photodiode_val(int diode_pin, int thresh=60) {
  int temp_val = analogRead(diode_pin);
  if (temp_val > thresh) {
    return 0;
  } else {
    return 1;
  }
}

void update_photodiodes() {
  photoval_a = get_photodiode_val(photo_a, photothresh_a);
  photoval_b = get_photodiode_val(photo_b, photothresh_b);
}


/*
 * 
 * === Serial Functions ===
 * 
 * Communucate over Serial to the Pi.
 * This implements no mechanism to get messages from the Pi because that shouldn't happen. 
 * Control is cleaner if it is one-way, so the Pi is just a follower of the Arduino.
 * 
*/

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
