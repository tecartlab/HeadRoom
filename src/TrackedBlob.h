//
//  TrackedBlob.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofConstants.h"

#include <cmath>


class TrackedBlob {
    
public:
    TrackedBlob();
    
    ofVec3f     bodyCenter;
    ofVec2f     bodySize;
    
    ofVec3f     headTop;
    ofVec2f     headSize;
    
    ofVec3f     eyePoint;
    
};


