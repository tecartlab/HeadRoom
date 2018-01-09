//
//  BlobFinder.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "BlobFinder.h"

void BlobFinder::setup(ofxGui &gui){

    //////////
    //GUI   //
    //////////
    
    captureScreenSize = ofVec2f(640, 480);
    gazePointer.setRadius(1000);
    
    panel = gui.addPanel();
    
    panel->loadTheme("theme/theme_light.json");
    panel->setName("Tracking...");

    blobSmoothGroup.setName("NoiseReduction");
    blobSmoothGroup.add(smoothOffset.set("MinSamples", 2, 1, 10));
    blobSmoothGroup.add(smoothFactor.set("DistanceFac.",  1., 0., 5.));
    panel->addGroup(blobSmoothGroup);

    sensorBoxLeft.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxRight.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxFront.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxBack.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxTop.addListener(this, &BlobFinder::updateSensorBox);
    sensorBoxBottom.addListener(this, &BlobFinder::updateSensorBox);
    
    sensorBoxGuiGroup.setName("SensorBox");
    sensorBoxGuiGroup.add(sensorBoxLeft.set("left", -500, 0, -4000));
    sensorBoxGuiGroup.add(sensorBoxRight.set("right", 500, 0, 4000));
    sensorBoxGuiGroup.add(sensorBoxFront.set("front", 0, 0, 10000));
    sensorBoxGuiGroup.add(sensorBoxBack.set("back", 2000, 0, 7000));
    sensorBoxGuiGroup.add(sensorBoxTop.set("top", 2200, 0, 3000));
    sensorBoxGuiGroup.add(sensorBoxBottom.set("bottom", 1000, 0, 3000));
    panel->addGroup(sensorBoxGuiGroup);
    
    blobGuiGroup.setName("Blobs");
    blobGuiGroup.add(blobAreaMin.set("AreaMin", 1000, 0, 30000));
    blobGuiGroup.add(blobAreaMax.set("AreaMax", 6000, 0, 50000));
    blobGuiGroup.add(countBlob.set("MaxBlobs", 5, 1, N_MAX_BLOBS));
    panel->addGroup(blobGuiGroup);

    blobEyeGroup.setName("Gazing");
    blobEyeGroup.add(gazePoint.set("Gaze Point", ofVec3f(0, 0, 1500), ofVec3f(-2000, 0, 0), ofVec3f(2000, 5000, 3000)));
    blobEyeGroup.add(eyeLevel.set("EyeLevel", 140, 0, 200));
    blobEyeGroup.add(eyeInset.set("EyeInset", .8, 0, 1));
    panel->addGroup(blobEyeGroup);
    
    panel->loadFromFile("trackings.xml");

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
    
    ofFbo::Settings s;
    s.width             = captureScreenSize.x;
    s.height			= captureScreenSize.y;
    s.internalformat    = GL_RGB;
    s.useDepth			= true;
    // and assigning this values to the fbo like this:
    captureFBO.allocate(s);

}

void BlobFinder::captureBegin(){
    captureFBO.begin();
    ofClear(0, 0, 0, 0);
    captureCam.scale = 0.01;
    // FBO capturing
    captureCam.begin(ofRectangle(0, 0, captureScreenSize.x, captureScreenSize.y), sensorBoxLeft.get(), sensorBoxRight.get(), sensorBoxBack.get(), sensorBoxFront.get(), - sensorBoxTop.get(), sensorBoxTop.get());
    
}

void BlobFinder::captureEnd(){
    captureCam.end();
    captureFBO.end();
    
}


