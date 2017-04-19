# HeadRoom

by Martin Fröhlich

## Overview

**HeadRoom**, the missing kinect network solution for *interaction space developers*.

It is based on openFrameworks and libfreenect, should run on OSX (tested), Windows and Linux.

NOTE: Xbox Kinect models > 1414 (1473, etc) or Kinect4Windows devices that have been plugged into an XBox will not work with ofxKinect on Mac OSX

If you have an Xbox Kinect model 1473+ or Kinect4Windows device and want to use it with ofxKinect on OSX, DO NOT plug it into an XBox! If you do, it will receive a firmware update which will cause it to freeze after about 20 secs with libfreenect/ofxKinect on OSX. Linux and Windows are fine. The fix is in the works.

In the meantime, we suggest you get the original Xbox Kinect model 1414 and keep those new toys away from an XBox ...

### Functionality

HeadRoom runs as an application on a dedicated computer with one attached kinect device. The machine needs to be powerfull enough to do all the tracking analysis necessary. 

It is built for a accurate tracking of bodies. The tracker can send the bodies position and heigth, head-toptip-position, head-center-position, eye-center-position and gaze direction.

It has a simple but powerfull calibration functionality. This makes it very easy to tell how the kinect is located towards a floor, ceilling or a wall.

All data is in a metric cartesian coordinatesystem referenced from a floor.

The results are sent as UDP and/or TCP data over the network to a requesting client.

## Communication

On startup, HeadRoom will send every second a OSC-broadcast-announcement to the broadcast address (of the newtork the machine resides in) to port 43500. The server itself will listen to a different port (default = 43600), indicated inside the broadcast message.

Assuming the IP-address of the kinectServer is 192.168.1.100, it will send the handshake to 192.168.1.255 / 43500.

> **/ks/server/broadcast** \<kinectid> \<serverID> \<ServerIP> \<ServerListeningPort>

for example

> **/ks/server/broadcast** A00363A14660053A 0 192.168.1.100 43600

every client in the network can respond to this broadcast and send a handshake request back to the HeadRoom:

> **/ks/request/handshake** \<ClientListeningPort>

upon receiving this request, the server will send the calibration data. If the client misses some of the calibration data it has to resend the handshake request. 

In order to get the trackingdata, the server then needs an update message every 10 seconds.

> **/ks/request/update** \<ClientListeningPort>
> 
upon receiving this request, the server will send a continous stream of the tracking data for the next 11 seconds. Since the server will keep on sending its broadcast message every 10 seconds, the clients resend of the update-message can be triggered by the broadcast-message and thus the connection will never drop.

HeadRoom will stop sending the stream of tracking data if no update-message is received anymore and drop the registration of the client. 

If no client is registered anymore, the server will again start sending the broadcast message every second.

If the server encounters a problem or stops, it will atempt to broadcast an exit message:

> **/ks/server/broadcast/exit** \<kinectid> \<serverID>


##calibration data

####transformation

> **/ks/server/calib/trans** \<serverID> \<x-rotate[deg]> \<y-rotate[deg]> \<z-translate[m]>
> 

with this info the kinect transformation matrix in relation to the floor can be calcualted like this (example code for openframeworks):

    kinectTransform = ofMatrix4x4();
    kinectTransform.rotate(<x-rotate>, 1, 0, 0);
    kinectTransform.rotate(<y-rotate>, 0, 1, 0);
    kinectTransform.translate(0, 0, <z-translate>);

The transformation matrix is mainly used to correctly transform the frustum and pointcloud data.

---
####kinects frustum:

> **/ks/server/calib/frustum** \<serverID> \<left[m]> \<right[m]> \<bottom[m]> \<top[m]> \<near[m]> \<far[m]>
> 

the frustum needs to translated by the above transformation matrix to be in the correct space

---
####sensorbox:

> **/ks/server/calib/sensorbox** \<serverID> \<left(x-axis)[m]> \<right(x-axis)[m]> \<bottom(z-axis)[m]> \<top(z-axis)[m]> \<near(y-axis)[m]> \<far(y-axis)[m]>  
> 

##tracking data


####bodyBlob

> **/ks/server/track/bodyblob** \<serverID> \<frameNo> \<blobID> \<sortPos> \<bodyBlobXPos[m]> \<bodyBlobYPos[m]> \<bodyBlobWidth(x-axis)[m]> \<bodyBlobDepth(y-axis)[m]> \<bodyHeight(z-axis)[m]>

---
####headBlob

> **/ks/server/track/headblob** \<serverID> \<frameNo> \<blobID> \<sortPos> \<headBlobXPos[m]> \<headBlobYPos[m]> \<headBlobZPos[m]> \<headBlobWidth(x-axis)[m]> \<headBlobDepth(y-axis)[m]>

---
####head

> **/ks/server/track/head** \<serverID> \<frameNo> \<blobID> \<sortPos> \<headTopXPos[m]> \<headTopYPos[m]> \<headTopZPos[m]> \<headCenterXPos[m] \<headCenterYPos[m] \<headCenterZPos[m]>
> 

---
####eye

> **/ks/server/track/eye** \<serverID> \<frameNo> \<blobID> \<sortPos> \<eyeXPos[m]> \<eyeYPos[m]> \<eyeZPos[m]> \<eyeGazeX> \<eyeGazeY> \<eyeGazeZ>
> 

eyeGazeX, Y, Z is a normalized vector. Beware: The gaze is calculated based on a defined gaze-point and not through facial feature tracking. It assumes that each tracked person looks at this gaze-point.

---
####frame

> **/ks/server/track/frame** \<serverID> \<frameNo>

the frame message is sent each time after the other messages

---

##Download and installation

in order to compile this app on your system, you need to install openframeworks.

###openFrameworks
To grab a copy of openFrameworks for your platform, check the [download page](http://openframeworks.cc/download) on the main site.  
 
###HeadRoom
To grab a copy of HeadRoom for your platform, check here [download page](http://github.com/tecartlab).  
 
The `master` branch of this repository corresponds to the most recent release. This GitHub repository contains code and libs for all the platforms.

drop it into the \<openframeworksfolder>/apps/\<yourappfolder>

you also need a copy of the openframeworks addon [ofxGuiExtended](https://github.com/frauzufall/ofxGuiExtended.git)

drop it onto the \<openframeworksfolder>/addons/\<ofxGuiExtended>

##How to
to come...

##Version
HeadRoom uses [Semantic Versioning](http://semver.org/), 

Version 0.0.2
- switch to ofxGuiExtended
- redesign of GUI structure
- removal of MatrixServer

Version 0.0.1		
- initial release

##Credits

(c) by tecartlab.com

created by Martin Froehlich for [tecartlab.com](http://tecartlab.com)

loosely based on a concept by Andrew Sempre and his [performance-space](https://bitbucket.org/tezcatlipoca/performance-space)

##Licence
MIT and see license.md