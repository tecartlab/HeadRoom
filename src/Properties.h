#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

#define NUM_PTS 800

class Properties{

	public:

        void load(string propertyFileName);
        void save();
    
		ofxXmlSettings XML;

        string filepath;

		int pointCount;
		int lineCount;
		int lastTagNumber;

		float red;
		float green;
		float blue;
};

