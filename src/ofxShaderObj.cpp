/*
 *  ofxShaderObj.h
 *
 *  Created by Patricio Gonzalez Vivo on 03/07/11.
 *  Copyright 2011 http://www.PatricioGonzalezVivo.com All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *  OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  ************************************************************************************
 *  
 */ 

#include "ofxShaderObj.h"

ofxShaderObj::ofxShaderObj():nTextures(0){
    // Simple effect just need this three variables
    // For something more complex that require another structure, logic or more shaders working together
    // think on making a new stand-alone class as the ofxBlur, ofxFluid, ofxGlow, etc ...
    // Collaborations are welcome
    
    passes = 1;                 // Number of itinerations needs. Default it´s 1;
    internalFormat = GL_RGBA;   // Tipe of GL textures 
    
    // And the fragSahder it self. Note that are defaul variables:
    //
    // - time
    // - mouse position (normalized)
    // - resolution
    // - backbuffer texture
    // - tex0, tex1, tex2, ... : this are dynamicaly defined and allocated and can be
    //   filled with information by using .begin(0) and .end(0), or .begin(1) and .end(1), etc 
    //
    // This dafault shader it's timer made of a mix on Ricardo Caballero's webGL Sandbox
    // http://mrdoob.com/projects/glsl_sandbox/
    //
    
    fragmentShader = "\n\
// \n\
// Empty Shader Patch for ofxComposer \n\
// by http://PatricioGonzalezVivo.com \n\
//\n\
// For quick GLSL prototyping this Patch have the next native variables\n\
//\n\
uniform sampler2DRect backbuffer;  // Previus frameBuffer \n\
uniform sampler2DRect tex0;        // Input texture number 0 \n\
                                   // You can add as many as you want\n\
                                   // just type name them 'tex'+ N\n\
\n\
uniform vec2  size0;               // tex0 resolution\n\
uniform vec2  resolution;          // Patch resolution\n\
uniform vec2  window;              // Window resolution\n\
uniform vec2  screen;              // Screen resolution\n\
uniform vec2  mouse;               // Mouse position on the screen\n\
uniform float time;                // seconds \n\
\n\
void main( void ){\n\
    vec2 st = gl_TexCoord[0].st;    // gl_FragCoord.st;\n\
    vec4 color = texture2DRect(tex0, st);\n\
    \n\
    gl_FragColor = vec4( color.rgb, color.a );\n\
}\n";
    
    vertexShader = "void main(){\n\
\n\
gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}\n";
}

ofxShaderObj::~ofxShaderObj(){
    if (textures != NULL ){
        if (nTextures > 0) {
            delete [] textures;
        }
    }
}

ofxShaderObj& ofxShaderObj::operator =(ofxShaderObj& parent){
    passes = parent.getPasses();
    internalFormat = parent.getInternalFormat();
    fragmentShader = parent.getFragmentShader();
    ofVec2f resolution = parent.getResolution();
    allocate(resolution.x, resolution.y);
    
    return * this;
}


void ofxShaderObj::allocate(int _width, int _height, int _internalFormat){
    width = _width;
    height = _height;
    
    if (_internalFormat != -1)
        internalFormat = _internalFormat;
    
    pingPong.allocate(width, height, internalFormat);
    doFragmentShader();
};

bool ofxShaderObj::setFragmentShader(string _fragShader){
    bool loaded = false;
    
    if ( fragmentShader != _fragShader ){
        
        ofShader test;
        test.setupShaderFromSource(GL_FRAGMENT_SHADER, _fragShader);
        bFine = test.linkProgram();
        
        if( bFine ){
            fragmentShader = _fragShader;
            loaded = doFragmentShader();
        }
    }
    
    return loaded;
}

/*
bool ofxShaderObj::setVertexShader(string _vertShader){
    bool loaded = false;
    
    if ( vertexShader != _vertShader ){
        
        ofShader test;
        test.setupShaderFromSource(GL_VERTEX_SHADER, _vertShader);
        bFine = test.linkProgram();
        
        if( bFine ){
            vertexShader = _vertShader;
            loaded = doFragmentShader();
        }
    }
    
    return loaded;
}*/

// A simplified way of filling the insides texture
void ofxShaderObj::setTexture(ofTexture& tex, int _texNum){ 
    if ((_texNum < nTextures) && ( _texNum >= 0)){
        textures[_texNum].begin(); 
        ofClear(0,255);
        ofSetColor(255);
        tex.draw(0,0); 
        textures[_texNum].end();
    }
};


