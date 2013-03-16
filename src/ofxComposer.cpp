//
//  ofxComposer.cpp
//  GPUBurner
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#include "ofxComposer.h"

//  HELP screen -> F1
//
string helpScreen = "\n \
    | ofxComposer help\n \
    ---------------------------------------------------\n \
    \n \
    - F1:   Turn ON/OFF this help message\n \
    - F2:   Surface Edit-Mode on/off\n \
    - F3:   Masking-Mode ON/OFF (need Edit-Mode ) \n \
    \n \
            On mask mode on:\n \
                            - x: delete mask path point\n \
                            - r: reset mask path\n \
    \n \
    - F4:   Reset surface coorners\n \
    - F5:   Add ofxGLEditor (temporal!!!) and if have it add ofVideoGrabber (temporal!!!)\n \
    - F6:   Add ofShader (temporal!!!)\n \
    - F7:   Turn ON/OFF the fullscreen-mode\n \
    \n \
    Mouse and Coorners: \n \
    - Left Button Drag:     coorner proportional scale \n \
    - Left Button Drag + R: Rotate Patch\n \
    - Middle Button Drag \n \
            or \n \
      Right Drag + A:       centered proportional scale\n \
    - Right Button Drag:    coorner transformation\n ";
    
ofxComposer::ofxComposer(){
    
    //  Event listeners
    //
    ofAddListener(ofEvents().mouseMoved, this, &ofxComposer::_mouseMoved);
	ofAddListener(ofEvents().mousePressed, this, &ofxComposer::_mousePressed);
	ofAddListener(ofEvents().mouseReleased, this, &ofxComposer::_mouseReleased);
	ofAddListener(ofEvents().keyPressed, this, &ofxComposer::_keyPressed);
    ofAddListener(ofEvents().windowResized, this, &ofxComposer::_windowResized);
    
#ifdef USE_OFXGLEDITOR       
    editor.setup("menlo.ttf");
    editor.setCurrentEditor(1);
    editorBgColor.set(0,0);
    editorFgColor.set(0,0);
    editorFbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
    editorFbo.begin();
    ofClear(editorBgColor);
    editorFbo.end();
#endif
    
    //  Default parameters
    //
    configFile = "config.xml";
    selectedDot = -1;
    selectedID = -1;
    bEditMode = true;
    bGLEditorPatch = false;
    bHelp = false;
}

void ofxComposer::load(string _fileConfig){
    if (_fileConfig != "default")
        configFile = _fileConfig;
    
    ofxXmlSettings XML;
    
    patches.clear();
    if (XML.loadFile(_fileConfig)){
        
#ifdef USE_OFXGLEDITOR       
        editor.setup(XML.getValue("general:console:font", "menlo.ttf"));
#endif
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
                    nPatch->setTexture( editorFbo.getTextureReference(), 0);
                    bGLEditorPatch = true;
                }
#endif
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

void ofxComposer::save(string _fileConfig ){
    if (_fileConfig != "default"){
        configFile = _fileConfig;
    }
    
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        it->second->saveSettings(configFile);
    }
}

bool ofxComposer::addPatchFromFile(string _filePath, ofPoint _position){
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

bool ofxComposer::addPatchWithOutFile(string _type, ofPoint _position){
    bool loaded = false;
    
    ofxPatch *nPatch = new ofxPatch();
    loaded = nPatch->loadType( _type, "config.xml" );
    
    if ( loaded ){
        nPatch->move( _position );
        nPatch->scale(0.5);
        nPatch->saveSettings();
        ofAddListener( nPatch->title->close , this, &ofxComposer::closePatch);
#ifdef USE_OFXGLEDITOR
        if (nPatch->getType() == "ofxGLEditor"){
            nPatch->setTexture( editorFbo.getTextureReference(), 0);
        }
#endif
        
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
        
        if (patches[_nID]->getType() == "ofxGLEditor")
            bGLEditorPatch = false;
        
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
        
        // Delete object from memory and then from vector 
        //
        selectedID = -1;
        delete &patches[_nID];
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
    }
}

//-------------------------------------------------------------- LOOP
void ofxComposer::update(){
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        it->second->update();
    }
    
    if ( (bEditMode) && (selectedID >= 0)){
#ifdef USE_OFXGLEDITOR
        if (patches[selectedID]->getType() == "ofShader"){
            editorBgColor.lerp(ofColor(0,150), 0.01);
            editorFgColor.lerp(ofColor(255,255), 0.1);
        } else {
            editorBgColor.lerp(ofColor(0,0), 0.01);
            editorFgColor.lerp(ofColor(0,0), 0.05);
        }
        
        editorFbo.begin();
        //ofEnableAlphaBlending();
        ofClear(editorBgColor);
        ofDisableBlendMode();
        ofRotate(180, 0, 1, 0);
        ofSetColor(255,255);
        editor.draw();
        editorFbo.end();
#endif
    } 
}


