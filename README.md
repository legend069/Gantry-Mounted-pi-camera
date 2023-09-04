# Gantry-Mounted-pi-camera
description - work in progress  
all the credit for the code goes to this guy https://github.com/ayufan/camera-streamer

![ApplicationFrameHost_hAYocPCnF6](https://github.com/legend069/Gantry-Mounted-pi-camera/assets/40685552/c8b8f9ff-bf4a-4b5e-b091-0c0ae0ff47a6)



## repo download
```
git clone https://github.com/legend069/Gantry-Mounted-pi-camera.git
```
## install commands
```
sudo apt-get -y install libavformat-dev libavutil-dev libavcodec-dev libcamera-dev liblivemedia-dev v4l-utils pkg-config xxd build-essential
cd ~
ln -s /home/pi/Gantry-Mounted-pi-camera/repo/camera-streamer /home/pi/camera-streamer
cd camera-streamer/
make
sudo make install
```
## setup commands for camera
```
systemctl enable $PWD/service/camera-streamer-raspi-v2-8MP.service 
systemctl start camera-streamer-raspi-v2-8MP 
------or------ 
systemctl enable $PWD/service/camera-streamer-raspi-v3-12MP.service 
systemctl start camera-streamer-raspi-v3-12MP.service 
```
### to view logs
``journalctl -xef -u camera-streamer-raspi-v2-8MP ``
