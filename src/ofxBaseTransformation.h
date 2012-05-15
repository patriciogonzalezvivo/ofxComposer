//
//  ofxPatchTransformation.h
//  ofxComposer-example
//
//  Created by Patricio González Vivo on 5/15/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#include "ofxXmlSettings.h"

class ofxBaseTransformation: public ofPolyline {
public:
    ofxBaseTransformation();
    
    bool            loadXmlSettings(ofxXmlSettings &_XML);
    bool            saveXmlSettings(ofxXmlSettings &_XML);
    
    ofPoint         getSurfaceToScreen(ofPoint _pos){ return surfaceToScreenMatrix * _pos; };
    ofPoint         getScreenToSurface(ofPoint _pos){ return screenToSurfaceMatrix * _pos; };
    GLfloat*        getGlMatrix() { return glMatrix; };
    
    void            move(ofPoint _pos);
    void            scale(float _scale);
    void            rotate(float _angle);

    void            update();
    
    int             selectedPoint;
    
private:
    void            doSurfaceToScreenMatrix();
    void            doScreenToSurfaceMatrix();
    void            doGaussianElimination(float *input, int n);
    
    ofMatrix4x4     surfaceToScreenMatrix;
    ofMatrix4x4     screenToSurfaceMatrix;
    GLfloat         glMatrix[16];
    ofPoint         src[4];
};