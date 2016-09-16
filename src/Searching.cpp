//
//  Searching.cpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/14/16.
//
//

#include "Searching.hpp"

Searching::Searching(float ofWidth, float ofHeight, float radiusPublic, vector<ofVideoPlayer>& vids)
    :_w(ofWidth), _h(ofHeight)
{
    
    // load vids from vids vector
    for (int i=0; i<=Full; i++){
        _vids.push_back(&(vids[i]));
    }
    mask.allocate(_w,_h);
    ofSetCircleResolution(60);
    
    for (auto vid : _vids){
        vid->stop();
        vid->firstFrame();
        vid->setVolume(0);
    }
    
    // calc radii
    float radPer = radiusPublic/3;
    float radSoc = radPer*2;
    float radInt = radPer*0.5;
    float radFul = ofVec2f(0,0).distance(ofVec2f(_w*0.5,_w*0.5));
    _radii.push_back(radInt);
    _radii.push_back(radPer);
    _radii.push_back(radSoc);
    _radii.push_back(radiusPublic);
    _radii.push_back(radFul);
}

bool Searching::update(ofVec2f pos, bool hasIR){
    
    if (bubble == Full && hasIR) {
        ofLogNotice("Searching") << "next stage please!";
       return true; // done, next stage
    }
    
    // distance from center to create successive bubbles
    float minHitRadius = (bubble == -1) ? 0 : _radii[bubble];
    float maxHitRadius = _radii[bubble+1];
    
    float dist = pos.distance(ofVec2f(_w*0.5,_h*0.5));
                              
    if (dist >= minHitRadius && dist <= maxHitRadius) nextBubble();
    
    for (int b=0; b<=bubble; b++){
        _vids[b]->update();
    }
    
    return false; // still searching
}

void Searching::draw(float x, float y, float w, float h){
    
    for (int b=bubble; b>=0; b--){ // draw full -> intimate
        // note, the masking could be altered to scale the video
        // i.e.
        // - mask the video at radius of 1/2 video height
        // - then draw at height scale of 2*circle radius
        // here, masking is done without video scale (video must be size of screen):
        ofTexture vTex = _vids[b]->getTexture();
        vTex.setAlphaMask(circleMask(_radii[b]));
        vTex.draw(x,y,w,h);
    }
}

int Searching::nextBubble() {
    if (bubble < Full){
        bubble++;
        ofLogNotice("Searching") << "next bubble, now: " << bubble;
        _vids[bubble]->play();
        _vids[bubble]->setVolume(1);
    }
    return bubble;
}

ofTexture& Searching::circleMask(float radius){
    mask.begin();
    ofClear(0);
    ofPushStyle();
    ofSetColor(255);
    ofFill();
    ofDrawCircle(_w*0.5,_h*0.5,radius);
    ofPopStyle();
    mask.end();
    return mask.getTexture();
}