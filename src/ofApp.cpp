#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    isShaderDirty = true;
    ofSoundStreamSetup(0, 1, this, 44100, beat.getBufferSize(), 4);
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    ofBuffer dataBuffer;
    dataBuffer = ofBufferFromFile(ofToDataPath("wave.glsl"), false);
   
    std::string shaderTemplate = "#version 150\nuniform vec3 iResolution;\nuniform float iGlobalTime;\nuniform float iChannelTime[4];\nuniform vec3 iChannelResolution[4];\nuniform vec4 iMouse;\n"
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    }else{
        shader.load("shadersGL2/shader");
    }
#endif
}

//--------------------------------------------------------------
void ofApp::update(){
    beat.update(ofGetElapsedTimeMillis());
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    shader.begin();
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
		case 'f':
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
    beat.audioReceived(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::onDirectoryWatcherItemModified(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
    isShaderDirty = true;
    ofLogNotice("Modified: " + evt.item.path());
}
