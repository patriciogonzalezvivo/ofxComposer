//
//  ofxComposer.h
//  GPUBurner
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#ifndef OFXCOMPOSER
#define OFXCOMPOSER

#include "ofMain.h"
#include "ofxPatch.h"

#include "ofxGLEditor.h"

class ofxComposer {
public:
    ofxComposer();
    
    ofTexture& operator[](int _nId);
    
    void    load(string _fileConfig = "default");
    bool    addPatch(string _filePath, ofPoint _position);
    
    void    setTexture(ofTexture &_texture, int _nId);
    ofTexture& getTexture(int _nId);
    
    void    update();
    void    draw();
    
private:
    // Events
    void    _mouseMoved(ofMouseEventArgs &e);
    void    _keyPressed(ofKeyEventArgs &e);
	void    _mousePressed(ofMouseEventArgs &e);
	void    _mouseReleased(ofMouseEventArgs &e);
	void    _windowResized(ofResizeEventArgs &e);
    
    void    focusOnPatch( int _arrayPos );
    void    closePatch( int &_nId );
    bool    connect( int _fromPatchN, int _toPatchN, int _inDotN );
    int     fromIDtoArrayPos( int _nId );
    
	ofxGLEditor editor;
    ofFbo       editorFbo;
    
    vector<ofxPatch*>    patches;
    string  configFile;
    
    int     selectedDot;
    int     selected;
    
    bool    bGLEditor;
};


#endif
