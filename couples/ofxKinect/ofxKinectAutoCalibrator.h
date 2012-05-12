//
//  ofxKinectAutoSurface.h
//  mdt-Core
//
//  Created by Patricio González Vivo on 3/28/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#ifndef OFXKINECTAUTOCALIBRATOR
#define OFXKINECTAUTOCALIBRATOR

#include "ofMain.h"

#include "ofxGui.h"

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxPatch.h"

class ofxKinectAutoCalibrator {
public:
    
    ofxKinectAutoCalibrator();
    
    void        init(ofxKinect *_kinect, int _aproxSurfaceArea = (640*480)*0.4);
    bool        update(ofxPatch *_patch);
    
    int         getCurrentStep() const { return nStep; };
    float       getSurfaceDistance() const { return surfaceDistance; };
    float       getCleanDistance() const { return surfaceMinDistance; };
    
    ofPolyline& getSurface() { return surfaceContour; };
    ofTexture&  getTextureReference() { return fbo.getTextureReference(); };
    ofPoint     getkinectToScreen(ofPoint _pos){ return kinectToScreenMatrix * _pos; };
    
    ofFbo       debugFbo;
    
private:
    bool        doStep0();
    bool        doStep1();
    bool        doStep2();
    bool        doStep3();
    bool        doStep4();
    
    bool        isClean(ofxCvGrayscaleImage &img, float _normTolerance = 0);
    bool        isBlobSolid(ofxCvGrayscaleImage &img, ofxCvBlob &blob, float _normTolerance);
    bool        isBlobCircular(ofxCvBlob &blob);
    
    float       getAverage(ofxCvGrayscaleImage &img, ofPolyline &_polyline);
    ofMatrix4x4 getHomographyMatrix(ofPoint src[4], ofPoint dst[4]);
    void        getGaussianElimination(float *input, int n);
    
    ofxKinect   *kinect;
    
    ofxCvGrayscaleImage grayImage;
    ofxCvColorImage     colorImage;
    ofxCvContourFinder  contourFinder;
    
    ofFbo       fbo;
    
    ofPoint     realDots[4];
    ofPoint     screenDots[4];
    ofPoint     trackedDots[4];
    
    ofMatrix4x4 kinectToScreenMatrix;
    
    int         nStep;                      // current step
    
    //          Step 0 & 1
    float       surfaceDistance;            // current distance
    int         surfaceMinArea;             // Minium area to search for blobs that match the surface
    int         surfaceMinDistance;         // Starting scanning distance
    int         surfaceMaxDistance;         // Max scanning distance
    float       scanningSliceHeight;        // Height of each slice of depth data
    float       scanningStepingDistance;    // Distance between each search 
    
    //          Step 2
    ofPolyline  surfaceContour;
    float       scaleAreaFactor;
    
    //          Step 3
    int         countDown;
    int         redThreshold;
    float       minDotArea;                 // Min area for finding blobs on red thresholded image
    float       maxDotArea; 
    
    //          Step 4
    int         scannedDot;
    
    bool        bDone;
};

#endif
