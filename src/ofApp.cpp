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
    
    blobFinder.setup();

    gui.setup("Kinect Panel");
    iMainCamera = 0;
    
    setupViewports();
    createHelp();
    
    nearFrustum.addListener(this, &ofApp::updateFrustumCone);
    farFrustum.addListener(this, &ofApp::updateFrustumCone);

    setupGuiGroup.setName("Setup on starup");
    setupGuiGroup.add(kinectServerID.set("serverID", 0, 0, 20));
    setupGuiGroup.add(captureVideo.set("use video", true));
    gui.add(setupGuiGroup);
    
    gui.add(calibPoint1.set("calibA", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint2.set("calibB", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    gui.add(calibPoint3.set("calibC", ofVec2f(320, 240), ofVec2f(0, 0), ofVec2f(640, 480)));
    
    frustumGuiGroup.setName("frustumField");
    frustumGuiGroup.add(nearFrustum.set("nearFrustum", 400, 200, 2000));
    frustumGuiGroup.add(farFrustum.set("farFrustum", 4000, 2000, 6000));
    gui.add(frustumGuiGroup);

    intrinsicGuiGroup.setName("Corrections");
    intrinsicGuiGroup.add(depthCorrectionBase.set("base", 1.0, 0.9, 1.1));
    intrinsicGuiGroup.add(depthCorrectionDivisor.set("divisor", 100000, 90000, 110000));
    intrinsicGuiGroup.add(pixelSizeCorrector.set("pixl factor", 1.0, 0.9, 1.1));
    gui.add(intrinsicGuiGroup);
    
    gui.add(transformation.set("matrix rx ry tz", ofVec3f(0, 0, 0), ofVec3f(-90, -90, -6000), ofVec3f(90, 90, 6000)));
 
    gui.loadFromFile("settings.xml");
    
    updateMatrix();
    
    //////////
    //KINECT//
    //////////
    
    ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
    if(captureVideo.get())
        kinect.init();
    else
        kinect.init(false, false);// disable video image (faster fps)
    
	//kinect.init(true); // shows infrared instead of RGB video image
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
        
    ofFbo::Settings s;
    s.width             = 640;
    s.height			= 480;
    s.internalformat    = GL_RGB;
    s.useDepth			= true;
    // and assigning this values to the fbo like this:
    captureFBO.allocate(s);

    blobFinder.allocate();
		
	// creating preview point cloud is bogging the system down, so switched off at startup
	bPreviewPointCloud = false;
    
    networkMng.setup(NETWORK_LISTENING_PORT, NETWORK_BROADCAST_PORT, kinect.getSerial(), kinectServerID.get());
    
    int * val;
    updateFrustumCone(*val);
    
}


//--------------------------------------------------------------
void ofApp::setupViewports(){
	//call here whenever we resize the window
 
    gui.setPosition(ofGetWidth() - MENU_WIDTH, 20);
    blobFinder.gui.setPosition(ofGetWidth() - MENU_WIDTH / 2, 20);
    //ofLog(OF_LOG_NOTICE, "ofGetWidth()" + ofToString(ofGetWidth()));

	//--
	// Define viewports
    
	float xOffset = VIEWGRID_WIDTH; //ofGetWidth() / 3;
	float yOffset = VIEWPORT_HEIGHT / N_CAMERAS;
    
	viewMain.x = xOffset;
	viewMain.y = 0;
	viewMain.width = ofGetWidth() - xOffset - MENU_WIDTH; //xOffset * 2;
	viewMain.height = VIEWPORT_HEIGHT;
    
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
        double ref_pix_size = kinect.getZeroPlanePixelSize();
        double ref_distance = kinect.getZeroPlaneDistance();
        ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
        
        kinectFrustum.near = nearFrustum.get();
        kinectFrustum.far = farFrustum.get();
        
        double factorNear = 2 * ref_pix_size * kinectFrustum.near / ref_distance;
        double factorFar = 2 * ref_pix_size * kinectFrustum.far / ref_distance;
        
        //ofVec3f((x - DEPTH_X_RES/2) *factor, (y - DEPTH_Y_RES/2) *factor, raw[y * w + x]));
        
        kinectFrustum.left = (0 - DEPTH_X_RES/2) *factorNear;
        kinectFrustum.right = (640 - DEPTH_X_RES/2) *factorNear;
        kinectFrustum.top = (0 - DEPTH_Y_RES/2) *factorNear;
        kinectFrustum.bottom = (480 - DEPTH_Y_RES/2) *factorNear;
        
        kinectFrustum.leftFar = (0 - DEPTH_X_RES/2) *factorFar;
        kinectFrustum.rightFar = (640 - DEPTH_X_RES/2) *factorFar;
        kinectFrustum.topFar = (0 - DEPTH_Y_RES/2) *factorFar;
        kinectFrustum.bottomFar = (480 - DEPTH_Y_RES/2) *factorFar;
        
        kinectFrustum.update();
        //createFrustumCone();
    }
}

void ofApp::measurementCycleRaw(){
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
        bUpdateMeasurmentFine = true;
        cycleCounter = 0;
    }
}

