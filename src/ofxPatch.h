//
//  ofxPatch.h
//  emptyExample
//
//  Created by Patricio Gonzalez Vivo on 3/9/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#ifndef OFXPATCH
#define OFXPATCH

#include "ofMain.h"

#include "ofxXmlSettings.h"

#include "ofxTitleBar.h"
#include "ofxShaderObj.h"
#include "ofxPingPong.h"

struct LinkDot{
    LinkDot(){
        to = NULL;
        toShader = NULL;
        nTex = 0;
    }
    
    ofPoint     pos;
    LinkDot     *to;
    int         nTex;
    int         toId;
    ofxShaderObj *toShader;
};

class ofxPatch{
public:
    
    ofxPatch();
    ~ofxPatch();

    bool            loadFile(string _filePath, string _configFile = "none");
    bool            loadType(string _type, string _configFile = "none");
    
    bool            loadSettings(int _nTag, string _configFile = "none");
    bool            saveSettings(string _configFile = "none");

    void            setFrag( string _code);
    //void          setVert( string _code);
    void            setMask(ofPolyline& _polyLine){ maskCorners = _polyLine; bMasking = true; bUpdateMask = true; };
    void            setCoorners(ofPoint _coorners[4]);
    void            setTexture(ofTexture& tex, int _texNum = 0);
    
    int             getId() const { return nId; };
    ofPoint         getPos() const { return ofPoint(x,y); };
    string          getType() const { return (shader != NULL)? "ofShader" : type; };
    ofPoint         getSurfaceToScreen(ofPoint _pos){ return surfaceToScreenMatrix * _pos; };
    ofPoint         getScreenToSurface(ofPoint _pos){ return screenToSurfaceMatrix * _pos; };
    GLfloat*        getGlMatrix() { return glMatrix; };
    string          getFrag();
    //string        getVert();
    ofTexture&      getTextureReference();
    ofxShaderObj*   getShader(){ if (getType() == "ofShader") return shader; else return NULL; };
    ofPoint&        getOutPutPosition(){ return outPutPos; };
    
    void            move(ofPoint _pos);
    void            scale(float _scale);
    void            rotate(float _angle);
    
    void            update();
    void            draw();
    
    bool            isOver(ofPoint _pos);//{ return textureCorners.inside(_pos); };
    
    vector<LinkDot> outPut;
    vector<LinkDot> inPut;
    
    ofxTitleBar     *title;
    
    bool            bActive;
    bool            bEditMode;
    bool            bEditMask;
    bool            bVisible;
    
private:
    void            doSurfaceToScreenMatrix();      // Update the SurfaceToScreen transformation matrix
    void            doScreenToSurfaceMatrix();      // Update the ScreenToSurface transformation matrix
    void            doGaussianElimination(float *input, int n); // This is used making the matrix
    
    // Mouse & Key Events ( it´s not better if is centralized on the composer )
    //
    void            _mousePressed(ofMouseEventArgs &e);
    void            _mouseDragged(ofMouseEventArgs &e);
    void            _mouseReleased(ofMouseEventArgs &e);
    void            _keyPressed(ofKeyEventArgs &e); 
    void            _reMakeFrame( int &_nId );
    
    
    // 5 Sources Objects and one interface to rule them all
    //
    ofTexture&      getSrcTexture();
    
    ofImage         *image;
    ofVideoPlayer   *videoPlayer;
    ofVideoGrabber  *videoGrabber;
    ofxShaderObj    *shader;
    ofTexture       *texture;
    
    // Mask variables
    //
    ofxPingPong     maskFbo;
    ofShader        maskShader;
    ofPolyline      maskCorners;
	int             selectedMaskCorner;
    
    // Texture varialbes
    //
    ofPolyline      textureCorners;
    int             selectedTextureCorner;
    int             textureWidth, textureHeight;
    
    ofPoint         src[4];
    ofMatrix4x4     surfaceToScreenMatrix;
    ofMatrix4x4     screenToSurfaceMatrix;
    GLfloat         glMatrix[16];
    
    // General Variables
    //
    ofRectangle     box;
    ofColor         color;
    ofPoint         outPutPos;
    string          configFile;
    string          filePath;
    string          type;
    float           x, y;
    float           width, height;
    float           texOpacity, maskOpacity;
    int             nId;
    
    bool            bMasking;
    bool            bUpdateMask;
    bool            bUpdateCoord;
};

#endif
