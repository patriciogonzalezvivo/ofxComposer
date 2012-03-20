//
//  ofxTitleBar.h
//  GPUBurner
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#ifndef OFXTITLEBAR
#define OFXTITLEBAR

#include "ofMain.h"

enum ButtonType {
    TOGGLE_BUTTON,
    PUSH_BUTTON
};

struct ofxTitleBarButton{
    char letter;
    bool *state;
    ButtonType type;
};

class ofxTitleBar {
public:
    
    ofxTitleBar( ofRectangle* wBox, int* _windowsId );
    
    void setTitle(string _title){ title = _title; };
    
    void addButton( char letter, bool *variableToControl, ButtonType _type);
    
    void draw();
    
    ofEvent<int> close;
    ofEvent<int> reset;
    ofEvent<int> drag;
    
private:
    void _mousePressed(ofMouseEventArgs &e);
    void _mouseReleased(ofMouseEventArgs &e);
    
    ofRectangle    tittleBox;
    ofRectangle    *windowsBox;
    string         title;
    vector<ofxTitleBarButton> buttons;
    
    int             *windowsId;
    int             letterWidth, letterHeight, offSetWidth;
    int             height;
};


#endif
