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


class TrackingClient {
    
public:
    TrackingClient(string _destination, int _port);
 
    ofxOscSender    clientSender; // all-important ofxOscSender object
    string          clientDestination; // IP address we're sending to
    int             clientSendPort; // port we're sending to
    
};


