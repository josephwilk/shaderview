#include "ofApp.h"
#define STRINGIFY(A) #A

void ofApp::setup(){
    listeningOnPort = 9177;
    shaderErrored = false;
    showFreqGraph = false;
    ofDisableArbTex();

    editor.addCommand('a', this, &ofApp::toggleEditor);
    
    mainFrag    = "nil.glsl";
    currentAmp = 1.0;
    
    defaultVert = STRINGIFY(
                            uniform mat4 modelViewProjectionMatrix;
                            
                            void main(){
                                vec4 pos = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
                                gl_Position = pos;
                                gl_TexCoord[0] = gl_MultiTexCoord0;
                                gl_FrontColor = gl_Color;
                            }
    );
    
    defaultVert = "#version 120\n" + defaultVert;
    
    editor.loadFile(ofToDataPath(mainFrag, true), 1);
    editor.update();
    
	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(loadFileShader(ofToDataPath(mainFrag, true))));
    shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
    shader.linkProgram();
    
    receiver.setup(listeningOnPort);
    ofAddListener(receiver.onMessageReceived, this, &ofApp::onMessageReceived);
    
  //  ofSetVerticalSync(true);
 //   ofSetFrameRate(60);

    mTexture.allocate(512,2,GL_LUMINANCE, false);
   
    post.init(ofGetWidth(), ofGetHeight());
    post.setFlip(false);
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<VerticalTiltShifPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(false);
    post.createPass<LimbDarkeningPass>()->setEnabled(false);
    post.createPass<RGBShiftPass>()->setEnabled(false);
    
    isShaderDirty = true;
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    fft.setup(16384);
}

void ofApp::update(){
    fft.update();
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

    for(auto const &it1 : uniforms) {
        map<string,float>::iterator it = tickingUniforms.find(it1.first);
        if(it != tickingUniforms.end() && fabs(uniforms[it1.first]) > 0.001){
            uniforms[it1.first] -= decayRate[it1.first];
        }
        else{
            tickingUniforms.erase(it1.first);
            decayRate.erase(it1.first);
        }
    }
    
    if(isShaderDirty){
        string oldShader = shader.getShaderSource(GL_FRAGMENT_SHADER);
        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(loadFileShader(ofToDataPath(mainFrag, true))));
        shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
        if(!r){
            shader.setupShaderFromSource(GL_FRAGMENT_SHADER, oldShader);
            shaderErrored = true;
        }
        else{
            shaderErrored = false;
        }
        shader.linkProgram();
        isShaderDirty = false;
    }
}

void ofApp::draw(){
    fbo.begin();

    if(showFreqGraph){
        ofBackground(0, 0, 0);
        ofPushMatrix();
        ofTranslate(16, 16);
        ofSetColor(255);
        ofDrawBitmapString("Frequency Domain", 0, 0);
        plot(fft.getBins(), 128);
        ofPopMatrix();
        string msg = ofToString((int) ofGetFrameRate()) + " fps";
        ofDrawBitmapString(msg, ofGetWidth() - 80, ofGetHeight() - 20);
    }

    ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
    
    mTexture.bind();
    shader.begin();

    float mx = mouseX / (float)ofGetWidth();
    mx = ofClamp(mx, 0,1);
    float my = mouseY / (float)ofGetHeight();
    my = ofClamp(my, 0,1);
    shader.setUniform2f("iMouse", mx, my);

    shader.setUniform1f("iGlobalTime", ofGetElapsedTimef() );
    shader.setUniform3f("iResolution", ofGetWidth() , ofGetHeight(), 1) ;
    shader.setUniform1f("iVolume", currentAmp);
    for(auto const &it1 : uniforms) {
        shader.setUniform1f(it1.first, uniforms[it1.first]);
    }
    shader.setUniformTexture("iChannel0", mTexture, 0);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(0,0,0);
    glTexCoord2f(1,0); glVertex3f(ofGetWidth(),0,0);
    glTexCoord2f(1,1); glVertex3f(ofGetWidth(),ofGetHeight(),0);
    glTexCoord2f(0,1); glVertex3f(0,ofGetHeight(),0);
    glEnd();
    
    shader.end();
    mTexture.unbind();
    
    if (editorVisible) {
        editor.draw();
    }
    
    if (shaderErrored){
        string errorMsg = "ERROR";
        ofColor red(255, 0, 0);
        ofColor black(0, 0, 0);
        ofDrawBitmapStringHighlight(errorMsg, ofGetWidth() - 80, 20.0, red, black);
    }

    string msg = ofToString((int) ofGetFrameRate()) + " fps";
    ofDrawBitmapString(msg, ofGetWidth()-80, ofGetHeight()-20, 0);
   
    fbo.end();
    fbo.draw(0,0,ofGetWidth(), ofGetHeight());
}

