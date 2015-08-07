#include "ofApp.h"
#define STRINGIFY(A) #A

void ofApp::setup(){
    listeningOnPort = 9177;
    shaderErrored = false;
    showFreqGraph = false;
    beatHit = 0.0;
    ofDisableArbTex();
    meshing = true;
    orbiting = true;
    
    //shader.setGeometryInputType(GL_TRIANGLES);
    //shader.setGeometryOutputType(GL_TRIANGLES);
    //shader.setGeometryOutputCount(10);
    //bool succ = image.loadImage("/Users/josephwilk/Desktop/hs-2006-17-c-xlarge_web.jpg");
    //bool succ = image.loadImage("/Users/josephwilk/Dropbox/Screenshots/Screenshot 2015-07-26 20.09.55.png");
    string filename = "Screenshot 2015-07-26 21.12.43";
    bool succ = image.loadImage("/Users/josephwilk/Dropbox/Screenshots/"+filename+".png");
    image.mirror(true, false);
    
    image.resize(image.getWidth()*0.25, image.getHeight()*0.25);
    editor.addCommand('a', this, &ofApp::toggleEditor);
    
    mainFrag    = "nil.glsl";
    currentAmp = 0.0;
   
    defaultVert = STRINGIFY(
                            uniform mat4 modelViewProjectionMatrix;
                            uniform float phase = 0.5;		//Phase for "sin" function
                            uniform float distortAmount = 1.0; 	//Amount of distortion
                            uniform sampler2D iChannel0;
                            
                            void main(){
                                
                                //Get original position of the vertex
                                vec3 v = gl_Vertex.xyz;
                                
                                //the original code for distort occured in the vertex shader
                                /*
                                 //Compute value of distortion for current vertex
                                 float distort = distortAmount * sin( phase + 0.015 * v.y );
                                 
                                 //Move the position
                                 v.x /= 1.0 + distort;
                                 v.y /= 1.0 + distort;
                                 v.z /= 1.0 + distort;
                                 */
                                //Set output vertex position
                                //vec4 posHomog = vec4( v, 1.0 );
                                vec4 pos = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
                                gl_Position = pos;
                                gl_TexCoord[0] = gl_MultiTexCoord0;
                                gl_FrontColor = gl_Color;                            }
    );
    
    defaultVert = "#version 120\n" + defaultVert;
    
    
    defaultGeom = STRINGIFY(
                                   // get the variables for speed and phase (mousex and mousey)
                                   uniform float phase = 0.5;		//Phase for "sin" function
                                   uniform float distortAmount = 1.0; 	//Amount of distortion
                                   //the geometry shader has been set to recieve triangles
                                   //therefor it has three points to work with
                                   uniform sampler2D iChannel0;
                                   void main(void)
    {
//        vec3 p0 = gl_PositionIn[0].xyz;
//        vec3 p1 = gl_PositionIn[1].xyz;
//        vec3 p2 = gl_PositionIn[2].xyz;
//        vec3 dir0 = p1 - p0;
//        vec3 dir1 = p2 - p1;
        
        //	find the normal of the face (direction it is facing)
//        vec3 N = normalize(cross(dir1,dir0));
        
//        float boomx = smoothstep(0.0,1.0, texture2D(iChannel0, vec2(0.0,p0.y)).x);
//        float boomy = smoothstep(0.0,1.0, texture2D(iChannel0, vec2(0.0,p0.y)).x);

//        float distort = distortAmount * sin( phase + 0.01 * p0.y ); //deform along y axis
        
		//Move the position
//        N.x /= 1.0 + distort +boomx;
//        N.y /= 1.0 + distort + boomy;
//        N.z /= 1.0 + distort;
        
        //uncomment this section if you would like the deformation to be along two axis instead of just one.
        /*
         float distort2 = distortAmount * sin( phase + 0.01 * p0.x ); //deform along x axis
         //Move the position
         N.x /= 1.0 + distort2;
         N.y /= 1.0 + distort2;
         N.z /= 1.0 + distort2;
         */ 
		//remember there are three points to be moved
        for (int i = 0; i < 2; i++)
        {
        gl_Position = gl_PositionIn[i];
        EmitVertex();
        }
        
        EndPrimitive();
        
    }

    );
    
    defaultGeom = "#version 120\n" + defaultGeom;
    
    editor.loadFile(ofToDataPath(mainFrag, true), 1);
    editor.update();
   
	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(ofToDataPath(mainFrag, true)));
    shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
    //shader.setupShaderFromSource(GL_GEOMETRY_SHADER, defaultGeom);
    shader.linkProgram();
    
    ofSetFrameRate(60);
  
    receiver.setup(listeningOnPort);
    ofAddListener(receiver.onMessageReceived, this, &ofApp::onMessageReceived);

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
    
    testMesh.setMode(OF_PRIMITIVE_POINTS);
    float intensityThreshold = 250.0;
    int noise = 10;
    float stretch = 0.25;
    int w = image.getWidth();
    int h = image.getHeight();
    for (int x=0; x<w; ++x) {
        for (int y=0; y<h; ++y) {
            ofColor c = image.getColor(x, y);
            float intensity = c.getLightness();
            if (intensity >= intensityThreshold) {
                float saturation = c.getSaturation();
                float z = ofMap(saturation, 0, 255, -100, 100);
                ofVec3f pos((x/stretch)+ofRandom(0-noise,noise), (y/stretch)+ofRandom(0-noise,noise), z+ofRandom(0-noise,noise));
                testMesh.addVertex(pos);
                testMesh.addColor(c);
                offsets.push_back(ofVec3f(ofRandom(0,100000), ofRandom(0,100000), ofRandom(0,100000)));
            }
        }
    }
    
    testMesh.enableColors();
    testMesh.setMode(OF_PRIMITIVE_LINES);
    
    float connectionDistance = 30;
    int numVerts = testMesh.getNumVertices();
    for (int a=0; a<numVerts; ++a) {
        ofVec3f verta = testMesh.getVertex(a);
        for (int b=a+1; b<numVerts; ++b) {
            ofVec3f vertb = testMesh.getVertex(b);
            float distance = verta.distance(vertb);
            if (distance <= connectionDistance) {
                testMesh.addIndex(a);
                testMesh.addIndex(b);
            }
        }
    }
    
    origMesh = testMesh;
    //testMesh.mergeDuplicateVertices();
    
    meshCentroid = testMesh.getCentroid();
    
    for (int i=0; i<numVerts; ++i) {
        ofVec3f vert = testMesh.getVertex(i);
        float distance = vert.distance(meshCentroid);
        float angle = atan2(vert.y-meshCentroid.y, vert.x-meshCentroid.x);
        distances.push_back(distance);
        angles.push_back(angle);
    }
    
    isShaderDirty = true;
    watcher.registerAllEvents(this);
    std::string folderToWatch = ofToDataPath("", true);
    bool listExistingItemsOnStart = true;
    watcher.addPath(folderToWatch, listExistingItemsOnStart, &fileFilter);
    
    fft.setup(16384);
}

