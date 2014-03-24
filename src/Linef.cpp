//
//  Planef.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "Linef.h"

Linef::Linef(ofVec3f _origin, ofVec3f _pointA){
    origin = ofVec3f(_origin);
    direction = ofVec3f(_pointA-_origin);
}

Linef::Linef(){
    origin = ofVec3f();
    direction = ofVec3f(0, 0, -1);
}

void Linef::originize(){
    ofVec3f unity = origin.getCrossed(direction);
    ofVec3f center = unity.getCrossed(direction);
    center.normalize();
    center.scale(center.angle(origin) * origin.length());
    origin = center;
}

float Linef::getDistance(ofVec3f point){
    ofVec3f temp = point - origin;
    temp.cross(direction);
    return temp.length() / direction.length();
}

