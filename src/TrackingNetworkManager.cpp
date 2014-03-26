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


void TrackingNetworkManager::setup(int _listeningPort, int _broadcastPort, string _kinectSerial){
    kinectSerial = _kinectSerial;
    
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
void TrackingNetworkManager::update(){
    
	//Check if its about time to send a broadcast message
    if(knownClients.size() > 0 && (ofGetElapsedTimeMillis() - broadCastTimer) > BROADCAST_CLIENT_FREQ){
        sendBroadCastAddress();
    } else if(knownClients.size() == 0 && (ofGetElapsedTimeMillis() - broadCastTimer) > BROADCAST_NOCLIENT_FREQ){
        sendBroadCastAddress();
    }
    
    // OSC receiver queues up new messages, so you need to iterate
	// through waiting messages to get each incoming message
    
    
	// check for waiting messages
	while(serverReceiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		serverReceiver.getNextMessage(&m);
		//Log received message for easier debugging of participants' messages:
		ofLogVerbose("Server recvd msg " + getOscMsgAsString(m) + " from " + m.getRemoteIp());
        
		// check the address of the incoming message
		if(m.getAddress() == "/ks/handshake/request"){
			//Identify host of incoming msg
			string incomingHost = m.getRemoteIp();
			//See if incoming host is a new one:
			// get the first argument (we're only sending one) as a string
			if(m.getNumArgs() > 0){
				if(m.getArgType(0) == OFXOSC_TYPE_STRING){
					//reimplemented message display:
					//If vector has reached max size, delete the first/oldest element
					if(serverMessages.size() == maxServerMessages){
						serverMessages.erase(serverMessages.begin());
					}
					//Add message text at the end of the vector
					serverMessages.push_back(m.getArgAsString(0));
                    
				}
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


void TrackingNetworkManager::sendBroadCastAddress(){
    ofxOscMessage broadcast;
    broadcast.setAddress("/ks/broadcast");
	broadcast.addStringArg(kinectSerial);
	broadcast.addIntArg(0);
	broadcast.addStringArg(serverAddress);
	broadcast.addIntArg(serverRecvPort);
    
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
