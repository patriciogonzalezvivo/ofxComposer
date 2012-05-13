//
//  ofxBaseMask.h
//  ofxComposer-example
//
//  Created by Patricio González Vivo on 5/13/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#ifndef OFXBASEMASK
#define OFXBASEMASK

#include "ofxXmlSettings.h"

class ofxBaseMask : public ofPolyline {
public:
    ofxBaseMask();
    
    bool            loadXmlSettings(ofxXmlSettings &_XML);
    bool            saveXmlSettings(ofxXmlSettings &_XML);
    
    // Mask variables
    //
    ofxPingPong     maskFbo;
    ofShader        maskShader;
	int             selectedMaskCorner;
};


#endif
