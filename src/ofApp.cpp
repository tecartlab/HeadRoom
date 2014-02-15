#include "ofApp.h"

#define RECONNECT_TIME 400

#define DEPTH_X_RES 640
#define DEPTH_Y_RES 480


//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running to fast
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

    //create the socket and set to send to 127.0.0.1:11999
	//udpConnection.Create();
	//udpConnection.Connect("127.0.0.1",4653);
	//udpConnection.SetNonBlocking(true);

    //setup the servers to listen
    rgbaMatrixServer.setup(34101);
	depthMatrixServer.setup(34102);
	rawMatrixServer.setup(34103);

	//TCP.setMessageDelimiter("\n");

    //////////
    //GUI   //
    //////////
    
    tiltAngle.addListener(this, &ofApp::setKinectTiltAngle);
    
    gui.setup("panel");
    gui.add(calibPoint1.set("calib1", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint2.set("calib2", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint3.set("calib3", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(tiltAngle.set("tilt", 0, -30, 30));
    
    gui.loadFromFile("settings.xml");
    
    //////////
    //KINECT//
    //////////
    
    ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
		
	// start from the front
	bDrawPointCloud = false;
    
    createCone(-.1, .1, -.1, .1, .2, 5000.);
    
}

void ofApp::createCone(float f_left, float f_right, float f_top, float f_bottom, float f_near, float f_far){
    double ref_pix_size = kinect.getZeroPlanePixelSize();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    double factorNear = 2 * ref_pix_size * 400 / ref_distance;
    double factorFar = 2 * ref_pix_size * 4000 / ref_distance;
    
    int near = 400;
    int far = 4000;
	//ofVec3f((x - DEPTH_X_RES/2) *factor, (y - DEPTH_Y_RES/2) *factor, raw[y * w + x]));

    frustum.clear();
    frustum.setMode(OF_PRIMITIVE_LINES);
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));
    
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));

    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));

    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));

    
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));

    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));
    
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));

    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (480 - DEPTH_Y_RES/2) *factorNear, near));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorNear, (0 - DEPTH_Y_RES/2) *factorNear, near));

    
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));
    
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));
    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));

    frustum.addVertex(ofPoint((640 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));

    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (480 - DEPTH_Y_RES/2) *factorFar, far));
    frustum.addVertex(ofPoint((0 - DEPTH_X_RES/2) *factorFar, (0 - DEPTH_Y_RES/2) *factorFar, far));


    /*
    float coneRatio = f_far / f_near;
    //	outlet(0,"linesegment", 0, f_left, -f_near, 0, f_left * coneRatio, -f_far);
//    frustum.addVertex(ofPoint(f_left, f_bottom, -f_near));
//    frustum.addVertex(ofPoint(f_left * coneRatio, f_bottom * coneRatio, -f_far));
    
    //	outlet(0,"linesegment", 0, f_right, -f_near, 0, f_right * coneRatio, -f_far);
    frustum.addVertex(ofPoint(f_right, f_bottom, -f_near));
    frustum.addVertex(ofPoint(f_right * coneRatio, f_bottom * coneRatio, -f_far));
    
    //	outlet(0,"linesegment", f_top, 0, -f_near, f_top * coneRatio, 0, -f_far);
    frustum.addVertex(ofPoint(f_left, f_top, -f_near));
    frustum.addVertex(ofPoint(f_left * coneRatio, f_top * coneRatio, -f_far));
    
    //outlet(0,"linesegment", f_bottom, 0, -f_near,f_bottom * coneRatio, 0, -f_far);
    frustum.addVertex(ofPoint(f_right, f_top, -f_near));
    frustum.addVertex(ofPoint(f_right * coneRatio, f_top * coneRatio, -f_far));
    */
}

void ofApp::setKinectTiltAngle(int & angle){
	kinect.setCameraTiltAngle(angle);
    ofResetElapsedTimeCounter();
    bUpdateCalc = true;
}

