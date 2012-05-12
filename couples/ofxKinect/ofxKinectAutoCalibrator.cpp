//
//  ofxKinectAutoCalibrator.cpp
//  mdt-Core
//
//  Created by Patricio González Vivo on 3/28/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#include "ofxKinectAutoCalibrator.h"

ofxKinectAutoCalibrator::ofxKinectAutoCalibrator(){
    nStep = 0;
}

void ofxKinectAutoCalibrator::init(ofxKinect *_kinect, int _aproxSurfaceArea){
    kinect = _kinect;
    
    surfaceMinArea = _aproxSurfaceArea; // Aprox minimal Area of the surface
    surfaceMinDistance = 500;       // mm
    surfaceMaxDistance = 600;       // mm
    scanningSliceHeight = 10.0;     // mm
    scanningStepingDistance = 0.5;  // mm
    
    surfaceDistance = surfaceMaxDistance;   //
    kinect->setDepthClipping(surfaceDistance-scanningSliceHeight,surfaceDistance);
    
    redThreshold = 255;
    minDotArea = 50;
    maxDotArea = 100;
    scannedDot = 0;
    
    grayImage.allocate(kinect->getWidth(), kinect->getHeight());
    colorImage.allocate(kinect->getWidth(), kinect->getHeight());
    fbo.allocate(kinect->getWidth(), kinect->getHeight());
    fbo.begin();
    ofClear(0,255);
    fbo.end();
    
    debugFbo.allocate(kinect->getWidth(), kinect->getHeight());
    debugFbo.begin();
    ofClear(0,255);
    debugFbo.end();

    nStep = 0;
    bDone = false;
}

bool ofxKinectAutoCalibrator::update(ofxPatch *_patch){

    if (nStep == 0){
    
        if ( doStep0() ){
            // Reset the mask of the patch
            //
            ofPolyline maskCorners;
            maskCorners.addVertex(0.0,0.0);
            maskCorners.addVertex(1.0,0.0);
            maskCorners.addVertex(1.0,1.0);
            maskCorners.addVertex(0.0,1.0);
            _patch->setMask(maskCorners);
            
            nStep++;
        }
        
    } else if (nStep == 1){
        
        if ( doStep1() ){
            
            //  Normalize the position of each vertex with the image width and height
            //
            ofPolyline normalizedContour;
            for (int i = 0; i < surfaceContour.size(); i++){
                normalizedContour.addVertex(surfaceContour[i].x/kinect->getWidth(), surfaceContour[i].y/kinect->getHeight());
            }
            
            //  Pass it to a ofxPatch::maskCorners
            //
            _patch->setMask(normalizedContour);
            
            nStep++;
        }
        
    } else if (nStep == 2){
        if ( doStep2() ){
            
            //  Once it found it get the real dots position in relation with the screen.
            //
            for (int i = 0; i < 4; i++){
                screenDots[i] = _patch->getSurfaceToScreen(realDots[i]);
            }
            
            nStep++;
        }
        
    } else if (nStep == 3){
        if ( doStep3() )
            nStep++;
    } else if (nStep == 4){
        if ( doStep4() )
            nStep++;
    } else if (nStep == 5){
        //  Make the transformation matrix
        //
        kinectToScreenMatrix = getHomographyMatrix(trackedDots,screenDots);
        
        //  Aply the matrix to each corner of the patch
        //
        ofPoint coorners[4];
        coorners[0].set(0,0);
        coorners[1].set(kinect->getWidth(),0);
        coorners[2].set(kinect->getWidth(),kinect->getHeight());
        coorners[3].set(0,kinect->getHeight());
        for (int i = 0; i < 4; i++){
            coorners[i] = getkinectToScreen(coorners[i]);
        }
        _patch->setCoorners(coorners);
        
        // Clear the fbo
        //
        fbo.begin();
        ofClear(0,255);
        fbo.end();
        
        // Job well done
        //
        bDone = true;
    }
    
    _patch->setTexture(fbo.getTextureReference());
    
    return bDone;
}

