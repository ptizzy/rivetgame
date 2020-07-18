cd /home/pi/rivetgame/rivetgame
amixer set PCM -- 70%
#echo "--------------------------------------------"
echo "Waiting 5 seconds to connect to the internet"
#echo "--------------------------------------------"
sleep 5
#echo "----------------------------"
echo "Pulling new code from github"
#echo "----------------------------"
git pull origin master
sleep 5
##echo "---------------------------------------------"
#echo "Waiting 10 seconds for arduino to upload code"
##echo "---------------------------------------------"
#sh upload_arduino.sh &
#sleep 10
python3 launcher.py