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
bool do_print_diagnostics = true;

// Inputs
const int training_btn = 2;  // Start training
const int play_btn = 3;  // Start playing
// Player A
const int trigger_a = 4;  // trigger button
const int motor_a = 5;  // Vibration motor to indicate rivet event
Adafruit_BNO055 bno_a = Adafruit_BNO055(55, 0x28);
float x_a, y_a, z_a = 0;
// Player B
const int trigger_b = 8;  // trigger button
const int motor_b = 9;  // Vibration motor to indicate rivet event
Adafruit_BNO055 bno_b = Adafruit_BNO055(55, 0x29);
float x_b, y_b, z_b = 0;
// Orientation windows
uint8_t x_holster = 320;
uint8_t y_holster = -20;
uint8_t z_holster = 60;
uint8_t x_rivet = 170;
uint8_t y_rivet = -30;
uint8_t z_rivet = 15;


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

  // initialize serial communications at 115200 bps for status updates
  Serial.begin(115200);
  delay(1000);
  
  /* Initialise the sensor */
  if(!bno_a.begin())
  {
    serial_update("E", 0); // Error #0 - Gun A not connected
    while(1);
  }
//  if(!bno_b.begin())
//  {
//    serial_update("E", 1); // Error #1 - Gun B not connected
//    while(1);
//  }
  /* Use external crystal for better accuracy */
  bno_a.setExtCrystalUse(true);
  bno_b.setExtCrystalUse(true);

}


// === Main Loop ===


void loop() {
  counter = (counter + 1) % 1000000;
  
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
      case GAME:
        game();
        break;
      case WINNER:
        winner();
        break;
    }
  }
  
  // Update the RasPi on the current state.
  if (counter % 1000 == 0 && do_print_diagnostics == false) {
    serial_update("S", state);
  }
  // Or print plain text to a serial terminal
  if (counter % 1000 == 0 && do_print_diagnostics == true) {
    print_diagnostics();
  }
  
  
  delayMicroseconds(10);

}


// === State Loops ===


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
  // game loop
}


void winner()
{
  // winner loop
}


// === State Transitions ===


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
}


void to_winner()
{
  // start winner loop
  state = WINNER;

  // Winner loop animation timeout (milliseconds)
  countdown = 10000;
  state_timer = millis();
}


// === Helper Functions ===


void serial_update(char* key, int val) {
  // Standard update message to communicate with RasPi
  // {key: value} pair defines updates of specific variables
  Serial.print(key);
  Serial.println(val);
}

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