bool ofxKinectAutoCalibrator::doStep0(){
    string msg = "Step 0: Searching for an empty space at " + ofToString(surfaceDistance*0.1) + "cm";
    
    // Make a "slice cut" of depth data at the minim distance to find if everthing it´s all right
    //
    kinect->setDepthClipping(surfaceMinDistance,surfaceDistance);
    grayImage.setFromPixels(kinect->getDepthPixelsRef());
    
    // Console feedback
    //
    ofLog(OF_LOG_NOTICE,msg);
    
    // Visual feedback
    //
    fbo.begin();
    ofClear(0,255);
    ofSetColor(150);
    grayImage.draw(0, 0);
    contourFinder.draw();
    ofSetColor(255);
    ofDrawBitmapString(msg, 15, 15);
    fbo.end();
    
    // If it found a black screen, jump to the next step
    if ( isClean(grayImage) ){
        ofLog(OF_LOG_NOTICE, "Step 0: COMPLETED. Empty space found at " + ofToString(surfaceDistance*0.1) + "cm" );
        surfaceMinDistance = surfaceDistance;
        return true;
    } else {
        surfaceDistance--;
        return false;
    }
}

bool ofxKinectAutoCalibrator::doStep1(){
    bool rta = false;
    
    string  msg = "Step 1: Searching for a BIG surface (more than " + ofToString(surfaceMinArea) + ") at " + ofToString(surfaceDistance*0.1) + "cm";
    bool    found = false;
    
    // Make a "slice cut" of depth data at a progressive distance...
    //
    kinect->setDepthClipping(surfaceDistance-scanningSliceHeight,surfaceDistance);
    grayImage.setFromPixels(kinect->getDepthPixelsRef());
    
    // ... searching for ONE big ( surfaceMinArea ) almost white blob ( the "posible" table )
    //
    contourFinder.findContours(grayImage, surfaceMinArea, (640*480), 1, false);
    
    // Console Feedback
    //
    ofLog(OF_LOG_NOTICE,msg);
    
    // Visual Feedback
    //
    fbo.begin();
    ofClear(0,255);
    ofSetColor(255);
    grayImage.draw(0, 0);
    contourFinder.draw();
    ofDrawBitmapString(msg, 15, 15);
    fbo.end();
    
    // if ONE big blob it´s found ...
    //
    if ( contourFinder.nBlobs == 1 ){
        
        // ... check if it´s almost solid white or if need more distance to get good sharp edges.
        //
        if ( isBlobSolid(grayImage, contourFinder.blobs[0], 0.99)){
            ofLog(OF_LOG_NOTICE, "Step 1: COMPLETED. Surface found at " + ofToString( surfaceDistance*0.1) + "cm" );
            found  = true;
            
            //  a. Store ofPolyLine of the contour of that figure.
            //
            surfaceContour.clear();
            surfaceContour.addVertexes(contourFinder.blobs[0].pts);
            surfaceContour.simplify(1);
            
            //  b. Reset the scale factor of the surface area to 1.0 (non-scalation).Because on next step
            //      it´s going to search for the right amount of scalation in order to place the dots
            //      inside the surface.
            //
            scaleAreaFactor = 1.0;
            
            //  c. Move to the next step
            rta = true;
        }
    }
    
    if (!found){
        // If the surface it´s not found it will decende one steping distance and search again
        //
        surfaceDistance += scanningStepingDistance;
        
        // If it´s too far away it will start again (back to step 0) but form a little longer 
        // max distance
        //
        if (surfaceDistance >= surfaceMaxDistance){
            surfaceMinDistance = 500;
            surfaceMaxDistance += 100;
            surfaceDistance = surfaceMaxDistance;
            
            fbo.begin();
            ofClear(0,255);
            fbo.end();
            kinect->setDepthClipping(surfaceMinDistance,surfaceDistance);
            nStep = 0;
        }
    }
    
    return rta;
}

