#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "ofxVideoRecorder.h"
#include "Commander.hpp"
#include "Searching.hpp"
#include "Mapping.hpp"
#include "Drawing.hpp"

class ofApp : public ofBaseApp{

public:
    
    void setup();
    void update();
    void draw();
    void exit();
    
    void updateKinect();
    
    void IRsignal();
    void IRbreak();
    
    void nextStage();
    void prevStage();
    
    void saveSettings();

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
    
    float width, height;
    
    // data
    bool bHasIR = false;
    ofVec2f pos;
    
    // stages
    enum Stage { // performance stages
        SearchingStage,
        MappingStage,
        DrawingStage,
        SynthStage
    };
    
    int stage = SearchingStage;
    Searching searching;
    Mapping mapping;
    Drawing drawing;
    
    
    // videos
    vector<ofVideoPlayer> searchingVids;
    vector<ofVideoPlayer> mappingVids;
    vector<ofVideoPlayer> drawingVids;
    
    // arduino
    ofSerial serial;
    Commander commander;
    bool bCmdConnected = false;
    
    // kinect
    ofxKinect kinect;
    ofxCvGrayscaleImage depthImg;
    ofxCvGrayscaleImage depthFar;
    ofxCvGrayscaleImage depthNear;
    ofxCvGrayscaleImage depthThresh;
    ofxCvContourFinder contourFinder;
    int blobIndex = -1;
    ofVec2f centroid;
    ofRectangle roi;
    
    // kinect recorder
    ofxVideoRecorder vidRecorder;
    ofFbo kinectFbo;
    ofPixels kinectPix;
    
    // gui
    ofxPanel gui;
    ofParameter<int> threshFar;
    ofParameter<int> threshNear;
    ofParameter<float> irWidth;
    ofParameter<float> irMinDist;
    ofParameter<float> irMaxDist;
    ofParameter<float> pyrY;
    ofParameter<float> pyrWidth;
    ofParameter<bool> bUseMouse;
    ofParameter<bool> bDrawKinect;
    ofParameter<bool> bDrawPos;
    ofParameter<bool> bMapIRBounds;
    ofParameter<bool> bDrawIRBounds;
    ofParameter<bool> bDrawPyramid;
    
    bool bDrawGui = false;
    
    // xml presets
    ofxXmlSettings settings;

};
