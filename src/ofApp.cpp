#include "ofApp.h"
#define STRINGIFY(A) #A

void ofApp::setup(){
    clearErrorLog();
    ofSetLogLevel(OF_LOG_VERBOSE);
    //ofLogToFile("errors.log", true);
    
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    vertexType = GL_POINTS;
    vertexCount = 0;
    listeningOnPort = 9177;
    shaderErrored = false;
    showFreqGraph = false;
    isFullscreen = true;
    isVertexDirty = false;
    rBackground = 0;
    gBackground = 0;
    bBackground = 0;
    
    postFxMode = false;
    
    cameraMode = false;
    
    textString = "";
    textStringWidth = ofGetWidth()/2.0;
    textStringHeight = ofGetHeight()/2.0;
    
    renderSmallFont.loadFont("monof55.ttf", 30, true, true, true);
    renderFont.loadFont("monof55.ttf", 700, true, true, true);

    post.init(ofGetWidth(), ofGetHeight());
    
    ofDisableArbTex();
    
    editor.addCommand('e', this, &ofApp::toggleErrors);
    editor.addCommand('a', this, &ofApp::toggleEditor);

    mainFrag    = "wave.glsl";
    mainVert    = "default.vert";
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
    editor.loadFile(ofToDataPath(mainVert, true), 2);

    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(loadFileShader(ofToDataPath(mainFrag, true))));
    shader.setupShaderFromSource(GL_VERTEX_SHADER,   prepareVertex(loadFileShader(ofToDataPath(mainVert, true))));
    shader.linkProgram();
    
    receiver.setup(listeningOnPort);
    //ofAddListener(receiver.onMessageReceived, this, &ofApp::onMessageReceived);
    
    //  ofSetVerticalSync(true);
    //   ofSetFrameRate(60);
    
    mTexture.allocate(512,2,GL_LUMINANCE, false);

    
    isShaderDirty = true;
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    fft.setup(16384);
}

void ofApp::update(){
    if (cameraMode){
        cam.update();
    }
    
    if (receiver.hasWaitingMessages()){
        ofxOscMessage msg;
        receiver.getNextMessage(msg);
        ofApp::onMessageReceived(msg);
    }
    
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
        map<string,float>::iterator it  = growingUniforms.find(it1.first);
        map<string,float>::iterator itD = tickingUniforms.find(it1.first);
        if(itD == tickingUniforms.end()){
            if(it != growingUniforms.end()){
                if(fabs(uniforms[it1.first]) < growingUniforms[it1.first]){
                    uniforms[it1.first] += growthRate[it1.first];
                }
                else{
                    tickingUniforms[it1.first] = uniforms[it1.first];
                    growingUniforms.erase(it1.first);
                    growthRate.erase(it1.first);
                }
            }
        }
    }
    
    for(auto const &it1 : uniforms) {
        map<string,float>::iterator it = tickingUniforms.find(it1.first);
        map<string,float>::iterator itD  = growingUniforms.find(it1.first);
        if(itD == growingUniforms.end()){
            if(it != tickingUniforms.end()){
                if(fabs(uniforms[it1.first]) > 0.001){
                    uniforms[it1.first] -= decayRate[it1.first];
                }
                else{
                    tickingUniforms.erase(it1.first);
                    decayRate.erase(it1.first);
                }
            }
        }
    }
    
    if(isVertexDirty){
        clearErrorLog();
        //NOTE: While we don't need to do this, it seems to speed up the vertex changes reload time... So for speed of update
        bool r2 = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(loadFileShader(ofToDataPath(mainFrag, true))));
        bool r = shader.setupShaderFromSource(GL_VERTEX_SHADER, prepareVertex(loadFileShader(ofToDataPath(mainVert, true))));
        
        if(!r){
            shaderErrored = true;
        }
        else{
            shaderErrored = false;
        }
        shader.linkProgram();
        isVertexDirty = false;
        editor.loadFile(ofToDataPath("errors.log", true), 0);
        editor.update();
    }
    if(isShaderDirty){
        clearErrorLog();

        if(!shaderErrored){
          oldShader = shader.getShaderSource(GL_FRAGMENT_SHADER);
        }

        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(loadFileShader(ofToDataPath(mainFrag, true))));
        shader.setupShaderFromSource(GL_VERTEX_SHADER, prepareVertex(loadFileShader(ofToDataPath(mainVert, true))));
        if(!r){
            shader.setupShaderFromSource(GL_FRAGMENT_SHADER, oldShader);
            shaderErrored = true;
        }
        else{
            shaderErrored = false;
        }
        shader.linkProgram();
        isShaderDirty = false;
        editor.loadFile(ofToDataPath("errors.log", true), 0);
        editor.update();
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
    
    //ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
    
    mTexture.bind();
    if(postFxMode){
        post.begin();
    }
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
    
    ofBackground(rBackground,gBackground,bBackground);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(0,0,0);
    glTexCoord2f(1,0); glVertex3f(ofGetWidth(),0,0);
    glTexCoord2f(1,1); glVertex3f(ofGetWidth(),ofGetHeight(),0);
    glTexCoord2f(0,1); glVertex3f(0,ofGetHeight(),0);
    glEnd();

    if(!shaderErrored){
      glDrawArrays(vertexType, 0, vertexCount);
    }

    if (cameraMode){
        cam.draw(0, 0);
    }

    shader.end();
    mTexture.unbind();
    if(postFxMode){
        post.end();
    }

  if (editorVisible) {
    ofSetOrientation(OF_ORIENTATION_DEFAULT, true);
    editor.draw();
    //editor.update();
  }
  else{
    ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
  }


    renderSmallFont.drawString(textSmallString, textStringWidth, textStringHeight);
    renderFont.drawString(textString, textStringWidth, textStringHeight);
    
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
        case OF_KEY_F11:
            ofToggleFullscreen();
            ofHideCursor();
            isFullscreen= true;
            break;
        case OF_KEY_F12:
            if(cameraMode== true){
                cam.close();
                cameraMode = false;
            }
            else{
                cam.setup(1280, 720);
                //cam.listDevices();
                cam.setDeviceID(0);
                cameraMode = true;
            }
        default:
            break;
    }
}

