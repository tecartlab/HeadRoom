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
    gazePointer.setRadius(50);
    
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
    blobGuiGroup.add(blobAreaMin.set("AreaMin", 1000, 0, 30000));
    blobGuiGroup.add(blobAreaMax.set("AreaMax", 6000, 0, 30000));
    blobGuiGroup.add(countBlob.set("MaxBlobs", 5, 1, N_MAX_BLOBS));
    gui.add(blobGuiGroup);

    blobEyeGroup.setName("Gazing");
    blobEyeGroup.add(gazePoint.set("Gaze Point", ofVec3f(0, 0, 1500), ofVec3f(-2000, 0, 0), ofVec3f(2000, 5000, 3000)));
    blobEyeGroup.add(eyeLevel.set("EyeLevel", 140, 0, 200));
    blobEyeGroup.add(eyeInset.set("EyeInset", .8, 0, 1));
    gui.add(blobEyeGroup);
    
    blobSmoothGroup.setName("Smoothing");
    blobSmoothGroup.add(smoothOffset.set("Offset", 2, 1, 10));
    blobSmoothGroup.add(smoothFactor.set("Factor",  1., 0., 5.));
    gui.add(blobSmoothGroup);

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
    float sensorFieldWidth = sensorFieldRight - sensorFieldLeft;
    float sensorFieldHeigth = sensorFieldTop - sensorFieldBottom;
    float sensorFieldDepth = sensorFieldBack - sensorFieldFront;
    
    int eyeLevelColor = eyeLevel.get() / sensorFieldHeigth * 255;
    
    //ofLog(OF_LOG_NOTICE, "eyref size : " + ofToString(eyeRef.size()));
    
    //tells all the blobs that the next frame is comming
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].updateStart();
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
        ofVec3f blobPos = ofVec3f(((float)bounds.getCenter().x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / captureScreenSize.y ) * sensorFieldDepth, (brightness / 255.0) * sensorFieldHeigth + sensorFieldBottom);

        //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+" blobPos : " + ofToString(blobPos));

        //calculate the blob size in worldspace
        ofVec2f blobSize = ofVec2f(((float)bounds.getWidth() / captureScreenSize.x) * sensorFieldWidth, ((float)bounds.getHeight() / captureScreenSize.y ) * sensorFieldDepth);
        
        // find all the pixels below the eyelevel threshold. this yealds an image with blobs that mark the size of the head at eyelevel.
        ofVec2f headtop2d = ofVec2f();
        int brighCounter = 0;
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                if(pixelBrightness > (brightness - eyeLevelColor)){
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
        
        ofVec3f headTop = ofVec3f((headtop2d.x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - (headtop2d.y / captureScreenSize.y ) * sensorFieldDepth, (brightness / 255.0) * sensorFieldHeigth + sensorFieldBottom);

        
        // try finding a matching trackedBlob
        bool foundBlob = false;
        for(int i = 0; i < trackedBlobs.size(); i++){
            if(trackedBlobs[i].finder(bounds)){
                trackedBlobs[i].updateBody(bounds, blobPos, blobSize, headTop);
                foundBlob = true;
            }
        }
        // if none is found, create a new one.
        if(!foundBlob){
            trackedBlobs.insert(trackedBlobs.begin(), BlobTracker(bounds));
            trackedBlobs[0].updateBody(bounds, blobPos, blobSize, headTop);
        }
    }
    
    //checks for double blobs and kills the lower ones.
    for(int i = 0; i < (trackedBlobs.size() - 1); i++){
        for(int j = 1; j < trackedBlobs.size(); j++){
            if(trackedBlobs[i].isAlive() && trackedBlobs[j].isAlive() && trackedBlobs[i].finder(trackedBlobs[j].baseRectangle2d)){
                if (trackedBlobs[i].headTop.z > trackedBlobs[j].headTop.z ) {
                    trackedBlobs[j].kill();
                } else {
                    trackedBlobs[i].kill();
                }
            }
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
        ofRectangle bounds = contourEyeFinder.blobs[i].boundingRect;
        for(int bid = 0; bid < trackedBlobs.size(); bid++){
            // find the blob
            if(trackedBlobs[bid].finder(bounds)){
        
                //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+" brightness] : " + ofToString(brightness));
                
                //calculate the blob pos in worldspace
                ofVec3f headBlobCenter = ofVec3f(((float)bounds.getCenter().x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / captureScreenSize.y ) * sensorFieldDepth, trackedBlobs[bid].headCenter.z);
                
                //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(i)+" blobPos : " + ofToString(blobPos));
                
                //calculate the blob size in worldspace
                ofVec2f headBlobSize = ofVec2f(((float)bounds.getWidth() / captureScreenSize.x) * sensorFieldWidth, ((float)bounds.getHeight() / captureScreenSize.y ) * sensorFieldDepth);
            
                //calculate the gazeVector
                ofVec3f gaze = trackedBlobs[bid].headCenter - gazePoint.get();
                
                float smalestAngle = 180;
                ofVec3f eyePoint;
                
                trackedBlobs[bid].countour.clear();
                
                for(int v = 0; v < contourEyeFinder.blobs[i].pts.size(); v++){
                    ofVec3f headPoint = ofVec3f(((float)contourEyeFinder.blobs[i].pts[v].x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)contourEyeFinder.blobs[i].pts[v].y / captureScreenSize.y ) * sensorFieldDepth, trackedBlobs[bid].headCenter.z);
                    trackedBlobs[bid].countour.push_back(headPoint);
                    
                    ofVec3f gaze2 = trackedBlobs[bid].headCenter - headPoint;
                    
                    float angle = gaze.angle(gaze2);
                    //ofLog(OF_LOG_NOTICE, "headtop["+ofToString(bid)+"] headPoint : " + ofToString(headPoint));
                    if(smalestAngle > angle){
                        smalestAngle = angle;
                        eyePoint = trackedBlobs[bid].headCenter - gaze.normalize().scale(gaze2.length() * eyeInset.get());
                    }
                }
        
                trackedBlobs[bid].updateHead(headBlobCenter, headBlobSize, eyePoint, eyeLevel.get());
            }
        }
    }
    
    //updates all alive blobs and removes all the blobs that havent had an update for a specific number of frames or have been killed
    for(int i = 0; i < trackedBlobs.size(); i++){
        if(trackedBlobs[i].isAlive()){
            trackedBlobs[i].updateEnd(kinectPos, smoothOffset.get(), smoothFactor.get());
        } else {
            //ofLog(OF_LOG_NOTICE, "blob[" + ofToString(i) + "]\n has died");
            trackedBlobs[i] = trackedBlobs.back();
            trackedBlobs.pop_back();
            i--;
        }
    }

}

