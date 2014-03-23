//
//  BlobTracker.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofConstants.h"
#include "TrackedBlob.h"

#include <cmath>
#include <vector>
#include <iterator>

//defines after how many frames without update a Blob dies.
#define N_EMPTYFRAMES 10


class BlobTracker {
    
public:
    BlobTracker(ofRectangle _rect);
    
    void nextFrame();
    bool hasDied();
    
    bool finder(ofRectangle _rect);
    
    void update(ofRectangle _rect, ofVec3f _bodyCenter, ofVec2f _bodySize, ofVec3f _headTop);
    void updateHead(ofVec2f _headSize, ofVec3f _eyePoint);
    
    void drawBodyBox();
    void drawHeadTop();
    void drawHeadSize();
    void drawEyePosition();
    
    ofBoxPrimitive bodyBox;
    ofSpherePrimitive bodyHeadTop;
    
    ofRectangle baseRectangle2d;
    
    ofVec3f     bodyCenter;
    ofVec2f     bodySize;
    
    ofVec3f     headTop;
    ofVec2f     headSize;
    
    ofVec3f     eyePoint;
 
    int trackerSize;
    vector <TrackedBlob> tracker;

    int waitForUpdates;
    
};


