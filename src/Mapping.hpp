//
//  Mapping.hpp
//  lovid_perf_sep2016
//
//  Created by Tyler on 9/15/16.
//
//

#pragma once
#include "ofMain.h"

class Mapping{
    
public:
    typedef pair<ofVideoPlayer*, ofRectangle> PyrVid;
    
    Mapping(){};
    Mapping(float ofWidth, float ofHeight,
            float minVidWidth, float maxVidWidth, float pyramidY, float pyramidWidth,
            vector<ofVideoPlayer>& vids);
    
    void start();
    bool update(ofVec2f pos, bool hasIR);
    void draw();
    void drawMiniPyramid(float x, float y, float w, float h);

protected:
    float _w, _h, _minVW, _maxVW, _pyrY, _pyrW;
    vector<ofVideoPlayer*> _vids;
    vector<PyrVid> _pyrVids; // saves vid and location
    bool _bHasIR = true;
    bool _bFull = false;
    ofVec2f _lastPos;
    
    ofFbo _miniPyramid; // used to determine actual pyramid coverage speedily
    float _pyrArea = 0, _curArea = 0;
};