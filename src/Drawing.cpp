//
//  Drawing.cpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/19/16.
//
//

#include "Drawing.hpp"

Drawing::Drawing(float ofWidth, float ofHeight, float vidWidth, vector<ofVideoPlayer>& vids):
_w(ofWidth), _h(ofHeight), _vidW(vidWidth)
{
    
    for (auto& vid : vids){
        vid.setVolume(0);
        _vids.push_back(&vid);
    }
    
    // mask shader setup
    _mask.load("","shaders/circleMask.frag");
    _maskFbo.allocate(640,480,GL_RGBA);
    _vidFbo.allocate(640,480,GL_RGBA);
    
    // synth mask shader
    _alphaMask.load("","shaders/alphaMask.frag");
    
}

bool Drawing::update(ofVec2f pos, bool hasIR){
    
    if (_bSynthStart) {
        float synthExpandTime = 0;
        
        // synth path going, add pos to synth path
        _synthPath.addVertex(pos);
        
        // check path length, start bg expansion if long enough
        float l = _synthPath.getPerimeter();
        if (l > 4000 && _synthExpandStart == 0) {
            _synthExpandStart = ofGetElapsedTimef(); // start expansion
            ofLogNotice("synthPath") << "starting synth expansion";
        } else {
            ofLogNotice("synthPath") << "perimeter = " << l;
        }
        
        _synthMask.begin();
        ofPushStyle();
        
        // draw bg
        if (_synthExpandStart==0){
            ofSetColor(0);
        } else {
            synthExpandTime = ofGetElapsedTimef()-_synthExpandStart;
            
            float brt = ofMap(synthExpandTime, 0,_synthExpandWait, 0,255, true);
            ofSetColor(brt);
        }
        ofDrawRectangle(0,0,_w,_h);
        
        // draw synth path circles
        auto& p = _synthPath.getVertices();
        ofSetColor(255);
        for (int i=0; i<p.size(); i++){
            float r = _vidW/1.7778 * ofRandom(0.95,1.05);
            ofDrawCircle(p[i], r);
        }
        
        ofPopStyle();
        _synthMask.end();
        
        if (synthExpandTime > _synthExpandWait) {
            return true;
        }
    }
    
    else {
        // check for new path or continued path
        
        bool newPath = false;
        bool continuePath = false;
        
        if (ofGetElapsedTimef() - _lastIR < _IRTimeout){ // continue path
            continuePath = true;
            if (hasIR) _lastIR = ofGetElapsedTimef();
        }
        else if (hasIR){ // new path
            newPath = true;
            _lastIR = ofGetElapsedTimef();
        }
        
        // create/continue vidPath
        
        if (newPath){ // new ir signal, new path
            if (_vidPaths.size() >= _maxVidPaths) _bSynthStart = true;
            else {
                // create vid path
                _vidPaths.push_back(VidPath());
                VidPath& path = _vidPaths.back();
                
                // load random video
                int i = ofRandom(_vids.size());
                if (i >= _vids.size()) i = _vids.size()-1;
                path.vid = _vids[i];
                if (!path.vid->isPlaying()) path.vid->play();
                
                // add start point to polyline
                path.path.addVertex(pos);
                path.delTime = ofGetElapsedTimef()+1.5;
                
                // mask wobble offset
                path.maskOffset = ofRandom(100);
            }
            
        } else if (continuePath) { // continued signal, continue current vid path
            
            if (_vidPaths.size() > 0){
                auto& vidPath = _vidPaths.back();

                if (!vidPath.dead) {
                    // try adding point, if distance is far enough
                    ofVec2f lastPt = vidPath.path.getVertices().back();
                    float dist = lastPt.distance(pos);
                    if (dist > 3) vidPath.path.addVertex(pos);
                    else ofLogNotice("vidPaths") << "didn't add point to path, dist = " << dist;
                }
            }
        }
    }
    
    // shrink vidPaths
    for (int i=0; i<_vidPaths.size(); i++){
        
        VidPath& vidPath = _vidPaths[i];
        
        if (!vidPath.dead){ // skip dead worms
        
            if (ofGetElapsedTimef() - vidPath.delTime > _delThresh){
                
                // delete a point
                
                // -- save pts
                deque<ofPoint> pts;
                for (auto it=vidPath.path.begin(); it!=vidPath.path.end(); ++it){
                    ofPoint pt = *it;
                    pts.push_back(pt);
                }
                // -- clear polyline
                vidPath.path.clear();
                if (pts.size() > 0) pts.pop_front(); // delete 1 point
                vidPath.delTime = ofGetElapsedTimef();
                
                // -- add points back to polyline, unless empty
                if (pts.size() == 0) {
                    vidPath.dead = true;
                    ofLogNotice("vidPaths") << "vidPath " << i << " is dead";
                } else {
                    for (auto& pt : pts){
                        vidPath.path.addVertex(pt);
                    }
                }
            }
        }
    }
    
    // adjust sound
    // 1 -- mute all
    for (auto vid : _vids){
        vid->setVolume(0);
    }
    // 2 -- turn on sound for playing vids
    for (auto& vidPath : _vidPaths){
        if (vidPath.dead) continue; // skip dead worms
        vidPath.vid->setVolume(1);
    }
    
    // update videos
    for (auto vid : _vids){
        vid->update();
    }
    
    _bHasIR = hasIR;
    if (_bSynthStart) _bHasIR = true;
    return false;
}

