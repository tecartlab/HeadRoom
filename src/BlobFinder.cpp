//
//  BlobFinder.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "BlobFinder.h"

void BlobFinder::setup(){

    //////////
    //GUI   //
    //////////
    
    captureScreenSize = ofVec2f(640, 480);
    
    gui.setup("Tracking Panel");
    
    sensorBoxLeft.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxRight.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxFront.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxBack.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxTop.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxBottom.addListener(this, &BlobFinder::updateSensorBox);
    
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

void BlobFinder::allocate(){
    fbopixels.allocate(captureScreenSize.x, captureScreenSize.y, OF_PIXELS_RGB);
    
	colorImg.allocate(captureScreenSize.x, captureScreenSize.y);
    
	grayImage.allocate(captureScreenSize.x, captureScreenSize.y);
	grayEyeLevel.allocate(captureScreenSize.x, captureScreenSize.y);
	grayThreshFar.allocate(captureScreenSize.x, captureScreenSize.y);
	
	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
}

void BlobFinder::update(ofFbo & captureFBO){
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
    
    //tells all the blobs that the next frame is comming
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].nextFrame();
    }

    nBlobs = contourFinder.nBlobs;
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        ofRectangle bounds = contourFinder.blobs[i].boundingRect;
        float pixelBrightness = 0;
        float brightness = 0;
        
        // find the brightest pixel within the blob. this defines the height of the blob
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                brightness = (pixelBrightness > brightness)?pixelBrightness: brightness;
            }
        }
        
        //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+" brightness] : " + ofToString(brightness));

        //calculate the blob pos in worldspace
        ofVec3f blobPos = ofVec3f(((float)bounds.getCenter().x / captureScreenSize.x) * (sensorFieldRight - sensorFieldLeft) + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / captureScreenSize.y ) * (sensorFieldBack - sensorFieldFront), (brightness / 255.0) * (sensorFieldTop - sensorFieldBottom) + sensorFieldBottom);

        //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+" blobPos : " + ofToString(blobPos));

        //calculate the blob size in worldspace
        ofVec2f blobSize = ofVec2f(((float)bounds.getWidth() / (float)grayImage.width) * (sensorFieldRight - sensorFieldLeft), ((float)bounds.getHeight() / (float)grayImage.height ) * (sensorFieldBack - sensorFieldFront));
        
        // find all the pixels below the eyelevel threshold. this yealds an image with blobs that mark the size of the head at eyelevel.
        ofVec2f headtop2d = ofVec2f();
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
                    headtop2d += ofVec2f(x, y);
                    brighCounter++;
                }
            }
        }
        headtop2d /= brighCounter;
        
        ofVec3f headTop = ofVec3f((headtop2d.x / (float)grayImage.width) * (sensorFieldRight - sensorFieldLeft) + sensorFieldLeft, sensorFieldBack - (headtop2d.y / (float)grayImage.height ) * (sensorFieldBack - sensorFieldFront), (brightness / 255.0) * (sensorFieldTop - sensorFieldBottom) + sensorFieldBottom);

        
        // try finding a matching trackedBlob
        bool foundBlob = false;
        for(int i = 0; i < trackedBlobs.size(); i++){
            if(trackedBlobs[i].finder(bounds)){
                trackedBlobs[i].update(bounds, blobPos, blobSize, headTop);
                foundBlob = true;
            }
        }
        // if none is found, create a new one.
        if(!foundBlob){
            trackedBlobs.insert(trackedBlobs.begin(), BlobTracker(bounds));
            trackedBlobs[0].update(bounds, blobPos, blobSize, headTop);
        }
    }
    
    grayEyeLevel.setFromPixels(eyeRef.getPixels(), eyeRef.getWidth(), eyeRef.getHeight());
    grayEyeLevel.invert();
    grayEyeLevel.threshold(20);
    grayEyeLevel.invert();
    grayEyeLevel.blurGaussian();
    
    //find head shape on eye height contours
    contourEyeFinder.findContours(grayEyeLevel, blobAreaMin.get()/4, blobAreaMax.get(), countBlob.get(), false);
    
    //ofLog(OF_LOG_NOTICE, "contourEyeFinder nBlobs : " + ofToString(contourEyeFinder.nBlobs));

    for(int i = 0; i < contourEyeFinder.nBlobs; i++){
        //contourEyeFinder.blobs[i].pts
        
    }
    
    //removes all the blobs that hasnt had an update for a specific number of frames
    for(int i = 0; i < trackedBlobs.size(); i++){
        if(trackedBlobs[i].hasDied()){
            trackedBlobs[i] = trackedBlobs.back();
            trackedBlobs.pop_back();
            i--;
        }
    }

}

void BlobFinder::drawBodyBlobs(ofRectangle _rect){
    float xFactor = _rect.width / captureScreenSize.x;
    float yFactor = 1.f;
    
    for(int i = 0; i < trackedBlobs.size(); i++){
        ofRect(_rect.x + trackedBlobs[i].baseRectangle2d.x * xFactor, _rect.y + trackedBlobs[i].baseRectangle2d.y * yFactor, trackedBlobs[i].baseRectangle2d.width * xFactor, trackedBlobs[i].baseRectangle2d.height * yFactor);
    }
}

void BlobFinder::drawBodyBlobsBox(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].drawBodyBox();
    }
}

void BlobFinder::drawBodyBlobsHeadTop(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].drawHeadTop();
    }
}


void BlobFinder::updateSensorBox(int & value){
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
