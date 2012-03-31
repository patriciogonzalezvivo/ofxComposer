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
    
#ifdef USE_OFXGLEDITOR
    fbo.allocate(640, 480);
    fbo.begin();
    ofClear(0, 0);
    fbo.end();
#endif
    
    configFile = "config.xml";
    selectedDot = -1;
    selectedID = -1;
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
#ifdef USE_OFXGLEDITOR
                if (nPatch->getType() == "ofxGLEditor"){
                    ofLog(OF_LOG_NOTICE,"ofxComposer: ofxGLEditor loaded");
                    editor.setup("menlo.ttf");
                    editor.setCurrentEditor(1);
                    fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
                    fbo.begin();
                    ofClear(0, 150);
                    fbo.end();
                    
                    nPatch->setTexture( fbo.getTextureReference(), 0);
                    
                    bGLEditor = true;
                }
#endif
                
                if (nPatch->getType() == "input"){
                    nPatch->setTexture( fbo.getTextureReference(), 0);
                }
                
                
                // Listen to close bottom on the titleBar
                //
                ofAddListener( nPatch->title->close , this, &ofxComposer::closePatch);
                
                // Insert the new patch into the map
                //
                patches[nPatch->getId()] = nPatch;
            }
        }
        
        // Load links between Patchs
        //
        for(int i = 0; i < totalPatchs ; i++){
            if (XML.pushTag("surface", i)){
                int fromID = XML.getValue("id", -1);
                
                if (XML.pushTag("out")){
    
                    int totalLinks = XML.getNumTags("dot");
                    for(int j = 0; j < totalLinks ; j++){
                        
                        if (XML.pushTag("dot",j)){
                            int toID = XML.getValue("to", 0);
                            int nTex = XML.getValue("tex", 0);
                            
                            // If everything goes ok "i" will match the position of the vector
                            // with the position on the XML, in the same place of the vector array
                            // defined on the previus loop
                            //
                            connect( fromID, toID, nTex);
                            
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

bool ofxComposer::addPatch(string _filePath, ofPoint _position){
    bool loaded = false;
    
    ofxPatch *nPatch = new ofxPatch();
    loaded = nPatch->loadFile( _filePath, "config.xml" );
    
    if ( loaded ){
        nPatch->move( _position );
        nPatch->scale(0.5);
        nPatch->saveSettings();
        ofAddListener( nPatch->title->close , this, &ofxComposer::closePatch);
        patches[nPatch->getId()] = nPatch;
    }
    
    return loaded;
}

bool ofxComposer::connect( int _fromID, int _toID, int nTexture ){
    bool connected = false;
    
    if ((_fromID != -1) && (patches[_fromID] != NULL) && 
        (_toID != -1) && (patches[_toID] != NULL) && 
        (patches[ _toID ]->getType() == "ofShader") ) {
        
        LinkDot newDot;
        newDot.pos = patches[ _fromID ]->getOutPutPosition();
        newDot.toId = patches[ _toID ]->getId();
        newDot.to = &(patches[ _toID ]->inPut[ nTexture ]);
        newDot.toShader = patches[ _toID ]->getShader();
        newDot.nTex = nTexture;
        
        patches[ _fromID ]->outPut.push_back( newDot );
        connected = true;
    }
    
    return connected;
}

void ofxComposer::closePatch( int &_nID ){
    bool deleted = false;
         
    if ( (_nID != -1) && (patches[_nID] != NULL) ){
        int targetTag = 0;
        
        // Delete object from the vector 
        //
        patches.erase(_nID);
        
        // Delete XML Data
        //
        ofxXmlSettings XML;
        if ( XML.loadFile( configFile ) ){
            int totalSurfaces = XML.getNumTags("surface");
            for (int i = 0; i < totalSurfaces; i++){
                if (XML.pushTag("surface", i)){
                    if ( XML.getValue("id", -1) == _nID){
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
        for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
            for (int j = it->second->outPut.size()-1; j >= 0 ; j--){
                if ( it->second->outPut[j].toId == _nID){
                    it->second->outPut.erase( it->second->outPut.begin() + j );
                    it->second->saveSettings();
                }
            }
        }
        
        selectedID = -1;
    }
}

//-------------------------------------------------------------- LOOP
void ofxComposer::update(){
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        it->second->update();
    }
}


void ofxComposer::draw(){
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    
    ofEnableAlphaBlending();
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        it->second->draw();
    }
    
    if ( bGLEditor && (selectedID >= 0)){
        if (patches[selectedID]->getType() == "ofShader"){
#ifdef USE_OFXGLEDITOR
            fbo.begin();
            ofClear(0, 150);
            ofRotate(180, 0, 1, 0);
            editor.draw();
            fbo.end();
#endif
        } else {
            fbo.begin();
            ofClear(0, 50);
            fbo.end();    
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
#ifdef USE_OFXGLEDITOR
    if (bGLEditor)
        editor.keyPressed(e.key);
    
    if (selectedID >= 0){
        if (patches[selectedID]->getType() == "ofShader"){
            patches[selectedID]->setFrag(editor.getText(1));
            patches[selectedID]->saveSettings();
        }
    }
#endif
}

void ofxComposer::_mouseMoved(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    for(map<int,ofxPatch*>::reverse_iterator rit = patches.rbegin(); rit != patches.rend(); rit++ ){
        if (rit->second->isOver(mouse)){
            activePatch( rit->first );
            break;
        }
    }
}

void ofxComposer::activePatch( int _nID ){
    if ( (_nID != -1) && (patches[_nID] != NULL) ){
        selectedID = _nID;
        
        for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
            if (it->first == _nID)
                it->second->bActive = true;
            else
                it->second->bActive = false;
        }
    }
}

void ofxComposer::_mousePressed(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);

    selectedDot = -1;    
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        if ( (it->second->getOutPutPosition().distance(mouse) < 5) && (it->second->bEditMode) && !(it->second->bEditMask) ){
            selectedDot = it->first;
            it->second->bActive = false;
            selectedID = -1;
        }
    }
    
    if (selectedDot == -1){
        for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
            if ((it->second->bActive) && (it->second->bEditMode) && !(it->second->bEditMask)){
                selectedID = it->first;
#ifdef USE_OFXGLEDITOR
                if (bGLEditor && (it->second->getType() == "ofShader")){
                    editor.setText(it->second->getFrag(), 1);
                }
#endif
            }
        }
    }
}

void ofxComposer::_mouseReleased(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    if (selectedDot != -1){
        for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
            if ((selectedDot != it->first) &&                   // If not him self
                (it->second->getType() == "ofShader") &&   // The target it´s a shader
                (it->second->bEditMode) &&               // And we are in editMode and not on maskMode
                !(it->second->bEditMask) ){
                
                for (int j = 0; j < it->second->inPut.size(); j++){
                    
                    // And after checking in each dot of each shader...
                    // ... fin the one where the mouse it´s over
                    //
                    if ( it->second->inPut[j].pos.distance(mouse) < 5){
                        
                        // Once he founds it
                        // make the link and forget the selection
                        //
                        connect( selectedDot , it->first, j );
                        patches[ selectedDot ]->saveSettings();
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
#ifdef USE_OFXGLEDITOR
    if (bGLEditor ){
        editor.reShape();
        fbo.allocate(e.width, e.height);
        fbo.begin();
        ofClear(0, 150);
        fbo.end();
    }
#endif
}