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
        float maskOffset = 0;
    };

public:
    Drawing() {}
    Drawing(float ofWidth, float ofHeight, float vidWidth, vector<ofVideoPlayer>& vids);
    
    bool update(ofVec2f pos, bool hasIR);
    void draw(float x, float y, float w, float h, ofTexture* bgPtr=nullptr, ofTexture* synthPtr=nullptr);
    
    void start();
    void end();
    
protected:
    float _w, _h, _vidW;
    bool _bHasIR = false;
    vector<ofVideoPlayer*> _vids;
    vector<VidPath> _vidPaths;
    int _maxVidPaths = 1;
    
    ofPolyline _synthPath;
    bool _bSynthStart = false;
    float _synthExpandStart = 0;
    float _synthExpandWait = 6.0; // 6 seconds

    float _lastBgHue = 220;
    float _delThresh = 0.3; // 0.6 second before delete point
    float _lastIR = 0.0; // time of last IR signal
    float _IRTimeout = 1.5; // wait until IR signal drop
    
    ofShader _mask;
    ofFbo _vidFbo;
    ofFbo _maskFbo;
    
    ofFbo _synthMask; // mask
    ofFbo _synthDraw; // synth (to be masked)
    ofShader _alphaMask;
};