void ofApp::measurementCycleFine(){
    if(cycleCounter < N_MEASURMENT_CYCLES){
        ofVec3f p1meas = calcPlanePoint(calibPoint1, 0, 1);
        ofVec3f p2meas = calcPlanePoint(calibPoint2, 0, 1);
        ofVec3f p3meas = calcPlanePoint(calibPoint3, 0, 1);
        if(planePoint1.z / 1.05 < p1meas.z &&
           p1meas.z < planePoint1.z * 1.05 &&
           planePoint2.z / 1.05 < p2meas.z &&
           p2meas.z < planePoint2.z * 1.05 &&
           planePoint3.z / 1.05 < p3meas.z &&
           p3meas.z < planePoint3.z * 1.05){
            planePoint1Meas[cycleCounter] = p1meas;
            planePoint2Meas[cycleCounter] = p2meas;
            planePoint3Meas[cycleCounter] = p3meas;
            cycleCounter++;
        }
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
        bUpdateMeasurmentFine = false;
        cycleCounter = 0;
        updateCalc();
    }
}

//--------------------------------------------------------------
void ofApp::updateCalc(){
        
    sphere1.setPosition(planePoint1);
    sphere2.setPosition(planePoint2);
    sphere3.setPosition(planePoint3);

    sphere1.setRadius(25);
    sphere2.setRadius(25);
    sphere3.setRadius(25);

    
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
    float kinectRransform_yAxisRot = -CenterPointRotXtoZAxis.angle(planeZAxis);
    
    float kinectRransform_zTranslate = frustumCenterPoint.length();
    
    ofMatrix4x4 zTranMatrix = ofMatrix4x4();
    zTranMatrix.translate(0, 0, kinectRransform_zTranslate);
    zTranMatrix.rotate(kinectRransform_xAxisRot, 1, 0, 0);
    zTranMatrix.rotate(kinectRransform_yAxisRot, 0, 1, 0);
    
    transformation.set(ofVec3f(kinectRransform_xAxisRot, kinectRransform_yAxisRot, zTranMatrix.getTranslation().z));

    //ofLog(OF_LOG_NOTICE, "zpos: " + ofToString(kinectTransform.getTranslation().z));

    ofMatrix4x4 centerMatrix = ofMatrix4x4();
    centerMatrix.rotate(kinectRransform_xAxisRot, 1, 0, 0);
    centerMatrix.rotate(kinectRransform_yAxisRot, 0, 1, 0);
    centerMatrix.translate(0, 0, zTranMatrix.getTranslation().z);

    planeCenterPoint = centerMatrix.preMult(frustumCenterPoint);
    //planeCenterPoint.rotate(kinectRransform_xAxisRot, ofVec3f(1, 0, 0));

    calcdata = string("distance to plane center point: " + ofToString(frustumCenterPoint.length()) + "\n");
    calcdata += "distance to A: " + ofToString(planePoint1.length()) + "\n";
    calcdata += "distance to B: " + ofToString(planePoint2.length()) + "\n";
    calcdata += "distance to C: " + ofToString(planePoint3.length()) + "\n";
    calcdata += "distance A to B: " + ofToString(ofVec3f(planePoint1 - planePoint2).length()) + "\n";
    calcdata += "reference pixel size: " + ofToString(kinect.getZeroPlanePixelSize()) + "\n";
    calcdata += "reference distance: " + ofToString(kinect.getZeroPlaneDistance()) + "\n";


    //ofLog(OF_LOG_NOTICE, "planeCenterPoint.x" + ofToString(planeCenterPoint.x));
    //ofLog(OF_LOG_NOTICE, "planeCenterPoint.y" + ofToString(planeCenterPoint.y));
    //ofLog(OF_LOG_NOTICE, "planeCenterPoint.z" + ofToString(planeCenterPoint.z));
    //ofLog(OF_LOG_NOTICE, "yAxisRot" + ofToString(kinectRransform_yAxisRot));

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
    
    blobFinder.kinectPos = ofVec3f(0, 0, transformation.get().z);    
}

