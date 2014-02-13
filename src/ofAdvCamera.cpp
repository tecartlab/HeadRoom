/*
 *  ofCamera.cpp
 *  openFrameworksLib
 *
 *  Created by Memo Akten on 10/01/2011.
 *  Copyright 2011 MSA Visuals Ltd. All rights reserved.
 *
 */

#include "ofAdvCamera.h"
#include "ofLog.h"


ofAdvCamera::ofAdvCamera():ofCamera()
{
    useFrustum = false;
}

//----------------------------------------
void ofAdvCamera::setFov(float f) {
	fov = f;
}

float ofAdvCamera::getFov(){
	return fov;
}

//----------------------------------------
void ofAdvCamera::setNearClip(float f) {
	nearClip = f;
}

//----------------------------------------
void ofAdvCamera::setFarClip(float f) {
	farClip = f;
}

//----------------------------------------
void ofAdvCamera::setupPerspective(bool vFlip, float fov, float nearDist, float farDist){
	float viewW = ofGetViewportWidth();
	float viewH = ofGetViewportHeight();

	float eyeX = viewW / 2;
	float eyeY = viewH / 2;
	float halfFov = PI * fov / 360;
	float theTan = tanf(halfFov);
	float dist = eyeY / theTan;

	if(nearDist == 0) nearDist = dist / 10.0f;
	if(farDist == 0) farDist = dist * 10.0f;

	setFov(fov);
	setNearClip(nearDist);
	setFarClip(farDist);

	setPosition(eyeX,eyeY,dist);
	lookAt(ofVec3f(eyeX,eyeY,0),ofVec3f(0,1,0));


	if(vFlip){
		setScale(1,-1,1);
	}
    useFrustum = false;
}

void ofAdvCamera::setFrustum(float left, float right, float top, float bottom, float nearDist, float farDist){
    leftF = left;
    rightF = right;
    topF = top;
    bottomF = bottom;
    
    setNearClip(nearDist);
    setFarClip(farDist);
    
    useFrustum = true;
}

void ofAdvCamera::setupFrustum(float left, float right, float top, float bottom, float nearDist, float farDist){
    float viewW = ofGetViewportWidth();
    float viewH = ofGetViewportHeight();
    float eyeX = viewW / 2;
    float eyeY = viewH / 2;
    float dist = -2.0f;

    if(nearDist == 0) nearDist = dist / 10.0f;
    if(farDist == 0) farDist = dist * 10.0f;

    setFrustum(left, right, top, bottom, nearDist, farDist);
    
    setPosition(eyeX,eyeY,dist);
    lookAt(ofVec3f(eyeX,eyeY,0),ofVec3f(0,1,0));

}


//----------------------------------------
void ofAdvCamera::enableOrtho() {
	isOrtho = true;
}

//----------------------------------------
void ofAdvCamera::disableOrtho() {
	isOrtho = false;
}

//----------------------------------------
bool ofAdvCamera::getOrtho() const {
	return isOrtho;
}

//----------------------------------------
float ofAdvCamera::getImagePlaneDistance(ofRectangle viewport) const {
	return viewport.height / (2.0f * tanf(PI * fov / 360.0f));
}

//----------------------------------------
void ofAdvCamera::begin(ofRectangle viewport) {
	if(!isActive) ofPushView();
	isActive = true;

	ofSetCoordHandedness(OF_RIGHT_HANDED);

	// autocalculate near/far clip planes if not set by user
	calcClipPlanes(viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(isOrtho) {
		//			if(vFlip) glOrtho(0, width, height, 0, nearDist, farDist);
		//			else
#ifndef TARGET_OPENGLES
		glOrtho(0, viewport.width, 0, viewport.height, nearClip, farClip);
#else
		ofMatrix4x4 ortho;
		ortho.makeOrthoMatrix(0, viewport.width, 0, viewport.height, nearClip, farClip);
		glLoadMatrixf(ortho.getPtr());
#endif
	} else {
        if (useFrustum) {
            glFrustum(leftF, rightF, bottomF, topF, nearClip, farClip);
        }else{
            gluPerspective(fov, viewport.width/viewport.height, nearClip, farClip);
        }
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(ofMatrix4x4::getInverseOf(getGlobalTransformMatrix()).getPtr());
	ofViewport(viewport);
}

// if begin(); pushes first, then we need an end to pop
//----------------------------------------
void ofAdvCamera::end() {
	if (isActive)
	{
		ofPopView();
		isActive = false;
	}
}
//----------------------------------------
ofMatrix4x4 ofAdvCamera::getProjectionMatrix(ofRectangle viewport) {
	ofMatrix4x4 matProjection;
    if (useFrustum) {
        matProjection.makeFrustumMatrix(leftF, rightF, bottomF, topF, nearClip, farClip);
    }else{
        matProjection.makePerspectiveMatrix(fov, viewport.width/viewport.height, nearClip, farClip);
    }
	return matProjection;
}
//----------------------------------------
ofMatrix4x4 ofAdvCamera::getModelViewMatrix() {
	ofMatrix4x4 matModelView;
	matModelView.makeInvertOf(getGlobalTransformMatrix());
	return matModelView;
}
//----------------------------------------
ofMatrix4x4 ofAdvCamera::getModelViewProjectionMatrix(ofRectangle viewport) {
	return getModelViewMatrix() * getProjectionMatrix(viewport);
}
//----------------------------------------
ofVec3f ofAdvCamera::worldToScreen(ofVec3f WorldXYZ, ofRectangle viewport) {

	ofVec3f CameraXYZ = WorldXYZ * getModelViewProjectionMatrix();
	ofVec3f ScreenXYZ;

	ScreenXYZ.x = (CameraXYZ.x + 1.0f) / 2.0f * viewport.width + viewport.x;
	ScreenXYZ.y = (1.0f - CameraXYZ.y) / 2.0f * viewport.height + viewport.y;

	ScreenXYZ.z = CameraXYZ.z;

	return ScreenXYZ;

}
//----------------------------------------
ofVec3f ofAdvCamera::screenToWorld(ofVec3f ScreenXYZ, ofRectangle viewport) {

	//convert from screen to camera
	ofVec3f CameraXYZ;
	CameraXYZ.x = 2.0f * (ScreenXYZ.x - viewport.x) / viewport.width - 1.0f;
	CameraXYZ.y = 1.0f - 2.0f *(ScreenXYZ.y - viewport.y) / viewport.height;
	CameraXYZ.z = ScreenXYZ.z;

	//get inverse camera matrix
	ofMatrix4x4 inverseCamera;
	inverseCamera.makeInvertOf(getModelViewProjectionMatrix(viewport));

	//convert camera to world
	return CameraXYZ * inverseCamera;

}
//----------------------------------------
ofVec3f ofAdvCamera::worldToCamera(ofVec3f WorldXYZ, ofRectangle viewport) {
	return WorldXYZ * getModelViewProjectionMatrix(viewport);
}
//----------------------------------------
ofVec3f ofAdvCamera::cameraToWorld(ofVec3f CameraXYZ, ofRectangle viewport) {

	ofMatrix4x4 inverseCamera;
	inverseCamera.makeInvertOf(getModelViewProjectionMatrix(viewport));

	return CameraXYZ * inverseCamera;
}
