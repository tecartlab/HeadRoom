//
//  ProjectorProperties.cpp
//  of_elSantoGrial
//
//  Created by Martin Froehlich on 30.06.12.
//  Copyright (c) 2012 Maybites. All rights reserved.
//

#include <iostream>
#include "ProjectorProperties.h"


projectorProps::projectorProps(string path, int x, int y, int width, int height){
    load(path);
    camP2.setParent(camP1);
    guiX = x;
    guiY = y;
    guiWidth = width; 
    guiHeight = height;
    resetGUI();
}

void projectorProps::load(string path){
    filepath = path;
    
    //-----------
	//the string is printed at the top of the app
	//to give the user some feedback
	message = "loading mySettings.xml";
    
	//we load our settings file
	//if it doesn't exist we can still make one
	//by hitting the 's' key
	if( XML.loadFile(filepath) ){
		message = filepath + " loaded!";
	}else{
		message = "unable to load " + filepath + ". check data/ folder";
	}
    needsUpdate = true;
}

void projectorProps::save(){
    XML.saveFile(filepath);
    message ="settings saved to xml!";
}

void projectorProps::saveas(string path){
    filepath = path;
    XML.saveFile(filepath);
    message ="settings saved to xml!";
}

projectorProps::Properties projectorProps::getProperties(){
    Properties props;
    props.distance = getDistance();
    props.focalPoint = getFocalPoint();
    props.angleAzimuth = getAngleAzimuth();
    props.anglePolar = getAnglePolar();
    props.angleRoll = getAngleRoll();
    props.angleTilt = getAngleTilt();
    props.screenWidth = getScreenWidth();
    props.screenRatio = getScreenRatio();
    props.sceneOffsetX = getSceneOffset().x;
    props.sceneOffsetY = getSceneOffset().y;
    props.sceneOffsetZ = getSceneOffset().z;
    props.frustumFar = getFrustumFar();
    props.frustumNear = getFrustumNear();
    props.frustumShiftX = getFrustumShiftX();
    props.frustumShiftY = getFrustumShiftY();
    return props;
}


void projectorProps::setProperties(Properties props){
    setDistance(props.distance);
    setFocalPoint(props.focalPoint);
    
    setAnglePolar(props.anglePolar);
    setAngleAzimuth(props.angleAzimuth);
    setAngleTilt(props.angleTilt);
    setAngleRoll(props.angleRoll);
    
    setScreenWidth(props.screenWidth);
    setScreenRatio(props.screenRatio);
    
    setFrustumShiftX(props.frustumShiftX);
    setFrustumShiftY(props.frustumShiftY);
    setFrustumNear(props.frustumNear);
    setFrustumFar(props.frustumFar);
    
    setSceneOffsetX(props.sceneOffsetX);
    setSceneOffsetY(props.sceneOffsetY);
    setSceneOffsetZ(props.sceneOffsetZ);
    
    vector<ofxUIWidget *> widgets = gui->getWidgetsOfType(OFX_UI_WIDGET_NUMBERDIALER);    
    for(int i = 0; i < widgets.size(); i++){
        if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_DISTANCE){
            ((ofxUINumberDialer *)widgets[i])->setValue(getDistance());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_FOCALPOINT){
            ((ofxUINumberDialer *)widgets[i])->setValue(getFocalPoint());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_POLARANGLE){
            ((ofxUINumberDialer *)widgets[i])->setValue(getAnglePolar());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_AZIMUTHANGLE){
            ((ofxUINumberDialer *)widgets[i])->setValue(getAngleAzimuth());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_TILTANGLE){
            ((ofxUINumberDialer *)widgets[i])->setValue(getAngleTilt());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_ROLLANGLE){
            ((ofxUINumberDialer *)widgets[i])->setValue(getAngleRoll());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_SCREENWIDTH){
            ((ofxUINumberDialer *)widgets[i])->setValue(getScreenWidth());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_SCREENRATIO){
            ((ofxUINumberDialer *)widgets[i])->setValue(getScreenRatio());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_FSHIFTX){
            ((ofxUINumberDialer *)widgets[i])->setValue(getFrustumShiftX());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_FSHIFTY){
            ((ofxUINumberDialer *)widgets[i])->setValue(getFrustumShiftY());
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_SCEENOFFSETX){
            ((ofxUINumberDialer *)widgets[i])->setValue(getSceneOffset().x);
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_SCEENOFFSETY){
            ((ofxUINumberDialer *)widgets[i])->setValue(getSceneOffset().y);
        } else if(((ofxUINumberDialer *)widgets[i])->getName() == GUI_SCEENOFFSETZ){
            ((ofxUINumberDialer *)widgets[i])->setValue(getSceneOffset().z);
        } 
    }

    needsUpdate = true;
}

