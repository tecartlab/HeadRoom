#pragma once

#include "ofMain.h"

// Ortho camera is a custom
//	camera we've created in
//  this example
//
// We inherit from ofCamera

class orthoCamera : public ofCamera {
	public:
		orthoCamera();
        void begin(ofRectangle rect = ofGetWindowRect());
        void begin(ofRectangle rect, float left, float right, float front, float back, float near, float far);
		float scale;
};