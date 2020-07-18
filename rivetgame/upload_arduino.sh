cd /home/pi/rivetgame/RivetGameController
#sudo apt-get install xvfb -y
Xvfb :4 -nolisten tcp -screen :$SCREEN 1280x800x24 &
xvfb="$!"
DISPLAY=:4 arduino --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyUSB0 --upload /home/pi/rivetgame/RivetGameController/RivetGameController.ino
kill -9 $xvfb