//--------------------------------------------------------------
void ofApp::updateCalc(){
    if(kinect.isConnected() && kinect.isFrameNewDepth()){
        
        planePoint1 = calcPlanePoint(calibPoint1, 1, 1);
        planePoint2 = calcPlanePoint(calibPoint2, 1, 1);
        planePoint3 = calcPlanePoint(calibPoint3, 1, 1);
        
        sphere1.setPosition(planePoint1);
        sphere2.setPosition(planePoint2);
        sphere3.setPosition(planePoint3);

        sphere1.setRadius(10);
        sphere2.setRadius(10);
        sphere3.setRadius(10);

        //sphere1.enableColors();
        //sphere2.enableColors();
        //sphere3.enableColors();
        
        Planef floorPlane = Planef(planePoint1, planePoint2, planePoint3);
        Linef centerLine = Linef(ofVec3f(0, 0, 0), ofVec3f(0, 0, -1));
        
        Planef verticalViewPlane = Planef(ofVec3f(0, 0, 0), ofVec3f(0, 0, -1), ofVec3f(0, 1, -1));
        Planef horizontalViewPlane = Planef(ofVec3f(0, 0, 0), ofVec3f(0, 0, -1), ofVec3f(1, 0, -1));
        
        Linef verticalLine;
        if(verticalViewPlane.intersects(floorPlane))
           verticalLine = verticalViewPlane.getIntersection(floorPlane);
        
        frustumCenterPoint = floorPlane.getIntersection(centerLine);
        
        ofVec3f CenterPointXAxis = ofVec3f(frustumCenterPoint).cross(ofVec3f(verticalLine.direction).scale(100));
        
        ofVec3f planeZAxis = ofVec3f(floorPlane.normal).scale(1000);
        
        ofVec3f CenterPointRotXtoZAxis = ofVec3f(CenterPointXAxis).cross(ofVec3f(verticalLine.direction).scale(100));

    
        geometry.clear();
        geometry.setMode(OF_PRIMITIVE_LINES);
        geometry.addColor(ofColor::blueSteel);
        geometry.addVertex(ofVec3f(0, 0, 0));
        geometry.addColor(ofColor::blueSteel);
        geometry.addVertex(frustumCenterPoint);
        geometry.addColor(ofColor::greenYellow);
        geometry.addVertex(frustumCenterPoint);
        geometry.addColor(ofColor::greenYellow);
        geometry.addVertex(frustumCenterPoint + ofVec3f(verticalLine.direction).scale(1000));
        
        geometry.addColor(ofColor::red);
        geometry.addVertex(frustumCenterPoint);
        geometry.addColor(ofColor::red);
        geometry.addVertex(frustumCenterPoint + CenterPointXAxis);
        
        geometry.addColor(ofColor::blue);
        geometry.addVertex(frustumCenterPoint);
        geometry.addColor(ofColor::blue);
        geometry.addVertex(frustumCenterPoint + planeZAxis);

        geometry.addColor(ofColor::blueViolet);
        geometry.addVertex(frustumCenterPoint);
        geometry.addColor(ofColor::blueViolet);
        geometry.addVertex(frustumCenterPoint + CenterPointRotXtoZAxis);

        
        kinectRransform_xAxisRot = frustumCenterPoint.angle(ofVec3f(CenterPointRotXtoZAxis).scale(-1.));
        kinectRransform_yAxisRot = CenterPointRotXtoZAxis.angle(planeZAxis);
        
        kinectRransform_zTranslate = - frustumCenterPoint.length();
        
        kinectRransform = ofMatrix4x4();
        kinectRransform.translate(0, 0, kinectRransform_zTranslate);
        //kinectRransform.rotate(ofQuaternion(kinectRransform_xAxisRot, ofVec3f(1, 0, 0), kinectRransform_yAxisRot, ofVec3f(0, 1, 0), 0, ofVec3f(0, 0, 1)));
        
        kinectRransform.rotate(kinectRransform_xAxisRot, 1, 0, 0);
        kinectRransform.rotate(kinectRransform_yAxisRot, 0, 1, 0);
        
        //ofLog(OF_LOG_NOTICE, "xAxisRot" + ofToString(kinectRransform_xAxisRot));
        //ofLog(OF_LOG_NOTICE, "yAxisRot" + ofToString(kinectRransform_yAxisRot));
        //ofLog(OF_LOG_NOTICE, "zTranslate: " + ofToString(kinectRransform_zTranslate));

        frustumCenterSphere.setPosition(frustumCenterPoint);
        frustumCenterSphere.setRadius(20);
        
        bUpdateCalc = false;
    }
}

