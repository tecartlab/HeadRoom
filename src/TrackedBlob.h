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
    
    ofVec3f     bodyBlobCenter;
    ofVec2f     bodyBlobSize;
    
    ofVec3f     headTop;
    ofVec3f     headCenter;

    ofVec3f     headBlobCenter;
    ofVec2f     headBlobSize;
    
    ofVec3f     eyeCenter;
    
};


