//
//  BlobTracker.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofVec3f.h"
#include "ofxGui.h"
#include "ofConstants.h"
#include "Linef.h"

#include <cmath>

#define N_MAX_BLOBS 20
#define EYE_DIFF_TO_HEADTOP 160 //the eyes are 130 mm below the top of the head


class BlobTracker {
    
public:
    void setup();
    void allocate(int width, int height);
    
    void update(ofFbo & captureFBO);

    void updateSensorBox(int & value);

    void drawSeonsorBox();
    
    /////////////////
    //COLOR CONTOUR//
    /////////////////
        
    ofPixels fbopixels;
    
    ofxCvColorImage colorImg;
    
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayEyeLevel; // the eyelevel thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    
    ofxCvContourFinder contourFinder;
    ofxCvContourFinder contourEyeFinder;
    
    int nBlobs; //number of detected blobs
    
    bool bThreshWithOpenCV;
    
    int nearThreshold;
    int farThreshold;
    
    ofVec3f blobPos[N_MAX_BLOBS];
    ofVec2f blobSize[N_MAX_BLOBS];
    
    ofVec2f headtop[N_MAX_BLOBS];
    
    ofVec3f blobTopPos[N_MAX_BLOBS];
    ofVec3f blobEyePos[N_MAX_BLOBS];


    //////////////
    //PROPERTIES//
    //////////////

    ofxPanel gui;
    
    ofParameterGroup sensorBoxGuiGroup;
    
    ofParameter<int> sensorBoxLeft;
    ofParameter<int> sensorBoxRight;
    ofParameter<int> sensorBoxTop;
    ofParameter<int> sensorBoxBottom;
    ofParameter<int> sensorBoxFront;
    ofParameter<int> sensorBoxBack;
    ofParameter<int> nearFrustum;
    ofParameter<int> farFrustum;
    
    ofParameterGroup blobGuiGroup;
    
    ofParameter<int> blobAreaMin;
    ofParameter<int> blobAreaMax;
    ofParameter<int> countBlob;

    ofVboMesh sensorBox;
   
    ofVec3f normal;
    float p;

};