//--------------------------------------------------------------
ofVec3f ofApp::calcPlanePoint(ofParameter<ofVec2f> & cpoint, int _size, int _step){
    int width = kinect.getWidth();
    int height = kinect.getHeight();
    double ref_pix_size = 2 * kinect.getZeroPlanePixelSize() * pixelSizeCorrector.get();
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
    
    float corrDistance;
    
    for(int y = minY; y <= maxY; y = y + step) {
        for(int x = minX; x <= maxX; x = x + step) {
            corrDistance = (float)raw[y * width + x] * (depthCorrectionBase.get() + (float)raw[y * width + x] / depthCorrectionDivisor.get());
            factor = ref_pix_size * corrDistance / ref_distance;
            if(raw[y * width + x] > 0) {
                ppoint += ofVec3f((x - DEPTH_X_RES/2) *factor, -(y - DEPTH_Y_RES/2) *factor, -corrDistance);
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
        if(bUpdateMeasurment){
            measurementCycleRaw();
        }
        if(bUpdateMeasurmentFine){
            measurementCycleFine();
        }

        updatePointCloud(capturemesh, 1, true, false);
        if(bPreviewPointCloud) {
            updatePointCloud(previewmesh, 2, false, true);
        }
    
        // Cature captureCloud to FBO
        //////////////////////////////////
        
        captureFBO.begin();
        ofClear(0, 0, 0, 0);
        captureCam.scale = 0.01;
        // FBO capturing 
        captureCam.begin(ofRectangle(0, 0, 640, 480), blobFinder.sensorBoxLeft.get(), blobFinder.sensorBoxRight.get(), blobFinder.sensorBoxBack.get(), blobFinder.sensorBoxFront.get(), - blobFinder.sensorBoxTop.get(), blobFinder.sensorBoxTop.get());
        drawCapturePointCloud();
        captureCam.end();
        captureFBO.end();
        
        //////////////////////////////////
        
        // BlobFinding on the captured FBO
        /////////////////////////////////////
        blobFinder.update(captureFBO);
	}
    
    networkMng.update(blobFinder, kinectFrustum, transformation.get());
    
    rgbaMatrixServer.update();
	depthMatrixServer.update();
	rawMatrixServer.update();

    rgbaMatrixServer.sendFrame(kinect.getPixelsRef());
    depthMatrixServer.sendFrame(kinect.getDepthPixelsRef());
    rawMatrixServer.sendFrame(kinect.getRawDepthPixelsRef());
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
	ofSetColor(255, 255, 255);

    if(bShowVisuals){
        //Draw viewport previews
        kinect.drawDepth(viewGrid[0]);
        kinect.draw(viewGrid[1]);
        captureFBO.draw(viewGrid[2]);
        blobFinder.contourFinder.draw(viewGrid[3]);
        blobFinder.contourEyeFinder.draw(viewGrid[4]);
        
        switch (iMainCamera) {
            case 0:
                kinect.drawDepth(viewMain);
                drawCalibrationPoints();
                break;
            case 1:
                kinect.draw(viewMain);
                drawCalibrationPoints();
                break;
            case 2:
                captureFBO.draw(viewMain);
                break;
            case 3:
                ofSetColor(255, 0, 0, 255);
                blobFinder.contourFinder.draw(viewMain);

                ofNoFill();
                ofSetColor(255, 0, 255, 255);
                blobFinder.drawBodyBlobs2d(viewMain);
                
               break;
            case 4:
                blobFinder.contourEyeFinder.draw(viewMain);

                ofNoFill();
                ofSetColor(255, 0, 255, 255);
                blobFinder.drawBodyBlobs2d(viewMain);
                break;
            case 5:
                previewCam.begin(viewMain);
                mainGrid.drawPlane(50., 5, false);
                drawPreview();
                previewCam.end();
                break;
                
            default:
                break;
        }
        
        //Draw opengl viewport previews (ofImages dont like opengl calls before they are drawn
        if(iMainCamera != 5){ // make sure the camera is drawn only once (so the interaction with the mouse works)
            previewCam.begin(viewGrid[5]);
            mainGrid.drawPlane(50., 5, false);
            drawPreview();
            previewCam.end();
        }
        
        
        gui.draw();
        blobFinder.gui.draw();

        glDisable(GL_DEPTH_TEST);
        ofPushStyle();
        // Highlight background of selected camera
        ofSetColor(255, 0, 255, 255);
        ofNoFill();
        ofSetLineWidth(3);
        ofRect(viewGrid[iMainCamera]);
    } else {
        blobFinder.contourEyeFinder.draw(viewMain);

        ofNoFill();
        ofSetColor(255, 0, 255, 255);
        blobFinder.drawBodyBlobs2d(viewMain);
    }

    //--
    

	// draw instructions
	ofSetColor(255, 255, 255);
    
    if(bShowHelp) {
        ofDrawBitmapString(help, 20 ,VIEWPORT_HEIGHT + 20);
        //ofLog(OF_LOG_NOTICE, "help printed");
    }else if(bShowCalcData){
        ofDrawBitmapString(calcdata, 20 ,VIEWPORT_HEIGHT + 20);
    }
    
	if (false) {
        rgbaMatrixServer.draw(10, 640);
    }

    ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), ofGetWidth() - 200, 10);
    

	ofPopStyle();
	glEnable(GL_DEPTH_TEST);

}

