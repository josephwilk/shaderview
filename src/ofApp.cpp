#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
//    bufferSize = 2048;
//    ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4);
    
    ofDisableArbTex();
    mTexture.allocate(512,2,GL_LUMINANCE, false);
   
    
    isShaderDirty = true;
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    ofBuffer dataBuffer;
    dataBuffer = ofBufferFromFile(ofToDataPath("wave.glsl"), false);
   
    string shaderTemplate = "#version 150\nuniform vec3 iResolution;\nuniform float iGlobalTime;\nuniform float iChannelTime[4];\nuniform vec3 iChannelResolution[4];\nuniform vec4 iMouse;\n";

    fft.setup(16384);
    
    shader.load(ofToDataPath("default.vert", true), ofToDataPath("wave.glsl", true));
}

//--------------------------------------------------------------
void ofApp::update(){
    fft.update();
    
    vector<float>& buffer = fft.getBins();
    unsigned char signal[1024];
    for(int i=0; i < 512; i++){
        float audioSig = fft.getAudio()[i];
        signal[512+i] = (unsigned char) audioSig;
    }
    mTexture.loadData(signal, 512, 2, GL_LUMINANCE);
    
    //beat.update(ofGetElapsedTimeMillis());
    if(isShaderDirty){
        shader.load(ofToDataPath("default.vert", true), ofToDataPath("wave.glsl", true));
        isShaderDirty = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    shader.begin();
    shader.setUniform1f("iGlobalTime", ofGetElapsedTimef());
    shader.setUniform3f("iResolution", ofGetWidth() , ofGetHeight(), 1 ) ;  
    shader.setUniformTexture("iChannel0", mTexture, 0);  
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    shader.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch (key) {
		case ' ':
			isShaderDirty = true;
			break;
		case OF_KEY_F11:
			ofToggleFullscreen();
			break;
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::audioReceived(float* input, int bufferSize, int nChannels) {
    //beat.audioReceived(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::onDirectoryWatcherItemModified(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
    isShaderDirty = true;
    ofLogNotice("Modified: " + evt.item.path());
}
