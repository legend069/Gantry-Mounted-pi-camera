# Gantry-Mounted-pi-camera
description - work in progress  

![ApplicationFrameHost_hAYocPCnF6](https://github.com/legend069/Gantry-Mounted-pi-camera/assets/40685552/c8b8f9ff-bf4a-4b5e-b091-0c0ae0ff47a6)

# summary
there are two pi camera versions to choose from.
plug and play - exactly as it sounds. plug cable into PCB's and the FPC cables in and away you go!
  but this one costs more $$ in parts.

raw dog - i like to call it raw dog because you're basically raw dogging it.
  this one is for the soldering experts(or atleast above beginner) you cut up a DP cable(any length as long as it will reach point a to point b)
  strip the very tiny and fragile wires and solder them directly to the PCB.
  is a cheaper for parts.

i originally wanted this to be cheap

## whats the differences between the two?
*  plug-play takes up more Z height(this is mainly because of the cable)
*  raw-dog much more compact, but requires a skilled and patient soldering

  the max FPS you can get between the two is the same, the limiter is the camera, my target for the project was 30fps. during my testing phase it can push up to 45(pi camera v3 wide) before it starts to have frame stutter/visable noise.
  if you're going with the raw-dog method getting maxfps (40+) gets harder and harder with the soldering work. because you need to get all 8 wires to the PCB the **same length!**

## tips for soldering raw-dog
* this takes a long time, it'll take around 2 hours to solder both PCB's + add some more time if some wires break :angry: during the trials i broke around 10 wires during moving from the solder work bench to the printer.
* the pro tip i found that helped out a bunch was to use some cat5e(any solid core network cable) and extend the ends of the DisplayPort cable. and heatshrink the bits in between.[picture](https://github.com/legend069/Gantry-Mounted-pi-camera/blob/main/Pictures/Build%20Process%20Pictures/IMG_20230618_065408.png)
* as long as you **cut** the lengths as good as you can, then tin the wires and start soldering. use very light solder with the tinning and PCB work.
  because you can use tweezers and slightly pull/press on the wire to help get it aligned up with the other **this is a critical step!** [reference](https://github.com/legend069/Gantry-Mounted-pi-camera/blob/main/Pictures/Build%20Process%20Pictures/IMG_20230614_181707.jpg)
* if you can't get the wires length matching as close as possible to perfect. don't worry, it's not the end of the world. you'll just get frame [stutter/visual noise](https://www.google.com.au/search?q=pi+camera+purple+lines&sca_esv=581692793&tbm=isch&sxsrf=AM9HkKmksJV1oYjezeAQssLkEamQde-6yA:1699791685282&source=lnms&sa=X&ved=2ahUKEwj1od-8ub6CAxUy-TgGHfx2BIAQ_AUoAXoECAMQAw&biw=1920&bih=927&dpr=1) on high framerates.


if you want more pictures they're [here](https://github.com/legend069/Gantry-Mounted-pi-camera/tree/main/Pictures)

the picture below is the raw-dog camera in action- pi camera V3 wide
![snapshot](https://github.com/legend069/Gantry-Mounted-pi-camera/assets/40685552/941b0a4e-c5f0-426c-9324-be48a6b15a85)

the picture below is the raw-dog camera in action- pi camera V2
![snapshotV2cam](https://github.com/legend069/Gantry-Mounted-pi-camera/assets/40685552/6eda33bf-09a4-4384-833e-e797fe02dddc)

and this one is the micro-hdmi (plug and play) pi camera V2
![micro1](https://github.com/legend069/Gantry-Mounted-pi-camera/assets/40685552/4159db74-939b-44f4-96c7-2cbd8669d326)

i don't have the photo for the micro-hdmi (plug and play) pi camera V3 wide(since i didn't have the v3 camera at the time) 



# BOM (build of materials)
below links is what i bought and are guarenteed to work. you're more than welcome to find others but they might not work.
* for rawdog
[FPC cable 50mm]	(https://au.mouser.com/ProductDetail/710-686715050001)
[raw-dog DP-DP cable 2m](https://cplonline.com.au/alogic-eldp-02-elements-2m-displayport-cable-ver-1-2-male-to-male.html) - PN alogic ELDP-02
M2 screws 12mm x4 & 16mm x4

* plug and play(micro-hdmi)
[FPC cable 50mm]	(https://au.mouser.com/ProductDetail/710-686715050001)
[micro-hdmi to hdmi cable 2m]	(https://www.digikey.com.au/en/products/detail/tripp-lite/P570-006-MICRO/4438902)
[adapter]	(https://www.digikey.com.au/en/products/detail/seeed-technology-co-ltd/320210001/5487747)
M2 screws 12mm-14mm x4 & 20mm x4

### alternates
[adapter alternate-UN TESTED]	(https://au.mouser.com/ProductDetail/Seeed-Studio/320210001?qs=SElPoaY2y5IpesjEUALrcQ%3D%3D)
[cheaper option-UN TESTED]	(https://au.rs-online.com/web/p/raspberry-pi-cables/1871376)
[FPC cable 50mm alternate]	(https://www.digikey.com.au/short/bmt4hz0f)

# FAQ
* can i destroy my pi camera using this?
  no, not unless you mess up the wiring with raw-dog method.
* do i have to use your repo ?
  no, you're more than welcome to use any other pi camera streaming software.
* do i need supports for the printable files ?
  for most of them yes, but use your 3d printing knowledge to determine if some areas are needed or not.
* will the cable that's inside the y extrusion wear out over time?
  no, the cable i spefically looked for would fit nicely in the gaps and not have any clearance issues.
* do i need a second drag chain for this?
  only if your current drag chain won't be able to fit the extra cable.
* will i lose any printable height in XYZ?
  no, not for the raw-dog camera
  for the micro-hdmi version you shouldn't but then you have a different printer to me, so check the cad file 😄

# sounds great! how i get one?
  order the parts from digikey/mouser
  [gerber files](https://github.com/legend069/Gantry-Mounted-pi-camera/tree/main/gerbers/ready%20to%20order%20files)
  steps for PCB ordering coming soon....

```diff
- note:PCB thickness must be 0.8mm!
```


## repo download
```
cd ~
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



all the credit for the code goes to this guy https://github.com/ayufan/camera-streamer
