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