bool ofxKinectAutoCalibrator::doStep2(){
    bool rta = false;
    
    string  msg = "Step 2: Calculate the dot´s position ( scale " + ofToString(scaleAreaFactor) + " factor from coorners )";
    
    // Extract the boundingBox of the tracked surfaceContour and scale progressively 
    // until it found the right amount of scalation.
    //
    ofRectangle surfaceArea;
    surfaceArea.setFromCenter(surfaceContour.getBoundingBox().getCenter(),
                              surfaceContour.getBoundingBox().width * scaleAreaFactor, 
                              surfaceContour.getBoundingBox().height * scaleAreaFactor);
    
    // Place the dots on the coorners of the scaled boundingBox
    //
    realDots[0].set(surfaceArea.x, surfaceArea.y);
    realDots[1].set(surfaceArea.x+surfaceArea.width, surfaceArea.y);
    realDots[2].set(surfaceArea.x+surfaceArea.width, surfaceArea.y+surfaceArea.height);
    realDots[3].set(surfaceArea.x, surfaceArea.y+surfaceArea.height);
    
    // Console Feedback
    //
    ofLog(OF_LOG_NOTICE,msg);
    
    // Visual Feedback
    //
    fbo.begin();
    ofClear(0,255);
    ofPushStyle();
    ofSetColor(255, 0, 0);
    for (int i = 0; i < 4; i++){
        ofCircle(realDots[i], 20);
    }
    ofPopStyle();
    ofSetColor(255);
    ofDrawBitmapString(msg, 15, 15);
    fbo.end();
    
    // Check if every one it´s insde the the surfaceContour
    //
    int insideCount = 0;
    for (int i = 0; i < 4; i++){
        if ( surfaceContour.inside( realDots[i] ) ){
            insideCount++;
        }
    }
    
    if (insideCount == 4){
        ofLog(OF_LOG_NOTICE, "Step 2: COMPLETED. Points placed on a scale " + ofToString(scaleAreaFactor) + " factor from coorners");
        
        //  Set some defaul valuables for next time
        //
        countDown = 100;    // Number of times it have to see the 4 dot´s in order to be sure that it´s ok
        
        //  Proced to next step
        //
        rta = true;
    } 
    
    scaleAreaFactor -= 0.2;
    
    return rta;
}

bool ofxKinectAutoCalibrator::doStep3(){
    bool rta = false;
    
    string  msg1 = "Step 3: Searching for a clear view at threshold " + ofToString( redThreshold );
    string  msg2 = "        and aprox. area of " + ofToString(minDotArea) + "-" + ofToString(maxDotArea);
    
    //  Grab the RGB image of the kinect, take the RED channel, do a threshold and check
    //  That are no dot´s
    //
    colorImage.setFromPixels(kinect->getPixelsRef());
    grayImage.setFromPixels( colorImage.getPixelsRef().getChannel(0) );
    grayImage.threshold(redThreshold);
    contourFinder.findContours(grayImage, 20, surfaceMinArea*0.25, 10, false);
    
    // Console Feedback
    //
    ofLog(OF_LOG_NOTICE,msg1);
    ofLog(OF_LOG_NOTICE,msg2);
    
    fbo.begin();
    ofClear(0,255);
    
    ofFill();
    //  Draw the red dots.
    //
    ofSetColor(255, 0, 0,255);
    for (int i = 0; i < 4; i++){
        ofCircle(realDots[i], 20);
    }
    ofPopStyle();
    fbo.end();
    
    
    debugFbo.begin();
    ofClear(0,255);
    ofPushStyle();
    ofSetColor(255, 255);
    grayImage.draw(0, 0);
    contourFinder.draw();
    
    //  For each circular dot extract the area and make an average
    //
    int circularBlobs = 0;
    for (int i = 0; i < contourFinder.nBlobs; i++){
        if ( isBlobCircular(contourFinder.blobs[i]) ){
            ofPushStyle();
            ofSetColor(255,0,0, 255);
            ofDrawBitmapString("Dot", contourFinder.blobs[i].boundingRect.x, contourFinder.blobs[i].boundingRect.y );
            float area = contourFinder.blobs[i].area;
            
            if ( area < minDotArea )
                minDotArea = area - 1;
            
            if ( area > maxDotArea)
                maxDotArea = area + 1;
            ofPopStyle();
            circularBlobs++;
        }
    }
    
    ofSetColor(255, 255);
    ofDrawBitmapString(msg1, 15, 15);
    ofDrawBitmapString(msg2, 15, 30);
    debugFbo.end();
    
    if ( circularBlobs == 4){
        if (countDown == 0){
            ofLog(OF_LOG_NOTICE,"Setp 3: COMPLETED. 4 Dots found with an area between " + ofToString(minDotArea) + " and " + ofToString(maxDotArea) + " pixels.");
            
            // Prepare everthing for next step
            scannedDot  = 0;    // Start for first dot
            countDown   = 100;  // restart the timer
            
            // Passthrou to next step
            rta = true;
        } else {
            countDown--;    // If found 4 circular dots count down
        }
    } else if (circularBlobs < 4){
        countDown   = 100;
        redThreshold--;
        
        if (redThreshold < 10)
            redThreshold = 255;
            
    } else if (circularBlobs > 4){
        countDown   = 100;
        redThreshold = ofClamp(redThreshold+1, 0, 255);
    }
    
    return rta;
}

