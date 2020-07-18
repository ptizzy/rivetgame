cd /home/pi/rivetgame/rivetgame
amixer set PCM -- 100%
echo "Waiting 5 seconds to connect to the internet"
sleep 5
echo "pulling github"
git pull origin master
sleep 5
echo "waiting 10 seconds for arduino to upload code"
arduino --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyUSB0 --upload /home/pi/rivetgame/RivetGameController/RivetGameController.ino &
arduino --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyUSB1 --upload /home/pi/rivetgame/RivetGameController/RivetGameController.ino &
sleep 10
python3 launcher.py