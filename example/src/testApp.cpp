#include "testApp.h"

//-------------------------------------------------------------- SETING
void testApp::setup(){
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofSetVerticalSync(false);
    
    composer.load("config.xml");
}

//-------------------------------------------------------------- LOOP
void testApp::update(){
    composer.update();
    
    ofSetWindowTitle( ofToString( ofGetFrameRate()));
}


void testApp::draw(){
    ofBackgroundGradient(ofColor::gray, ofColor::black);
    ofSetColor(255,255);

    composer.draw();
}


//-------------------------------------------------------------- EVENTS
void testApp::keyPressed(int key){
}

void testApp::keyReleased(int key){
}

void testApp::mouseMoved(int x, int y ){
}

void testApp::mouseDragged(int x, int y, int button){
}

void testApp::mousePressed(int x, int y, int button){
  
}

void testApp::mouseReleased(int x, int y, int button){
    
}

void testApp::windowResized(int w, int h){
    
}

void testApp::gotMessage(ofMessage msg){
}


void testApp::dragEvent(ofDragInfo dragInfo){
    if( dragInfo.files.size() > 0 ){
		for(int i = 0; i < dragInfo.files.size(); i++){
            composer.addPatch( dragInfo.files[i], dragInfo.position );
		}
	}
}