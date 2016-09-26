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
    
    // invert shader setup
    //_mask.load("","shaders/circleMask.frag");
    //_maskFbo.allocate(_w,_h,GL_RGBA);
}

void Drawing::update(ofVec2f pos, bool hasIR){
    
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
        
    } else if (continuePath) { // continued signal, continue current vid path
        
        if (_vidPaths.size() > 0){
            auto& vidPath = _vidPaths.back();
            if (!vidPath.dead) vidPath.path.addVertex(pos);
        }
    }
    
    // update vid paths' ages (and shrink if old)
    for (auto& vidPath : _vidPaths){
        
        if (vidPath.dead) continue; // skip dead worms
        
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
            
            // -- add points back to polyline, unless empty
            if (pts.size() == 0) vidPath.dead = true;
            else {
                for (auto& pt : pts){
                    vidPath.path.addVertex(pt);
                }
                vidPath.delTime = ofGetElapsedTimef();
            }

        }
        

    }
    
    // update videos
    for (auto vid : _vids){
        vid->update();
    }
    
    _bHasIR = hasIR;

}

void Drawing::draw(float x, float y, float w, float h, ofTexture* bgPtr){
    
    if (_bHasIR){
        //bg
        float hue = ofRandom(150,240);
        hue = (_lastBgHue + hue) * 0.5;
        ofPushStyle();
        ofSetColor(ofColor::fromHsb(hue,255,220));
        if (bgPtr != nullptr){
            bgPtr->draw(x,y,w,h);
        }
        else ofDrawRectangle(x,y,w,h);
        ofPopStyle();
    }
    
    // draw vid paths
    for (auto& vidPath : _vidPaths){
        if (vidPath.dead) continue; // skip dead worms
        
        // loop through polyline points
        for (auto it=vidPath.path.begin(); it!=vidPath.path.end(); ++it){
            
//            _maskFbo.begin();
//            vidPath.vid->draw(0,0,_w,_h);
//            _maskFbo.end();
            
            float vidH = _vidW/vidPath.vid->getWidth()*vidPath.vid->getHeight();
//
//            _mask.begin();
//            _mask.setUniform2f("u_resolution", _w, _h);
//            _mask.setUniform1f("u_time", ofGetElapsedTimef());
//            _mask.setUniformTexture("u_tex0", _maskFbo.getTexture(), 4);
//            
//            ofDrawRectangle(*it-ofVec2f(_vidW*0.5,vidH*0.5),_vidW,vidH);
//            _mask.end();
            
//            _maskFbo.draw(*it-ofVec2f(_vidW*0.5,vidH*0.5),_vidW,vidH);
            vidPath.vid->draw(*it-ofVec2f(_vidW*0.5,vidH*0.5),_vidW,vidH);
        }
    }
}

void Drawing::start(){
    
}

void Drawing::end(){
    for (auto vid : _vids){
        vid->stop();
        _vidPaths.clear();
    }
}