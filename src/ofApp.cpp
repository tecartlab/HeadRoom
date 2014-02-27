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

    gui.setup("panel");
    iMainCamera = 0;
    
    setupViewports();
    createHelp();

    nearFrustum.addListener(this, &ofApp::updateFrustumCone);
    farFrustum.addListener(this, &ofApp::updateFrustumCone);
    tiltAngle.addListener(this, &ofApp::setKinectTiltAngle);
    
    gui.add(calibPoint1.set("calib1", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint2.set("calib2", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint3.set("calib3", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(nearFrustum.set("near", 400, 200, 2000));
    gui.add(farFrustum.set("far", 4000, 2000, 6000));
    gui.add(tiltAngle.set("tilt", 0, -30, 30));
    gui.add(transformation.set("matrix rx ry tz", ofVec3f(0, 0, 0), ofVec3f(-90, -90, -6000), ofVec3f(90, 90, 6000)));
    
    gui.loadFromFile("settings.xml");
    
    updateMatrix();
    
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
    
    createFrustumCone();
    
}


//--------------------------------------------------------------
void ofApp::setupViewports(){
	//call here whenever we resize the window
 
    gui.setPosition(ofGetWidth() - 200, 20);
    //ofLog(OF_LOG_NOTICE, "ofGetWidth()" + ofToString(ofGetWidth()));

	//--
	// Define viewports
    
	float xOffset = 160; //ofGetWidth() / 3;
	float yOffset = ofGetHeight() / N_CAMERAS;
    
	viewMain.x = xOffset;
	viewMain.y = 0;
	viewMain.width = ofGetWidth() - xOffset - 200; //xOffset * 2;
	viewMain.height = ofGetHeight();
    
	for(int i = 0; i < N_CAMERAS; i++){
        
		viewGrid[i].x = 0;
		viewGrid[i].y = yOffset * i;
		viewGrid[i].width = xOffset;
		viewGrid[i].height = yOffset;
	}
    
	//
	//--
}

void ofApp::updateFrustumCone(int & value){
    if(kinect.isConnected()){
        createFrustumCone();
        ofLog(OF_LOG_NOTICE, "updateFrustumCone()" + ofToString(value));
    }
}

void ofApp::createFrustumCone(){
    double ref_pix_size = kinect.getZeroPlanePixelSize();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    
    int near = nearFrustum.get();
    int far = farFrustum.get();

    double factorNear = 2 * ref_pix_size * near / ref_distance;
    double factorFar = 2 * ref_pix_size * far / ref_distance;
    
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
}

void ofApp::setKinectTiltAngle(int & angle){
	kinect.setCameraTiltAngle(angle);
}

void ofApp::measurementCycle(){
    if(cycleCounter < N_MEASURMENT_CYCLES){
        planePoint1Meas[cycleCounter] = calcPlanePoint(calibPoint1, 0, 1);
        planePoint2Meas[cycleCounter] = calcPlanePoint(calibPoint2, 0, 1);
        planePoint3Meas[cycleCounter] = calcPlanePoint(calibPoint3, 0, 1);
        cycleCounter++;
    } else {
        planePoint1 = ofVec3f();
        planePoint2 = ofVec3f();
        planePoint3 = ofVec3f();
        for(int y = 0; y < N_MEASURMENT_CYCLES; y++){
            planePoint1 += planePoint1Meas[y];
            planePoint2 += planePoint2Meas[y];
            planePoint3 += planePoint3Meas[y];
        }
        planePoint1 /= N_MEASURMENT_CYCLES;
        planePoint2 /= N_MEASURMENT_CYCLES;
        planePoint3 /= N_MEASURMENT_CYCLES;
        bUpdateMeasurment = false;
        cycleCounter = 0;
        updateCalc();
    }
}

//--------------------------------------------------------------
void ofApp::updateCalc(){
        
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
    
    ofVec3f frustumCenterPoint = floorPlane.getIntersection(centerLine);
    
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

    float kinectRransform_xAxisRot = frustumCenterPoint.angle(ofVec3f(CenterPointRotXtoZAxis).scale(-1.));
    float kinectRransform_yAxisRot = CenterPointRotXtoZAxis.angle(planeZAxis);
    
    float kinectRransform_zTranslate = - frustumCenterPoint.length();
    
    ofMatrix4x4 kinectTransform = ofMatrix4x4();
    kinectTransform.translate(0, 0, kinectRransform_zTranslate);
    //kinectRransform.rotate(ofQuaternion(kinectRransform_xAxisRot, ofVec3f(1, 0, 0), kinectRransform_yAxisRot, ofVec3f(0, 1, 0), 0, ofVec3f(0, 0, 1)));
    
    kinectTransform.rotate(kinectRransform_xAxisRot, 1, 0, 0);
    kinectTransform.rotate(kinectRransform_yAxisRot, 0, 1, 0);
    
    ofLog(OF_LOG_NOTICE, "zpos: " + ofToString(kinectTransform.getTranslation().z));

    
    transformation.set(ofVec3f(kinectRransform_xAxisRot, kinectRransform_yAxisRot, kinectTransform.getTranslation().z));
           
    //ofLog(OF_LOG_NOTICE, "xAxisRot" + ofToString(kinectRransform_xAxisRot));
    //ofLog(OF_LOG_NOTICE, "yAxisRot" + ofToString(kinectRransform_yAxisRot));

    frustumCenterSphere.setPosition(frustumCenterPoint);
    frustumCenterSphere.setRadius(20);
    
    bUpdateCalc = false;
    
    updateMatrix();
}

//--------------------------------------------------------------
void ofApp::updateMatrix(){
    kinectRransform = ofMatrix4x4();
    
    kinectRransform.rotate(transformation.get().x, 1, 0, 0);
    kinectRransform.rotate(transformation.get().y, 0, 1, 0);

    kinectRransform.translate(0, 0, transformation.get().z);
    //kinectRransform.rotate(ofQuaternion(kinectRransform_xAxisRot, ofVec3f(1, 0, 0), kinectRransform_yAxisRot, ofVec3f(0, 1, 0), 0, ofVec3f(0, 0, 1)));
    
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
    	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
        if(bDrawPointCloud) {
            updatePointCloud();
        }
		
        if(bUpdateMeasurment){
            measurementCycle();
        }
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
	    
    rgbaMatrixServer.update();
	depthMatrixServer.update();
	rawMatrixServer.update();

    rgbaMatrixServer.sendFrame(kinect.getPixelsRef());
    depthMatrixServer.sendFrame(kinect.getDepthPixelsRef());
    rawMatrixServer.sendFrame(kinect.getRawDepthPixelsRef());
    
}

void ofApp::createHelp(){
    string help = string("accel is: " + ofToString(kinect.getMksAccel().x, 2) + " / ");
	help += ofToString(kinect.getMksAccel().y, 2) + " / ";
	help += ofToString(kinect.getMksAccel().z, 2) + "\n";
	help += "press k to update the calculation\n";
	help += "press p to start updateing the pointcloud\n";
	help += "press s to save current settings\n";
	help += "press l to load last saved settings\n";
	help += "using opencv threshold = " + ofToString(bThreshWithOpenCV) + " (press spacebar)\n";
	help += "set near threshold " + ofToString(nearThreshold) + " (press: + -)\n";
	help += "set far threshold " + ofToString(farThreshold) + " (press: < >) num blobs found " + ofToString(contourFinder.nBlobs) + "\n";
	help += ", fps: " + ofToString(ofGetFrameRate()) + "\n";
	help += "press c to close the connection and o to open it again, connection is: " + ofToString(kinect.isConnected()) + "\n";
	help += "press UP and DOWN to change the tilt angle: " + ofToString(tiltAngle) + " degrees\n";
	help += "press 0 - 5 to change the viewport\n";
    help += "Using mouse inputs to navigate (press 'M' to toggle): " + ofToString(cam.getMouseInputEnabled() ? "YES" : "NO");
	help += "\npress 'h' to show help \n";
    help += "\n....\n";
    help += "LEFT MOUSE BUTTON DRAG + TRANSLATION KEY (" + ofToString(cam.getTranslationKey()) + ") PRESSED\n";
    help += "OR MIDDLE MOUSE BUTTON (if available):\n";
    help += "move over XY axes (truck and boom).\n";
    help += "RIGHT MOUSE BUTTON:\n";
    help += "move over Z axis (dolly)\n";
}

//--------------------------------------------------------------
void ofApp::draw(){
    
	ofSetColor(255, 255, 255);

    //Draw viewport previews
    kinect.drawDepth(viewGrid[0]);
    kinect.draw(viewGrid[1]);
    grayImage.draw(viewGrid[2]);
    contourFinder.draw(viewGrid[3]);

    switch (iMainCamera) {
        case 0:
            kinect.drawDepth(viewMain);
            
            glDisable(GL_DEPTH_TEST);
            ofPushStyle();
            ofSetColor(255, 0, 0);
            ofNoFill();
            ofCircle(calibPoint1.get().x + 150, calibPoint1.get().y, 2);
            ofCircle(calibPoint2.get().x + 150, calibPoint2.get().y, 2);
            ofCircle(calibPoint3.get().x + 150, calibPoint3.get().y, 2);
            ofPopStyle();
            glEnable(GL_DEPTH_TEST);

            break;
        case 1:
            kinect.draw(viewMain);
            break;
        case 2:
            grayImage.draw(viewMain);
            break;
        case 3:
            contourFinder.draw(viewMain);
            break;
        case 4:
            easyCam.begin(viewMain);
            drawPointCloud();
            easyCam.end();
            break;
            
        default:
            break;
    }

    //Draw opengl viewport previews (pfImages dont like opengl calls before they are drawn
    if(iMainCamera != 4){ // make sure the camera is drawn only once (so the interaction with the mouse works)
        easyCam.begin(viewGrid[4]);
        drawPointCloud();
        easyCam.end();
    }

	// draw instructions
	ofSetColor(255, 255, 255);
    
    if(bShowHelp) {
        ofDrawBitmapString(help,430,320);
    }
    
	if(!bDrawPointCloud) {
        rgbaMatrixServer.draw(10, 640);
    }
    
    gui.draw();
    
    //--
	// Highlight background of selected camera
    
	glDisable(GL_DEPTH_TEST);
	ofPushStyle();
	ofSetColor(100, 0, 100, 100);
	ofRect(viewGrid[iMainCamera]);
	ofPopStyle();
	glEnable(GL_DEPTH_TEST);

}

void ofApp::updatePointCloud() {
    
    int w = 640;
	int h = 480;
    //	ofMesh mesh;
    mesh.clear();
	mesh.setMode(OF_PRIMITIVE_POINTS);
    
    double ref_pix_size = kinect.getZeroPlanePixelSize();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    double factor = 0;
    
    int minRaw = 10000;
    int maxRaw = 0;
    
    ofVec3f vertex;
    
    int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
            factor = 2 * ref_pix_size * raw[y * w + x] / ref_distance;
 			if(nearFrustum.get() < raw[y * w + x] && raw[y * w + x] < farFrustum.get()) {
                minRaw = (minRaw > raw[y * w + x])?raw[y * w + x]:minRaw;
                maxRaw = (maxRaw < raw[y * w + x])?raw[y * w + x]:maxRaw;
                vertex = ofVec3f((x - DEPTH_X_RES/2) *factor, (y - DEPTH_Y_RES/2) *factor, raw[y * w + x]);
                vertex = kinectRransform.preMult(vertex);
				//mesh.addColor(kinect.getColorAt(x,y));
				mesh.addColor(- vertex.z / 2500.);
				mesh.addVertex(vertex);
			}
		}
	}
}

void ofApp::drawPointCloud() {
    
    mainGrid.drawPlane(10., 5, false);
          
	glPointSize(3);
	ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
	ofScale(0.01, -0.01, -0.01);

    mesh.drawVertices();

	ofMultMatrix(kinectRransform);

	glEnable(GL_DEPTH_TEST);
    
    glColor3i(100, 50, 100);
    
    sphere1.draw();
    sphere2.draw();
    sphere3.draw();
    frustumCenterSphere.draw();
    
    geometry.draw();
    
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
            break;
            
        case 'k':
            bUpdateMeasurment = true;
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
			bShowHelp = !bShowHelp;
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
            iMainCamera = 0;
			kinect.setLed(ofxKinect::LED_OFF);
			break;
            
		case '1':
            iMainCamera = 0;
			kinect.setLed(ofxKinect::LED_GREEN);
			break;
			
		case '2':
            iMainCamera = 1;
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;
			
		case '3':
            iMainCamera = 2;
			kinect.setLed(ofxKinect::LED_RED);
			break;
			
		case '4':
            iMainCamera = 3;
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;
			
		case '5':
            iMainCamera = 4;
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
            
		case '6':
            iMainCamera = 5;
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
						
		case OF_KEY_UP:
			break;
			
		case OF_KEY_DOWN:
			break;
            
		case OF_KEY_RIGHT:
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
    setupViewports();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


