//
//  ProjectorWindow.cpp
//  of_elSantoGrial
//
//  Created by Martin Froehlich on 01.07.12.
//  Copyright (c) 2012 maybites.ch. All rights reserved.
//

#include <iostream>
#include "ProjectorWindow.h"

projectorWin::projectorWin() {
    rotX = ofRandom(-20, 20);
    rotY = ofRandom(-10, 10);
}

void projectorWin::setup(ofAdvCamera * _cam, ofxModel * _model, int width, int heigth) {
    cam = _cam;
    model = _model;
    //viewports
    viewPort.x = 0;
    viewPort.y = 0;
    viewPort.width = width;
    viewPort.height = heigth;
}

void projectorWin::update() {
}

void projectorWin::draw(){
    
    model->draw2d(&viewPort);

    cam->begin(viewPort);

    nodeGrid.draw();

    //nodeGrid.draw();
    
    ofPushStyle();
    ofPushMatrix();
    
    //glScalef(.1f, .1f, .1f);
    
	ofSetColor(ofColor::green);
	model->drawWireframe();
	
	glPointSize(2);
	ofSetColor(ofColor::green);
	model->drawVertices();
    
    
    ofPopStyle();
    ofPopMatrix();

    //ofNoFill();
    //ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5, 0);
    //ofRotateX(rotX);
    //ofRotateY(rotY);
    //ofBox(0, 0, 0, 100);
    cam->end();
    
}

void projectorWin::mouseMoved(int x, int y) {
    rotY = ofMap(x, 0, ofGetWidth(), -20, 20);
    rotX = ofMap(y, 0, ofGetHeight(), 60, -60);
}

void projectorWin::dragEvent(ofDragInfo dragInfo) {
    cout << "GOT SOME FILES: "<<endl;
    std::vector<string>::iterator it = dragInfo.files.begin();
    while(it != dragInfo.files.end()) {
        cout << *it << endl;
        ++it;
    }
}