void Drawing::draw(float x, float y, float w, float h, ofTexture* bgPtr, ofTexture* synthPtr){
    
    //bg
    ofPushStyle();
    ofSetColor(255);
    if (_bHasIR){ // colorize
        float hue = ofRandom(150,240);
        hue = (_lastBgHue + hue) * 0.5;
        ofSetColor(ofColor::fromHsb(hue,255,255));
    }
    
    if (bgPtr != nullptr){ // draw kinect
        bgPtr->draw(x,y,w,h);
        cout << "drawing bg ptr" << endl;
    }
    else { // or draw solid color
        if (!_bHasIR) ofSetColor(0);
        ofDrawRectangle(x,y,w,h);
    }
    ofPopStyle();

    
    // draw vid paths
    for (auto& vidPath : _vidPaths){
        if (vidPath.dead) continue; // skip dead worms
        
        _vidFbo.begin();
        ofClear(0);
        vidPath.vid->draw(0,0,640,480);
        _vidFbo.end();
        
        _mask.begin();
        _mask.setUniform2f("u_resolution", 640, 480);
        _mask.setUniform1f("u_time", ofGetElapsedTimef()+vidPath.maskOffset);
        _mask.setUniformTexture("u_tex0", _vidFbo.getTexture(), 1);
        _maskFbo.begin();
        ofClear(0);
        ofDrawRectangle(0,0,640,480);
        _maskFbo.end();
        _mask.end();
    
        
        // loop through polyline points
        for (auto it=vidPath.path.begin(); it!=vidPath.path.end(); ++it){
        
            
            float vidH = _vidW/vidPath.vid->getWidth()*vidPath.vid->getHeight();
            _maskFbo.draw(*it-ofVec2f(_vidW*0.5,_vidW*0.5),_vidW,_vidW);
        }
    }
    
    // synth
    if (_bSynthStart){
        _synthDraw.begin();
        if (synthPtr!=nullptr){
            synthPtr->draw(0,0,_w,_h);
        } else {
            ofPushStyle();
            ofSetColor(0);
            ofDrawRectangle(0,0,_w,_h);
            ofPopStyle();
        }
        _synthDraw.end();
        
        _alphaMask.begin();
        _alphaMask.setUniformTexture("u_tex0", _synthDraw.getTexture(), 0);
        _alphaMask.setUniformTexture("u_tex1", _synthMask.getTexture(), 1);
        _alphaMask.setUniform2f("u_resolution", _w,_h);
        ofDrawRectangle(0,0,_w,_h);
        _alphaMask.end();
    }
    

}

void Drawing::start(){
    _synthPath.clear();
    _synthExpandStart = 0;
    _bSynthStart = false;

    // synth mask setup
    _synthMask.allocate(_w,_h,GL_RGBA);
    _synthMask.begin();
    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(0,0,_w,_h);
    ofPopStyle();
    _synthMask.end();

    _synthDraw.allocate(_w,_h,GL_RGBA);

}

void Drawing::end(){
    for (auto vid : _vids){
        vid->stop();
        _vidPaths.clear();
    }
}