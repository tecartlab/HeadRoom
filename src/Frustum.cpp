//
//  Frustum.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "Frustum.h"

Frustum::Frustum(){
}

void Frustum::draw(){
    frustum.draw();
}

void Frustum::drawWireframe(){
    frustum.drawWireframe();
}

void Frustum::update(){
    frustum.clear();
    frustum.setMode(OF_PRIMITIVE_LINES);
    frustum.addVertex(ofPoint(left, -top, -near));
    frustum.addVertex(ofPoint(leftFar, -topFar, -far));
    
    frustum.addVertex(ofPoint(right, -top, -near));
    frustum.addVertex(ofPoint(rightFar, -topFar, -far));
    
    frustum.addVertex(ofPoint(right, -bottom, -near));
    frustum.addVertex(ofPoint(rightFar, -bottomFar, -far));
    
    frustum.addVertex(ofPoint(left, -bottom, -near));
    frustum.addVertex(ofPoint(leftFar, -bottomFar, -far));
    
    
    frustum.addVertex(ofPoint(left, -top, -near));
    frustum.addVertex(ofPoint(right, -top, -near));
    
    frustum.addVertex(ofPoint(right, -top, -near));
    frustum.addVertex(ofPoint(right, -bottom, -near));
    
    frustum.addVertex(ofPoint(right, -bottom, -near));
    frustum.addVertex(ofPoint(left, -bottom, -near));
    
    frustum.addVertex(ofPoint(left, -bottom, -near));
    frustum.addVertex(ofPoint(left, -top, -near));
    
    
    frustum.addVertex(ofPoint(leftFar, -topFar, -far));
    frustum.addVertex(ofPoint(rightFar, -topFar, -far));
    
    frustum.addVertex(ofPoint(rightFar, -topFar, -far));
    frustum.addVertex(ofPoint(rightFar, -bottomFar, -far));
    
    frustum.addVertex(ofPoint(rightFar, -bottomFar, -far));
    frustum.addVertex(ofPoint(leftFar, -bottomFar, -far));
    
    frustum.addVertex(ofPoint(leftFar, -bottomFar, -far));
    frustum.addVertex(ofPoint(leftFar, -topFar, -far));

}