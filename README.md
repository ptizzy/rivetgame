# Rivet Race

This project is a combination of code to drive an Arduino controller and a pygame display for a Rivet race game.

The RivetGameController is the Arduino logic for the game, which manages state and the main counters/timers.

The rivetgame folder contains python code to run the pygame UI on the Raspberry Pi to run animations, display scores, and track high scores.

The two communicate with each other over serial using a variant of the 2B protocol. All messages start with a 1 byte "message type" followed by an integer value, and then a newline.

## Getting Started

Install the RivetGameController on an Arduino with interfaces to plug in the sensors for the two rivet guns. The logic has more details on the pinout and wiring.

Plug the arduino USB into the Raspberry Pi that is running the custom game image. The Pi will auto-start in a full-screen game mode. 

To update the Pi, press ESC to exit to the console and, while on an internet connection, do a "git pull" to download the latest code changes.

### Prerequisites

#### Arduino

You'll need the Arduino IDE to run the controller logic. Use the Library Manager to install the libraries for FastLED and the Adafruit_BNO055 interface.


```
pip install -r requirements.txt
```

Then create a launch.sh file in the pi user home directory, give it execute permission, and add it to the
.bashrc to auto-start with it.

### Installing
#### Python

To run on your local machine, you'll need Python 3.5+. Then simply run 

```
pip install -r requirements.txt
```

to install the other dependencies.

#### Pi

It is recommended that the Pi be installed from the base image for this project, which is already pre-configured and 
pre-installed with the correct dependencies and auto-start behavior.

If necessary, the Pi can be rebuilt from the ground up with Raspian Lite with

```
sudo apt install -y python3-pip git
```

and

```
git clone https://github.com/Steamboat/rivetgame.git
```

installing git and python3-pip, cloning the repo, and installing
the dependencies with.


## Running the tests

Because the RivetGameController is the driving controller on the project and the Pi is more of a follower display, we
aren't currently using unit tests on the project.

Instead, the Controller has a "test mode" where human-readable data is fed to the serial monitor instead of the encoded
messages, giving a window into the game state and sensor data.

### End to end tests

Hook up all of the controls for the guns to the Arduino and run with do_print_diagnostics=true while ensuring that all
of the relevant sensor data is coming through on the monitor and is correct.

Turn do_print_diagnostics to false and verify that the messages are now streaming from the Arduino as a character and
integer pair per line.

Hook the arduino into the pi and boot up. Ensure that the Pi finds the arduino and drops straight into the game screen
on boot. If the pi can't find an Arduino it will throw an exception.

TODO - specify the arduino port specifically with a command-line argument.

To manually launch the game screen on the pi, run

```
python3 /rivetgame/rivetgame/launcher.py
```

from the pi user home folder ~/home/pi

## Implementation Details

### Finite-State Machine

The core of the RivetGameController logic is a Finite State Machine. The game has many states and this
logic controls the current state and the transitions in between. The current states are:

* DEMO - Auto-started demo loop after a timeout or a winner. Various actions can exit to TRAINING or GAME.
* TRAINING - A "lobby" before the game where the user can learn more or choose a training loop.
* GAME - Two users race against each other as scores accumulate and a timer counts down.
* WINNER - After the game state times out, a winner is announced. automatically cycles bacK to DEMO after.

The Arduino updates the Pi periodically about its current state so that the display content and styling can be updated.


## Built With

* [pygame](https://www.pygame.org/wiki/GettingStarted) - The pi interface used.
* [pyserial](https://pythonhosted.org/pyserial/) - Interface to the arduino
* [FastLED](http://fastled.io/) - High-speed LED controller
* [Adafruit_BNO055](https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor/arduino-code) - Interface to 9-axis IMU to determine gun orientation.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

