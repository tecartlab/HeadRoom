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


void BlobTracker::allocate(int width, int height){
    fbopixels.allocate(640, 480, OF_PIXELS_RGB);
    
	colorImg.allocate(width, height);
    
	grayImage.allocate(width, height);
	grayEyeLevel.allocate(width, height);
	grayThreshFar.allocate(width, height);
	
	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
}

void BlobTracker::update(ofFbo & captureFBO){
    captureFBO.readToPixels(fbopixels);
    
    colorImg.setFromPixels(fbopixels);
    
    // load grayscale captured depth image from the color source
    grayImage.setFromColorImage(colorImg);
    
    
    // find contours in the raw grayImage
    contourFinder.findContours(grayImage, blobAreaMin.get(), blobAreaMax.get(), countBlob.get(), false);
    
    grayEyeLevel = grayImage;
    
    ofPixelsRef eyeRef = grayEyeLevel.getPixelsRef();
    ofPixelsRef greyref = grayImage.getPixelsRef();
    ofColor white = ofColor::white;
    ofColor black = ofColor::black;
    
    float sensorFieldFront = sensorBoxFront.get();
    float sensorFieldBack = sensorBoxBack.get();
    float sensorFieldLeft = sensorBoxLeft.get();
    float sensorFieldRight = sensorBoxRight.get();
    float sensorFieldTop = sensorBoxTop .get();
    float sensorFieldBottom = sensorBoxBottom.get();
    
    // the eyelevel calculated from the sensorfield
    int eyeLevel = EYE_DIFF_TO_HEADTOP / (sensorFieldTop - sensorFieldBottom) * 255;
    
    //ofLog(OF_LOG_NOTICE, "eyref size : " + ofToString(eyeRef.size()));
    
    nBlobs = contourFinder.nBlobs;
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        ofRectangle bounds = contourFinder.blobs[i].boundingRect;
        int pixelBrightness = 0;
        float brightness = 0;
        
        // find the brightest pixel within the blob. this defines the height of the blob
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                brightness = (pixelBrightness > brightness)?pixelBrightness: brightness;
            }
        }
        
        //calculate the blob pos in worldspace
        blobPos[i] = ofVec3f(((float)bounds.getCenter().x / (float)grayImage.width) * (sensorFieldRight - sensorFieldLeft) + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / (float)grayImage.height ) * (sensorFieldBack - sensorFieldFront), (brightness / 255.0) * (sensorFieldTop - sensorFieldBottom) + sensorFieldBottom);
        
        //calculate the blob size in worldspace
        blobSize[i] = ofVec3f(((float)bounds.getWidth() / (float)grayImage.width) * (sensorFieldRight - sensorFieldLeft), ((float)bounds.getHeight() / (float)grayImage.height ) * (sensorFieldBack - sensorFieldFront));
        
        // find all the pixels below the eyelevel threshold. this yealds an image with blobs that mark the size of the head at eyelevel.
        headtop[i] = ofVec2f();
        int brighCounter = 0;
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                if(pixelBrightness > (brightness - eyeLevel)){
                    eyeRef.setColor(x, y, white);
                }else{
                    eyeRef.setColor(x, y, black);
                }
                if(pixelBrightness == brightness){
                    headtop[i] += ofVec2f(x, y);
                    brighCounter++;
                }
            }
        }
        headtop[i] /= brighCounter;
        //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+"] : " + ofToString(headtop[i]));
        
        
    }
    grayEyeLevel.setFromPixels(eyeRef.getPixels(), eyeRef.getWidth(), eyeRef.getHeight());
    grayEyeLevel.invert();
    grayEyeLevel.threshold(20);
    grayEyeLevel.invert();
    grayEyeLevel.blurGaussian();
    
    //find head shape on eye height contours
    contourEyeFinder.findContours(grayEyeLevel, blobAreaMin.get()/4, blobAreaMax.get(), countBlob.get(), false);
    
    ofLog(OF_LOG_NOTICE, "contourEyeFinder nBlobs : " + ofToString(contourEyeFinder.nBlobs));

    for(int i = 0; i < contourEyeFinder.nBlobs; i++){
        //contourEyeFinder.blobs[i].pts
        
    }
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
