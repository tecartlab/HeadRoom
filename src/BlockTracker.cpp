//
//  Planef.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "BlobTracker.h"

void BlobTracker::setup(){

    //////////
    //GUI   //
    //////////
    
    gui.setup("Tracking Panel");
    
    sensorBoxLeft.addListener(this, &BlobTracker::updateSensorBox);
    sensorBoxRight.addListener(this, &BlobTracker::updateSensorBox);
    sensorBoxFront.addListener(this, &BlobTracker::updateSensorBox);
    sensorBoxBack.addListener(this, &BlobTracker::updateSensorBox);
    sensorBoxTop.addListener(this, &BlobTracker::updateSensorBox);
    sensorBoxBottom.addListener(this, &BlobTracker::updateSensorBox);
    
    sensorBoxGuiGroup.setName("sensorField");
    sensorBoxGuiGroup.add(sensorBoxLeft.set("left", -500, 0, -2000));
    sensorBoxGuiGroup.add(sensorBoxRight.set("right", 500, 0, 2000));
    sensorBoxGuiGroup.add(sensorBoxFront.set("front", 0, 0, 7000));
    sensorBoxGuiGroup.add(sensorBoxBack.set("back", 2000, 0, 7000));
    sensorBoxGuiGroup.add(sensorBoxTop.set("top", 2200, 0, 3000));
    sensorBoxGuiGroup.add(sensorBoxBottom.set("bottom", 1000, 0, 3000));
    gui.add(sensorBoxGuiGroup);
    
    blobGuiGroup.setName("Blobs");
    blobGuiGroup.add(blobAreaMin.set("AreaMin", 1000, 0, 40000));
    blobGuiGroup.add(blobAreaMax.set("AreaMax", 6000, 0, 40000));
    blobGuiGroup.add(countBlob.set("MaxBlobs", 5, 1, N_MAX_BLOBS));
    gui.add(blobGuiGroup);
    
    gui.loadFromFile("trackings.xml");

}

void BlobTracker::updateSensorBox(int & value){
    sensorBox.clear();
    sensorBox.setMode(OF_PRIMITIVE_LINES);
    
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxFront.get(), sensorBoxBottom.get()));
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxFront.get(), sensorBoxBottom.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxFront.get(), sensorBoxBottom.get()));
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxBack.get(), sensorBoxBottom.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxBack.get(), sensorBoxBottom.get()));
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxBack.get(), sensorBoxBottom.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxBack.get(), sensorBoxBottom.get()));
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxFront.get(), sensorBoxBottom.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxFront.get(), sensorBoxTop.get()));
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxFront.get(), sensorBoxTop.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxFront.get(), sensorBoxTop.get()));
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxBack.get(), sensorBoxTop.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxRight.get(), sensorBoxBack.get(), sensorBoxTop.get()));
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxBack.get(), sensorBoxTop.get()));
    
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxBack.get(), sensorBoxTop.get()));
    sensorBox.addVertex(ofPoint(sensorBoxLeft.get(), sensorBoxFront.get(), sensorBoxTop.get()));
    
    //captureCam.setPosition((sensorBoxLeft.get() + sensorBoxRight.get())/2, (sensorBoxBack.get() + sensorBoxBack.get())/2, sensorBoxTop.get());
    //captureCam.setPosition(5, 5, 0);
    //captureCam.
}
