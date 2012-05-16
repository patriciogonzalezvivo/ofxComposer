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

#include "ofxBaseMap.h"
#include "ofxBaseMask.h"
#include "ofxBaseFilter.h"

#include "ofxShaderObj.h"

struct Link{
    Link(){
        to = NULL;
        toShader = NULL;
        nTex = 0;
    }
    
    ofPoint     pos;
    Link        *to;
    int         nTex;
    int         toId;
    ofxShaderObj *toShader;
};

class ofxBasePatch : public ofRectangle {
public:
    
    ofxBasePatch();
    ~ofxBasePatch();
    
    //  Set
    //
    bool            loadType(string _type);
    bool            loadFile(string _filePath);
    
    bool            loadXmlSettings(ofxXmlSettings &_XML);
    bool            saveXmlSettings(ofxXmlSettings &_XML);
    
    //  Ask
    int             getId(){ return nId; };
    string          getType();
    ofTexture&      getTextureReference();
    ofTexture&      getContentTextureReference();
    
    //  Loops
    //
    void            update();
    void            draw();
    
    //  Content
    //
    ofImage         *image;
    ofVideoPlayer   *videoPlayer;
    ofVideoGrabber  *videoGrabber;
    ofxShaderObj    *shader;
    ofTexture       *texture;
    
    //  Post-Process
    //
    ofxBaseMask     *mask;
    ofxBaseFilter   *filter;
    ofxBaseMap      *map;
    
    //  Links
    //
    vector<Link> outPut;
    vector<Link> inPut;
    
    //  Flags
    //
    bool            bActive, bEditMode, bEditMask, bVisible;
    
protected:
    //  Events
    //
    void            _mousePressed(ofMouseEventArgs &e);
    void            _mouseDragged(ofMouseEventArgs &e);
    void            _mouseReleased(ofMouseEventArgs &e);
    void            _keyPressed(ofKeyEventArgs &e); 
    void            _reMakeFrame( int &_nId );
    
    //  Variables
    //
    ofColor         color;
    int             nId;
};
