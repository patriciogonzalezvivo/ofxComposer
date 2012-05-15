//
//  ofxBasePatch.h
//  ofxComposer-example
//
//  Created by Patricio González Vivo on 5/13/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#include "ofMain.h"

#include "ofxXmlSettings.h"

#include "ofxPatchMask.h"
#include "ofxPatchFilter.h"
#include "ofxPatchTransformation.h"

#include "ofxShaderObj.h"

struct LinkDot{
    LinkDot(){
        to = NULL;
        toShader = NULL;
        nTex = 0;
    }
    
    ofPoint     pos;
    LinkDot     *to;
    int         nTex;
    int         toId;
    ofxShaderObj *toShader;
};

class ofxPatchBase : public ofRectangle {
public:
    
    ofxPatchBase();
    ~ofxPatchBase();
    
    bool            loadXmlSettings(ofxXmlSettings &_XML);
    bool            saveXmlSettings(ofxXmlSettings &_XML);
    
    int             getId();
    string          getType();
    ofTexture&      getTextureReference();
    
    void            update();
    void            draw();
    
    ofxPatchMask            *mask;
    ofxPatchFilter          *filter;
    ofxPatchTransformation  *transformation;
    
    vector<LinkDot> outPut;
    vector<LinkDot> inPut;
    
private:
    void            _mousePressed(ofMouseEventArgs &e);
    void            _mouseDragged(ofMouseEventArgs &e);
    void            _mouseReleased(ofMouseEventArgs &e);
    void            _keyPressed(ofKeyEventArgs &e); 
    void            _reMakeFrame( int &_nId );
    
    ofTexture&      getSrcTexture();
    
    ofImage         *image;
    ofVideoPlayer   *videoPlayer;
    ofVideoGrabber  *videoGrabber;
    ofxShaderObj    *shader;
    ofTexture       *texture;
    
    ofColor         color;
    string          type;
    
    int             nId;
};