void ofApp::updatePointCloud(ofVboMesh & mesh, int step, bool useFrustumCone, bool useVideoColor) {
    
    int w = 640;
	int h = 480;
    //	ofMesh mesh;
    mesh.clear();
	mesh.setMode(OF_PRIMITIVE_POINTS);
    
    double ref_pix_size = 2 * kinect.getZeroPlanePixelSize() * pixelSizeCorrector.get();
    double ref_distance = kinect.getZeroPlaneDistance();
    ofShortPixelsRef raw = kinect.getRawDepthPixelsRef();
    double factor = 0;
    double corrDistance;
    
    int minRaw = 10000;
    int maxRaw = 0;
    
    ofVec3f vertex;
    
    float sensorFieldFront = blobFinder.sensorBoxFront.get();
    float sensorFieldBack = blobFinder.sensorBoxBack.get();
    float sensorFieldLeft = blobFinder.sensorBoxLeft.get();
    float sensorFieldRight = blobFinder.sensorBoxRight.get();
    float sensorFieldTop = blobFinder.sensorBoxTop .get();
    float sensorFieldBottom = blobFinder.sensorBoxBottom.get();
    
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
            vertex.z = 0;
            corrDistance = (float)raw[y * w + x] * (depthCorrectionBase.get() + (float)raw[y * w + x] / depthCorrectionDivisor.get());
            factor = ref_pix_size * corrDistance / ref_distance;
            if(useFrustumCone){
                if(nearFrustum.get() < corrDistance && corrDistance < farFrustum.get()) {
                    vertex = ofVec3f((x - DEPTH_X_RES/2) *factor, -(y - DEPTH_Y_RES/2) *factor, -corrDistance);
                    vertex = kinectRransform.preMult(vertex);
                }
            } else {
                vertex = ofVec3f((x - DEPTH_X_RES/2) *factor, -(y - DEPTH_Y_RES/2) *factor, -corrDistance);
                vertex = kinectRransform.preMult(vertex);
            }
            if(vertex.z != 0){
                if(sensorFieldLeft < vertex.x && vertex.x < sensorFieldRight &&
                   sensorFieldFront < vertex.y && vertex.y < sensorFieldBack &&
                   sensorFieldBottom < vertex.z && vertex.z < sensorFieldTop){
                    mesh.addColor((vertex.z - sensorFieldBottom) / (sensorFieldTop - sensorFieldBottom));
                } else {
                    if(useVideoColor)
                        mesh.addColor(kinect.getColorAt(x,y));
                    else
                        mesh.addColor(ofColor::black);
                }
                mesh.addVertex(vertex);
			}
		}
	}
}

