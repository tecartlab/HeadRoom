#include "OrthoCamera.h"

// Ortho camera is a custom
//	camera we've created in
//  this example
//
// We inherit from ofCamera

orthoCamera::orthoCamera(){
	enableOrtho();
	scale = 1;
}

void orthoCamera::begin(ofRectangle rect){
	ofCamera::begin(rect);

	//--
	// Calculate aspect ratio

	float vw = ofGetViewportWidth();
	float vh = ofGetViewportHeight();

	//aspect ratio
	float ar = vw / vh;

	float scalex, scaley;

	if(ar > 1.0f){
		//wide
		scalex = scale * ar;
		scaley = scale;
	}
	else{
		//tall
		scalex = scale;
		scaley = scale / ar;
	}

	//
	//--



	//--
	// Setup projection

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-scalex, scalex, -scaley, scaley, -20 * scale, 20 * scale);
	glMatrixMode(GL_MODELVIEW);

	//
	//--
}

void orthoCamera::begin(ofRectangle rect, float left, float right, float front, float back, float near, float far){
	ofCamera::begin(rect);
    
	//--
	// Setup projection
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left * scale, right * scale, front * scale, back * scale, near * scale, far * scale);
	glMatrixMode(GL_MODELVIEW);
    
	//
	//--
}