void projectorProps::calculations(Properties * const props, ofNode * parent, Frustum * fr){
    ofNode * grandparent = parent->getParent();

    // update frustum
    float focalDistance = props->distance + props->focalPoint;
    float frustumWith = props->frustumNear * props->screenWidth / (focalDistance);
    float frustumHeight = frustumWith / props->screenRatio;
    
    fr->left = - props->frustumNear * props->frustumShiftX / focalDistance - frustumWith / 2;
    fr->right = - props->frustumNear * props->frustumShiftX / focalDistance + frustumWith / 2;
    fr->bottom = - props->frustumNear * props->frustumShiftY / focalDistance - frustumHeight / 2;
    fr->top = - props->frustumNear * props->frustumShiftY / focalDistance + frustumHeight / 2;
    fr->near = props->frustumNear;
    fr->far = props->frustumFar;        

    grandparent->setOrientation(ofVec3f(90.0f - props->anglePolar, props->angleRoll, props->angleAzimuth));
    grandparent->setPosition(props->sceneOffsetX, props->sceneOffsetY, props->sceneOffsetZ);
    
    parent->setOrientation(ofVec3f(atan(props->frustumShiftY/focalDistance)*180/pi, -atan(props->frustumShiftX/focalDistance)*180/pi, props->angleTilt));
    parent->setPosition(0, 0, focalDistance);

}


void projectorProps::update(ofAdvCamera * cam){
    if(needsUpdate){
        //set camera properties
        Properties props = getProperties();
        calculations(&props, &camP2, &frustum);
        
        cam->setFrustum(frustum.left, frustum.right, frustum.top, frustum.bottom, frustum.near, frustum.far);
        cam->setParent(camP2);
        needsUpdate = false;
    }
}

void projectorProps::resetGUI(){
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
    float length = 255-xInit; 

    gui = new ofxUICanvas(guiX, guiY, length+xInit, guiHeight); 
            
    gui->addWidgetDown(new ofxUILabel("PANEL 1: Projector", OFX_UI_FONT_MEDIUM)); 

    gui->addWidgetDown(new ofxUISpacer(length-xInit, 2)); 
    gui->addWidgetDown(new ofxUINumberDialer(0, 1000, getDistance(), 1, GUI_DISTANCE, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_DISTANCE, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(0, 10, getFocalPoint(), 1, GUI_FOCALPOINT, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_FOCALPOINT, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(0, 90, getAnglePolar(), 1, GUI_POLARANGLE, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_POLARANGLE, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-360, 360, getAngleAzimuth(), 1, GUI_AZIMUTHANGLE, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_AZIMUTHANGLE, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-10, 10, getAngleTilt(), 1, GUI_TILTANGLE, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_TILTANGLE, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-10, 10, getAngleRoll(), 1, GUI_ROLLANGLE, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_ROLLANGLE, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(0, 500, getScreenWidth(), 1, GUI_SCREENWIDTH, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_SCREENWIDTH, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(0, 3, getScreenRatio(), 3, GUI_SCREENRATIO, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_SCREENRATIO, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-100, 100, getFrustumShiftX(), 3, GUI_FSHIFTX, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_FSHIFTX, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-100, 100, getFrustumShiftY(), 3, GUI_FSHIFTY, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_FSHIFTY, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-500, 500, getSceneOffset().x, 2, GUI_SCEENOFFSETX, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_SCEENOFFSETX, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-500, 500, getSceneOffset().y, 2, GUI_SCEENOFFSETY, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_SCEENOFFSETY, OFX_UI_FONT_SMALL));
    gui->addWidgetDown(new ofxUINumberDialer(-500, 500, getSceneOffset().z, 2, GUI_SCEENOFFSETZ, OFX_UI_FONT_SMALL)); 
    gui->addWidgetRight(new ofxUILabel(GUI_SCEENOFFSETZ, OFX_UI_FONT_SMALL));

    
    gui->setColorBack(ofColor(255, 0, 0));
    gui->setDrawOutline(true);
                      
    ofAddListener(gui->newGUIEvent,this,&projectorProps::guiEvent);
}

//--------------------------------------------------------------
void projectorProps::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
    int id = e.widget->getParent()->getID();
	//cout << "got event from: " << name << " with id: " << id << endl; 	
	
	if(name == GUI_DISTANCE)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setDistance(dialer->getValue()); 
	}
	if(name == GUI_FOCALPOINT)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setFocalPoint(dialer->getValue()); 
	}
	if(name == GUI_POLARANGLE)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setAnglePolar(dialer->getValue()); 
	}
	if(name == GUI_AZIMUTHANGLE)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setAngleAzimuth(dialer->getValue()); 
	}
	if(name == GUI_TILTANGLE)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setAngleTilt(dialer->getValue()); 
	}
	if(name == GUI_ROLLANGLE)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setAngleRoll(dialer->getValue()); 
	}
	if(name == GUI_SCREENWIDTH)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setScreenWidth(dialer->getValue()); 
	}
	if(name == GUI_SCREENRATIO)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setScreenRatio(dialer->getValue()); 
	}
	if(name == GUI_FSHIFTX)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setFrustumShiftX(dialer->getValue()); 
	}
	if(name == GUI_FSHIFTY)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setFrustumShiftY(dialer->getValue()); 
	}
	if(name == GUI_SCEENOFFSETX)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setSceneOffsetX(dialer->getValue()); 
	}
	if(name == GUI_SCEENOFFSETY)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setSceneOffsetY(dialer->getValue()); 
	}
	if(name == GUI_SCEENOFFSETZ)
	{
		ofxUINumberDialer *dialer = (ofxUINumberDialer *) e.widget; 
		setSceneOffsetZ(dialer->getValue()); 
	}
    
    needsUpdate = true;
}

