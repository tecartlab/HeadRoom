//
//  Frustum.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofVec3f.h"
#include "ofConstants.h"
#include "Linef.h"

#include <cmath>


class Frustum {
    
public:
    Frustum();
    
    void update();
    
    void draw();
    void drawWireframe();
    
    ofVboMesh frustum;

    float left;
    float right;
    float top;
    float bottom;
    float leftFar;
    float rightFar;
    float topFar;
    float bottomFar;
    float near;
    float far;

};


