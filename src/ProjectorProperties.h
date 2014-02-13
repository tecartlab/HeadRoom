//
//  ProjectorProperties.h
//  of_elSantoGrial
//
//  Created by Martin Froehlich on 30.06.12.
//  Copyright (c) 2012 Maybites. All rights reserved.
//

#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxUI.h"
#include "ofAdvCamera.h"

#ifndef of_elSantoGrial_projectorProps_h
#define of_elSantoGrial_projectorProps_h

#define SPRING_CONSTANT 0.1f
#define MAX_VELOCITY 30.0f

#define GUI_DISTANCE        "Distance [cm]"
#define GUI_FOCALPOINT      "Focal Point [cm]"
#define GUI_POLARANGLE      "Polar   Angle [deg]"
#define GUI_AZIMUTHANGLE    "Azimuth Angle [deg]"
#define GUI_TILTANGLE       "Tilt    Angle [deg]"
#define GUI_ROLLANGLE       "Roll    Angle [deg]"
#define GUI_SCREENWIDTH     "Screen Width [cm]"
#define GUI_SCREENRATIO     "Screen Ratio [1:x]"
#define GUI_FSHIFTX         "Frustum Shift X [unit]"
#define GUI_FSHIFTY         "Frustum Shift Y [unit]"
#define GUI_FRUSTUMNEAR     "Frustum Near [unit]"
#define GUI_FRUSTUMFAR      "Frustum Far [unit]"
#define GUI_SCEENOFFSETX    "Scene Offset X [unit]"
#define GUI_SCEENOFFSETY    "Scene Offset Y [unit]"
#define GUI_SCEENOFFSETZ    "Scene Offset Z [unit]"


// This 'swarm' object demonstrates a simple particle system
//  with 'simple harmonic motion'
class projectorProps {    
public:
    struct Frustum {
		float left;
		float right;
		float bottom;
		float top;
		float near;
		float far;
	};
    
    struct Properties {
        float distance;
        float focalPoint;
        float anglePolar;
        float angleAzimuth;
        float angleTilt;
        float angleRoll;
        float screenWidth;
        float screenRatio;
        float frustumShiftX;
        float frustumShiftY;
        float frustumNear;
        float frustumFar;
        float sceneOffsetX;
        float sceneOffsetY;
        float sceneOffsetZ;
    };

    projectorProps(string path, int x, int y, int width, int height);
    void load(string path);
    void save();
    void saveas(string path);
    void update(ofAdvCamera * cam);

    static void calculations(Properties * const props, ofNode * parent, Frustum * fr);
    
    Properties getProperties();
    void setProperties(Properties props);

    void draw();
    string getMessage();

    void setDistance(float distance);
    float getDistance();
    void setFocalPoint(float focal);
    float getFocalPoint();
    
    void setAnglePolar(float polar);
    float getAnglePolar();
    void setAngleAzimuth(float azimut);
    float getAngleAzimuth();
    void setAngleTilt(float tilt);
    float getAngleTilt();
    void setAngleRoll(float roll);
    float getAngleRoll();
    
    void setScreenWidth(float width);
    float getScreenWidth();
    void setScreenRatio(float ratio);
    float getScreenRatio();
    
    void setFrustumShiftX(float shiftX);
    float getFrustumShiftX();
    void setFrustumShiftY(float shiftY);
    float getFrustumShiftY();
    void setFrustumNear(float near);
    float getFrustumNear();
    void setFrustumFar(float far);
    float getFrustumFar();
    
    void setSceneOffset(ofVec3f offset);
    void setSceneOffsetX(float offsetX);
    void setSceneOffsetY(float offsetY);
    void setSceneOffsetZ(float offsetZ);
    ofVec3f getSceneOffset();
    

    ofxXmlSettings XML;

protected:
    void resetGUI();
    void guiEvent(ofxUIEventArgs &e);
    
    ofxUICanvas *gui;   	

    ofNode camP1, camP2;

    bool needsUpdate;
    Frustum frustum;
    string filepath;
    string message;
    
    int guiX, guiY, guiWidth, guiHeight;

};

#endif