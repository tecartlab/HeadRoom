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
#include "ofxGuiExtended.h"
#include "ofConstants.h"
#include "Planef.h"
#include "Linef.h"
#include "OrthoCamera.h"

#include "BlobTracker.h"

#include <cmath>

#define N_MAX_BLOBS 20

class BlobFinder {
    
public:
    void setup();
    void allocate();
    
    void captureBegin();
    void captureEnd();
    
    void update();

    void updateSensorBox(int & value);
    
    void drawSensorBox();
    void drawBodyBlobs2d(ofRectangle _rect);

    void drawBodyBlobsBox();
    void drawBodyBlobsHeadTop();
    void drawHeadBlobs();
    void drawEyeCenters();
    
    void drawGazePoint();

    vector <BlobTracker> trackedBlobs;
    
    ofVec2f captureScreenSize;
    
    ofVec3f kinectPos;

    ///////////////////
    // FBO CAPTURING //
    ///////////////////
    orthoCamera captureCam;
    
    //ofImage capturedImage;
    ofFbo captureFBO;

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
    
    bool bThreshWithOpenCV;
    
    int nearThreshold;
    int farThreshold;
    
    //////////////
    //PROPERTIES//
    //////////////

    ofxGui gui;
    
    ofxGuiPanel *panel1;

    ofParameterGroup streamingGuiGroup;
    
    ofParameter<bool> streamingBodyBlob;
    ofParameter<bool> streamingHeadBlob;
    ofParameter<bool> streamingHead;
    ofParameter<bool> streamingEye;
    
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

    ofParameterGroup blobEyeGroup;

    ofParameter<float> eyeLevel;
    ofParameter<float> eyeInset;

    ofParameter<ofVec3f> gazePoint;
    ofSpherePrimitive gazePointer;

    ofParameterGroup blobSmoothGroup;
    ofParameter<int> smoothOffset;
    ofParameter<float> smoothFactor;

    ofVboMesh sensorBox;
   
    ofVec3f normal;
    float p;

};