void ofApp::drawPreview() {
	glPointSize(4);
	ofPushMatrix();

	ofScale(0.01, 0.01, 0.01);
    //This moves the crossingpoint of the kinect center line and the plane to the center of the stage
    ofTranslate(-planeCenterPoint.x, -planeCenterPoint.y, 0);
    if(bPreviewPointCloud)
        previewmesh.drawVertices();

    ofSetColor(255, 255, 0);
    blobFinder.sensorBox.draw();
    
    ofNoFill();
    ofSetColor(255, 100, 255);
    blobFinder.drawBodyBlobsBox();
    blobFinder.drawBodyBlobsHeadTop();
    ofSetColor(100, 100, 255);
    blobFinder.drawHeadBlobs();
    ofSetColor(0, 0, 255);
    blobFinder.drawEyeCenters();

    ofFill();
    ofSetColor(255, 100, 100);
    blobFinder.drawGazePoint();

	glEnable(GL_DEPTH_TEST);
    
    ofMultMatrix(kinectRransform);

    ofFill();

    ofSetColor(255, 0, 0);
    sphere1.draw();
    sphere2.draw();
    sphere3.draw();
    frustumCenterSphere.draw();
    
    geometry.draw();

    ofSetColor(0, 0, 255);
    kinectFrustum.drawWireframe();
    

	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
    
}

void ofApp::drawCapturePointCloud() {
	glPointSize(3);
	ofPushMatrix();
	ofScale(0.01, 0.01, 0.01);
    capturemesh.drawVertices();
	ofPopMatrix();    
}

void ofApp::drawCalibrationPoints(){
    glDisable(GL_DEPTH_TEST);
    ofPushStyle();
    ofSetColor(255, 0, 0);
    ofNoFill();
    ofDrawBitmapString("A", calibPoint1.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH + 5, calibPoint1.get().y -5);
    ofDrawBitmapString("B", calibPoint2.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH + 5, calibPoint2.get().y -5);
    ofDrawBitmapString("C", calibPoint3.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH + 5, calibPoint3.get().y -5);
    ofCircle(calibPoint1.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH, calibPoint1.get().y, 2);
    ofCircle(calibPoint2.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH, calibPoint2.get().y, 2);
    ofCircle(calibPoint3.get().x/KINECT_IMG_WIDTH*viewMain.width + VIEWGRID_WIDTH, calibPoint3.get().y, 2);
    ofPopStyle();
    glEnable(GL_DEPTH_TEST);
}

//--------------------------------------------------------------
void ofApp::exit() {
    ofLog(OF_LOG_NOTICE, "exiting application...");

	kinect.close();
	
    rgbaMatrixServer.exit();
    depthMatrixServer.exit();
    rawMatrixServer.exit();
}

