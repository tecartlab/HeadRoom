#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxGui.h"
#include "ofxMatrixNetworkServer.h"
#include "Planef.h"
#include "Linef.h"
#include "Grid.h"

#include <ofMatrix4x4.h>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
    
        void drawPointCloud();
    
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
        ofxMatrixNetworkServer depthMatrixServer;
        ofxMatrixNetworkServer rawMatrixServer;

        vector <string> storeText;
		
        //ofxUDPManager udpConnection;

		ofTrueTypeFont  mono;
		ofTrueTypeFont  monosm;
		vector<ofPoint> stroke;
    
        ofEasyCam cam;
        ofVboMesh mesh, meshraw;
        bool dispRaw;
        
        ofVboMesh frustum;
        
        vector<ofVboMesh> meshes;
        vector<ofVboMesh>::iterator mit;
    
    grid mainGrid;
    
        //////////
        //KINECT//
        //////////
        
        ofxKinect kinect;
    
        ofMatrix4x4 unprojection;
    
    #ifdef USE_TWO_KINECTS
        ofxKinect kinect2;
    #endif
    
    /////////////////
    //COLOR CONTOUR//
    /////////////////

        ofxCvColorImage colorImg;
        
        ofxCvGrayscaleImage grayImage; // grayscale depth image
        ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
        ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
        
        ofxCvContourFinder contourFinder;
        
        bool bThreshWithOpenCV;
        bool bDrawPointCloud;
        
        int nearThreshold;
        int farThreshold;
            
        // used for viewing the point cloud
        ofEasyCam easyCam;
        
        bool bShowHelp;
    

    ///////////////
    //CALCUALTION//
    ///////////////
    void updateCalc();
    ofVec3f calcPlanePoint(ofParameter<ofVec2f> & cpoint, int _size, int _step);
    
    bool bUpdateCalc = false;
   
    ofVec3f planePoint1;
    ofVec3f planePoint2;
    ofVec3f planePoint3;
    
    ofSpherePrimitive sphere1;
    ofSpherePrimitive sphere2;
    ofSpherePrimitive sphere3;
    
    ofSpherePrimitive frustumCenterSphere;
    ofSpherePrimitive frustumTopSphere;

    ofVec3f frustumCenterPoint;
    ofVec3f frustumTopPoint;

    ofVboMesh geometry;
    
    float kinectRransform_xAxisRot;
    float kinectRransform_yAxisRot;
    
    float kinectRransform_zTranslate;
    
    ofMatrix4x4 kinectRransform;

    
    //////////////
    //PROPERTIES//
    //////////////
    ofxPanel gui;
    
    ofParameter<ofVec2f> calibPoint1;
    ofParameter<ofVec2f> calibPoint2;
    ofParameter<ofVec2f> calibPoint3;
    
    ofParameter<int> tiltAngle;
    
    void setKinectTiltAngle(int & tiltAngle);
    

    
};

