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


bool is_testing = false;
bool do_print_diagnostics = true;

// Inputs
const int training_btn = 0;  // Start training
const int play_btn = 1;  // Start playing
// Player A
const int trigger_a = 4;  // trigger button
const int accel_sda_a = 20;  // Accelerometer input
const int accel_scl_a = 21;  // Accelerometer input
const int motor_a = 5;  // Vibration motor to indicate rivet event
// Player B
const int trigger_b = 8;  // trigger button
const int accel_sda_b = 70;  // Accelerometer input
const int accel_scl_b = 71;  // Accelerometer input
const int motor_b = 9;  // Vibration motor to indicate rivet event

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

  pinMode(training_btn, INPUT);
  pinMode(play_btn, INPUT);
  pinMode(trigger_a, INPUT);
  pinMode(trigger_b, INPUT);
  
  // Set the idle timeout
  state_timer = millis();

  // initialize serial communications at 115200 bps for status updates
  Serial.begin(115200);

}


// === Main Loop ===


void loop() {
  counter = (counter + 1) % 1000000;
  
  // Update the RasPi on the current state.
  if (counter % 100 == 0) {
    serial_update("S", state);
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
  delayMicroseconds(10);

}


// === State Loops ===


void demo()
{
  // demo loop
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