void projectorProps::draw(){
    gui->draw();
}

void projectorProps::setDistance(float distance){
    XML.setValue("PROJECTOR:DISTANCE", distance);    
}
float projectorProps::getDistance(){
    return XML.getValue("PROJECTOR:DISTANCE", 170.0f);
}


void projectorProps::setFocalPoint(float focal){
    XML.setValue("PROJECTOR:FOCALPOINT", focal);    
}
float projectorProps::getFocalPoint(){
    return XML.getValue("PROJECTOR:FOCALPOINT", 40.0f);
}


void projectorProps::setAnglePolar(float polar){
    XML.setValue("PROJECTOR:ANGLE:POLAR", polar);    
}

float projectorProps::getAnglePolar(){
    return XML.getValue("PROJECTOR:ANGLE:POLAR", 0.0f);
}

void projectorProps::setAngleAzimuth(float azimut){
    XML.setValue("PROJECTOR:ANGLE:AZIMUTH", azimut);    
}

float projectorProps::getAngleAzimuth(){
    return XML.getValue("PROJECTOR:ANGLE:AZIMUTH", 0.0f);
}

void projectorProps::setAngleTilt(float tilt){
    XML.setValue("PROJECTOR:ANGLE:TILT", tilt);    
}

float projectorProps::getAngleTilt(){
    return XML.getValue("PROJECTOR:ANGLE:TILT", 0.0f);
}

void projectorProps::setAngleRoll(float roll){
    XML.setValue("PROJECTOR:ANGLE:ROLL", roll);    
}

float projectorProps::getAngleRoll(){
    return XML.getValue("PROJECTOR:ANGLE:ROLL", 0.0f);
}


void projectorProps::setScreenWidth(float width){
    XML.setValue("PROJECTOR:SCREEN:WIDTH", width);    
}

float projectorProps::getScreenWidth(){
    return XML.getValue("PROJECTOR:SCREEN:WIDTH", 200.0f);
}

void projectorProps::setScreenRatio(float ratio){
    XML.setValue("PROJECTOR:SCREEN:RATIO", ratio);    
}

float projectorProps::getScreenRatio(){
    return XML.getValue("PROJECTOR:SCREEN:RATIO", 1.33f);
}


void projectorProps::setFrustumShiftX(float shiftX){
    XML.setValue("PROJECTOR:FRUSTUM:SHIFT:X", shiftX);    
}

float projectorProps::getFrustumShiftX(){
    return XML.getValue("PROJECTOR:FRUSTUM:SHIFT:X", 0.0f);
}

void projectorProps::setFrustumShiftY(float shiftY){
    XML.setValue("PROJECTOR:FRUSTUM:SHIFT:Y", shiftY);    
}

float projectorProps::getFrustumShiftY(){
    return XML.getValue("PROJECTOR:FRUSTUM:SHIFT:Y", 0.0f);
}

void projectorProps::setFrustumNear(float near){
    XML.setValue("PROJECTOR:FRUSTUM:NEAR", near);    
}

float projectorProps::getFrustumNear(){
    return XML.getValue("PROJECTOR:FRUSTUM:NEAR", 0.01f);
}

void projectorProps::setFrustumFar(float far){
    XML.setValue("PROJECTOR:FRUSTUM:FAR", far);    
}

float projectorProps::getFrustumFar(){
    return XML.getValue("PROJECTOR:FRUSTUM:FAR", 1000.0f);
}


void projectorProps::setSceneOffset(ofVec3f offset){
    XML.setValue("PROJECTOR:SCENE:OFFSET:X", offset.x);    
    XML.setValue("PROJECTOR:SCENE:OFFSET:Y", offset.y);    
    XML.setValue("PROJECTOR:SCENE:OFFSET:Z", offset.z);    
}

void projectorProps::setSceneOffsetX(float offsetX){
    XML.setValue("PROJECTOR:SCENE:OFFSET:X", offsetX);    
}
void projectorProps::setSceneOffsetY(float offsetY){
    XML.setValue("PROJECTOR:SCENE:OFFSET:Y", offsetY);    
}
void projectorProps::setSceneOffsetZ(float offsetZ){
    XML.setValue("PROJECTOR:SCENE:OFFSET:Z", offsetZ);    
}

ofVec3f projectorProps::getSceneOffset(){
    ofVec3f offset;
    offset.x = XML.getValue("PROJECTOR:SCENE:OFFSET:X", 0.0f);
    offset.y = XML.getValue("PROJECTOR:SCENE:OFFSET:Y", 0.0f);
    offset.z = XML.getValue("PROJECTOR:SCENE:OFFSET:Z", 0.0f);
    return offset;
}

