#pragma once

#include "ofAdvCamera.h"
#include "ofEvents.h"
#include <set>

class ofAdvEasyCam : public ofAdvCamera {
public:
	ofAdvEasyCam();
	~ofAdvEasyCam();

	// TODO: this should be ofGetViewRect() eventually
	virtual void begin(ofRectangle viewport = ofGetCurrentViewport());
	void reset();

	//----------------------------------------
	// advanced functions

	void setTarget(const ofVec3f& target);
	void setTarget(ofNode& target);
	ofNode& getTarget();

	void setDistance(float distance);
	float getDistance() const;

	// drag is how quickly the camera picks up and slows down
	// it is a normalized value between 0-1
	void setDrag(float drag);
	float getDrag() const;
	
	void mouseDragged(ofMouseEventArgs& mouse);
	void mouseMoved(ofMouseEventArgs& mouse);
	void mousePressed(ofMouseEventArgs& mouse);
	void mouseReleased(ofMouseEventArgs& mouse);

    void mouseScrolled(ofMouseEventArgs& mouse);
    void mouseEntered(ofMouseEventArgs& mouse);
    void mouseExited(ofMouseEventArgs& mouse);

	// enable or disable mouse input to navigate
	void enableMouseInput();
	void disableMouseInput();
	bool getMouseInputEnabled();

    bool getMousePressed(int button);

private:
	void setDistance(float distance, bool save);

	ofNode target;
	
	float drag;
	float zoomSpeed;
	bool bMouseInputEnabled;

	ofVec3f mousePosViewPrev;
	ofVec3f mousePosScreenPrev;
	int lastFrame;
    int mouseX, mouseY;
    set<int> pressedMouseButtons;
	
	unsigned long lastTap;
	bool mousePressedPrev[2];

	bool bDistanceSet;
	float lastDistance;
	float distanceScaleVelocity;
	
	ofQuaternion rotation;
	ofVec3f translation;
};