void ofApp::update(){
    fft.update();
    //TODO: Volume
    currentAmp = 0.0;
    
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

    if(isShaderDirty == true){
        string oldShader = shader.getShaderSource(GL_FRAGMENT_SHADER);
        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(ofToDataPath(mainFrag, true)));
        shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
        //shader.setupShaderFromSource(GL_GEOMETRY_SHADER, defaultGeom);

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
    
    if(meshing == true){
    
    int numVerts = testMesh.getNumVertices();
    float bumpFactor = beatHit;
    for (int i=0; i<numVerts; ++i) {
        ofVec3f vert = testMesh.getVertex(i);
        ofVec3f vertbck = origMesh.getVertex(i);
        
        float time = ofGetElapsedTimef();
        float timeScale = 5.0;
        float displacementScale = 2.0;
        ofVec3f timeOffsets = offsets[i];
        //vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
        
        int effected = 3;
        //int fftKey = ofMap(i, 0, numVerts, 0, 511);
        int fftKey = i%512;
        
        if (fft.getAudio()[fftKey] > 0.8){
            effected = 2;
        }
        if (fft.getAudio()[fftKey] > 0.9){
            effected = 1;
        }
        
        if(i % effected == 0){
            if(fft.getAudio()[fftKey] < 0.3 || fft.getAudio()[fftKey] > 0.3){
                vert.x += (fft.getAudio()[fftKey] * displacementScale);
                vert.y += (fft.getAudio()[fftKey] * displacementScale);
                vert.z += (fft.getAudio()[fftKey] * displacementScale);
            }
        }

/*
        if(i % effected == 0){
        if(bumpFactor == 1.0){
            vert.x +=  ofSignedNoise(time*timeScale+timeOffsets.x)*2.0;
            vert.y +=  ofSignedNoise(time*timeScale+timeOffsets.x)*2.0;
        }
        else if(vert.x > vertbck.x){
            vert.x -= 0.1;
            vert.y -= 0.1;
        }
        else{
            vert.x = vertbck.x;
            vert.y = vertbck.y;
        }
        }
*/
        testMesh.setVertex(i, vert);
    }
    }
    
    if (false) {
        int numVerts = testMesh.getNumVertices();
        for (int i=0; i<numVerts; ++i) {
            ofVec3f vert = testMesh.getVertex(i);
            float distance = distances[i];
            float angle = angles[i];
            float elapsedTime = ofGetElapsedTimef() - startOrbitTime;
            
            // Lets adjust the speed of the orbits such that things that are closer to
            // the center rotate faster than things that are more distant
            float speed = ofMap(distance, 0, ofGetElapsedTimef()*100, 0.2, 0.001, true);
            
            // To find the angular rotation of our vertex, we use the current time and
            // the starting angular rotation
            float rotatedAngle = elapsedTime * speed + angle;
            
            // Remember that our distances are calculated relative to the centroid of the mesh, so
            // we need to shift everything back to screen coordinates by adding the x and y of the centroid
            vert.x = distance * cos(rotatedAngle) + meshCentroid.x;
            vert.y = distance * sin(rotatedAngle) + meshCentroid.y;
            
            vert.x +=  ofSignedNoise(ofGetElapsedTimef()*0.1)*2.0;
            vert.y +=  ofSignedNoise(ofGetElapsedTimef()*0.1)*2.0;

            
            testMesh.setVertex(i, vert);
        }
    }
    
}

