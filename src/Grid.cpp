#include "Grid.h"

// grid is a custom 3D
//  object that we've
//  created in this
//  example.

void grid::customDraw(){
	ofPushStyle();

	ofSetColor(255, 0, 0);
    drawArrow(ofVec3f(0, 0, 0), ofVec3f(4, 0, 0), 1);
	ofSetColor(0, 255, 0);
    drawArrow(ofVec3f(0, 0, 0), ofVec3f(0, 4, 0), 1);
	ofSetColor(0, 0, 255);
    drawArrow(ofVec3f(0, 0, 0), ofVec3f(0, 0, 4), 1);

    //give a saturation and lightness
    
	drawPlane(50.0f, 10, true);

	ofPopStyle();
}

//--------------------------------------------------------------
void grid::drawPlane(float scale, float ticks, bool labels) {
	
	float minor = scale / ticks;
	float major =  minor * 2.0f;
	
	ofPushStyle();

    ofSetColor(100, 100, 255);
    
    for (int iDimension=0; iDimension<2; iDimension++)
	{
		for (float yz=-scale; yz<=scale; yz+= minor)
		{
			//major major
			if (fabs(yz) == scale || yz == 0)
				ofSetLineWidth(2);
			
			//major
			else if (yz / major == floor(yz / major) )
				ofSetLineWidth(1.5);
			
			//minor
			else
				ofSetLineWidth(1);
			if (iDimension==0)
				ofLine(yz, -scale, 0, yz, scale, 0);
			else
				ofLine(-scale, yz, 0, scale, yz, 0);
		}
	}
	ofPopStyle();
	
	if (labels) {
		//draw numbers on axes
		ofPushStyle();
        ofSetColor(150, 150, 255);
		
		float accuracy = ceil(-log(scale/ticks)/log(10.0f));
		
		ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
		for (float yz = -scale; yz<=scale; yz+=minor)
		{
			ofDrawBitmapString(ofToString(yz, accuracy), yz, 0, 0);
			ofDrawBitmapString(ofToString(yz, accuracy), 0, yz, 0);		
		}
		ofPopStyle();
	}
	
}

//--------------------------------------------------------------
void grid::drawArrow(const ofVec3f& start, const ofVec3f& end, float headSize) {
	
	//draw line
	ofLine(start, end);
	
	//draw cone
	ofMatrix4x4 mat;
	mat.makeRotationMatrix(ofVec3f(0,0,1), end - start);
	ofPushMatrix();
	ofTranslate(end);
	glMultMatrixf(mat.getPtr());
	ofTranslate(0,0,-headSize);
	ofCone(headSize / 4, headSize);	
	ofPopMatrix();
}
