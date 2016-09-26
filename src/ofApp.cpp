#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofBackground(0);
    
    pos = ofVec2f(-1,-1); // off screen
    
    width = ofGetWidth();
    height = ofGetHeight();
    
    // load settings
    settings.loadFile("settings.xml");
    threshFar = settings.getValue("threshFar",100);
    threshNear = settings.getValue("threshNear",230);
    irWidth = settings.getValue("irWidth",100);
    irMinDist = settings.getValue("irMinDist",0);
    irMaxDist = settings.getValue("irMaxDist",100);
    pyrY = settings.getValue("pyrY",620);
    pyrWidth = settings.getValue("pyrWidth",1080);
    
    // init arduino
    serial.listDevices();
    serial.setup(0,115200);
    commander = Commander(&serial);
    bCmdConnected = commander.connect();
    
    // init kinect
    kinect.setRegistration(true);
    kinect.init(true);
    kinect.open();
    depthImg.allocate(kinect.width, kinect.height);
    depthNear.allocate(kinect.width, kinect.height);
    depthFar.allocate(kinect.width, kinect.height);
    depthThresh.allocate(kinect.width, kinect.height);
    
    roi.set(0,0,kinect.width,kinect.height-180);
    
    // record kinect video
    kinectFbo.allocate(1280,480,GL_RGB);
    kinectPix.allocate(1280,480,OF_PIXELS_RGB);
    
    string filename = "kinect_" +ofGetTimestampString()+ ".mov";
    
    vidRecorder.setFfmpegLocation(ofToDataPath("ffmpeg/ffmpeg"));
    
    vidRecorder.setVideoCodec("mpeg4");
    vidRecorder.setVideoBitrate("10000k");
    
    vidRecorder.setup(filename, 1280, 480, 30);
    vidRecorder.start();
    
    
    // load videos
    
    // searching vids
    ofDirectory searchingDir(ofToDataPath("searching"));
    searchingDir.allowExt("mp4");
    searchingDir.listDir();
    for (auto file : searchingDir.getFiles()){
        ofVideoPlayer vid;
        vid.load(file.path());
        vid.setLoopState(OF_LOOP_NORMAL); // loop
        searchingVids.push_back(vid);

    }
    
    // mapping vids
    ofVideoPlayer mapBgVid;
    mapBgVid.load("mapping/bg.mp4");
    mapBgVid.setLoopState(OF_LOOP_NORMAL);
    mappingVids.push_back(mapBgVid); // bg = vector[0]
    
    ofDirectory mappingDir(ofToDataPath("mapping"));
    mappingDir.allowExt("mp4");
    mappingDir.listDir();
    for (auto file : mappingDir.getFiles()){
        if (file.getFileName() != "bg.mp4"){
            ofVideoPlayer vid;
            vid.load(file.path());
            vid.setLoopState(OF_LOOP_NORMAL); // loop
            mappingVids.push_back(vid);
        }
    }
    
    // drawing vids
    ofDirectory drawingDir(ofToDataPath("drawing"));
    drawingDir.allowExt("mp4");
    drawingDir.listDir();
    for (auto file : drawingDir.getFiles()){
        ofVideoPlayer vid;
        vid.load(file.path());
        vid.setLoopState(OF_LOOP_NORMAL); // loop
        drawingVids.push_back(vid);
    }
    
    // stages
    searching = Searching(width, height, height*0.5-100, searchingVids);
        // width, height, max radius, vids
    mapping = Mapping(width, height, 20, width/5, 100, width-200, mappingVids);
        // width, height, min vid width, max vid width, pyramid y, pyramid width, vids
    drawing = Drawing(width, height, 100, drawingVids);
        // width, height, vid width, vids
    
    // gui
    gui.setup();
    gui.add(threshFar.set("kinect thresh far", threshFar, 0, 255));
    gui.add(threshNear.set("kinect thresh near", threshNear, 0, 255));
    gui.add(irWidth.set("IR beam width", irWidth, 0, 100));
    gui.add(irMinDist.set("IR min dist", irMinDist, 0, 100));
    gui.add(irMaxDist.set("IR max dist", irMaxDist, 0, 100));
    gui.add(pyrY.set("pyramid peak", pyrY, 0, 720));
    gui.add(pyrWidth.set("pyramid width", pyrWidth, 0, 1280));
    gui.add(bUseMouse.set("use mouse?", false));
    gui.add(bDrawKinect.set("draw kinect?", false));
    gui.add(bDrawPos.set("draw pos dot?", true));
    gui.add(bMapIRBounds.set("use IR bounds?", true));
    gui.add(bDrawIRBounds.set("draw IR bounds?", false));
    gui.add(bDrawPyramid.set("draw pyramid?", false));
    
    
    
    // start
    
    searching.start();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // get kinect pos
    updateKinect();
    
    if (bUseMouse){ // override kinect pos
        pos.x = ofGetMouseX();
        pos.y = ofGetMouseY();
    }
    
    if (bMapIRBounds){
        
        float irW = ofMap(irWidth,0,100,0,width);
        float irMinH = ofMap(irMinDist,0,100,height,0);
        float irMaxH = ofMap(irMaxDist,0,100,height,0);
        float irX = width*0.5-irW*0.5;
        float irY = irMaxH;
        float irH = irMinH-irMaxH;
        
        pos.x = ofMap(pos.x, irX,irX+irW, 0,width, true);
        pos.y = ofMap(pos.y, irY,irY+irH, 0,height, true);
        
    }
    
    // arduino comm
    if (bCmdConnected) {
        commander.update();
        char cmd = 'x'; unsigned long val = 0;
        while (commander.getNext(&cmd, &val)){} // get latest in serial queue and clear everything
        if (cmd == 'I') {
            if (val == 62500){
                IRbreak();
            } else {
                IRsignal();
                
            }
        }
    }
    
    // stages
    switch (stage) {
        case SearchingStage:{
            
            if (searching.update(pos,bHasIR)) nextStage();
            break;
        }
        case MappingStage:{
            
            if (mapping.update(pos,bHasIR)) nextStage();
            break;
        }
        case DrawingStage:{
            
            drawing.update(pos,bHasIR);
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
            if (bDrawPyramid) {
                float mW = width/5;
                float mH = mW/width*height;
                mapping.drawMiniPyramid(width-mW-20,20,mW,mH);
            }
            
            break;
        }
        case DrawingStage:{
            ofTexture* kTexPtr = nullptr;
            if (kinect.isConnected()) kTexPtr = &kinect.getTexture();
            drawing.draw(0,0,width,height,kTexPtr);
            
            break;
        }

        case SynthStage:{
            
            break;
        }
    }
    
    if (bDrawKinect){
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
    
    if (bDrawIRBounds){
        ofPushStyle();
        ofSetColor(255,0,0);
        ofNoFill();
        float irW = ofMap(irWidth,0,100,0,width);
        float irMinH = ofMap(irMinDist,0,100,height,0);
        float irMaxH = ofMap(irMaxDist,0,100,height,0);
        float irX = width*0.5-irW*0.5;
        float irY = irMaxH;
        float irH = irMinH-irMaxH;
        ofDrawRectangle(irX,irY,irW,irH);
        ofPopStyle();
    }
    
    if (bDrawPos){
        int hue = bHasIR ? 255 : 180;
        ofSetColor(ofColor().fromHsb(hue,255,255));
        int rad = bHasIR ? 15 : 10;
        ofDrawCircle(pos,rad);
        ofSetColor(255);
    }
    
    if (bDrawGui){
        gui.draw();
    }

}