void ofApp::keyReleased(int key){
    switch (key) {
		case ' ':
			isShaderDirty = true;
			break;
		case OF_KEY_F11:
			ofToggleFullscreen();
            isFullscreen= true;
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
        ofLogNotice("Uniform change. "+ uniformName + " => " +ofToString(uniforms[uniformName]));
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
        ofLogNotice("Smoothed Uniform change. "+ uniformName + " => " +ofToString(uniforms[uniformName]));
    }
    if(addr == "/decaying-uniform"){//An update to a Uniform which ticks back to 0 in the draw loop
        string uniformName  = msg.getArgAsString(0);
        float  uniformValue = msg.getArgAsFloat(1);
        if(msg.getNumArgs() > 2){
            float  rate = msg.getArgAsFloat(2);
            decayRate[uniformName] = rate;
        }
        else{
            decayRate[uniformName] = 0.01;
        }

        uniforms[uniformName] = uniformValue;
        tickingUniforms[uniformName] = uniformValue;
        ofLogNotice("Tick Uniform change. "+ uniformName + " => " +ofToString(tickingUniforms[uniformName]));
    }
    if(addr == "/shader-string"){ //Load a shader from a string
        string shaderString  = msg.getArgAsString(0);
        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(shaderString));
        shader.linkProgram();
    }
    if(addr == "/shader"){ //Load a new shader
        string shaderFile  = msg.getArgAsString(0);
        int found = shaderFile.find_last_of("/");
        string shaderPath = shaderFile.substr(0, found);
        found = mainFrag.find_last_of("/");
        string currentPath = shaderFile.substr(0, found);

        if(watcher.isWatching(currentPath)){
            watcher.removePath(currentPath);
        }
        if(!watcher.isWatching(shaderPath)){
            watcher.addPath(shaderPath, true, &fileFilter);
        }

        mainFrag = shaderFile;
        isShaderDirty = true;
    }
    if(addr == "/volume"){
        currentAmp = msg.getArgAsFloat(0);
    }
    if(addr == "/texture"){
        string textureFile  = msg.getArgAsString(0);
        int textureId  = msg.getArgAsInt32(1);
        if(textureId <= 3){
            ofImage image;  //TODO: Cleanup images if reloaded

            if(textureFile == "tex10" || textureFile == "tex11" ||
               textureFile == "tex15" || textureFile == "tex16"){
                image.loadImage(ofToDataPath("textures/"+textureFile + ".png", true));
            }
            else{
                image.loadImage(textureFile);
            }
            shader.setUniformTexture("iChannel"+ofToString(textureId), image.getTextureReference(), 1);
        }
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

string ofApp::loadFileShader(string path){
    ofFile file;
    
	if(!file.open(ofToDataPath(path), ofFile::ReadOnly)){
        return false;
	}
    string shaderText = file.readToBuffer().getText();
	file.close();
    return shaderText;
}

string ofApp::prepareShader(string shaderText){
    shaderText =  STRINGIFY(
                            uniform vec3 iResolution;
                            uniform float iGlobalTime;
                            uniform vec2 iMouse;
                            uniform vec4 iDate;
                            uniform float iVolume;
) + shaderText;
    
    shaderText = "#version 120\n" + shaderText;
    return shaderText;
}

void ofApp::toggleEditor(void * _o){
    ((ofApp *)_o)->editorVisible = !((ofApp *)_o)->editorVisible;
    ((ofApp *)_o)->editor.loadFile(ofToDataPath(((ofApp *)_o)->mainFrag, true), 1);
}

void ofApp::toggleEditorSave(){
    editor.saveFile(ofToDataPath(mainFrag, true), 1);
}

void ofApp::keyPressed(int key){
    bool alt   = (bool) (ofGetKeyPressed(OF_KEY_ALT));
    bool shift = (bool) (ofGetKeyPressed(OF_KEY_SHIFT));
    bool cmd   = (bool) (ofGetKeyPressed(OF_KEY_COMMAND));
    bool ctrl  = (bool) (ofGetKeyPressed(OF_KEY_CONTROL));
    if(cmd && key == 's'){
        toggleEditorSave();
    }
    else if(cmd && key == 'f'){
        ofToggleFullscreen();
        isFullscreen= true;

    }
}

void ofApp::mouseMoved(int x, int y ){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){ }
