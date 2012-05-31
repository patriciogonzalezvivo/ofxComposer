//
//  ofxBaseMask.h
//  ofxComposer-example
//
//  Created by Patricio González Vivo on 5/13/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#include "ofxXmlSettings.h"
#include "ofxPingPong.h"


class ofxBaseMask : public ofxPingPong {
public:
    ofxBaseMask();
    
    void    allocate(int _width, int _height){
        for(int i = 0; i < 2; i++)
            FBOs[i].allocate(_width,_height, GL_RGBA );
        
        src = &(FBOs[0]);
        dst = &(FBOs[1]);
        
        flag = 0;
    }
    
    bool    loadXmlSettings(ofxXmlSettings &_XML){
        if ( _XML.pushTag("mask") ){
            int totalMaskCorners = _XML.getNumTags("point");
            if (totalMaskCorners > 0){
                corners.clear();
            }
            
            for(int i = 0; i < totalMaskCorners; i++){
                _XML.pushTag("point",i);
                corners.addVertex(_XML.getValue("x", 0.0),
                                  _XML.getValue("y", 0.0));
                _XML.popTag(); // Pop "point"
            }
            _XML.popTag(); // Pop "mask"
            
            return true;
        } else 
            return false;
    }
    
    bool    saveXmlSettings(ofxXmlSettings &_XML){
        if (_XML.pushTag("mask")){
            int totalSavedPoints = _XML.getNumTags("point");
            
            for(int i = 0; i < corners.size(); i++){
                int tagNum = i;
                
                if (i >= totalSavedPoints)
                    tagNum = _XML.addTag("point");
                
                _XML.setValue("point:x", corners[i].x, tagNum);
                _XML.setValue("point:y", corners[i].y, tagNum);
            }
            
            int totalCorners = corners.size();
            totalSavedPoints = _XML.getNumTags("point");
            
            if ( totalCorners < totalSavedPoints){
                for(int i = totalSavedPoints; i > totalCorners; i--){
                    _XML.removeTag("point",i-1);
                }
            }
            _XML.popTag();
            
            return true;
        } else
            return false;
    }
    
    ofTexture&  getTextureReference(){
        return dst->getTextureReference();
    };
    
    void    makeMask(){
        src->begin();
        ofClear(0, 0);
        ofBeginShape();
        ofSetColor(255, 255, 255);
        for(int i = 0; i < corners.size(); i++ ){
            ofVertex(corners[i].x * src->getWidth(), 
                     corners[i].y * src->getHeight() );
        }
        ofEndShape(true);
        src->end();
    }
    
    void    begin(){
        dst->begin();
        ofClear(0, 0);
        shader.begin();
        shader.setUniformTexture("maskTex", src->getTextureReference(), 1 );
        shader.setUniform1f("texOpacity", texOpacity);
        shader.setUniform1f("maskOpacity", maskOpacity);
    }
    
    void    end(){
        shader.end();
        dst->end();
    }
    
    void    draw(float _width, float _height){
        /*
        for(int i = 0; i < corners.size(); i++){
            ofVec3f pos = ofVec3f( corners[i].x * _width, corners[i].y * _height, 0.0);
            //pos = surfaceToScreenMatrix * pos;
            
            if ( (selectedMaskCorner == i) || ( ofDist(ofGetMouseX(), ofGetMouseY(), pos.x, pos.y) <= 4 ) ) {
                ofSetColor(255,255);
                ofCircle( pos, 4);
                ofSetColor(255,100);
                ofFill();
            } else {
                ofNoFill();
                ofSetColor(255,100);
            }
            
            ofCircle( pos, 4);
            
            // Draw contour mask line
            //
            ofSetColor(255,200);
            ofVec3f nextPos = ofVec3f(maskCorners[(i+1)%maskCorners.size()].x*width, 
                                      maskCorners[(i+1)%maskCorners.size()].y*height, 0.0);
            nextPos = surfaceToScreenMatrix * nextPos;
            ofLine(pos.x,pos.y,nextPos.x,nextPos.y);
        }
         */
    }
    
    ofShader        shader;
    ofPolyline      corners;
    
    float           texOpacity, maskOpacity;
    int             selected;
    bool            needUpdate;
};
