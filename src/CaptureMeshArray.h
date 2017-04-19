//
//  CaptureMeshArray.hpp
//  HeadRoom
//
//  Created by maybites on 04/04/17.
//
//

#include "ofMain.h"

#ifndef CaptureMeshArray_h
#define CaptureMeshArray_h

#include <stdio.h>

class CaptureMeshArray {
    
public:
    CaptureMeshArray();
    
    void reSize(int size);
    
    void addColor(ofColor _color);
    void addVertex(ofVec3f _vertex);
    
    ofVboMesh& update();
    
    void draw();
    
    int arraySize = 0;
    
    int index = 0;
    
    ofVboMesh *meshArray;
        
};

#endif /* captureMeshArray_h */