void ofApp::createHelp(){
    help = string("press k -> to update the calculation\n");
    help += "press K -> to show the calculation results \n";
	help += "press v -> to show visualizations\n";
	help += "press p -> to show pointcloud\n";
	help += "press s -> to save current settings.\n";
	help += "press l -> to load last saved settings\n";
	help += "press 1 - 6 -> to change the viewport\n";
	help += "press A|B|C + mouse-release -> to change the calibration points in viewport 1\n";
    
	help += "press c -> to close the connection, connection is: " + ofToString(kinect.isConnected()) + "\n";
	help += "press o -> to open the connection it again\n";
    help += "ATTENTION: Setup-Settings (ServerID and Video) will only apply after restart\n";
 	help += "Broadcasting ip: "+networkMng.broadcastAddress+" port: "+ofToString(networkMng.broadcastSendPort)+" serverID: "+ofToString(networkMng.kinectID)+" \n";
 	help += "Correction Distance Math -> corrected distance = distance * (Base + distance / Divisor)\n";
	help += "Correction pixel Site    -> corrected pixel size = pixel size * Factor\n";
    /*
     help += "using opencv threshold = " + ofToString(bThreshWithOpenCV) + " (press spacebar)\n";
     help += "set near threshold " + ofToString(nearThreshold) + " (press: + -)\n";
     help += "set far threshold " + ofToString(farThreshold) + " (press: < >) num blobs found " + ofToString(contourFinder.nBlobs) + "\n";
     */
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		case ' ':
			break;
			
		case'p':
			bPreviewPointCloud = !bPreviewPointCloud;
            break;
            
		case'v':
			bShowVisuals = !bShowVisuals;
            break;
            
		case 'o':
			kinect.open();
			break;
			
		case 'c':
			kinect.close();
			break;
            
        case 'K':
            bShowCalcData = !bShowCalcData;
            break;
            
        case 'k':
            bUpdateMeasurment = true;
            break;
 
        case 's':
            gui.saveToFile("settings.xml");
            blobFinder.gui.saveToFile("trackings.xml");
            break;

        case 'l':
            gui.loadFromFile("settings.xml");
            blobFinder.gui.loadFromFile("trackings.xml");
            break;

		case 'm':
			if(cam.getMouseInputEnabled()) cam.disableMouseInput();
			else cam.enableMouseInput();
			break;
            
		case 'h':
			bShowHelp = !bShowHelp;
            if (bShowHelp) {
                createHelp();
            }
			break;
            
		case '>':
		case '.':
			//farThreshold ++;
			//if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			//farThreshold --;
			//if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			//nearThreshold ++;
			//if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			//nearThreshold --;
			//if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
						
		case '0':
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
    if(iMainCamera == 0 || iMainCamera == 1) {
        if(ofGetKeyPressed('A')){
            int posX = (x - VIEWGRID_WIDTH) / viewMain.width * KINECT_IMG_WIDTH;
            int posY = y;
            if(0 <= posX && posX < KINECT_IMG_WIDTH &&
               0 <= posY && posY < KINECT_IMG_HEIGHT)
                calibPoint1.set(ofVec3f(posX, posY));
        }else if(ofGetKeyPressed('B')){
            int posX = (x - VIEWGRID_WIDTH) / viewMain.width * KINECT_IMG_WIDTH;
            int posY = y;
            if(0 <= posX && posX < KINECT_IMG_WIDTH &&
               0 <= posY && posY < KINECT_IMG_HEIGHT)
                calibPoint2.set(ofVec3f(posX, posY));
        }else if(ofGetKeyPressed('C')){
            int posX = (x - VIEWGRID_WIDTH) / viewMain.width * KINECT_IMG_WIDTH;
            int posY = y;
            if(0 <= posX && posX < KINECT_IMG_WIDTH &&
               0 <= posY && posY < KINECT_IMG_HEIGHT)
                calibPoint3.set(ofVec3f(posX, posY));
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
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


