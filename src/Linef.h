//
//  Planef.h
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//
#pragma once

#include "ofMain.h"
#include "ofVec3f.h"
#include "ofConstants.h"

#include <cmath>


class Linef {
    
public:
    Linef(ofVec3f origin, ofVec3f pointA);
    
    //ofVec3f getIntersection(Linef line);

    ofVec3f origin;
    ofVec3f direction;

};

