#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofBackground(0);
    
    pos = ofVec2f(-1,-1); // off screen
    
    if (bUseKinect){
        kinect.setRegistration(true);
        kinect.init();
        kinect.open();
        depthImg.allocate(kinect.width, kinect.height);
        depthNear.allocate(kinect.width, kinect.height);
        depthFar.allocate(kinect.width, kinect.height);
        depthThresh.allocate(kinect.width, kinect.height);
        
        roi.set(0,0,kinect.width,kinect.height-180);
    }
    
    width = ofGetWidth();
    height = ofGetHeight();
    
    // load videos
    ofDirectory dir(ofToDataPath(""));
    dir.allowExt("mp4");
    dir.listDir();
    for (auto file : dir.getFiles()){
        ofVideoPlayer vid;
        vid.load(file.path());
        vid.setLoopState(OF_LOOP_NORMAL); // loop
        vids.push_back(vid);
    }
    
    // stages
    searching = Searching(width, height, height*0.5-100, vids);
    mapping = Mapping(width, height, 20, width/5, 0, width, vids);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (bUseKinect) updateKinect();
    
    switch (stage) {
        case SearchingStage:{
            
            if (searching.update(pos,bHasIR)) {
                mapping.start();
                stage++;
            }
            
            
            break;
        }
        case MappingStage:{
            
            if (mapping.update(pos,bHasIR)) stage++;

            break;
        }
        case DrawingStage:{

            break;
        }
        case TransitionStage:{

            break;
        }
        case SynthStage:{

            break;
        }
    }


}

//--------------------------------------------------------------
void ofApp::draw(){
    
    switch (stage) {
        case SearchingStage:{
            
            searching.draw();
            
            break;
        }
        case MappingStage:{
            
            mapping.draw();
            if (bDrawMiniPyramid) {
                float mW = width/5;
                float mH = mW/width*height;
                mapping.drawMiniPyramid(width-mW-20,20,mW,mH);
            }
            
            break;
        }
        case DrawingStage:{
            
            break;
        }
        case TransitionStage:{
            
            break;
        }
        case SynthStage:{
            
            break;
        }
    }
    
    if (bUseKinect && bDrawKinect){
        ofPushMatrix();
        ofPushStyle();
        ofScale(0.4,0.4);
        // kinect
        kinect.drawDepth(0,0,kinect.width,kinect.height);
        
        // depth tresholded
        ofSetColor(0,225,0,50);
        depthThresh.draw(0,0);
        
        // roi
        ofSetColor(0,255,0);
        ofNoFill();
        ofDrawRectangle(roi);
        ofFill();
        
        // contours
        ofSetColor(255);
        contourFinder.draw();
        
        // blob centroid
        int hue = bHasIR ? 255 : 180;
        ofSetColor(ofColor().fromHsb(hue,255,255));
        int rad = bHasIR ? 7 : 5;
        ofDrawCircle(centroid, rad);
        
        ofPopStyle();
        ofPopMatrix();
        
    }
    if (bDrawPos){
        int hue = bHasIR ? 255 : 180;
        ofSetColor(ofColor().fromHsb(hue,255,255));
        int rad = bHasIR ? 15 : 10;
        ofDrawCircle(pos,rad);
        ofSetColor(255);
    }

}

//--------------------------------------------------------------
void ofApp::updateKinect(){
    kinect.update();
    if (kinect.isFrameNew()){
        
        // get kinect depth img
        depthImg.setFromPixels(kinect.getDepthPixels());
        
        // do thresholds
        depthFar = depthImg;
        depthNear = depthImg;
        depthFar.threshold(threshFar);
        depthNear.threshold(threshNear,true);
        
        cvAnd(depthNear.getCvImage(),depthFar.getCvImage(),depthThresh.getCvImage(),NULL);
        depthThresh.flagImageChanged();
        depthThresh.setROI(roi);
        
        // get blob pos
        contourFinder.findContours(depthThresh, 100, kinect.width*kinect.height*0.5, 3, false);
        
        if (contourFinder.blobs.size() > 0){
            // find biggest blob
            int bIdx = 0;
            for (int i=1; i<contourFinder.blobs.size(); i++){
                if (contourFinder.blobs[i].area > contourFinder.blobs[bIdx].area)
                    bIdx = i;
            }
            // get centroid of blob
            centroid.set(contourFinder.blobs[bIdx].centroid);
            
            // x pos = x pos mapped to screen
            pos.x = ofMap(centroid.x,0,kinect.width,0,width);

            // y pos = depth mapped to screen y within thresholds
            ofPixels dPix = depthImg.getPixels();
            float depth = dPix.getColor(centroid.x,centroid.y).getBrightness();
            pos.y = ofMap(depth,threshFar,threshNear,0,height);
        }
        depthThresh.resetROI();
    }
}

//--------------------------------------------------------------
void ofApp::IRsignal(){
    if (!bHasIR) ofLogNotice() << "IR signal";
    bHasIR = true;
}

//--------------------------------------------------------------
void ofApp::IRbreak(){
    if (bHasIR) ofLogNotice() << "IR break";
    bHasIR = false;
}

//--------------------------------------------------------------
void ofApp::nextStage(){
    if (stage < SynthStage) stage++;
    ofLogNotice("nextStage") << "stage: " << stage;
}
//--------------------------------------------------------------
void ofApp::prevStage(){
    if (stage > SearchingStage) stage--;
    ofLogNotice("prevStage") << "stage: " << stage;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // simulate IR signal
    if (key == ' '){
        IRsignal();
    }
    
    // change thresholds
    else if (key == '+' || key == '='){
        ofLogNotice("keyPressed") << "threshFar: " << ++threshFar;
    }
    else if (key == '-' || key == '_'){
        ofLogNotice("keyPressed") << "threshFar: " << --threshFar;
    }
    else if (key == ')' || key == '0'){
        ofLogNotice("keyPressed") << "threshNear: " << ++threshNear;
    }
    else if (key == '(' || key == '9'){
        ofLogNotice("keyPressed") << "threshNear: " << --threshNear;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    // simulate IR break
    if (key == ' '){
        IRbreak();
    }
    
    // use kinect
    else if (key == 'k'){
        bUseKinect = !bUseKinect;
        string k = bUseKinect ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "using kinect: " << k;
    }
    // draw kinect
    else if (key == 'd' || key == 'D'){
        bDrawKinect = !bDrawKinect;
        string d = bDrawKinect ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "drawing kinect: " << d;
    }
    // draw pos
    else if (key == 'p' || key == 'P'){
        bDrawPos = !bDrawPos;
        string p = bDrawPos ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "drawing pos: " << p;
    }
    
    // prev/next stage
    else if (key == OF_KEY_LEFT){
        prevStage();
    }
    else if (key == OF_KEY_RIGHT){
        nextStage();
    }
    
    // draw mini pyramid
    else if (key == 'y' || key == 'Y'){
        bDrawMiniPyramid = !bDrawMiniPyramid;
        string p = bDrawMiniPyramid ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "drawing mini pyramid: " << p;
    }

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    if (!bUseKinect){
        pos.set(x,y);
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