bool ofxKinectAutoCalibrator::doStep4(){
    bool rta = false;
    
    colorImage.setFromPixels(kinect->getPixelsRef());
    grayImage.setFromPixels( colorImage.getPixelsRef().getChannel(0) );
    grayImage.threshold(redThreshold);
    contourFinder.findContours(grayImage, minDotArea, maxDotArea , 10, false);
    
    // Visual Feedback
    //
    fbo.begin();
    ofClear(0,255);
    ofPushStyle();
    ofFill();
    // Draw red circular dot to track
    ofSetColor(255, 0, 0,255);
    ofCircle(realDots[scannedDot], 20);
    // Draw blue marks to check for the consistency of the tracking blobs
    ofSetColor(0, 0, 255,255);
    for (int i = 0; i < contourFinder.nBlobs; i++){
        ofLine(contourFinder.blobs[i].centroid.x-5,contourFinder.blobs[i].centroid.y,contourFinder.blobs[i].centroid.x+5,contourFinder.blobs[i].centroid.y);
        ofLine(contourFinder.blobs[i].centroid.x,contourFinder.blobs[i].centroid.y-5,contourFinder.blobs[i].centroid.x,contourFinder.blobs[i].centroid.y+5);
    }
    ofPopStyle();
    fbo.end();
    
    
    debugFbo.begin();
    ofClear(0,255);
    grayImage.draw(0, 0);
    contourFinder.draw();
    //  Search for the only circular dot ( it have to be just one )
    //
    if (contourFinder.nBlobs > 0){
        ofPushStyle();
        //  Search for circular dots
        //
        int circularBlobs = 0;
        int circularBlobID = -1;
        for (int i = 0; i < contourFinder.nBlobs; i++){
            if ( isBlobCircular(contourFinder.blobs[i]) ){
                ofPushStyle();
                ofSetColor(255,0,0, 255);
                ofDrawBitmapString("Dot", contourFinder.blobs[i].boundingRect.x, contourFinder.blobs[i].boundingRect.y );
                circularBlobs++;
                circularBlobID = i; // store it position on the blob vector
                ofPopStyle();
            }
        }
        
        // Check that it´s only one
        //
        if (circularBlobs == 1){
            
            if (scannedDot < 4){
                
                // For each dot make a counter for geting an average of here it founds it
                if (countDown != 0){
                    trackedDots[scannedDot] += contourFinder.blobs[circularBlobID].centroid;
                    trackedDots[scannedDot] *= 0.5;
                    countDown--;
                } else {
                    // Once the timer it´s on 0 save the data and go to the next dot
                    //
                    ofLog(OF_LOG_NOTICE, "Step 4: Found dot nº " + ofToString(scannedDot + 1) + " at position " + ofToString(trackedDots[scannedDot]));
                    scannedDot++; 
                    countDown = 100;
                } 
            } else {
                // No more dots to track
                //
                ofLog(OF_LOG_NOTICE, "Step 4: COMPLETED. Four dots scanned succesfully");
                rta = true;
            }
        } 
    }
    ofPopStyle();
    debugFbo.end();
    
    return rta;
}

bool ofxKinectAutoCalibrator::isClean(ofxCvGrayscaleImage &img, float _normTolerance ){
    bool rta = false;
    
    ofPolyline imgFrame;
    imgFrame.addVertex(0, 0);
    imgFrame.addVertex(img.getWidth(), 0);
    imgFrame.addVertex(img.getWidth(), img.getHeight());
    imgFrame.addVertex(0, img.getHeight());
    
    if ( (int) getAverage(img, imgFrame) == (255*_normTolerance)){
        rta = true;
    }
    
    return rta;
}

bool ofxKinectAutoCalibrator::isBlobSolid(ofxCvGrayscaleImage &img, ofxCvBlob &blob, float _normTolerance){
    bool rta = false;
    
    ofPolyline  blobContour;
    blobContour.addVertexes(blob.pts);
    blobContour.simplify(5);
    
    ofLog(OF_LOG_NOTICE,"Searching for a blob with average brigness of " + ofToString(255*_normTolerance));
    if (getAverage(img, blobContour) > (255*_normTolerance)){
        rta = true;
    }
    
    return rta;
}