void ofxComposer::draw(){
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    
    ofEnableAlphaBlending();
    
#ifdef USE_OFXGLEDITOR
    //  Draw the GLEditor if it�s not inside a Patch
    //
    if (bEditMode && !bGLEditorPatch){
        ofPushMatrix();
        ofRotate(180, 1, 0, 0);
        ofTranslate(0, -ofGetWindowHeight());
        ofSetColor(editorFgColor);
        editorFbo.draw(0, 0);
        ofPopMatrix();
    }
#endif
    
    //  Draw Patches
    //
    for(map<int,ofxPatch*>::iterator it = patches.begin(); it != patches.end(); it++ ){
        it->second->draw();
    }
    
    //  Draw active line
    //
    if (selectedDot >= 0){
        ofLine(patches[selectedDot]->getOutPutPosition(), ofPoint(ofGetMouseX(),ofGetMouseY()));
    }
    
    //  Draw Help screen
    //
    if (bHelp){
        ofSetColor(255);
        ofDrawBitmapString(helpScreen, 20, ofGetWindowHeight()*0.5- 11.0*15.0);
    }
    
    ofDisableBlendMode();
    ofEnableAlphaBlending();
    
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
    
    
}

//-------------------------------------------------------------- EVENTS
void ofxComposer::_keyPressed(ofKeyEventArgs &e){
    if (e.key == OF_KEY_F1 ){
        bHelp = !bHelp;
    } else if (e.key == OF_KEY_F2 ){
        bEditMode = !bEditMode;
    } else if ((e.key == OF_KEY_F3 ) || (e.key == OF_KEY_F4 ) ){
        //  Special keys reserved for Patch Events
        //
    } else if (e.key == OF_KEY_F5 ){
        
        if ( bGLEditorPatch )
            addPatchWithOutFile("ofVideoGrabber", ofPoint(ofGetMouseX(),ofGetMouseY()));
        else
            bGLEditorPatch = addPatchWithOutFile("ofxGLEditor", ofPoint(ofGetMouseX(),ofGetMouseY()));
        
    } else if ( e.key == OF_KEY_F6 ){
        addPatchWithOutFile("ofShader", ofPoint(ofGetMouseX(),ofGetMouseY()));

    } else if (e.key == OF_KEY_F7){
        ofToggleFullscreen();

#ifdef USE_OFXGLEDITOR
        editor.reShape();
        editorFbo.allocate(ofGetWindowWidth(),ofGetWindowHeight());
        editorFbo.begin();
        ofClear(editorBgColor);
        editorFbo.end();
#endif
    } else {
        //  If no special key was pressed and the GLEditor is present pass the key
        //
#ifdef USE_OFXGLEDITOR
        editor.keyPressed(e.key);
        
        if (selectedID >= 0){
            if (patches[selectedID]->getType() == "ofShader"){
                patches[selectedID]->setFrag(editor.getText(1));
                patches[selectedID]->saveSettings();
            }
        }
#endif
        
    }
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
                //if (bGLEditorPatch
                if ((it->second->getType() == "ofShader")){ 
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
    editor.reShape();
    editorFbo.allocate(e.width, e.height);
    editorFbo.begin();
    ofClear(editorBgColor);
    editorFbo.end();
#endif
}