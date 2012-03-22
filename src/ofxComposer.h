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
    
    void    focusOnPatch( int _nID );
    void    setTexture(ofTexture &_texture, int _nID);
    ofTexture& getTexture(int _nID);
    
    void    update();
    void    draw();
    
private:
    // Events
    void    _mouseMoved(ofMouseEventArgs &e);
    void    _keyPressed(ofKeyEventArgs &e);
	void    _mousePressed(ofMouseEventArgs &e);
	void    _mouseReleased(ofMouseEventArgs &e);
	void    _windowResized(ofResizeEventArgs &e);
    
    
    void    closePatch( int &_nID );
    bool    connect( int _fromID, int _toID, int _nTexture );
    
	ofxGLEditor editor;
    ofFbo       editorFbo;
    
    map<int,ofxPatch*>  patches;
    
    string  configFile;
    
    int     selectedDot;
    int     selectedID;
    
    bool    bGLEditor;
};


#endif
