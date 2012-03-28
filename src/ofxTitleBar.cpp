//
//  ofxTitleBar.cpp
//  GPUBurner
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#include "ofxTitleBar.h"

ofxTitleBar::ofxTitleBar( ofRectangle* wBox, int* _windowsId ){
    ofAddListener(ofEvents().mousePressed, this, &ofxTitleBar::_mousePressed);
    
    windowsBox = wBox;
    windowsId = _windowsId;
    height = 15;
    letterWidth = 10;
    letterHeight = 12;
    offSetWidth = 5;
    
    addButton( 'x', NULL, PUSH_BUTTON);
    addButton( 'r', NULL, PUSH_BUTTON);
};

void ofxTitleBar::addButton( char letter, bool *variableToControl, ButtonType _type){
    ofxTitleBarButton newButton;
    newButton.letter = letter;
    newButton.state = variableToControl;
    newButton.type = _type;
    buttons.push_back( newButton );
}

void ofxTitleBar::draw(){
    // Update the information of the position
    //
    tittleBox.width = windowsBox->width;
    tittleBox.height = height;
    tittleBox.x = windowsBox->x;
    tittleBox.y = windowsBox->y - height;
    
    ofPushStyle();
    
    // Draw the Bar
    //
    ofFill();
    ofSetColor(0,50);
    ofRect(tittleBox);
    
    // Draw the tittle
    //
    ofFill();
    ofSetColor(230);
    ofDrawBitmapString(title, tittleBox.x + tittleBox.width - title.size() * 8, tittleBox.y + letterHeight);
    
    // Draw the bottoms
    //
    string buttonString;
    for (int i = 0; i < buttons.size(); i++){
        ofSetColor(100);
        if ( buttons[i].state != NULL){
            if ((*buttons[i].state) == true)
                ofSetColor(255);
        }
        
        ofDrawBitmapString( ofToString(buttons[i].letter) , tittleBox.x + offSetWidth + i*letterWidth, tittleBox.y + letterHeight);
    }
    ofPopStyle();
}

void ofxTitleBar::_mousePressed(ofMouseEventArgs &e){
    ofPoint mouse = ofPoint(e.x, e.y);
    if ( tittleBox.inside(mouse)){
        bool hit = false;
        for (int i = 0; i < buttons.size(); i++){
            if (((mouse.x - tittleBox.x - offSetWidth) > i * letterWidth ) &&
                ((mouse.x - tittleBox.x - offSetWidth) < (i+1) * letterWidth ) ){
                if ( i == 0){
                    ofNotifyEvent(close, *windowsId);
                    hit = true;
                } else if ( i == 1){
                    ofNotifyEvent(reset, *windowsId);
                    hit = true;
                } else {
                    if ( buttons[i].state != NULL ){
                        (*buttons[i].state) = !(*buttons[i].state);
                        hit = true;
                    }
                }
            }
        }
    }
};

void ofxTitleBar::_mouseReleased(ofMouseEventArgs &e){
    ofPoint mouse = ofPoint(e.x, e.y);
    if ( tittleBox.inside(mouse)){
        ofNotifyEvent(drag, *windowsId);
    }
}