void ofApp::onMessageReceived(ofxOscMessage &msg){
    string addr = msg.getAddress();
    ofLogNotice("OSC message, addr: ",addr);
    
    if(addr == "/background"){//An update for the ofbackground color in draw cycle.
        rBackground = msg.getArgAsInt32(0);
        bBackground = msg.getArgAsInt32(1);
        gBackground = msg.getArgAsInt32(2);
        ofLogNotice("Background change.");
    }
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
    if(addr == "/curve-uniform"){ //experiment
        string uniformName  = msg.getArgAsString(0);
        float  uniformValue = msg.getArgAsFloat(1);
        if(msg.getNumArgs() > 2){
            float  rate = msg.getArgAsFloat(2);
            growthRate[uniformName] = rate;
        }
        else{
            growthRate[uniformName] = 0.01;
        }
        if(msg.getNumArgs() > 3){
            float rate = msg.getArgAsFloat(3);
            decayRate[uniformName] = rate;
        }
        else{
            decayRate[uniformName] = 0.01;
        }
        
        uniforms[uniformName] = 0.0;
        growingUniforms[uniformName] = uniformValue;
        tickingUniforms.erase(uniformName);
        ofLogNotice("Curve Uniform change. "+ uniformName + " => " +ofToString(growingUniforms[uniformName]));
    }
    
    if(addr == "/growing-uniform"){//experiment , like smoothed but auto grows to n
        string uniformName  = msg.getArgAsString(0);
        float  uniformValue = msg.getArgAsFloat(1);
        if(msg.getNumArgs() > 2){
            float  rate = msg.getArgAsFloat(2);
            growthRate[uniformName] = rate;
        }
        else{
            growthRate[uniformName] = 0.01;
        }
        
        uniforms[uniformName] = 0.0;
        growingUniforms[uniformName] = uniformValue;
        tickingUniforms.erase(uniformName);
        ofLogNotice("Growing Uniform change. "+ uniformName + " => " +ofToString(growingUniforms[uniformName]));
    }
    if(addr == "/shader-string"){ //Load a shader from a string
        string shaderString  = msg.getArgAsString(0);
        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(shaderString));
        shader.linkProgram();
    }
    if(addr == "/shader"){ //Load a new shader
        string shaderFile  = msg.getArgAsString(0);
        
        if(msg.getNumArgs() >= 2){
            mainVert    = msg.getArgAsString(1);
            
            if(msg.getNumArgs() >=3){
                vertexType  = toVertexType(msg.getArgAsString(2));
            }
            else{
                vertexType  = GL_POINTS;
            }
            
            if(msg.getNumArgs() >= 4){
                vertexCount = msg.getArgAsInt32(3);
            }
            else{
                vertexCount = 0;
            }
            
            if(editorVisible){
                editor.loadFile(ofToDataPath(mainVert, true), 2);
                editor.update();
            }

        }
        else{
            mainVert = "default.vert";
        }
        
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
        
        if(editorVisible){
            editor.loadFile(ofToDataPath(mainFrag, true), 1);
            editor.update();
        }
        
        isShaderDirty = true;
    }
    if(addr == "/vertex"){
        string vertFile  = msg.getArgAsString(0);
        string type  = msg.getArgAsString(1);
        int count  = msg.getArgAsInt32(2);
        
        int found = vertFile.find_last_of("/");
        string vertPath = vertFile.substr(0, found);
        found = mainVert.find_last_of("/");
        string currentPath = vertFile.substr(0, found);
        
        if(watcher.isWatching(currentPath)){
            watcher.removePath(currentPath);
        }
        if(!watcher.isWatching(vertPath)){
            watcher.addPath(vertPath, true, &fileFilter);
        }
        
        vertexType = toVertexType(type);
        vertexCount = count;
        mainVert = vertFile;
        isVertexDirty = true;
    }
    if(addr == "/vertex-settings"){
        string type  = msg.getArgAsString(0);
        int count  = msg.getArgAsInt32(1);
        vertexType = toVertexType(type);
        vertexCount = count;
    }
    if(addr == "/volume"){
        currentAmp = msg.getArgAsFloat(0);
    }
    if(addr == "/texture"){
        string textureFile  = msg.getArgAsString(0);
        int textureId;
        if(msg.getNumArgs() == 1){
            textureId = 1;
        }
        else{
            textureId  = msg.getArgAsInt32(1);
        }
        if(textureId <= 3){
            ofImage image;  //TODO: Cleanup images if reloaded
            
            if(textureFile == "tex10.png" || textureFile == "tex11.png" ||
               textureFile == "tex15.png" || textureFile == "tex16.png"){
                textureFile = ofToDataPath("textures/" + textureFile, true);
            }
            string channel = "iChannel"+ofToString(textureId);
            image.loadImage(textureFile);
            ofLogNotice("Loading to ["+channel+"] texture: ["+ textureFile + "]");
            shader.setUniformTexture(channel, image.getTextureReference(), 1);
        }
    }
    if(addr == "/echo"){
        textString = msg.getArgAsString(0);
        
        if(msg.getNumArgs() == 1){
            textStringWidth = ofGetWidth()/2.0;
            textStringHeight = ofGetHeight()/2.0;
        }
        
        if(msg.getNumArgs() > 1){
            textStringWidth = msg.getArgAsInt(1);
            textStringHeight = ofGetHeight()/2.0;
        }
        if(msg.getNumArgs() > 2){
            textStringHeight = msg.getArgAsInt(2);
        }
        if(msg.getNumArgs() > 3){
            textSmallString = textString;
            textString = "";
        }else{
            textSmallString = "";
            textString = textString;
        }
        
        
        ofLogNotice("Text change.");
    }
    if(addr == "/fx"){
        
        if(postFxMode){
            postFxMode = false;
            post.setFlip(false);
            post.createPass<PixelatePass>()->setEnabled(false);
            post.createPass<KaleidoscopePass>()->setEnabled(false);
            post.createPass<RGBShiftPass>()->setEnabled(false);
        }
        else{
            post.reset();
            postFxMode = true;
        }

        if(postFxMode){
           
          post.setFlip(true);
            
            
        if(msg.getNumArgs() == 1){
            string mode = msg.getArgAsString(0);
            if(mode == "toon"){
                post.createPass<ToonPass>()->setEnabled(true);
            }
            else{
                post.createPass<ToonPass>()->setEnabled(false);
            }
            if(mode == "pixel"){
               post.createPass<PixelatePass>()->setEnabled(true);
            }
            else{
                post.createPass<PixelatePass>()->setEnabled(false);
            }
            if(mode == "kal"){
                post.createPass<KaleidoscopePass>()->setEnabled(true);
            }
            else{
                post.createPass<KaleidoscopePass>()->setEnabled(false);
            }
            if(mode == "rgb"){
                post.createPass<RGBShiftPass>()->setEnabled(true);
            }
            else{
                post.createPass<RGBShiftPass>()->setEnabled(false);
            }
            
        }
        
/*      post.createPass<FxaaPass>()->setEnabled(false);
        post.createPass<BloomPass>()->setEnabled(false);
        post.createPass<DofPass>()->setEnabled(false);
        post.createPass<NoiseWarpPass>()->setEnabled(false);
        post.createPass<EdgePass>()->setEnabled(false);
        post.createPass<VerticalTiltShifPass>()->setEnabled(false);
        post.createPass<GodRaysPass>()->setEnabled(false);
        post.createPass<LimbDarkeningPass>()->setEnabled(false);
        post.createPass<RGBShiftPass>()->setEnabled(false);
 */
        }

    }
    
    
}