// ---------------------------------------------------------- LOOPS
//
// As most objects on openFrameworks, ofxShaderObj have to be updated() in order to process the information on the GPU
void ofxShaderObj::update(){
    
    // This process it´s going to be repited as many times as passes variable said
    for(int i = 0; i < passes; i++) {
        
        // All the process it´s done on the pingPong ofxSwapBuffer ( basicaly two ofFbo that have a swap() funtion )
        pingPong.dst->begin();
        
        ofClear(0);
        shader.begin();
        
        // The other ofFbo of the ofxSwapBuffer can be access by calling the unicode "backbuffer"
        shader.setUniformTexture("backbuffer", pingPong.src->getTextureReference(), 0 );
        
        // All the needed textures are provided to the shader by this loop
        for( int i = 0; i < nTextures; i++){
            string texName = "tex" + ofToString(i); 
            shader.setUniformTexture(texName.c_str(), textures[i].getTextureReference(), i+1 );
            string texRes = "size" + ofToString(i); 
            shader.setUniform2f(texRes.c_str() , (float)textures[i].getWidth(), (float)textures[i].getHeight());
        }
        
        // Also there are some standar variables that are passes to the shaders
        // this ones follows the standar used by Ricardo Caballero´s webGL Sandbox
        // http://mrdoob.com/projects/glsl_sandbox/ and ShaderToy by Inigo Quilez http://www.iquilezles.org/apps/shadertoy/
        // webGL interactive GLSL editors
        //
        shader.setUniform2f("resolution", (float)width, (float)height);
        shader.setUniform2f("window", (float)ofGetWindowWidth(), (float)ofGetWindowHeight());
        shader.setUniform2f("screen", (float)ofGetScreenWidth(), (float)ofGetScreenHeight());
        shader.setUniform2f("mouse", (float)ofGetMouseX(), (float)ofGetMouseY());
        shader.setUniform1f("time", (float)ofGetElapsedTimef() );
        
        // doFrame() it´s a built-in funtion of ofxShaderObj that only draw a white box in order to 
        // funtion as a frame here the textures could rest.
        // If you want to distort the points of a textures, probably you want to re-define the renderFrame funtion.
        renderFrame();
        
        shader.end();
        
        pingPong.dst->end();
        
        pingPong.swap();    // Swap the ofFbo's. Now dst it's src and src its dst
    }
    
    pingPong.swap();        // After the loop the render information will be at the src ofFbo of the ofxSwapBuffer 
                            // this extra swap() call will put it on the dst one. Witch sounds more reasonable... isn´t?
    
    //time += 1.0/ofGetFrameRate();   // here it´s where the time it´s updated.
};


void ofxShaderObj::draw(int x, int y, float _width, float _height){
    if (_width == -1) _width = width;
    if (_height == -1) _height = height;
    
    ofPushStyle();
    ofEnableAlphaBlending();
    pingPong.dst->draw(x, y, _width, _height);
    ofPopStyle();
}


// ---------------------------------------------------------------------- ACTIONS
//
bool ofxShaderObj::doFragmentShader(){
    // Looks how many textures it´s need on the injected fragment shader
    int num = 0;
    for (int i = 0; i < 16; i++){
        string searchFor = "tex" + ofToString(i);
        if ( fragmentShader.find(searchFor)!= -1)
            num++;
        else 
            break;
    }
    
    // Check if it´s the same number of tectures already created and allocated
    if ( num != nTextures ){
        // If the number of textures it´s different
        if (textures != NULL ){
            if (nTextures > 0) {
                delete [] textures;
            }
        }
        // And initialate the right amount of textures
        nTextures = num;
        if (nTextures > 0){
            textures = new ofFbo[nTextures];
        } else if ( nTextures == 0 ){
            textures = NULL;
        }
        
        // In any case it will allocate the total amount of textures with the internalFormat need
        for( int i = 0; i < nTextures; i++){
            doFbo(textures[i], width, height, internalFormat);
        }
    }
    
    //bool loaded;
    // Compile the shader and loadit to the GPU
    shader.unload();
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
    bFine = shader.linkProgram();
    return bFine;
}

// Allocates and cleans an ofFbo´s
void ofxShaderObj::doFbo(ofFbo & _fbo, int _width, int _height, int _internalformat) {
    _fbo.allocate(_width, _height, _internalformat);
    _fbo.begin();
    ofClear(0,255);
    _fbo.end();
}

// Draw a white box in order to let the final texture could be render
// It acts as a frame where the dst textures could rest.
// If you want to distort the points of a textures, probably here you want to re-define something
void ofxShaderObj::renderFrame(float _width, float _height){
    if (_width == -1) _width = width;
    if (_height == -1) _height = height;
    
    // If it´s not well compiled it will show an image little more gray.
    //
    if (bFine)
        ofSetColor(255,255);  
    else
        ofSetColor(150,255);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(_width, 0); glVertex3f(_width, 0, 0);
    glTexCoord2f(_width, _height); glVertex3f(_width, _height, 0);
    glTexCoord2f(0,_height);  glVertex3f(0,_height, 0);
    glEnd();
}