// Get the average of brightness of a specific blob
//
float ofxKinectAutoCalibrator::getAverage(ofxCvGrayscaleImage &img,  ofPolyline &_polyline){
    unsigned char * pixels = img.getPixels();
    int imgWidth = img.getWidth();
    int imgHeight = img.getHeight();
    int totalPixles =  imgWidth * imgHeight;
    
    int counter = 0;
    float average = 0;
    
    for (int x = 0; x < imgWidth ; x++) {
        for (int y = 0; y < imgHeight ; y++) {
            int pos = x + y * imgWidth;
            
            if (_polyline.inside(x, y)){
                average += pixels[pos];
                counter++;
            }
        }
    }
    
    average /= counter;

    ofLog(OF_LOG_NOTICE,"Just analize a Blob with a bright average of " + ofToString(average) );
    
    return average;
}

bool ofxKinectAutoCalibrator::isBlobCircular(ofxCvBlob &blob){
    bool rta = false;
    
    ofPoint center = blob.centroid;
    
    float area = blob.area;
    
    float maxDistance = 0;
    float minDistance = 10000;
    
    for(int i = 0; i < blob.pts.size(); i++){
        float dist = center.distance( blob.pts[i] );
        
        if (dist > maxDistance)
            maxDistance = dist;
        
        if (dist < minDistance)
            minDistance = dist;
    }
    
    float diff = abs(maxDistance - minDistance);
    
    if ( diff < 4 )
        rta = true;
    
    return  rta;
}

ofMatrix4x4  ofxKinectAutoCalibrator::getHomographyMatrix(ofPoint src[4], ofPoint dst[4]){
	ofMatrix4x4 matrix;
	
	// create the equation system to be solved
	//
	// from: Multiple View Geometry in Computer Vision 2ed
	//       Hartley R. and Zisserman A.
	//
	// x' = xH
	// where H is the homography: a 3 by 3 matrix
	// that transformed to inhomogeneous coordinates for each point
	// gives the following equations for each point:
	//
	// x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
	// y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
	//
	// as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
	// so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
	// after ordering the terms it gives the following matrix
	// that can be solved with gaussian elimination:
	
	float P[8][9]={
		{-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
		{  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12
		
		{-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
		{  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21
		
		{-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
		{  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23
		
		{-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
		{  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
	};
	
	getGaussianElimination(&P[0][0],9);
	
	matrix(0,0)=P[0][8];
	matrix(0,1)=P[1][8];
	matrix(0,2)=0;
	matrix(0,3)=P[2][8];
	
	matrix(1,0)=P[3][8];
	matrix(1,1)=P[4][8];
	matrix(1,2)=0;
	matrix(1,3)=P[5][8];
	
	matrix(2,0)=0;
	matrix(2,1)=0;
	matrix(2,2)=0;
	matrix(2,3)=0;
	
	matrix(3,0)=P[6][8];
	matrix(3,1)=P[7][8];
	matrix(3,2)=0;
	matrix(3,3)=1;
	
	return matrix;
}

void ofxKinectAutoCalibrator::getGaussianElimination(float *input, int n){
	// ported to c from pseudocode in
	// http://en.wikipedia.org/wiki/Gaussian_elimination
	
	float * A = input;
	int i = 0;
	int j = 0;
	int m = n-1;
	while (i < m && j < n){
		// Find pivot in column j, starting in row i:
		int maxi = i;
		for(int k = i+1; k<m; k++){
			if(fabs(A[k*n+j]) > fabs(A[maxi*n+j])){
				maxi = k;
			}
		}
		if (A[maxi*n+j] != 0){
			//swap rows i and maxi, but do not change the value of i
			if(i!=maxi)
				for(int k=0;k<n;k++){
					float aux = A[i*n+k];
					A[i*n+k]=A[maxi*n+k];
					A[maxi*n+k]=aux;
				}
			//Now A[i,j] will contain the old value of A[maxi,j].
			//divide each entry in row i by A[i,j]
			float A_ij=A[i*n+j];
			for(int k=0;k<n;k++){
				A[i*n+k]/=A_ij;
			}
			//Now A[i,j] will have the value 1.
			for(int u = i+1; u< m; u++){
				//subtract A[u,j] * row i from row u
				float A_uj = A[u*n+j];
				for(int k=0;k<n;k++){
					A[u*n+k]-=A_uj*A[i*n+k];
				}
				//Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
			}
			
			i++;
		}
		j++;
	}
	
	//back substitution
	for(int i=m-2;i>=0;i--){
		for(int j=i+1;j<n-1;j++){
			A[i*n+m]-=A[i*n+j]*A[j*n+m];
			//A[i*n+j]=0;
		}
	}
}