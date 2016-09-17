#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "Searching.hpp"
#include "Mapping.hpp"

class ofApp : public ofBaseApp{

public:
    
    bool bUseKinect = true;
    int threshFar = 100;
    int threshNear = 230;
    ofRectangle roi;
    bool bDrawPos = true, bDrawKinect = false;
    
    enum Stage { // performance stages
        SearchingStage,
        MappingStage,
        DrawingStage,
        TransitionStage,
        SynthStage
    };
    
    void setup();
    void update();
    void draw();
    
    void updateKinect();
    
    void IRsignal();
    void IRbreak();
    
    void nextStage();
    void prevStage();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    int stage = SearchingStage;
    Searching searching;
    Mapping mapping;
    
    vector<ofVideoPlayer> searchingVids;
    vector<ofVideoPlayer> mappingVids;
    
    bool bHasIR = false;
    ofVec2f pos;
    
    ofxKinect kinect;
    ofxCvGrayscaleImage depthImg;
    ofxCvGrayscaleImage depthFar;
    ofxCvGrayscaleImage depthNear;
    ofxCvGrayscaleImage depthThresh;
    ofxCvContourFinder contourFinder;
    int blobIndex = -1;
    ofVec2f centroid;

    
    float width, height;
    
    // stages data
    
    bool bDrawMiniPyramid = false;
    
};
