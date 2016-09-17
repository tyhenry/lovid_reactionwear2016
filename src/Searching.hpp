//
//  Searching.hpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/14/16.
//
//

#pragma once
#include "ofMain.h"

class Searching{
public:
    enum Bubble {
        Intimate, // _vids 0
        Personal, // _vids 1
        Social, // vids 2
        Public, // vids 3
        Full // vids 4
    };
    Searching(){};
    Searching(float ofWidth, float ofHeight, float radiusPublic, vector<ofVideoPlayer>& vids);
    
    bool update(ofVec2f pos, bool hasIR); // returns true when done
    void draw(float x, float y, float w, float h);
    void draw(){ draw(0,0,_w,_h); }
    
    int nextBubble();
    
protected:
    float _w, _h;
    int bubble = Full;
    vector<ofVideoPlayer*> _vids;
    vector<float> _radii;
    
    ofFbo screen;
    ofFbo mask;
    ofTexture& circleMask(float width, float height, float inRadius, float outRadius); // makes a circle mask in mask fbo
    
};
