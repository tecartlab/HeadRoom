//
//  Planef.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "Planef.h"

Planef::Planef(ofVec3f _origin, ofVec3f _pointA, ofVec3f _pointB){
    normal = (_pointA-_origin).cross(_pointB-_origin);
    normal.normalize();
    p = normal.dot(_origin);
}

ofVec3f Planef::getIntersection(Linef _line){
    if(!intersects(_line))
        return ofVec3f();
    
    ofVec3f vector = ofVec3f(_line.direction);
    vector.scale((p - normal.dot(_line.origin))/normal.dot(vector.normalize()));
    return (_line.origin + vector);
}

bool Planef::intersects(Linef line){
    return (line.direction.dot(normal) == 0.0f)? false: true;
}
