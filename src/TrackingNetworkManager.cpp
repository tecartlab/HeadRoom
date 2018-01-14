//
//  TrackingNetworkManager.cpp
//  kinectServer
//
//  Created by maybites on 14.02.14.
//
//

#include "TrackingNetworkManager.h"

// we need to include the RegularExpression
// header file and say that we are using that
// name space
#include "Poco/RegularExpression.h"
using Poco::RegularExpression;


TrackingNetworkManager::TrackingNetworkManager(){
}


void TrackingNetworkManager::setup(ofxGui &gui, string _kinectSerial){
    kinectSerial = _kinectSerial;
    gatherLocalIPs();

    //RegularExpression regEx("\\b\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\b");
    
    // regex to find four numbered IP address that does NOT start with 127.d.d.d
    RegularExpression regEx("\\b^(?:(?!127).)+\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\b");
    RegularExpression::Match match;

    string localAddress = "127.0.0.1";
    
    for(int i = 0; i < localIpAddresses.size(); i++){
        if(regEx.match(localIpAddresses[i], match)) {
            localAddress = localIpAddresses[i];
            //            broadcastAddress = serverAddress.substr(0, serverAddress.find_last_of(".") + 1 ) + "255";
        }
    }

    panel = gui.addPanel();
    
    
    panel->loadTheme("theme/theme_light.json");
    panel->setName("Broadcasting..");

    streamingBodyBlob.addListener(this, &TrackingNetworkManager::listenerBool);
    streamingHeadBlob.addListener(this, &TrackingNetworkManager::listenerBool);
    streamingHead.addListener(this, &TrackingNetworkManager::listenerBool);
    streamingEye.addListener(this, &TrackingNetworkManager::listenerBool);

    broadcastIP.addListener(this, &TrackingNetworkManager::listenerString);
    broadcastPort.addListener(this, &TrackingNetworkManager::listenerInt);
    listeningIP.addListener(this, &TrackingNetworkManager::listenerString);
    listeningPort.addListener(this, &TrackingNetworkManager::listenerInt);
    
    broadcastGroup = panel->addGroup("Broadcast");
    //panel->add(broadcastLabel.set("Broadcast"));
    broadcastGroup->add<ofxGuiTextField>(broadcastIP.set("IP","127.0.0.1"));
    broadcastGroup->add<ofxGuiIntInputField>(broadcastPort.set("Port", NETWORK_BROADCAST_PORT, NETWORK_BROADCAST_PORT, NETWORK_BROADCAST_PORT + 99));
    
    listeningGroup = panel->addGroup("Listening");
    listeningGroup->add<ofxGuiTextField>(listeningIP.set("IP",localAddress));
    listeningGroup->add<ofxGuiIntInputField>(listeningPort.set("Port", NETWORK_LISTENING_PORT, NETWORK_LISTENING_PORT, NETWORK_LISTENING_PORT + 99));

    
    streamingGuiGroup.setName("Streaming");
    streamingGuiGroup.add(streamingBodyBlob.set("bodyBlob", true));
    streamingGuiGroup.add(streamingHeadBlob.set("headBlob", true));
    streamingGuiGroup.add(streamingHead.set("head", true));
    streamingGuiGroup.add(streamingEye.set("eye", true));
    panel->addGroup(streamingGuiGroup);

    panel->loadFromFile("broadcast.xml");
    

    //Server side
    //listen for incoming messages on a port; setup OSC receiver with usage:
    serverReceiver.setup(listeningPort.get());
    broadcastSender.setup(broadcastIP.get(), broadcastPort.get());
    ofLog(OF_LOG_NOTICE, "Choosen BroadcastAddress:  " + broadcastIP.get());
    
	maxServerMessages = 38;
    
    broadCastTimer = ofGetElapsedTimeMillis();
    
    scale = 0.001; // transform mm to m
    frameNumber = 0;
}

void TrackingNetworkManager::listenerString(string & _string){
    ofLog(OF_LOG_NOTICE, "listenerString " + _string + " from");
}

void TrackingNetworkManager::listenerInt(int & _int){
    ofLog(OF_LOG_NOTICE, "listenerInt " + ofToString(_int) + " ");
}