void BlobFinder::update(){
    captureFBO.readToPixels(fbopixels);
    
    colorImg.setFromPixels(fbopixels);
    
    // load grayscale captured depth image from the color source
    grayImage.setFromColorImage(colorImg);
    
    //grayImage.blurHeavily();
    
    grayEyeLevel = grayImage;
    
    ofPixelsRef eyeRef = grayEyeLevel.getPixels();
    ofPixelsRef greyref = grayImage.getPixels();
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
    
    int headTopThreshold = eyeLevelColor / 4;
    
    //ofLog(OF_LOG_NOTICE, "eyeLevelColor = " + ofToString(eyeLevelColor));
    
    //ofLog(OF_LOG_NOTICE, "eyref size : " + ofToString(eyeRef.size()));
    
    //tells all the blobs that the next frame is comming
    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].updateStart();
    }
  
    // find contours in the raw grayImage
    contourFinder.findContours(grayImage, blobAreaMin.get(), blobAreaMax.get(), countBlob.get(), false);

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

        float averageBrightness = 0;
        int averageCounter = 0;

        // go through the pixels again and get the average brightness for the headTopThreshold
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                if(pixelBrightness > brightness - headTopThreshold){
                    averageBrightness += pixelBrightness;
                    averageCounter++;
                }
            }
        }
        
        brightness = averageBrightness / averageCounter;

        //calculate the blob pos in worldspace
        ofVec3f blobPos = ofVec3f(((float)bounds.getCenter().x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / captureScreenSize.y ) * sensorFieldDepth, (brightness / 255.0) * sensorFieldHeigth + sensorFieldBottom);

        //calculate the blob size in worldspace
        ofVec2f blobSize = ofVec2f(((float)bounds.getWidth() / captureScreenSize.x) * sensorFieldWidth, ((float)bounds.getHeight() / captureScreenSize.y ) * sensorFieldDepth);
        
        // find all the pixels down to the eyelevel threshold. this yealds an image with blobs that mark the size of the head at eyelevel.
        ofVec2f headtop2d = ofVec2f();
        int brighCounter = 0;
        for(int x = bounds.x; x < bounds.x + bounds.width; x++){
            for(int y = bounds.y; y < bounds.y + bounds.height; y++){
                pixelBrightness = greyref.getColor(x, y).getBrightness();
                if(pixelBrightness > (brightness - eyeLevelColor)){
                    //writes the pixels above the eyelevel into the eyeRef image
                    eyeRef.setColor(x, y, white);
                }else{
                    eyeRef.setColor(x, y, black);
                }
                if(pixelBrightness >= brightness - (eyeLevelColor / 4)){
                    headtop2d += ofVec2f(x, y);
                    brighCounter++;
                }
            }
        }
        headtop2d /= brighCounter;
        
        //ofLog(OF_LOG_NOTICE, "headtop2d = " + ofToString(headtop2d));
        
        ofVec3f headTop = ofVec3f((headtop2d.x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - (headtop2d.y / captureScreenSize.y ) * sensorFieldDepth, (brightness / 255.0) * sensorFieldHeigth + sensorFieldBottom);

        ofVec3f headCenter = ofVec3f(headTop.x, headTop.y, headTop.z - eyeLevel.get());

        // try finding a matching trackedBlob
        bool foundBlob = false;
        for(int i = 0; i < trackedBlobs.size(); i++){
            if(trackedBlobs[i].finder(bounds)){
                trackedBlobs[i].updateBody(bounds, blobPos, blobSize, headTop, headCenter, eyeLevel.get());
                foundBlob = true;
            }
        }
        // if none is found, create a new one.
        if(!foundBlob){
            trackedBlobs.insert(trackedBlobs.begin(), BlobTracker(bounds));
            trackedBlobs[0].updateBody(bounds, blobPos, blobSize, headTop, headCenter, eyeLevel.get());
        }
    }

    //checks for double blobs and kills the lower ones.
    
    if( trackedBlobs.size() > 1 )
    {
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
    }

    //preprocesses the eyeRef image
    //grayEyeLevel.setFromPixels(eyeRef.getPixels(), eyeRef.getWidth(), eyeRef.getHeight());
    grayEyeLevel.invert();
    grayEyeLevel.threshold(20);
    grayEyeLevel.invert();
    grayEyeLevel.blurGaussian();


    //ofLog(OF_LOG_NOTICE, "contourEyeFinder nBlobs : " + ofToString(contourEyeFinder.nBlobs));


    //find head shape on eye height contours
    contourEyeFinder.findContours(grayEyeLevel, blobAreaMin.get()/4, blobAreaMax.get(), countBlob.get(), false);
    for(int i = 0; i < contourEyeFinder.nBlobs; i++){
        ofRectangle bounds = contourEyeFinder.blobs[i].boundingRect;
        for(int bid = 0; bid < trackedBlobs.size(); bid++){
            // find the blob
            if(trackedBlobs[bid].finder(bounds)){
                
                //calculate the blob pos in worldspace
                ofVec3f headBlobCenter = ofVec3f(((float)bounds.getCenter().x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)bounds.getCenter().y / captureScreenSize.y ) * sensorFieldDepth, trackedBlobs[bid].headCenter.z);
                
                //calculate the blob size in worldspace
                ofVec2f headBlobSize = ofVec2f(((float)bounds.getWidth() / captureScreenSize.x) * sensorFieldWidth, ((float)bounds.getHeight() / captureScreenSize.y ) * sensorFieldDepth);
            
                //calculate the gazeVector
                ofVec3f gaze = trackedBlobs[bid].getCurrentHeadCenter() - gazePoint.get();
                
                gaze.z = 0;
                
                float smalestAngle = 180;
                ofVec3f eyePoint;
                
                //clears the contour storage
                trackedBlobs[bid].countour.clear();
                
                // findes the closest contour point to the eyegave-vector, takes its distance to the headCenter and calculated
                // the eye - center - point
                for(int v = 0; v < contourEyeFinder.blobs[i].pts.size(); v++){
                    ofVec3f headPoint = ofVec3f(((float)contourEyeFinder.blobs[i].pts[v].x / captureScreenSize.x) * sensorFieldWidth + sensorFieldLeft, sensorFieldBack - ((float)contourEyeFinder.blobs[i].pts[v].y / captureScreenSize.y ) * sensorFieldDepth, trackedBlobs[bid].headCenter.z);
                    
                    trackedBlobs[bid].countour.push_back(headPoint);
                    
                    ofVec3f gaze2 = trackedBlobs[bid].getCurrentHeadCenter() - headPoint;
                    
                    float angle = gaze.angle(gaze2);
                    
                    if(smalestAngle > angle){
                        smalestAngle = angle;
                        eyePoint = trackedBlobs[bid].getCurrentHeadCenter() - gaze.normalize().scale(gaze2.length() * eyeInset.get());
                    }
                }
        
                trackedBlobs[bid].updateHead(headBlobCenter, headBlobSize, eyePoint);
            }
        }
    }


    //sorts the blobs in regards to the distance of the gazepoint.
    int sortPos = 0;

    for(int i = 0; i < trackedBlobs.size(); i++){
        trackedBlobs[i].sortPos = sortPos++;
    }
    if(trackedBlobs.size() > 0){
        for(int i = 0; i < (trackedBlobs.size() - 1); i++){
            for(int j = 1; j < trackedBlobs.size(); j++){
                if((trackedBlobs[i].headCenter - gazePoint.get()).length() < (trackedBlobs[j].headCenter - gazePoint.get()).length()){
                    if(trackedBlobs[i].sortPos > trackedBlobs[j].sortPos){
                        int savepos = trackedBlobs[j].sortPos;
                        trackedBlobs[j].sortPos = trackedBlobs[i].sortPos;
                        trackedBlobs[i].sortPos = savepos;
                    }
                } else {
                    if(trackedBlobs[i].sortPos < trackedBlobs[j].sortPos){
                        int savepos = trackedBlobs[j].sortPos;
                        trackedBlobs[j].sortPos = trackedBlobs[i].sortPos;
                        trackedBlobs[i].sortPos = savepos;
                    }
                }
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
        ofDrawRectangle(_rect.x + trackedBlobs[i].baseRectangle2d.x * xFactor, _rect.y + trackedBlobs[i].baseRectangle2d.y * yFactor, trackedBlobs[i].baseRectangle2d.width * xFactor, trackedBlobs[i].baseRectangle2d.height * yFactor);
        ofDrawBitmapString("blob[" + ofToString(i) + "]\n sort = " + ofToString(trackedBlobs[i].sortPos) + "\n x = " + ofToString(trackedBlobs[i].headTop.x) + "\n y = " + ofToString(trackedBlobs[i].headTop.y) + "\n z = " + ofToString(trackedBlobs[i].headTop.z),trackedBlobs[i].baseRectangle2d.getCenter().x + _rect.x, trackedBlobs[i].baseRectangle2d.getCenter().y + _rect.y);
        
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
    //gazePointer.setPosition(gazePoint.get());
    //gazePointer.ofNode::draw();
    ofDrawSphere(gazePoint.get().x, gazePoint.get().y, gazePoint.get().z, 50);
    ofDrawLine(gazePoint.get().x, gazePoint.get().y, 0, gazePoint.get().x, gazePoint.get().y, 3000);
}

bool BlobFinder::hasParamUpdate(){
    if(parameterHasUpdated){
        parameterHasUpdated = false;
        return true;
    }
    return false;
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
    parameterHasUpdated = true;
}
