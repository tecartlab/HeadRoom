//
//  ProjectorWindow.h
//  of_elSantoGrial
//
//  Created by Martin Froehlich on 01.07.12.
//  Copyright (c) 2012 Maybites.ch. All rights reserved.
//

#include "ofMain.h"
#include "ofxFensterManager.h"
#include "ofAdvCamera.h"
#include "Grid.h"
#include "Model.h"

class projectorWin: public ofxFensterListener{
public:
 	projectorWin();
    void setup(ofAdvCamera * _cam, ofxModel * _model, int width, int heigth);
    void update();
  	void draw();
    void mouseMoved(int x, int y);
    void dragEvent(ofDragInfo dragInfo);  

    //camera
    ofAdvCamera * cam;
    
    //Model
    ofxModel * model;
    
    //viewport
    ofRectangle viewPort;
    
    //grid 
    
    grid nodeGrid;

	float rotX;
	float rotY;
};
