# kinectServer

by Martin Fröhlich

## Overview

**kinectServer**, the missing kinect network solution for *interaction space developers*.

It is based on openFrameworks and libfreenect, should run on OSX (tested), Windows and Linux.

### Functionality

kinectServer runs as an application on a dedicated computer with one attached kinect device. The machine needs to be powerfull enough to do all the tracking analysis necessary. 

It has built in functionality to do a simple but powerfull calibration of the kinect-camera. It is very simple to tell the camera how it is located towards a floor, ceilling or a wall.

It is built for a accurate tracking of bodies. The tracker can send the bodies position and heigth, head-toptip-position, head-center-position, eye-center-position and gaze direction.

All data is in a metric cartesian coordinatesystem referenced from a floor.

The results are sent as UDP and/or TCP data over the network to a requesting client.

It is also capable to send video and pointcloud data via network.

## Communication

On startup, kinectServer will send every second a OSC-broadcast-announcement to the broadcast address (of the newtork the machine resides in) to port 43525.

Assuming the IP-address of the kinectServer is 192.168.1.100, it will send the handshake to 192.168.1.255 / 43525.

> **/ks/broadcast** \<kinectid> \<serverID> \<ServerIP> \<ServerListeningPort>

for example

> **/ks/broadcast** A00363A14660053A 0 192.168.1.100 43525

every client in the network can respond to this broadcast and send a handshake request back to the kinectServer:

> **/ks/handshake/request** \<ClientIP> \<ClientListeningPort>

upon receiving this request, the server will send in the sequence of  appearance the calibration data and then as a continous stream the tracking data. If the client misses some of the calibration data it has to resend the handshake request.

##calibration data

####transformation

> **/ks/calibration/transformation** \<serverID> \<x-rotate[deg]> \<y-rotate[deg]> \<z-translate[mm]>
> 

with this info the kinect transformation matrix in relation to the floor can be calcualted like this (example code for openframeworks):

    kinectTransform = ofMatrix4x4();
    kinectTransform.rotate(<x-rotate>, 1, 0, 0);
    kinectTransform.rotate(<y-rotate>, 0, 1, 0);
    kinectTransform.translate(0, 0, <z-translate>);

The transformation matrix is mainly used to correctly transform the frustum and pointcloud data.

---
####kinects frustum:

> **/ks/calibration/frustum** \<serverID> \<left[mm]> \<right[mm]> \<top[mm]> \<bottom[mm]> \<near[mm]> \<far[mm]>
> 

the frustum needs to translated by the above transformation matrix to be in the correct space

---
####sensorbox:

> **/ks/calibration/sensorbox** \<serverID> \<left(x-axis)[mm]> \<right(x-axis)[mm]> \<near(y-axis)[mm]> \<far(y-axis)[mm]> \<top(z-axis)[mm]> \<bottom(z-axis)[mm]>
> 

##tracking data

####bodyBlob

> **/ks/tracking/bodyblob** \<serverID> \<blobID> \<sortPos> \<bodyBlobXPos[mm]> \<bodyBlobYPos[mm]> \<bodyBlobWidth(x-axis)[mm]> \<bodyBlobDepth(y-axis)[mm]> \<bodyHeight(z-axis)[mm]>

---
####headBlob

> **/ks/tracking/headblob** \<serverID> \<blobID> \<sortPos> \<headBlobXPos[mm]> \<headBlobYPos[mm]> \<headBlobZPos[mm]> \<headBlobWidth(x-axis)[mm]> \<headBlobDepth(y-axis)[mm]>

---
####head

> **/ks/tracking/head** \<serverID> \<blobID> \<sortPos> \<headTopXPos[mm]> \<headTopYPos[mm]> \<headTopZPos[mm]> \<headCenterZPos[mm]>
> 

headCenterXPos and -YPos are the same as headTopXPos and -YPos

---
####eye

> **/ks/tracking/eye** \<serverID> \<blobID> \<sortPos> \<eyeXPos[mm]> \<eyeYPos[mm]> \<eyeZPos[mm]> \<eyeGazeX> \<eyeGazeY> \<eyeGazeZ>
> 

eyeGazeX, Y, Z is a normalized vector. Beware: The gaze is calculated based on a defined gaze-point and not through facial feature tracking. It assumes that each tracked person looks at this gaze-point.

---

##Download and installation

in order to compile this app on your system, you need to install openframeworks.

###openFrameworks
To grab a copy of openFrameworks for your platform, check the [download page](http://openframeworks.cc/download) on the main site.  
 
The `master` branch of this repository corresponds to the most recent release, with a few differences:  

1. The release includes a simple openFrameworks project generator.
2. This GitHub repository contains code and libs for all the platforms, but the releases are done on a per-platform basis.
3. This GitHub repository has no project files for the different examples. They are generated automatically for each release using a tool in `apps/devApps/projectGenerator/`.

###kinectServer
To grab a copy of kinectServer for your platform, check here [download page](http://github.com/maybites).  
 
The `master` branch of this repository corresponds to the most recent release. This GitHub repository contains code and libs for all the platforms.

drop it into the \<openframeworksfolder>/apps/\<yourappfolder>

##How to
to come...

##Version
kinectServer uses [Semantic Versioning](http://semver.org/), 

Version 0.0.1		
- initial release

##Credits

(c) by beamstreamer.com

created by Martin Froehlich for [beamstreamer.com](http://beamstreamer.com)

loosely based on a concept by Andrew Sempre and his [performance-space](https://bitbucket.org/tezcatlipoca/performance-space)

##Licence
see license.md