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

Linef Planef::getIntersection(Planef _plane){
    // first check if thePlane is parallel to plane:
    if(intersects(_plane)){
        ofVec3f direction = _plane.normal.cross(normal).normalize();
        ofVec3f point = _plane.getOrigin() + getOrigin();
        return Linef(point, point + direction);
    }
    return Linef();
}

bool Planef::intersects(Planef _plane){
    return (_plane.normal.cross(normal).length() == 0.0f)? false: true;
}

bool Planef::intersects(Linef line){
    return (line.direction.dot(normal) == 0.0f)? false: true;
}

ofVec3f Planef::getOrigin(){
    ofVec3f orig = ofVec3f(normal);
    orig.scale(p);
    return orig;
}

ofVec3f Planef::getNormal(){
    return normal;
}


