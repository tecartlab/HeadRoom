//
//  BlobTracker.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "BlobTracker.h"

BlobTracker::BlobTracker(ofRectangle _rect){
    baseRectangle2d = _rect;
    trackerSize = 5;
    waitForUpdates = 0;
}

void BlobTracker::nextFrame(){
    waitForUpdates++;
}

bool BlobTracker::hasDied(){
    if(waitForUpdates > N_EMPTYFRAMES)
        return true;
    
    return false;
}


bool BlobTracker::finder(ofRectangle _rect){
    if(baseRectangle2d.inside(_rect.getCenter()))
        return true;
    return false;
}

void BlobTracker::update(ofRectangle _rect, ofVec3f _bodyCenter, ofVec2f _bodySize, ofVec3f _headTop){
    baseRectangle2d = _rect;
    tracker.insert(tracker.begin(), TrackedBlob());
    
    tracker[0].bodyCenter = _bodyCenter;
    tracker[0].bodySize = _bodySize;
    tracker[0].headTop = _headTop;
    
    while(tracker.size() > trackerSize)
        tracker.pop_back();
    
    bodyCenter = ofVec3f();
    bodySize = ofVec3f();
    headTop = ofVec3f();

    for (int i = 0; i < tracker.size(); i++){
        bodyCenter += tracker[i].bodyCenter;
        bodySize += tracker[i].bodySize;
        headTop += tracker[i].headTop;
    }
    
    bodyCenter /= tracker.size();
    bodySize /= tracker.size();
    headTop /= tracker.size();
    
    waitForUpdates = 0;
    
    bodyBox.set(bodySize.x, bodySize.y, bodyCenter.z);
    bodyBox.setPosition(bodyCenter.x, bodyCenter.y, bodyCenter.z / 2);
    
    bodyHeadTop.setRadius(3);
    bodyHeadTop.setPosition(headTop.x, headTop.y, headTop.z);
}

void BlobTracker::updateHead(ofVec2f _headSize, ofVec3f _eyePoint){
    tracker[0].headSize = _headSize;
    tracker[0].eyePoint = _eyePoint;
    
    headSize = ofVec3f();
    eyePoint = ofVec3f();
    
    for (int i = 0; i < tracker.size(); i++){
        headSize += tracker[i].headSize;
        eyePoint += tracker[i].eyePoint;
    }
    
    headSize /= tracker.size();
    eyePoint /= tracker.size();
}

void BlobTracker::drawBodyBox(){
    //ofLog(OF_LOG_NOTICE, "bodyBox.size : " + ofToString(bodyBox.getSize()));
    //ofLog(OF_LOG_NOTICE, "bodyBox.pos : " + ofToString(bodyBox.getPosition()));

    bodyBox.drawWireframe();
}

void BlobTracker::drawHeadTop(){
    bodyHeadTop.drawWireframe();
}

void BlobTracker::drawHeadSize(){
    
}

void BlobTracker::drawEyePosition(){
    
}
