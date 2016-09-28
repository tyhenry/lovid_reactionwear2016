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
    
    bgVid = &vids[0];
    bgVid->setVolume(1);
    for (int i=1; i<vids.size(); i++){
        vids[i].stop();
        vids[i].setVolume(0);
        _vids.push_back(&vids[i]);
    }
    
    // make mini pyramid for coverage calc
    _miniPyramid.allocate(100,100/_w*_h,GL_RGBA);
    _miniPyramid.begin();
    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(0,0,_miniPyramid.getWidth(),_miniPyramid.getHeight());
    ofPopStyle();
    _miniPyramid.end();
    float mPW = _pyrW/_w*_miniPyramid.getWidth();
    float mPH = (_h-_pyrY)/_h*_miniPyramid.getHeight();
    _pyrArea = mPW*mPH*0.5; // mini pyramid area
    
    
    // draw pyramid mask texture
    ofFbo pyr;
    pyr.allocate(_w,_h,GL_RGBA);
    pyr.begin();
    ofClear(0);
    ofPushStyle();
    ofFill();
    ofDrawTriangle(_w*0.5,_pyrY,_w*0.5-_pyrW*0.5,_h,_w*0.5+_pyrW*0.5,_h);
    ofPopStyle();
    pyr.end();
    ofTexture pyrTex = pyr.getTexture();
    _pyrMask.allocate(_w,_h,GL_RGBA);
    _pyrMask = pyrTex;
    
    // set screen mask
    _screen.allocate(_w,_h,GL_RGBA);
    _screen.getTexture().setAlphaMask(_pyrMask);
    
    // invert shader setup
    _invert.load("","shaders/invert.frag");
    _bgFbo.allocate(_w,_h,GL_RGBA);
    
}

void Mapping::start(){
    bgVid->play();
    for (auto vid : _vids){
        vid->play();
    }
}

void Mapping::end(){
    bgVid->stop();
    for (auto vid : _vids){
        vid->stop();
    }
    _pyrVids.clear();
    
    _miniPyramid.begin();
    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(0,0,_miniPyramid.getWidth(),_miniPyramid.getHeight());
    ofPopStyle();
    _miniPyramid.end();
}

bool Mapping::update(ofVec2f pos, bool hasIR){
    
    if (hasIR){ // IR signal, add vid at pos
        
        // calc position/size of possible video
        
        pos.y = ofMap(pos.y,0,_h,_pyrY,_h,true); // map y to pyramid height
        float xRange = ofMap(pos.y, _pyrY,_h, 0,_pyrW,true); // get x range per y
        pos.x = ofMap(pos.x,0,_w,_w*0.5-xRange*0.5,_w*0.5+xRange*0.5,true); // map x to x range
        float vidW = ofMap(pos.y, _pyrY, _h, _minVW, _maxVW); // get vid width per y
        float vidH = vidW/1.77778; /*vid->getWidth()*vid->getHeight();*/ // calc vid height per vid width
        ofRectangle vidRect(pos-ofVec2f(vidW*0.5,vidH*0.5),vidW,vidH); // center vid on pos
        
        bool newVid = false;
        
        if (_pyrVids.size() < 1) newVid = true; // if no vids, make one
        else { // if vids, compare distance
            
            // determine if distance from last rect warrants new video
            ofRectangle& pVidRect = _pyrVids.back().second;
            float dist = vidRect.getCenter().distance(pVidRect.getCenter());
            
            if (dist > vidRect.height*0.5 + pVidRect.height*0.5) newVid = true;
        }
        
        if (newVid) { // add a video
        
            if (_curArea >= _pyrArea){ // pyramid full, next stage please
                ofLogNotice("Mapping") << "next stage please!";
                return true;
            }
            
            // select video
            ofVideoPlayer* vid;
            int idx = (int)ofRandom(1,_vids.size()); // random video
            if (idx >= _vids.size()) idx = _vids.size()-1; // rare edge case
            vid = _vids[idx];

            // switch sound
            vid->setVolume(1);
            // turn off other vids' sounds
            bgVid->setVolume(0);
            for (int i=0; i<_vids.size(); i++){
                if (i != idx) _vids[i]->setVolume(0);
            }
            
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
        
    }
    
    // update videos
    bgVid->update();
    for (auto pyrVid : _pyrVids){
        pyrVid.first->update();
    }
    
    _bHasIR = hasIR;
    return false;
}

void Mapping::draw(){
    // bg
    if (_bHasIR){
        _bgFbo.begin();
        ofClear(0);
        bgVid->draw(0,0,_w,_h);
        _bgFbo.end();
        _invert.begin();
        _invert.setUniform2f("u_resolution",_w,_h);
        _invert.setUniformTexture("u_tex0",_bgFbo.getTexture(),0);
        ofDrawRectangle(0,0,_w,_h); // draw shader
        _invert.end();
        //_bgFbo.draw(0,0,_w,_h);
    } else {
        bgVid->draw(0,0,_w,_h);
    }

    // pyramid
    _screen.begin();
    ofClear(0);
    for (auto pyrVid : _pyrVids){
        ofRectangle& r = pyrVid.second;
        pyrVid.first->draw(r.x,r.y,r.width,r.height);
    }
    _screen.end();
    _screen.draw(0,0,_w,_h);

}

void Mapping::drawMiniPyramid(float x, float y, float w, float h){
    _miniPyramid.draw(x,y,w,h);
    
    ofPushMatrix();
    ofTranslate(x,y);
    ofScale(w/_w,h/_h);
    ofPushStyle();
    ofSetColor(255,0,0);
    ofSetLineWidth(1);
    
    // pyramid outline
    ofDrawLine(_w*0.5,_pyrY,_w*0.5-_pyrW*0.5,_h);
    ofDrawLine(_w*0.5,_pyrY,_w*0.5+_pyrW*0.5,_h);
    
    // draw box outline
    ofNoFill();
    ofDrawRectangle(0,0,_w,_h);
    ofPopStyle();
    ofPopMatrix();
}