void TrackingNetworkManager::listenerBool(bool & _bool){
    ofLog(OF_LOG_NOTICE, "listenerBool " + ofToString(_bool) +
          " streamingBodyBlob:" + ofToString(streamingBodyBlob.get()) +
          " streamingHeadBlob:" + ofToString(streamingHeadBlob.get()) +
          " streamingHead:" + ofToString(streamingHead.get()) +
          " streamingEye:" + ofToString(streamingEye.get()));
}


//--------------------------------------------------------------
void TrackingNetworkManager::update(BlobFinder & _blobFinder, Frustum & _frustum, ofVec3f _trans, bool _calibUpdate){
    frameNumber++;
    
    long currentMillis = ofGetElapsedTimeMillis();
	//Check if its about time to send a broadcast message
    if(knownClients.size() > 0 && (currentMillis - broadCastTimer) > BROADCAST_CLIENT_FREQ){
        sendBroadCastAddress();
        checkTrackingClients(currentMillis);
    } else if(knownClients.size() == 0 && (currentMillis - broadCastTimer) > BROADCAST_NOCLIENT_FREQ){
        sendBroadCastAddress();
    }
    
    // OSC receiver queues up new messages, so you need to iterate
	// through waiting messages to get each incoming message
    
	// check for waiting messages
	while(serverReceiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		serverReceiver.getNextMessage(m);
		//Log received message for easier debugging of participants' messages:
        ofLog(OF_LOG_NOTICE, "Server recvd msg " + getOscMsgAsString(m) + " from " + m.getRemoteHost());
        
		// check the address of the incoming message
		if(m.getAddress() == "/ks/request/handshake"){
			//Identify host of incoming msg
			string incomingHost = m.getRemoteIp();
			//See if incoming host is a new one:
			// get the first argument (listeningport) as an int
			if(m.getNumArgs() == 1 && m.getArgType(0) == OFXOSC_TYPE_INT32){
                knownClients[getTrackingClientIndex(incomingHost, m.getArgAsInt32(0))].update(currentMillis);
                _calibUpdate = true;
            }else{
                ofLog(OF_LOG_WARNING, "Server recvd malformed message. Expected: /ks/request/handshake <ClientListeningPort> | received: " + getOscMsgAsString(m) + " from " + incomingHost);
            }
		} else if(m.getAddress() == "/ks/request/update"){
			//Identify host of incoming msg
			string incomingHost = m.getRemoteIp();
			//See if incoming host is a new one:
			// get the first argument (listeningport) as an int
			if(m.getNumArgs() == 1 && m.getArgType(0) == OFXOSC_TYPE_INT32){
                knownClients[getTrackingClientIndex(incomingHost, m.getArgAsInt32(0))].update(currentMillis);
			}else{
                ofLog(OF_LOG_WARNING, "Server recvd malformed message. Expected: /ks/request/update <ClientListeningPort> | received: " + getOscMsgAsString(m) + " from " + incomingHost);
            }
		}
		// handle getting random OSC messages here
		else{
			ofLogWarning("Server got weird message: " + m.getAddress());
		}
	}
    
    if(_calibUpdate){
        // Send calib-data
        sendCalibFrustum(_frustum);
        sendCalibSensorBox(_blobFinder);
        sendCalibTrans(_trans);
        sendCalibGazePoint(_blobFinder);
    }

    //send trackingdata to all connected clients
    sendTrackingData(_blobFinder);
    
	//this is purely workaround for a mysterious OSCpack bug on 64bit linux
	// after startup, reinit the receivers
	// must be a timing problem, though - in debug, stepping through, it works.
	if(ofGetFrameNum() == 60){
		serverReceiver.setup(listeningPort.get());
	}
}

