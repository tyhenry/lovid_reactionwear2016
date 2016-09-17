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
    screen.allocate(_w,_h, GL_RGBA);
    mask.allocate(_w,_h, GL_RGBA);
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
    
    // start outside vid
    _vids[Full]->setVolume(1);
    _vids[Full]->play();
}

bool Searching::update(ofVec2f pos, bool hasIR){
    
    if (bubble == Intimate) {
        if (hasIR){
            ofLogNotice("Searching") << "next stage please!";
            return true; // done, next stage
        }
    }
    else {
        // distance from center to create successive bubbles
        float maxHitRadius = _radii[bubble-1];
        float dist = pos.distance(ofVec2f(_w*0.5,_h*0.5));
        if (dist < maxHitRadius) nextBubble();
    }
    
    for (int b=Full; b>=bubble; b--){
        _vids[b]->update();
    }
    
    return false; // still searching
}

void Searching::draw(float x, float y, float w, float h){
    
    for (int b=Full; b>=bubble; b--){ // draw full -> intimate
        // note, the masking could be altered to scale the video:
        // - mask the video at radius of 1/2 video height
        // - then draw at height scale of 2*circle radius
        
        // here, masking is done without video scale (video must be size of screen):
        // - draw vid at screen size
        screen.begin();
        ofClear(0);
        _vids[b]->draw(0,0,_w,_h);
        screen.end();
        ofTexture sTex = screen.getTexture();
        // - set circle mask
        float inRad = b > 0 ? _radii[b-1] : 0;
        float outRad = _radii[b];
        sTex.setAlphaMask(circleMask(_w,_h,inRad,outRad));
        sTex.draw(x,y,w,h);
    }
}

int Searching::nextBubble() {
    if (bubble > Intimate){
        bubble--;
        ofLogNotice("Searching") << "next bubble, now: " << bubble;
        _vids[bubble]->play();
        _vids[bubble]->setVolume(1);
    }
    return bubble;
}

ofTexture& Searching::circleMask(float width, float height, float inRadius, float outRadius){
    mask.begin();
    ofClear(0);
    ofPopMatrix();
    ofTranslate(width*0.5,height*0.5);
    ofPushStyle();
    ofFill();
    ofSetColor(255);
    ofDrawCircle(0,0,outRadius);
    ofSetColor(0);
    ofDrawCircle(0,0,inRadius);
    ofPopStyle();
    ofPopMatrix();
    mask.end();
    return mask.getTexture();
}