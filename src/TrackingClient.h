//
//  TrackingClient.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofConstants.h"
#include "ofxOsc.h"

#include <cmath>

#define STAY_ALIVE       12000


class TrackingClient {
    
public:
    TrackingClient();
    void setup(string _destination, int _port);
   
    void sendMessage(ofxOscMessage _msg);
    
    void update(long currentMillis);
    bool stillAlive(long currentMillis);
 
    string          clientDestination; // IP address we're sending to
    int             clientSendPort; // port we're sending to
    
    long            aliveUntil;
    
};


