#include "ofApp.h"

void ofApp::setup(){
    defaultVert = "default.vert";
    mainFrag    = "wave.glsl";
    currentAmp = 0.0;
    
    receiver.setup(9177);
    ofAddListener(receiver.onMessageReceived, this, &ofApp::onMessageReceived);
    
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    
    ofDisableArbTex();
    mTexture.allocate(512,2,GL_LUMINANCE, false);
   
    w = ofGetWidth();
    h = ofGetHeight();
    
    fbo.allocate(ofGetWidth(),ofGetHeight(), GL_RGB);
    fbo.begin();
    ofClear(0,0,0,0);
    fbo.end();

    isShaderDirty = true;
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    shader.load(ofToDataPath(defaultVert, true), ofToDataPath(mainFrag, true));
    
    fft.setup(16384);
}

void ofApp::update(){
    fft.update();
    
    currentAmp = *(fft.fft->getAmplitude());
    if(currentAmp != currentAmp){
        currentAmp = 0.0;
    }
    
    vector<float>& buffer = fft.getBins();
    
    unsigned char signal[1024];
    for (int i = 0; i < 512; i++) {
        signal[i] = (unsigned char) (buffer.at(i)*255); // FFT
    }
    for (int i = 0; i < 512; i++) {
        float audioSig = fft.getAudio()[i];
        audioSig = (audioSig * 0.5 + 0.5) * 254;
        signal[512+i] = (unsigned char) audioSig;
    }
    mTexture.loadData(signal, 512, 2, GL_LUMINANCE);
    //beat.update(ofGetElapsedTimeMillis());

    if(isShaderDirty){
        shader.load(ofToDataPath(defaultVert, true), ofToDataPath(mainFrag, true));
        isShaderDirty = false;
    }
}

void ofApp::draw(){
    fbo.begin();

    ofBackground(0, 0, 0);
    
    ofPushMatrix();
    ofTranslate(16, 16);
    ofSetColor(255);
    ofDrawBitmapString("Frequency Domain", 0, 0);
    plot(fft.getBins(), 128);
    ofPopMatrix();
    string msg = ofToString((int) ofGetFrameRate()) + " fps";
    ofDrawBitmapString(msg, ofGetWidth() - 80, ofGetHeight() - 20);
    
    mTexture.bind();
    shader.begin();
    shader.setUniform1f("iGlobalTime", ofGetElapsedTimef() );
    shader.setUniform3f("iResolution", ofGetWidth() , ofGetHeight(), 1 ) ;
    shader.setUniform1f("iVolume", currentAmp) ;
    for(auto const &it1 : uniforms) {
        shader.setUniform1f(it1.first, uniforms[it1.first]);
    }
    shader.setUniformTexture("iChannel0", mTexture, 0);
    
    
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(0,0,0);
    glTexCoord2f(1,0); glVertex3f(w,0,0);
    glTexCoord2f(1,1); glVertex3f(w,h,0);
    glTexCoord2f(0,1); glVertex3f(0,h,0);
    glEnd();
    
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    
    shader.end();
    mTexture.unbind();
    fbo.end();

    //mTexture.draw(0,0,ofGetWidth(),ofGetHeight());
    fbo.draw(0,0,ofGetWidth(), ofGetHeight());
}

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

void ofApp::onMessageReceived(ofxOscMessage &msg){
    string addr = msg.getAddress();
    ofLogNotice("OSC message, addr:",addr);
    
    if(addr == "/uniform"){//An update on a Uniform variable
        string uniformName  = msg.getArgAsString(0);
        float  uniformValue = msg.getArgAsFloat(1);
        uniforms[uniformName] = uniformValue;
        ofLogNotice("Uniform change. "+ uniformName + " => " +ofToString(uniforms["iExample"]));
    }
    if(addr == "/smoothed-uniform"){//An update to a Uniform but smoothed
        string uniformName  = msg.getArgAsString(0);
        float  uniformValue = msg.getArgAsFloat(1);
        float smoothRate = 0.05;

        if(fabs(uniforms[uniformName]-uniformValue) < 0.001){}
        else if(uniforms[uniformName] > uniformValue){
            uniforms[uniformName] -= smoothRate;
        }
        else if(uniforms[uniformName] < uniformValue){
            uniforms[uniformName] += smoothRate;
        }
        ofLogNotice("Smoothed Uniform change. "+ uniformName + " => " +ofToString(uniforms["iExample"]));
    }

}

void ofApp::onDirectoryWatcherItemModified(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
    isShaderDirty = true;
    ofLogNotice("Modified: " + evt.item.path());
}

void ofApp::plot(vector<float>& buffer, float scale) {
    ofNoFill();
    int n = MIN(1024, buffer.size());
    ofRect(0, 0, n, scale);
    ofPushMatrix();
    ofTranslate(0, scale);
    ofScale(1, -scale);
    ofBeginShape();
    for (int i = 0; i < n; i++) {
        ofVertex(i, buffer[i]);
    }
    ofEndShape();
    ofPopMatrix();
}

void ofApp::keyPressed(int key){}
void ofApp::mouseMoved(int x, int y ){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){ }
