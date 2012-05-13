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

#ifndef OFXSHADEROBJ
#define OFXSHADEROBJ

#include "ofMain.h"
#include "ofxPingPong.h"


class ofxShaderObj{
public:
    
    ofxShaderObj();
    ~ofxShaderObj();

    void allocate(int _width, int _height, int _internalFormat = -1);;
    
    ofxShaderObj& operator =(ofxShaderObj& parent);
    ofxShaderObj& operator >>(ofxShaderObj& parent) { parent.setTexture( getTextureReference() ); return * this;};
    ofxShaderObj& operator <<(ofxShaderObj& parent){ setTexture( parent.getTextureReference() ); return * this;};
    ofTexture&  operator[](int _nText){ if ((_nText < nTextures) && (_nText >= 0) ) return textures[_nText].getTextureReference(); };
    
    bool        setFragmentShader(string _fragShader);
    //bool        setVertexShader(string _vertShader);
    void        setPasses(int _passes) { passes = _passes; };
    void        setInternalFormat(int _internalFormat) { internalFormat = _internalFormat; doFragmentShader(); };
    void        setTexture(ofTexture& tex, int _texNum = 0);
    
    string      getFragmentShader() const { return fragmentShader; };
    //string      getVertexShader() const { return vertexShader;};
    int         getPasses() const { return passes; };
    int         getInternalFormat() const { return internalFormat; };
    int         getNumberOfTextures() const { return nTextures; };
    ofVec2f     getResolution() const { return ofVec2f(width,height);};
    ofTexture&  getTextureReference() const { return pingPong.dst->getTextureReference(); };
    
    bool        isOk() const{ return bFine; };
    
    void        clear(){ pingPong.clear(); } 
    
    void        update();
    void        draw(int x = 0, int y = 0, float _width = -1, float _height = -1);
    
protected:
    bool        doFragmentShader();
    //bool            doVertexShader();
    void        doFbo(ofFbo & _fbo, int _width, int _height, int _internalformat = GL_RGBA );
    void        renderFrame(float _width = -1, float _height = -1);
    
    ofxPingPong pingPong;
    ofFbo       *textures;
    ofShader    shader;
    string      fragmentShader, vertexShader;
    float       width, height;//, time;
    int         nTextures, passes, internalFormat;
    bool        bFine;
};

#endif