void ofApp::draw(){
    fbo.begin();
    //ofBackground(0, 0, 0);
    //ofEnableDepthTest();				//Enable z-buffering
    
	//Set a gradient background from white to gray
	//for adding an illusion of visual depth to the scene
	//ofBackgroundGradient( ofColor( 255 ), ofColor( 128 ) );
    
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
    shader.setUniform1f("iVolume", currentAmp) ;
    
    if(meshing){
        shader.setUniform1f("iMesh", 1.0);
    }
    else{
        shader.setUniform1f("iMesh", 0.0);
    }
    
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
    
    //shader.setGeometryOutputCount(1024);

    
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



    if(meshing){
        //easyCam.begin();
        ofPushMatrix();	//Store the coordinate system
        ofTranslate((ofGetWidth()/2)-440.0, (ofGetHeight()/2.0)-210.0);
        
        float time = ofGetElapsedTimef();	//Get time in seconds
        float angle = time * 10;			//Compute angle. We rotate at speed 10 degrees per second
        ofRotate(1, 0, 1, 0 );
        //ofColor centerColor = ofColor(85, 78, 68);
        //ofColor edgeColor(0, 0, 0);
     //ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);

        testMesh.draw();
        ofPopMatrix();
        // easyCam.end();
    }

    shader.end();
    mTexture.unbind();
    
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
    if(addr == "/beat"){
        beatHit = msg.getArgAsFloat(0);
    }
    if(addr == "/mesh"){
        float flag = msg.getArgAsFloat(0);
        if(flag <= 0.0){
            meshing = false;
        }
        else{
            meshing = true;
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

string ofApp::prepareShader(string path){
	ofFile file;

	if(!file.open(ofToDataPath(path), ofFile::ReadOnly)){
		return false;
	}
    string shaderText = file.readToBuffer().getText();
	file.close();
    shaderText =  STRINGIFY(
                            uniform vec3 iResolution;
                            uniform float iGlobalTime;
                            uniform vec2 iMouse;
                            uniform vec4 iDate;
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
