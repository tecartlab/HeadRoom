//
//  captureMeshArray.cpp
//  HeadRoom
//
//  Created by maybites on 04/04/17.
//
//

#include "CaptureMeshArray.h"

CaptureMeshArray::CaptureMeshArray(){
}

void CaptureMeshArray::reSize(int size){
    if(arraySize > 0){
        delete [] meshArray;
    }
    
    if(size > 0){
        index = 0;
        meshArray = new ofVboMesh[size];
    }
    arraySize = size;
}

ofVboMesh& CaptureMeshArray::update(){
    index = (index == arraySize - 1)? 0: index + 1;
    return meshArray[index];
}

void CaptureMeshArray::draw(){
    for(int i = 0; i < arraySize; i++){
        meshArray[i].drawVertices();
    }
}

void CaptureMeshArray::addColor(ofColor _color){
    meshArray[index].addColor(_color);
}

void CaptureMeshArray::addVertex(ofVec3f _vertex){
    meshArray[index].addVertex(_vertex);
}