void ofApp::exit(){
    if (vidRecorder.isRecording()) vidRecorder.close();
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::updateKinect(){
    kinect.update();
    if (kinect.isFrameNew()){
        
        // kinect recorder
        kinectFbo.begin();
        ofClear(0);
        kinect.draw(0,0,640,480);
        kinect.drawDepth(640,0,640,480);
        kinectFbo.end();
        kinectFbo.readToPixels(kinectPix);
        
        bool success = vidRecorder.addFrame(kinectPix);
        if (!success) cout << "failed to add vid frame to kinect recording" << endl;
        
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
    switch (stage){
        case SearchingStage:{
            searching.end();
            mapping.start();
            stage++;
            break;
        }
        case MappingStage:{
            mapping.end();
            drawing.start();
            stage++;
            break;
        }
        default:{
            break;
        }
    }
    ofLogNotice("nextStage") << "stage: " << stage;
}

//--------------------------------------------------------------
void ofApp::prevStage(){
    switch (stage){
        case MappingStage:{
            mapping.end();
            searching.start();
            stage--;
            break;
        }
        case DrawingStage:{
            drawing.end();
            mapping.start();
            stage--;
            break;
        }
        default:{
            break;
        }
    }
    ofLogNotice("prevStage") << "stage: " << stage;
}

void ofApp::saveSettings(){
    settings.setValue("threshFar",threshFar);
    settings.setValue("threshNear",threshNear);
    settings.setValue("irWidth",irWidth);
    settings.setValue("irMinDist",irMinDist);
    settings.setValue("irMaxDist",irMaxDist);
    settings.setValue("pyrY",pyrY);
    settings.setValue("pyrWidth",pyrWidth);
    settings.saveFile("settings.xml");
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
    else if (key == 'm' || key == 'M'){
        bUseMouse = !bUseMouse;
        string m = bUseMouse ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "using mouse pos: " << m;
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
        bDrawPyramid = !bDrawPyramid;
        string p = bDrawPyramid ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "drawing mini pyramid: " << p;
    }
    
    else if (key == 'g' || key == 'G'){
        bDrawGui = !bDrawGui;
        string g = bDrawGui ? "ON" : "OFF";
        ofLogNotice("keyReleased") << "drawing gui: " << g;
    }
    
    else if (key == 's' || key == 'S'){
        saveSettings();
    }

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