//--------------------------------------------------------------
ofVec3f ofApp::calcPlanePoint(ofParameter<ofVec2f> & cpoint, int _size, int _step){
    int width = kinect.getWidth();
    int height = kinect.getHeight();
    double ref_pix_size = kinect.getZeroPlanePixelSize();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    
    int size = _size;
    int step = _step;
    float factor;
    int counter = 0;
    
    int minX = ((cpoint.get().x - size) >= 0)?(cpoint.get().x - 1): 0;
    int minY = ((cpoint.get().y - size) >= 0)?(cpoint.get().y - 1): 0;
    int maxY = ((cpoint.get().y + size) < cpoint.getMax().y)?(cpoint.get().y + size): cpoint.getMax().y - 1;
    int maxX = ((cpoint.get().x + size) < cpoint.getMax().x)?(cpoint.get().x + size): cpoint.getMax().x - 1;
    
    ofVec3f ppoint;
    
    for(int y = minY; y <= maxY; y = y + step) {
        for(int x = minX; x <= maxX; x = x + step) {
            factor = 2 * ref_pix_size * raw[y * width + x] / ref_distance;
            if(raw[y * width + x] > 0) {
                ppoint += ofVec3f((x - DEPTH_X_RES/2) *factor, (y - DEPTH_Y_RES/2) *factor, raw[y * width + x]);
                counter++;
            }
        }
    }
    ppoint /= counter;
    
    return ppoint;
    
}


//--------------------------------------------------------------
void ofApp::update(){
	
	ofBackground(100, 100, 100);
	
	kinect.update();
    
    if(bUpdateCalc && ofGetElapsedTimeMillis() > 1000){
        updateCalc();
    }
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
	
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
    
    rgbaMatrixServer.update();
	depthMatrixServer.update();
	rawMatrixServer.update();

    rgbaMatrixServer.sendFrame(kinect.getPixelsRef());
    depthMatrixServer.sendFrame(kinect.getDepthPixelsRef());
    rawMatrixServer.sendFrame(kinect.getRawDepthPixelsRef());
    
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255, 255, 255);
	
	if(bDrawPointCloud) {
		easyCam.begin();
		drawPointCloud();
		easyCam.end();
	} else {
		// draw from the live kinect
		kinect.drawDepth(10, 10, 400, 300);
		kinect.draw(420, 10, 400, 300);
		
		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
		
#ifdef USE_TWO_KINECTS
		kinect2.draw(420, 320, 400, 300);
#endif
	}
	
	// draw instructions
	ofSetColor(255, 255, 255);
	string msg = string("accel is: " + ofToString(kinect.getMksAccel().x, 2) + " / ");
	msg += ofToString(kinect.getMksAccel().y, 2) + " / ";
	msg += ofToString(kinect.getMksAccel().z, 2) + "\n";
	msg += "press p to switch between images and point cloud, rotate the point cloud with the mouse\n";
	msg += "press k to c(k)apture point cloud\n";
	msg += "press R to switch Raw Pointcloud and Normalized Pointcloud\n";
	msg += "using opencv threshold = " + ofToString(bThreshWithOpenCV) + " (press spacebar)\n";
	msg += "set near threshold " + ofToString(nearThreshold) + " (press: + -)\n";
	msg += "set far threshold " + ofToString(farThreshold) + " (press: < >) num blobs found " + ofToString(contourFinder.nBlobs) + "\n";
	msg += ", fps: " + ofToString(ofGetFrameRate()) + "\n";
	msg += "press c to close the connection and o to open it again, connection is: " + ofToString(kinect.isConnected()) + "\n";
	msg += "press UP and DOWN to change the tilt angle: " + ofToString(tiltAngle) + " degrees\n";
	msg += "press 1-5 & 0 to change the led mode (mac/linux only)\n";
    msg += "Using mouse inputs to navigate (press 'M' to toggle): " + ofToString(cam.getMouseInputEnabled() ? "YES" : "NO");
	msg += "\nShowing help (press 'h' to toggle) \n";
	if (bShowHelp) {
		msg += "\nLEFT MOUSE BUTTON DRAG:\nStart dragging INSIDE the yellow circle -> camera XY rotation .\nStart dragging OUTSIDE the yellow circle -> camera Z rotation (roll).\n";
		msg += "LEFT MOUSE BUTTON DRAG + TRANSLATION KEY (" + ofToString(cam.getTranslationKey()) + ") PRESSED\n";
		msg += "OR MIDDLE MOUSE BUTTON (if available):\n";
		msg += "move over XY axes (truck and boom).\n";
		msg += "RIGHT MOUSE BUTTON:\n";
		msg += "move over Z axis (dolly)\n";
	}
    
	if(!bDrawPointCloud) {
        ofDrawBitmapString(msg,430,320);
        
        rgbaMatrixServer.draw(10, 640);
    }
    
    gui.draw();
}

