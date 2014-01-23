#include "ofApp.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running to fast
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

    //create the socket and set to send to 127.0.0.1:11999
	//udpConnection.Create();
	//udpConnection.Connect("127.0.0.1",4653);
	//udpConnection.SetNonBlocking(true);

    //setup the server to listen on 11999
	TCP.setup(11999);
	//optionally set the delimiter to something else.  The delimter in the client and the server have to be the same, default being [/TCP]
	//TCP.setMessageDelimiter("\n");


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
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
	
	// start from the front
	bDrawPointCloud = false;
    
    createCone(-.1, .1, -.1, .1, .1, 10);

}

void ofApp::createCone(float f_left, float f_right, float f_top, float f_bottom, float f_near, float f_far){
	frustum.clear();
    float coneRatio = f_far / f_near;
    //	outlet(0,"linesegment", 0, f_left, -f_near, 0, f_left * coneRatio, -f_far);
    frustum.addVertex(ofPoint(f_left, f_bottom, -f_near));
    frustum.addVertex(ofPoint(f_left * coneRatio, f_bottom * coneRatio, -f_far));
    
    //	outlet(0,"linesegment", 0, f_right, -f_near, 0, f_right * coneRatio, -f_far);
    frustum.addVertex(ofPoint(f_right, f_bottom, -f_near));
    frustum.addVertex(ofPoint(f_right * coneRatio, f_bottom * coneRatio, -f_far));
    
    //	outlet(0,"linesegment", f_top, 0, -f_near, f_top * coneRatio, 0, -f_far);
    frustum.addVertex(ofPoint(f_left, f_top, -f_near));
    frustum.addVertex(ofPoint(f_left * coneRatio, f_top * coneRatio, -f_far));
    
    //outlet(0,"linesegment", f_bottom, 0, -f_near,f_bottom * coneRatio, 0, -f_far);
    frustum.addVertex(ofPoint(f_right, f_top, -f_near));
    frustum.addVertex(ofPoint(f_right * coneRatio, f_top * coneRatio, -f_far));
}

//--------------------------------------------------------------
void ofApp::update(){
	
	ofBackground(100, 100, 100);
	
	kinect.update();
	
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
    
    //for each client lets send them a message letting them know what port they are connected on
	for(int i = 0; i < TCP.getLastID(); i++){
		if(TCP.isClientConnected(i))
            TCP.send(i, "hello client - you are connected on port - ");//+ofToString(TCP.getClientPort(i)) );
	}
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
	msg += "press UP and DOWN to change the tilt angle: " + ofToString(angle) + " degrees\n";
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
        
        drawTCPConnection();
    }
}

void ofApp::drawTCPConnection(){
    
	ofSetHexColor(0xDDDDDD);
	ofDrawBitmapString("TCP SERVER Example connect on port: "+ofToString(TCP.getPort()), 10, 700);
    
	//ofSetHexColor(0x000000);
	//ofRect(10, 60, ofGetWidth()-24, ofGetHeight() - 65 - 15);
    
	ofSetHexColor(0xDDDDDD);
    
	//for each connected client lets get the data being sent and lets print it to the screen
	for(unsigned int i = 0; i < (unsigned int)TCP.getLastID(); i++){
        
		if( !TCP.isClientConnected(i) )continue;
        
		//give each client its own color
		ofSetColor(255 - i*30, 255 - i * 20, 100 + i*40);
        
		//calculate where to draw the text
		int xPos = 15;
		int yPos = 720 + (12 * i * 4);
        
		//get the ip and port of the client
		string port = ofToString( TCP.getClientPort(i) );
		string ip   = TCP.getClientIP(i);
		string info = "client "+ofToString(i)+" -connected from "+ip+" on port: "+port;
        
        
		//if we don't have a string allocated yet
		//lets create one
		if(i >= storeText.size() ){
			storeText.push_back( string() );
		}
        
		//we only want to update the text we have recieved there is data
		string str = TCP.receive(i);
        
		if(str.length() > 0){
			storeText[i] = str;
		}
        
		//draw the info text and the received text bellow it
		ofDrawBitmapString(info, xPos, yPos);
		ofDrawBitmapString(storeText[i], 25, yPos + 10);
        
	}
    
}

void ofApp::drawPointCloud() {
    
	frustum.drawWireframe();
    
    int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1);
	ofTranslate(0, 0, 0); // center the points a bit
	glEnable(GL_DEPTH_TEST);
	mesh.drawVertices();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
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
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
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
			
		case '0':
			kinect.setLed(ofxKinect::LED_OFF);
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
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
