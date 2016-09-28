//
//  Drawing.hpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/19/16.
//
//

#pragma once
#include "ofMain.h"

class Drawing {

protected:
    struct VidPath {
        ofVideoPlayer* vid;
        ofPolyline path;
        float delTime = 0;
        bool dead = false;
    };

public:
    Drawing() {}
    Drawing(float ofWidth, float ofHeight, float vidWidth, vector<ofVideoPlayer>& vids);
    
    void update(ofVec2f pos, bool hasIR);
    void draw(float x, float y, float w, float h, ofTexture* bgPtr=nullptr);
    
    void start();
    void end();
    
protected:
    float _w, _h, _vidW;
    bool _bHasIR = false;
    vector<ofVideoPlayer*> _vids;
    vector<VidPath> _vidPaths;
    float _lastBgHue = 220;
    float _delThresh = 0.3; // 0.6 second before delete point
    float _lastIR = 0.0; // time of last IR signal
    float _IRTimeout = 1.5; // wait until IR signal drop
    
    ofShader _mask;
    ofFbo _vidFbo;
    ofFbo _maskFbo;
};