#pragma once
#include "ofMain.h"

// grid is a custom 3D
//  object that we've
//  created in this
//  example.
//
// We inherit from ofNode

class grid : public ofNode {
	public:
		void    customDraw();
        void    drawPlane(float scale, float ticks, bool labels);
        void    drawArrow(const ofVec3f& start, const ofVec3f& end, float headSize);
};