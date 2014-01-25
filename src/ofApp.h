#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxMatrixNetworkServer.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
    
        void drawPointCloud();
        void drawTCPConnection();
    
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

        void createCone(float f_left, float f_right, float f_top, float f_bottom, float f_near, float f_far);

        ofxMatrixNetworkServer rgbaMatrixServer;
        vector <string> storeText;
		
        ofxUDPManager udpConnection;

		ofTrueTypeFont  mono;
		ofTrueTypeFont  monosm;
		vector<ofPoint> stroke;
    
    
        ofEasyCam cam;
        ofVboMesh mesh, meshraw;
        bool dispRaw;
        
        ofVboMesh frustum;
        
        vector<ofVboMesh> meshes;
        vector<ofVboMesh>::iterator mit;
        
        //////////
        //KINECT//
        //////////
        
        ofxKinect kinect;
        
    #ifdef USE_TWO_KINECTS
        ofxKinect kinect2;
    #endif
        
        ofxCvColorImage colorImg;
        
        ofxCvGrayscaleImage grayImage; // grayscale depth image
        ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
        ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
        
        ofxCvContourFinder contourFinder;
        
        bool bThreshWithOpenCV;
        bool bDrawPointCloud;
        
        int nearThreshold;
        int farThreshold;
        
        int angle;
        
        // used for viewing the point cloud
        ofEasyCam easyCam;
        
        bool bShowHelp;
    
};