void TrackingNetworkManager::sendTrackingData(BlobFinder & _blobFinder){
    // send frame number
    ofxOscMessage frame;
    frame.setAddress("/ks/server/track/frame");
    frame.addIntArg(kinectID);
    frame.addIntArg(frameNumber);
    frame.addIntArg(streamingBodyBlob.get());
    frame.addIntArg(streamingHeadBlob.get());
    frame.addIntArg(streamingHead.get());
    frame.addIntArg(streamingEye.get());
    sendMessageToTrackingClients(frame);
 
    for(int i = 0; i < _blobFinder.trackedBlobs.size(); i++){
        if(streamingBodyBlob.get()){
            ofxOscMessage bodyBlob;
            bodyBlob.setAddress("/ks/server/track/bodyblob");
            bodyBlob.addIntArg(kinectID);
            bodyBlob.addIntArg(frameNumber);
            bodyBlob.addIntArg(i);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            bodyBlob.addFloatArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.x * scale);
            bodyBlob.addFloatArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.y * scale);
            bodyBlob.addFloatArg(_blobFinder.trackedBlobs[i].bodyBlobSize.x * scale);
            bodyBlob.addFloatArg(_blobFinder.trackedBlobs[i].bodyBlobSize.y * scale);
            bodyBlob.addFloatArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.z * scale);
            
            sendMessageToTrackingClients(bodyBlob);
        }
        if(streamingHeadBlob.get()){
            ofxOscMessage headBlob;
            headBlob.setAddress("/ks/server/track/headblob");
            headBlob.addIntArg(kinectID);
            headBlob.addIntArg(frameNumber);
            headBlob.addIntArg(i);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            headBlob.addFloatArg(_blobFinder.trackedBlobs[i].headBlobCenter.x * scale);
            headBlob.addFloatArg(_blobFinder.trackedBlobs[i].headBlobCenter.y * scale);
            headBlob.addFloatArg(_blobFinder.trackedBlobs[i].headBlobCenter.z * scale);
            headBlob.addFloatArg(_blobFinder.trackedBlobs[i].headBlobSize.x * scale);
            headBlob.addFloatArg(_blobFinder.trackedBlobs[i].headBlobSize.y * scale);
            
            sendMessageToTrackingClients(headBlob);
        }
        if(streamingHead.get()){
            ofxOscMessage head;
            head.setAddress("/ks/server/track/head");
            head.addIntArg(kinectID);
            head.addIntArg(frameNumber);
            head.addIntArg(i);
            head.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headTop.x * scale);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headTop.y * scale);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headTop.z * scale);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headCenter.x * scale);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headCenter.y * scale);
            head.addFloatArg(_blobFinder.trackedBlobs[i].headCenter.z * scale);
            
            sendMessageToTrackingClients(head);
        }
        if(streamingEye.get()){
            ofxOscMessage eye;
            eye.setAddress("/ks/server/track/eye");
            eye.addIntArg(kinectID);
            eye.addIntArg(frameNumber);
            eye.addIntArg(i);
            eye.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeCenter.x * scale);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeCenter.y * scale);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeCenter.z * scale);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeGaze.x);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeGaze.y);
            eye.addFloatArg(_blobFinder.trackedBlobs[i].eyeGaze.z);
            
            sendMessageToTrackingClients(eye);
        }
        
        // send frame number
        ofxOscMessage framedone;
        framedone.setAddress("/ks/server/track/framedone");
        framedone.addIntArg(kinectID);
        framedone.addIntArg(frameNumber);
        sendMessageToTrackingClients(framedone);
    }
}

void TrackingNetworkManager::sendCalibFrustum(Frustum & _frustum){
    ofxOscMessage frustum;
    frustum.setAddress("/ks/server/calib/frustum");
    frustum.addIntArg(kinectID);
    frustum.addFloatArg(_frustum.left * scale);
    frustum.addFloatArg(_frustum.right * scale);
    frustum.addFloatArg(_frustum.bottom * scale);
    frustum.addFloatArg(_frustum.top * scale);
    frustum.addFloatArg(_frustum.near * scale);
    frustum.addFloatArg(_frustum.far * scale);
    
    sendMessageToTrackingClients(frustum);
}

void TrackingNetworkManager::sendCalibTrans(ofVec3f & _trans){
    ofxOscMessage trans;
    trans.setAddress("/ks/server/calib/trans");
    trans.addIntArg(kinectID);
    trans.addFloatArg(_trans.x);
    trans.addFloatArg(_trans.y);
    trans.addFloatArg(_trans.z * scale);
    
    sendMessageToTrackingClients(trans);
}

void TrackingNetworkManager::sendCalibSensorBox(BlobFinder & _blobFinder){
    ofxOscMessage sensorbox;
    sensorbox.setAddress("/ks/server/calib/sensorbox");
    sensorbox.addIntArg(kinectID);
    sensorbox.addFloatArg(_blobFinder.sensorBoxLeft.get() * scale);
    sensorbox.addFloatArg(_blobFinder.sensorBoxRight.get() * scale);
    sensorbox.addFloatArg(_blobFinder.sensorBoxBottom.get() * scale);
    sensorbox.addFloatArg(_blobFinder.sensorBoxTop.get() * scale);
    sensorbox.addFloatArg(_blobFinder.sensorBoxFront.get() * scale);
    sensorbox.addFloatArg(_blobFinder.sensorBoxBack.get() * scale);
    
    sendMessageToTrackingClients(sensorbox);
}

