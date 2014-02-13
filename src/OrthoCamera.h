#pragma once

#include "ofMain.h"
#include "ofAdvCamera.h"

// Ortho camera is a custom
//	camera we've created in
//  this example
//
// We inherit from ofCamera

class orthoCamera : public ofAdvCamera {
	public:
		orthoCamera();
		void begin(ofRectangle rect = ofGetWindowRect());
		float scale;
};