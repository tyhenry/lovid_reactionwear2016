//
//  Mapping.cpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/15/16.
//
//

#include "Mapping.hpp"

Mapping::Mapping(float ofWidth, float ofHeight,
                 float minVidWidth, float maxVidWidth, float pyramidY, float pyramidWidth,
                 vector<ofVideoPlayer>& vids)
    :_w(ofWidth), _h(ofHeight), _minVW(minVidWidth), _maxVW(maxVidWidth), _pyrY(pyramidY), _pyrW(pyramidWidth)
{
    
    for (auto& vid : vids){
        _vids.push_back(&vid);
    }
    
    // make mini pyramid for coverage calc
    _miniPyramid.allocate(100,100/_w*_h,GL_RGBA);
    
}

void Mapping::start(){
    for (auto vid : _vids){
        vid->stop();
    }
    _miniPyramid.begin();
    ofClear(0);
    _miniPyramid.end();
    float mPW = _pyrW/_w*_miniPyramid.getWidth();
    float mPH = (_h-_pyrY)/_h*_miniPyramid.getHeight();
    _pyrArea = mPW*mPH*0.5; // mini pyramid area
}

bool Mapping::update(ofVec2f pos, bool hasIR){
    
    if (!_bHasIR && hasIR){ // new IR signal, add vid at pos
        
        if (_curArea >= _pyrArea){ // pyramid full, next stage please
            ofLogNotice("Mapping") << "next stage please!";
            return true;
        }
        
        // select video
        ofVideoPlayer* vid;
        int idx = (int)ofRandom(0,_vids.size()); // random video
        if (idx >= _vids.size()) idx = _vids.size()-1; // rare edge case
        vid = _vids[idx];
        vid->play();
        
        // calc position/size
        float xRange = ofMap(pos.y, _pyrY,_h, 0,_pyrW);
        pos.x = ofMap(pos.x,0,_w,_pyrW*0.5-xRange*0.5,_pyrW*0.5+xRange*0.5);
        float vidW = ofMap(pos.y, 0, _h, _minVW, _maxVW);
        float vidH = ofMap(vidW, 0, vid->getWidth(), 0, vid->getHeight());
        ofRectangle vidRect(pos-ofVec2f(vidW*0.5,vidH*0.5),vidW,vidH);
        
        // add to pyramid
        PyrVid pyrVid;
        pyrVid.first = vid;
        pyrVid.second = vidRect;
        _pyrVids.push_back(pyrVid);
        
        // get new current area coverage
        // - draw mini rectangle in miniPyramid
        ofVec2f miniPos;
        miniPos.x = ofMap(pos.x,0,_w,0,_miniPyramid.getWidth());
        miniPos.y = ofMap(pos.y,0,_h,0,_miniPyramid.getHeight());
        float miniW = ofMap(vidRect.width,0,_w,0,_miniPyramid.getWidth());
        float miniH = ofMap(vidRect.height,0,_h,0,_miniPyramid.getHeight());
        _miniPyramid.begin();
        ofPushStyle();
        ofSetColor(255);
        ofDrawRectangle(miniPos-ofVec2f(miniW*0.5,miniH*0.5),miniW,miniH);
        ofPopStyle();
        _miniPyramid.end();
        // - calc # white pixels in miniPyramid
        _curArea = 0;
        ofPixels miniPix;
        _miniPyramid.getTexture().readToPixels(miniPix);
        for (int y=0; y<miniPix.getHeight(); y++){
            for (int x=0; x<miniPix.getWidth(); x++){
                ofColor c = miniPix.getColor(x,y);
                if (c.getBrightness() > 128){
                    _curArea++;
                }
            }
        }
        ofLogNotice("Mapping") << "current coverage: " << _curArea << " : " << _pyrArea;
        
    }
    
    // update videos
    for (auto pyrVid : _pyrVids){
        pyrVid.first->update();
    }
    
    _bHasIR = hasIR;
    return false;
}

void Mapping::draw(){
    for (auto pyrVid : _pyrVids){
        ofRectangle& r = pyrVid.second;
        pyrVid.first->draw(r.x,r.y,r.width,r.height);
    }
}

void Mapping::drawMiniPyramid(float x, float y, float w, float h){
    _miniPyramid.draw(x,y,w,h);
    
    ofPushMatrix();
    ofTranslate(x,y);
    ofPushStyle();
    ofSetColor(255,0,0);
    ofSetLineWidth(1);
    
    // pyramid outline
    ofDrawLine(w*0.5,0,0,h);
    ofDrawLine(w*0.5,0,w,h);
    
    // draw box outline
    ofNoFill();
    ofDrawRectangle(0,0,w,h);
    ofPopStyle();
    ofPopMatrix();
}