void ofApp::onDirectoryWatcherItemModified(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
    if (evt.item.path() != ofToDataPath("errors.log", true)){
        string file = evt.item.path();
        
        if(file.rfind(".vert")){
            isVertexDirty = true;
        }
        
        if(file.rfind(".glsl")){
            isShaderDirty = true;
        }
        ofLogNotice("Modified: " + evt.item.path());
    }
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
        return "";
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

string ofApp::prepareVertex(string vertexText){
    vertexText =  STRINGIFY(
                            uniform vec3 iResolution;
                            uniform float iGlobalTime;
                            uniform vec2 iMouse;
                            uniform vec4 iDate;
                            uniform float iVolume;
                            ) + vertexText;
    
    vertexText =  "#extension GL_EXT_gpu_shader4 : require\n" + vertexText;
    vertexText = "#version 120\n\n" + vertexText;
    return vertexText;
}

void ofApp::clearErrorLog(void){
    ofFile errorFile;
    errorFile.open(ofToDataPath("errors.log"), ofFile::WriteOnly, false);
    errorFile.close();
}

void ofApp::toggleEditor(void * _o){
    ((ofApp *)_o)->editorVisible = !((ofApp *)_o)->editorVisible;
    ((ofApp *)_o)->editor.currentBuffer = 1;
    ((ofApp *)_o)->editor.loadFile(ofToDataPath(((ofApp *)_o)->mainFrag, true), 1);
}

void ofApp::toggleErrors(void * _o){
    ((ofApp *)_o)->editorVisible = !((ofApp *)_o)->editorVisible;
    ((ofApp *)_o)->editor.currentBuffer = 0;
    ((ofApp *)_o)->editor.loadFile(ofToDataPath("errors.log", true), 0);
}

void ofApp::toggleEditorSave(){
    if(editor.currentBuffer == 1){
      editor.saveFile(ofToDataPath(mainFrag, true), 1);
      isShaderDirty = true;
    }
    else if(editor.currentBuffer == 2){
      editor.saveFile(ofToDataPath(mainVert, true), 2);
      isVertexDirty = true;
    } 
}

void ofApp::keyPressed(int key){
    bool alt   = (bool) (ofGetKeyPressed(OF_KEY_ALT));
    bool shift = (bool) (ofGetKeyPressed(OF_KEY_SHIFT));
    bool cmd   = (bool) (ofGetKeyPressed(OF_KEY_COMMAND));
    bool ctrl  = (bool) (ofGetKeyPressed(OF_KEY_CONTROL));

    //printf("key:%i\n",key);

    if(ctrl && prevKey == 24 && key == 19){
      toggleEditorSave();
    }
    else if(ctrl && key == 19){
      toggleEditorSave();
    }
    else if(cmd && key == 'f'){
        ofToggleFullscreen();
        if(this->isFullscreen){
            ofShowCursor();
#ifdef __APPLE__
            CGDisplayShowCursor(NULL);
#endif
            isFullscreen= false;
        }
        else{
            ofHideCursor();
#ifdef __APPLE__
            CGDisplayHideCursor(NULL);
#endif
            isFullscreen= true;
        }
        
    }
    prevKey = key;
}

int ofApp::toVertexType(string thing){
    int type;
    if(thing == "lines"){
        type = GL_LINES;
    }
    if(thing == "points"){
        type = GL_POINTS;
    }
    if(thing == "tri"){
        type = GL_TRIANGLES;
    }
    if(thing == "tri_fan"){
        type = GL_TRIANGLE_FAN;
    }
    if(thing == "tri_strip"){
        type = GL_TRIANGLE_STRIP;
    }
    if(thing == "line_loop"){
        type = GL_LINE_LOOP;
    }
    if(thing == "line_strip"){
        type = GL_LINE_STRIP;
    }
    
    return type;
}

void ofApp::mouseMoved(int x, int y ){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){ }