void BlobFinder::drawBodyBlobs2d(ofRectangle _rect){
    float xFactor = _rect.width / captureScreenSize.x;
    float yFactor = 1.f;
    
    ofNoFill();
    for(int i = 0; i < trackedBlobs.size(); i++){
        ofRect(_rect.x + trackedBlobs[i].baseRectangle2d.x * xFactor, _rect.y + trackedBlobs[i].baseRectangle2d.y * yFactor, trackedBlobs[i].baseRectangle2d.width * xFactor, trackedBlobs[i].baseRectangle2d.height * yFactor);
        ofDrawBitmapString("blob[" + ofToString(i) + "]\n x = " + ofToString(trackedBlobs[i].headTop.x) + "\n y = " + ofToString(trackedBlobs[i].headTop.y) + "\n z = " + ofToString(trackedBlobs[i].headTop.z),trackedBlobs[i].baseRectangle2d.getCenter().x + _rect.x, trackedBlobs[i].baseRectangle2d.getCenter().y + _rect.y);
        
    }
}

void BlobFinder::drawBodyBlobsBox(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        //ofLog(OF_LOG_NOTICE, "blob[" + ofToString(i) + "] box =" + ofToString(trackedBlobs[i].bodyCenter));
        trackedBlobs[i].drawBodyBox();
    }
}

void BlobFinder::drawBodyBlobsHeadTop(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].drawHeadTop();
    }
}

void BlobFinder::drawHeadBlobs(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].drawHeadBlob();
    }
}

void BlobFinder::drawEyeCenters(){
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].drawEyeCenter();
    }
}

void BlobFinder::drawGazePoint(){
    gazePointer.setPosition(gazePoint.get());
    gazePointer.ofNode::draw();
    ofLine(gazePoint.get().x, gazePoint.get().y, 0, gazePoint.get().x, gazePoint.get().y, 3000);
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