void TrackingNetworkManager::sendCalibGazePoint(BlobFinder & _blobFinder){
    ofxOscMessage sensorbox;
    sensorbox.setAddress("/ks/server/calib/gazepoint");
    sensorbox.addIntArg(kinectID);
    sensorbox.addFloatArg(_blobFinder.gazePoint.get().x * scale);
    sensorbox.addFloatArg(_blobFinder.gazePoint.get().y * scale);
    sensorbox.addFloatArg(_blobFinder.gazePoint.get().z * scale);
    
    sendMessageToTrackingClients(sensorbox);
}

void TrackingNetworkManager::sendMessageToTrackingClients(ofxOscMessage _msg){
    for(int j = 0; j < knownClients.size(); j++){
        broadcastSender.setup(knownClients[j].clientDestination, knownClients[j].clientSendPort);
        broadcastSender.sendMessage(_msg);
    }
}

void TrackingNetworkManager::checkTrackingClients(long _currentMillis){
    for(int i = 0; i < knownClients.size(); i++){
        if(!knownClients[i].stillAlive(_currentMillis)){
            ofLog(OF_LOG_NOTICE, "Server removed TrackingClient ip: " + knownClients[i].clientDestination + " port:  " + ofToString(knownClients[i].clientSendPort));
            knownClients[i] = knownClients.back();
            knownClients.pop_back();
            i--;
        }
    }
}

int TrackingNetworkManager::getTrackingClientIndex(string _ip, int _port){
    for(int i = 0; i < knownClients.size(); i++){
        if(knownClients[i].clientDestination.find(_ip) != string::npos && knownClients[i].clientSendPort == _port){
            return i;
        }
    }
    knownClients.push_back(TrackingClient(_ip, _port));
    ofLog(OF_LOG_NOTICE, "Server added new TrackingClient ip: " + _ip + " port:  " + ofToString(_port) + " knownClients:  " + ofToString(knownClients.size()));
    return knownClients.size() -1;
}

void TrackingNetworkManager::sendBroadCastAddress(){
    ofxOscMessage broadcast;
    broadcast.setAddress("/ks/server/broadcast");
	broadcast.addStringArg(kinectSerial);
	broadcast.addIntArg(kinectID);
	broadcast.addStringArg(listeningIP.get());
	broadcast.addIntArg(listeningPort.get());
    
    broadcastSender.setup(broadcastIP.get(), broadcastPort.get());
    broadcastSender.sendMessage(broadcast);
    
    broadCastTimer = ofGetElapsedTimeMillis();
    //ofLog(OF_LOG_NOTICE, "Sent Broadcastmessage");
}

//--------------------------------------------------------------
string TrackingNetworkManager::getOscMsgAsString(ofxOscMessage m){
	string msg_string;
	msg_string = m.getAddress();
	msg_string += ":";
	for(int i = 0; i < m.getNumArgs(); i++){
		// get the argument type
		msg_string += " " + m.getArgTypeName(i);
		msg_string += ":";
		// display the argument - make sure we get the right type
		if(m.getArgType(i) == OFXOSC_TYPE_INT32){
			msg_string += ofToString(m.getArgAsInt32(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
			msg_string += ofToString(m.getArgAsFloat(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
			msg_string += m.getArgAsString(i);
		}
		else{
			msg_string += "unknown";
		}
	}
	return msg_string;
}

void TrackingNetworkManager::gatherLocalIPs(){
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];
    
    if(getifaddrs(&myaddrs) != 0)
    {
        perror("getifaddrs");
        exit(1);
    }
    
    localIpAddresses.clear();
    
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        
        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }
                
            case AF_INET6:
            {
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }
                
            default:
                continue;
        }
        
        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
        {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        }
        else
        {
            localIpAddresses.push_back(string(buf));
            //printf("%s: %s\n", ifa->ifa_name, buf);
        }
    }
    
    freeifaddrs(myaddrs);
    return 0;
}


