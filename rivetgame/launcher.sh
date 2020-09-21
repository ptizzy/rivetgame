# Set the pi to reboot at midnight
echo "#! /bin/bash
/sbin/reboot" > /etc/cron.daily/rivet_reboot &
chmod +x /etc/cron.daily/rivet_reboot &

cd /home/pi/rivetgame/rivetgame
amixer set PCM -- 100%
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
unzip packed.zip
#pip install --upgrade firebase-admin
python3 launcher.py