void ofApp::drawPointCloud() {
    
    mainGrid.drawPlane(1000., 5, false);
        
    int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);

    double ref_pix_size = kinect.getZeroPlanePixelSize();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    double factor = 0;
    
    int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
            factor = 2 * ref_pix_size * raw[y * w + x] / ref_distance;
 			if(raw[y * w + x] > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(ofVec3f((x - DEPTH_X_RES/2) *factor, (y - DEPTH_Y_RES/2) *factor, raw[y * w + x]));
			}
		}
	}
  
	glPointSize(3);
	ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1);
    //ofRotateX(kinectRransform_xAxisRot);
    //ofRotateY(kinectRransform_yAxisRot);
	ofMultMatrix(kinectRransform);

	glEnable(GL_DEPTH_TEST);
    
    glColor3i(100, 50, 100);
    
    sphere1.draw();
    sphere2.draw();
    sphere3.draw();
    frustumCenterSphere.draw();
    
    geometry.draw();
    
	mesh.drawVertices();

    frustum.drawWireframe();

	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    ofLog(OF_LOG_NOTICE, "exiting application...");

	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif

    tiltAngle.removeListener(this, &ofApp::setKinectTiltAngle);
    
    rgbaMatrixServer.exit();
    depthMatrixServer.exit();
    rawMatrixServer.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
			
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
            break;
            
        case 'R':
            dispRaw = !dispRaw;
            break;
            
        case 'k':
            break;
 
        case 's':
            gui.saveToFile("settings.xml");
            break;

        case 'l':
            gui.loadFromFile("settings.xml");
            break;

		case 'M':
			if(cam.getMouseInputEnabled()) cam.disableMouseInput();
			else cam.enableMouseInput();
			break;
            
		case 'h':
			bShowHelp ^=true;
			break;
            
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.open();
			break;
			
		case 'c':
			kinect.close();
			break;
			
		case '0':
            //angle.set(0);
			kinect.setLed(ofxKinect::LED_OFF);
			break;
            
		case '1':
			kinect.setLed(ofxKinect::LED_GREEN);
			break;
			
		case '2':
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;
			
		case '3':
			kinect.setLed(ofxKinect::LED_RED);
			break;
			
		case '4':
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;
			
		case '5':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
            
		case '6':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
						
		case OF_KEY_UP:
			break;
			
		case OF_KEY_DOWN:
			break;
            
		case OF_KEY_RIGHT:
            advance(mit, 1);
            if( mit == meshes.end() ) {
                mit = meshes.begin();
            }
			break;
            
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){


}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	stroke.push_back(ofPoint(x,y));
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	stroke.clear();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    //kinect.getDepthPixels();

    /**
     
    char *byteData = (char*)malloc(640 * 480);
    
    unsigned char* bytePtr = reinterpret_cast<unsigned char*>(kinect.getDepthPixels());
    //bytePtr += (640 * 240);
    
    memcpy(byteData, bytePtr, 640 * 480);
    
	string message= "@%" + ofToString(640);
	//for(unsigned int i=0; i<stroke.size(); i++){
	//	message+=ofToString(stroke[i].x)+"|"+ofToString(stroke[i].y)+"[/p]";
	//}
	
    udpConnection.Send(message.c_str(),message.length());
    for(int i = 0; i < 480; i++)
        udpConnection.Send(byteData + 640 * i,640);

     **/
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


