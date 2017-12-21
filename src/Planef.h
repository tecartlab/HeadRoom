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
#include "Linef.h"

#include <cmath>


class Planef {
    
public:
    Planef(ofVec3f origin, ofVec3f pointA, ofVec3f pointB);
    
    ofVec3f getIntersection(Linef line);
    Linef getIntersection(Planef _plane);
    bool intersects(Linef line);
    bool intersects(Planef _plane);
    ofVec3f getOrigin();
    ofVec3f getNormal();
    
    ofVec3f normal;
    float p;

};


