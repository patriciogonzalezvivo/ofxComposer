//
//  ofxComposer.cpp
//  GPUBurner
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#include "ofxComposer.h"

ofxComposer::ofxComposer(){
    ofAddListener(ofEvents().mouseMoved, this, &ofxComposer::_mouseMoved);
	ofAddListener(ofEvents().mousePressed, this, &ofxComposer::_mousePressed);
	ofAddListener(ofEvents().mouseReleased, this, &ofxComposer::_mouseReleased);
	ofAddListener(ofEvents().keyPressed, this, &ofxComposer::_keyPressed);
    ofAddListener(ofEvents().windowResized, this, &ofxComposer::_windowResized);
    
    configFile = "config.xml";
    selected = -1;
    selectedDot = -1;
    bGLEditor = false;
}

void ofxComposer::load(string _fileConfig){
    if (_fileConfig != "default")
        configFile = _fileConfig;
    
    ofxXmlSettings XML;
    
    patches.clear();
    if (XML.loadFile(_fileConfig)){
        int totalPatchs = XML.getNumTags("surface");
        
        // Load each surface present on the xml file
        //
        for(int i = 0; i < totalPatchs ; i++){
            ofxPatch *nPatch = new ofxPatch();
            bool loaded = nPatch->loadSettings(i, "config.xml");
            
            if (loaded){
                if (nPatch->getType() == "ofxGLEditor"){
                    ofLog(OF_LOG_NOTICE,"ofxComposer: ofxGLEditor loaded");
                    editor.setup("menlo.ttf");
                    editor.setCurrentEditor(1);
                    editorFbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
                    editorFbo.begin();
                    ofClear(0, 150);
                    editorFbo.end();
                    
                    nPatch->setTexture( editorFbo.getTextureReference(), 0);
                    
                    bGLEditor = true;
                }
                
                if (nPatch->getType() == "input"){
                    nPatch->setTexture( editorFbo.getTextureReference(), 0);
                }
                
                // Insert the new patch into the verctor
                //
                patches.push_back(nPatch);
                
                // Listen to close bottom on the titleBar
                //
                ofAddListener( patches[ patches.size()-1 ]->title->close , this, &ofxComposer::closePatch);
            }
        }
        
        
        // Load links between Patchs
        //
        for(int i = 0; i < totalPatchs ; i++){
            if (XML.pushTag("surface", i)){
                if (XML.pushTag("out")){
                    
                    int totalLinks = XML.getNumTags("dot");
                    for(int j = 0; j < totalLinks ; j++){
                        
                        if (XML.pushTag("dot",j)){
                            int toId = XML.getValue("to", 0);
                            int toTex = XML.getValue("tex", 0);
                            
                            // If everything goes ok "i" will match the position of the vector
                            // with the position on the XML, in the same place of the vector array
                            // defined on the previus loop
                            //
                            connect( i , fromIDtoArrayPos( toId ), toTex);
                            
                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
                XML.popTag();
            }
        }
    }
}

void ofxComposer::setTexture(ofTexture &_texture, int _nId ){
    int n = fromIDtoArrayPos(_nId);
    if (n != -1)
        patches[n]->setTexture( _texture , 0);
}

ofTexture& ofxComposer::getTexture(int _nId){ 
    int n = fromIDtoArrayPos(_nId);
    
    if ( n != -1){
        if ((n < patches.size()) && (n >= 0) ) 
            return patches[n]->getTextureReference(); 
    } 
};

int ofxComposer::fromIDtoArrayPos(int _nId){
    int found = -1;
    
    for (int i = 0; i < patches.size(); i++){
        if (patches[i]->getId() == _nId){
            found = i;
        }
    }
    
    return found;
}

bool ofxComposer::addPatch(string _filePath, ofPoint _position){
    bool loaded = false;
    
    ofxPatch *nPatch = new ofxPatch();
    loaded = nPatch->loadFile( _filePath, "config.xml" );
    
    if ( loaded ){
        //nPatch->saveSettings();
        nPatch->move( _position );
        nPatch->scale(0.5);
        
        patches.push_back(nPatch);
        
        ofAddListener( patches[ patches.size()-1 ]->title->close , this, &ofxComposer::closePatch);
    }
    
    return loaded;
}

bool ofxComposer::connect( int fromPatchN, int toPatchN, int inDotN ){
    bool connected = false;
    
    if (patches[ toPatchN ]->getType() == "ofShader") {
        LinkDot newDot;
        newDot.pos = patches[ fromPatchN ]->getOutPutPosition();
        newDot.toId = patches[ toPatchN ]->getId();
        newDot.to = &(patches[ toPatchN ]->inPut[ inDotN ]);
        newDot.toShader = patches[ toPatchN ]->getShader();
        newDot.nTex = inDotN;
        
        patches[ fromPatchN ]->outPut.push_back( newDot );
        connected = true;
    }
    
    return connected;
}

void ofxComposer::closePatch( int &_nId ){
    int n = fromIDtoArrayPos(_nId);

    bool deleted = false;
    
    if ( (n >= 0) && ( n < patches.size() ) ){        
        
        int targetId = patches[n]->getId();
        int targetTag = 0;
        
        // Delete object from the vector 
        //
        patches.erase(patches.begin()+ n);
        
        // Delete XML Data
        //
        ofxXmlSettings XML;
        if ( XML.loadFile( configFile ) ){
            int totalSurfaces = XML.getNumTags("surface");
            for (int i = 0; i < totalSurfaces; i++){
                if (XML.pushTag("surface", i)){
                    if ( XML.getValue("id", -1) == targetId){
                        targetTag = i;
                    }
                    XML.popTag();
                }
            }
            
            XML.removeTag("surface", targetTag);
            XML.saveFile();
        }
        
        // Delete links Dependences
        //
        for (int i = 0; i < patches.size(); i++){
            for (int j = patches[i]->outPut.size()-1; j >= 0 ; j--){
                if ( patches[i]->outPut[j].toId == targetId ){
                    patches[i]->outPut.erase(patches[i]->outPut.begin() + j );
                    patches[i]->saveSettings();
                }
            }
        }
    }
}

//-------------------------------------------------------------- LOOP
void ofxComposer::update(){
    for(int i = 0; i < patches.size(); i++){
        patches[i]->update();
    }
}


void ofxComposer::draw(){
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    
    ofEnableAlphaBlending();
    for(int i = 0; i < patches.size(); i++){
        patches[i]->draw();
    }
    
    if ( bGLEditor && (selected >= 0)){
        if (patches[selected]->getType() == "ofShader"){
            editorFbo.begin();
            ofClear(0, 150);
            ofRotate(180, 0, 1, 0);
            editor.draw();
            editorFbo.end();
        } else {
            editorFbo.begin();
            ofClear(0, 50);
            editorFbo.end();    
        }
    } 
    
    if (selectedDot >= 0){
        ofLine(patches[selectedDot]->getOutPutPosition(), ofPoint(ofGetMouseX(),ofGetMouseY()));
    }
    
    ofDisableBlendMode();
    ofEnableAlphaBlending();
    
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
}

//-------------------------------------------------------------- EVENTS
void ofxComposer::_keyPressed(ofKeyEventArgs &e){
    if (bGLEditor)
        editor.keyPressed(e.key);
    
    if (selected >= 0){
        if (patches[selected]->getType() == "ofShader"){
            patches[selected]->setFrag(editor.getText(1));
            patches[selected]->saveSettings();
        }
    }
    
    switch (e.key) {
        case OF_KEY_F1:
            ofToggleFullscreen();
            break;
    }
}

void ofxComposer::_mouseMoved(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    for(int i = patches.size() -1 ; i >= 0; i--){
        if (patches[i]->isOver(mouse)){
            focusOnPatch(i);
            break;
        }
    }
}

void ofxComposer::focusOnPatch( int _arrayPos ){
    selected = _arrayPos;
    
    for(int i = 0; i < patches.size(); i++){
        if (i == selected){
            patches[i]->bActive = true;
        } else {
            patches[i]->bActive = false;
        }
    }
}

void ofxComposer::_mousePressed(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);

    selectedDot = -1;    
    for(int i = 0; i < patches.size(); i++){
        if ( (patches[i]->getOutPutPosition().distance(mouse) < 5) && (patches[i]->bEditMode) && !(patches[i]->bEditMask) ){
            selectedDot = i;
            patches[i]->bActive = false;
            selected = -1;
        }
    }
    
    if (selectedDot == -1){
        for(int i = 0; i < patches.size(); i++){
            if ((patches[i]->bActive) && (patches[i]->bEditMode) && !(patches[i]->bEditMask)){
                selected = i;
                
                if (bGLEditor && (patches[selected]->getType() == "ofShader")){
                    editor.setText(patches[i]->getFrag(), 1);
                }
            }
        }
    }
}

void ofxComposer::_mouseReleased(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    if (selectedDot != -1){
        for(int i = 0; i < patches.size(); i++){
            
            if ((selectedDot != i) &&                   // If not him self
                (patches[i]->getType() == "ofShader") &&   // The target it´s a shader
                (patches[i]->bEditMode) &&               // And we are in editMode and not on maskMode
                !(patches[i]->bEditMask) ){
                
                for (int j = 0; j < patches[i]->inPut.size(); j++){
                    
                    // And after checking in each dot of each shader...
                    // ... fin the one where the mouse it´s over
                    //
                    if ( patches[i]->inPut[j].pos.distance(mouse) < 5){
                        
                        // Once he founds it
                        // make the link and forget the selection
                        //
                        connect( selectedDot , i, j );
                        patches[selectedDot]->saveSettings();
                        selectedDot = -1;
                    }
                }
            }
        }
        
        // If he release the mouse over nothing it will clear all
        // the connections of that dot.
        //
        if (selectedDot != -1){
            patches[selectedDot]->outPut.clear();
            patches[selectedDot]->saveSettings();
            selectedDot = -1;
        }
    }
}

void ofxComposer::_windowResized(ofResizeEventArgs &e){
    if (bGLEditor ){
        editor.reShape();
        editorFbo.allocate(e.width, e.height);
        editorFbo.begin();
        ofClear(0, 150);
        editorFbo.end();
    }
}