//
//  TrackingNetworkManager.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofVec3f.h"
#include "ofConstants.h"
#include "ofxOsc.h"
#include "TrackingClient.h"
#include "BlobFinder.h"
#include "Frustum.h"

#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>

#define BROADCAST_NOCLIENT_FREQ     1000
#define BROADCAST_CLIENT_FREQ       10000

#define NETWORK_BROADCAST_PORT 47500
#define NETWORK_LISTENING_PORT 47600

class TrackingNetworkManager {
    
public:
    TrackingNetworkManager();
    
    void setup(ofxGui &gui, string _kinectSerial);
    void update(BlobFinder & _blobFinder, Frustum & _frustum, ofVec3f _trans);

    void sendTrackingData(BlobFinder & _blobFinder);

    void sendCalibFrustum(Frustum & _frustum, string ip, int port);
    void sendCalibTrans(ofVec3f & _trans, string _ip, int _port);
    void sendCalibSensorBox(BlobFinder & _blobFinder, string _ip, int _port);
    void sendGazePoint(BlobFinder & _blobFinder, string _ip, int _port);

    void sendMessageToTrackingClients(ofxOscMessage _msg);
    void checkTrackingClients(long _currentMillis);
    int getTrackingClientIndex(string _ip, int _port);

    void listenerString(string & _string);
    void listenerInt(int & _int);
    void listenerBool(bool & _bool);

    void sendBroadCastAddress();
    
    void gatherLocalIPs();
    string getOscMsgAsString(ofxOscMessage m);
    
    vector<string>  localIpAddresses;
    
    string          kinectSerial;
    int             kinectID;
    
    float           scale;
    
    long             frameNumber;
    
    //----------------------------------------
    // Server side:
    ofxOscReceiver  serverReceiver;         // OSC receiver
    
    // Message display variables
    vector<string>  serverMessages;         //vector containing the received messages for display
    unsigned int    maxServerMessages;      //nr of messages fitting on the screen
    
    vector<TrackingClient> knownClients;    //collected IP's of chat participants

    ofxOscSender    broadcastSender;        // broadcastSender object

    long            broadCastTimer;
    
    // GUI
    ofxGuiPanel *panel;
    
    ofxGuiGroup *colorToggles;
    
    ofxGuiGroup *broadcastGroup;
    
    ofParameter<string> broadcastIP;
    ofParameter<int> broadcastPort;

    ofxGuiGroup *listeningGroup;
    
    ofParameter<string> listeningIP;
    ofParameter<int> listeningPort;

    ofParameter<int> kinectServerID;

    ofParameterGroup streamingGuiGroup;
    
    ofParameter<bool> streamingBodyBlob;
    ofParameter<bool> streamingHeadBlob;
    ofParameter<bool> streamingHead;
    ofParameter<bool> streamingEye;


};


