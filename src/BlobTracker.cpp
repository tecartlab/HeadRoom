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
    headBlob.setResolution(1, 1);
    lastUpdateFrame = 0;
    isDead = false;
}

bool BlobTracker::isAlive(){
    if(lastUpdateFrame > N_EMPTYFRAMES || isDead)
        return false;
    return true;
}

bool BlobTracker::finder(ofRectangle _rect){
    if(baseRectangle2d.inside(_rect.getCenter()) || (baseRectangle2d.getCenter() - _rect.getCenter()).length() < (baseRectangle2d.getPerimeter() / 4.))
        return true;
    return false;
}

void BlobTracker::updateStart(){
    lastUpdateFrame++;
    hasBodyUpdated = false;
    hasHeadUpdated = false;
}

void BlobTracker::updateBody(ofRectangle _rect, ofVec3f _bodyBlobCenter, ofVec2f _bodyBlobSize, ofVec3f _headTop){
    if(!hasBodyUpdated){
        newBaseRectangle2d = _rect;
        tracker.insert(tracker.begin(), TrackedBlob());
        
        tracker[0].bodyBlobCenter = _bodyBlobCenter;
        tracker[0].bodyBlobSize = _bodyBlobSize;
        tracker[0].headTop = _headTop;
        
        headCenter = ofVec3f(headTop.x, headTop.y, headTop.z - eyeLevel);
    } else {
        // this means the previous blob was split up.
        if(tracker[0].headTop.z < _headTop.z){ //if this new blob is higher then take it
            
            tracker[0].bodyBlobCenter = _bodyBlobCenter;
            tracker[0].bodyBlobSize = _bodyBlobSize;
            tracker[0].headTop = _headTop;

            headCenter = ofVec3f(headTop.x, headTop.y, headTop.z - eyeLevel);
            
            // but keep the old rectangle
            newBaseRectangle2d = baseRectangle2d;
        }
    }
    
    hasBodyUpdated = true;
}

void BlobTracker::updateHead(ofVec3f _headBlobCenter, ofVec2f _headBlobSize, ofVec3f _eyeCenter, float _eyelevel){
    if(!hasHeadUpdated){
        tracker[0].headBlobCenter = _headBlobCenter;
        tracker[0].headBlobSize = _headBlobSize;
        tracker[0].eyeCenter = _eyeCenter;
        eyeLevel = _eyelevel;
    } else {
        // this means the previous blob was split up.
        if(tracker[0].headBlobCenter.z < _headBlobCenter.z){ //if this new blob is higher then take it
            tracker[0].headBlobCenter = _headBlobCenter;
            tracker[0].headBlobSize = _headBlobSize;
            tracker[0].eyeCenter = _eyeCenter;
            eyeLevel = _eyelevel;
        }
    }
    
    hasHeadUpdated = true;
}

void BlobTracker::updateEnd(ofVec3f _kinectPos, int _smoothOffset, float _smoothFactor){
    if(hasBodyUpdated && hasHeadUpdated){
        ofVec3f kinect = _kinectPos - headCenter;
        float dist = kinect.length();
        trackerSize = _smoothOffset + (int) (dist / 1000. * _smoothFactor);
        while(tracker.size() > trackerSize)
            tracker.pop_back();
        
        baseRectangle2d = newBaseRectangle2d;

        
        bodyBlobCenter = ofVec3f();
        bodyBlobSize = ofVec3f();
        headTop = ofVec3f();
        headBlobCenter = ofVec3f();
        headBlobSize = ofVec3f();
        eyeCenter = ofVec3f();
        
        for (int i = 0; i < tracker.size(); i++){
            bodyBlobCenter += tracker[i].bodyBlobCenter;
            bodyBlobSize += tracker[i].bodyBlobSize;
            headTop += tracker[i].headTop;
            headBlobCenter += tracker[i].headBlobCenter;
            headBlobSize += tracker[i].headBlobSize;
            eyeCenter += tracker[i].eyeCenter;
        }
        
        bodyBlobCenter /= tracker.size();
        bodyBlobSize /= tracker.size();
        headTop /= tracker.size();
        headBlobCenter /= tracker.size();
        headBlobSize /= tracker.size();
        eyeCenter /= tracker.size();

        lastUpdateFrame = 0;
        valid = true;
    } else {
        valid = false;
    }
}

void BlobTracker::kill(){
    isDead = true;
    valid = false;
}
    

void BlobTracker::drawBodyBox(){
    //ofLog(OF_LOG_NOTICE, "bodyBox.size : " + ofToString(bodyBox.getSize()));
    //ofLog(OF_LOG_NOTICE, "bodyBox.pos : " + ofToString(bodyBox.getPosition()));
    if(valid){
        bodyBox.set(bodyBlobSize.x, bodyBlobSize.y, bodyBlobCenter.z);
        bodyBox.setPosition(bodyBlobCenter.x, bodyBlobCenter.y, bodyBlobCenter.z / 2);
        bodyBox.drawWireframe();
        
    }
}

void BlobTracker::drawHeadTop(){
    if(valid){
        bodyHeadTopSphere.setRadius(20);
        bodyHeadTopSphere.setPosition(headTop.x, headTop.y, headTop.z);
        bodyHeadTopSphere.drawWireframe();
    }
}

void BlobTracker::drawHeadBlob(){
    if(valid){
        headBlob.set(headBlobSize.x, headBlobSize.y);
        headBlob.setPosition(headBlobCenter);
        headBlob.drawWireframe();
        
        headCenterSphere.setRadius(20);
        headCenterSphere.setPosition(headCenter);
        headCenterSphere.draw();
    }
}

void BlobTracker::drawEyeCenter(){
    if(valid){
        eyeCenterSphere.setRadius(20);
        eyeCenterSphere.setPosition(eyeCenter);
        eyeCenterSphere.draw();
        
        contourMesh.clear();
        contourMesh.setMode(OF_PRIMITIVE_LINES);
        
        for(int i = 0; i < countour.size(); i++){
            contourMesh.addVertex(countour[i]);
        }
        contourMesh.draw();
    }
}
