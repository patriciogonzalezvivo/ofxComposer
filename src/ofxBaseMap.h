//
//  ofxPatchTransformation.h
//  ofxComposer-example
//
//  Created by Patricio González Vivo on 5/15/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofxBaseMap: public ofPolyline {
public:
    ofxBaseMap();
    
    bool            loadXmlSettings(ofxXmlSettings &_XML);
    bool            saveXmlSettings(ofxXmlSettings &_XML);
    
    ofPoint         getSurfaceToScreen(ofPoint _pos){ return surfaceToScreenMatrix * _pos; };
    ofPoint         getScreenToSurface(ofPoint _pos){ return screenToSurfaceMatrix * _pos; };
    GLfloat*        getGlMatrix() { return glMatrix; };
    
    void            move(ofPoint _pos);
    void            scale(float _scale);
    void            rotate(float _angle);

    void            update(){
        doSurfaceToScreenMatrix();
    }
    
    void    draw(){
        // Draw dragables texture corners
        //
        for(int i = 0; i < 4; i++){
            if (( selectedCorner == i) || 
                ( ofDist(ofGetMouseX(), 
                         ofGetMouseY(),
                         getVertices()[i].x,
                         getVertices()[i].y) <= 4 ) ) 
                ofSetColor(200,255);
            else 
                ofSetColor(200,100);
            
            ofRect(getVertices()[i].x-4,
                   getVertices()[i].y-4, 
                   8,
                   8 );
            
            // Draw contour Line
            //
            ofLine(getVertices()[i].x, 
                   getVertices()[i].y, 
                   getVertices()[(i+1)%4].x, 
                   getVertices()[(i+1)%4].y );
        }
    }
    
    //ofPolyline      corners;
    int             selectedCorner;
    bool            needUpdate;
    
private:
    void            doSurfaceToScreenMatrix();
    void            doScreenToSurfaceMatrix();
    void            doGaussianElimination(float *input, int n);
    
    ofMatrix4x4     surfaceToScreenMatrix;
    ofMatrix4x4     screenToSurfaceMatrix;
    GLfloat         glMatrix[16];
    ofPoint         src[4];
};