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


void TrackingNetworkManager::setup(int _listeningPort, int _broadcastPort, string _kinectSerial, int _kinectID){
    kinectSerial = _kinectSerial;
    kinectID = _kinectID;
    gatherLocalIPs();

    //RegularExpression regEx("\\b\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\b");

    // regex to find four numbered IP address that does NOT start with 127.d.d.d
    RegularExpression regEx("\\b^(?:(?!127).)+\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\b");
    RegularExpression::Match match;
    
    serverAddress = "127.0.0.1";
    broadcastAddress = "127.0.0.1";
    
    for(int i = 0; i < localIpAddresses.size(); i++){
        if(regEx.match(localIpAddresses[i], match)) {
            serverAddress = localIpAddresses[i];
            broadcastAddress = serverAddress.substr(0, serverAddress.find_last_of(".") + 1 ) + "255";
        }
    }

    ofLog(OF_LOG_NOTICE, "Choosen Broadcastadress:  " + broadcastAddress);

	//Server side
	//listen for incoming messages on a port; setup OSC receiver with usage:
	serverRecvPort = _listeningPort;
	serverReceiver.setup(serverRecvPort);
    
	maxServerMessages = 38;
 
    broadcastSendPort = _broadcastPort;
    broadcastSender.setup(broadcastAddress, broadcastSendPort);
    
    broadCastTimer = ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void TrackingNetworkManager::update(BlobFinder & _blobFinder){
    long currentMillis = ofGetElapsedTimeMillis();
	//Check if its about time to send a broadcast message
    if(knownClients.size() > 0 && (currentMillis - broadCastTimer) > BROADCAST_CLIENT_FREQ){
        sendBroadCastAddress();
        checkTrackingClients(currentMillis);
    } else if(knownClients.size() == 0 && (currentMillis - broadCastTimer) > BROADCAST_NOCLIENT_FREQ){
        sendBroadCastAddress();
    }
    
    
    for(int i = 0; i < _blobFinder.trackedBlobs.size(); i++){
        if(_blobFinder.streamingBodyBlob.get()){
            ofxOscMessage bodyBlob;
            bodyBlob.setAddress("/ks/tracking/bodyblob");
            bodyBlob.addIntArg(kinectID);
            bodyBlob.addIntArg(i);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.x);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.y);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].bodyBlobSize.x);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].bodyBlobSize.y);
            bodyBlob.addIntArg(_blobFinder.trackedBlobs[i].bodyBlobCenter.z);
            
            sendMessageToTrackingClients(bodyBlob);
        }
        if(_blobFinder.streamingHeadBlob.get()){
            ofxOscMessage headBlob;
            headBlob.setAddress("/ks/tracking/headblob");
            headBlob.addIntArg(kinectID);
            headBlob.addIntArg(i);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].headBlobCenter.x);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].headBlobCenter.z);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].headBlobCenter.z);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].headBlobSize.x);
            headBlob.addIntArg(_blobFinder.trackedBlobs[i].headBlobSize.y);
            
            sendMessageToTrackingClients(headBlob);
        }
        if(_blobFinder.streamingHead.get()){
            ofxOscMessage head;
            head.setAddress("/ks/tracking/head");
            head.addIntArg(kinectID);
            head.addIntArg(i);
            head.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            head.addIntArg(_blobFinder.trackedBlobs[i].headTop.x);
            head.addIntArg(_blobFinder.trackedBlobs[i].headTop.y);
            head.addIntArg(_blobFinder.trackedBlobs[i].headTop.z);
            head.addIntArg(_blobFinder.trackedBlobs[i].headCenter.z);
            
            sendMessageToTrackingClients(head);
        }
        if(_blobFinder.streamingHead.get()){
            ofxOscMessage eye;
            eye.setAddress("/ks/tracking/eye");
            eye.addIntArg(kinectID);
            eye.addIntArg(i);
            eye.addIntArg(_blobFinder.trackedBlobs[i].sortPos);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeCenter.x);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeCenter.y);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeCenter.z);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeGaze.x);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeGaze.y);
            eye.addIntArg(_blobFinder.trackedBlobs[i].eyeGaze.z);
            
            sendMessageToTrackingClients(eye);
        }
    }
    // OSC receiver queues up new messages, so you need to iterate
	// through waiting messages to get each incoming message
    
    
	// check for waiting messages
	while(serverReceiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		serverReceiver.getNextMessage(&m);
		//Log received message for easier debugging of participants' messages:
        //ofLog(OF_LOG_NOTICE, "Server recvd msg " + getOscMsgAsString(m) + " from " + m.getRemoteIp());
        
		// check the address of the incoming message
		if(m.getAddress() == "/ks/handshake"){
			//Identify host of incoming msg
			string incomingHost = m.getRemoteIp();
			//See if incoming host is a new one:
			// get the first argument (we're only sending one) as a string
			if(m.getNumArgs() == 2 && m.getArgType(0) == OFXOSC_TYPE_STRING && m.getArgType(1) == OFXOSC_TYPE_INT32){
                knownClients[getTrackingClientIndex(m.getArgAsString(0), m.getArgAsInt32(1))].update(currentMillis);
			}else{
                ofLog(OF_LOG_WARNING, "Server recvd malformed message. Expected: /ks/handshake/request <ClientIP> <ClientListeningPort> | received: " + getOscMsgAsString(m) + " from " + m.getRemoteIp());
            }
		}
		// handle getting random OSC messages here
		else{
			ofLogWarning("Server got weird message: " + m.getAddress());
		}
	}
    
	//this is purely workaround for a mysterious OSCpack bug on 64bit linux
	// after startup, reinit the receivers
	// must be a timing problem, though - in debug, stepping through, it works.
	if(ofGetFrameNum() == 60){
		serverReceiver.setup(serverRecvPort);
	}
}

void TrackingNetworkManager::sendMessageToTrackingClients(ofxOscMessage _msg){
    for(int j = 0; j < knownClients.size(); j++){
        broadcastSender.setup(knownClients[j].clientDestination, knownClients[j].clientSendPort);
        broadcastSender.sendMessage(_msg);
    }
    //knownClients[j].sendMessage(_msg);
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
    TrackingClient newClient;
    newClient.setup(_ip, _port);
    knownClients.push_back(newClient);
    ofLog(OF_LOG_NOTICE, "Server added new TrackingClient ip: " + _ip + " port:  " + ofToString(_port) + " knownClients:  " + ofToString(knownClients.size()));
    return knownClients.size() -1;
}

void TrackingNetworkManager::sendBroadCastAddress(){
    ofxOscMessage broadcast;
    broadcast.setAddress("/ks/broadcast");
	broadcast.addStringArg(kinectSerial);
	broadcast.addIntArg(kinectID);
	broadcast.addStringArg(serverAddress);
	broadcast.addIntArg(serverRecvPort);
    
    broadcastSender.setup(broadcastAddress, broadcastSendPort);
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


