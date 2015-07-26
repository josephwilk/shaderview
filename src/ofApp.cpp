#include "ofApp.h"
#define STRINGIFY(A) #A

void ofApp::setup(){
    listeningOnPort = 9177;
    shaderErrored = false;
    showFreqGraph = false;
    ofDisableArbTex();
    
	shader.setGeometryInputType(GL_TRIANGLES);
	shader.setGeometryOutputType(GL_TRIANGLES);
	shader.setGeometryOutputCount(4);

    
//bool succ = image.loadImage("/Users/josephwilk/Desktop/hs-2006-17-c-xlarge_web.jpg");
    //bool succ = image.loadImage("/Users/josephwilk/Dropbox/Screenshots/Screenshot 2015-07-26 20.09.55.png");
    
    bool succ = image.loadImage("/Users/josephwilk/Dropbox/Screenshots/Screenshot 2015-07-26 21.12.43.png");
    
    
    image.resize(image.getWidth()*0.4, image.getHeight()*0.4);
    editor.addCommand('a', this, &ofApp::toggleEditor);
    
    mainFrag    = "nil.glsl";
    currentAmp = 0.0;
   
    defaultVert = STRINGIFY(
                            uniform mat4 modelViewProjectionMatrix;
                            uniform float phase = 0.0;		//Phase for "sin" function
                            uniform float distortAmount = 0.25; 	//Amount of distortion
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
                                vec4 posHomog = vec4( v, 1.0 );
                                gl_Position = gl_ModelViewProjectionMatrix * posHomog;
                                
                                //Set output texture coordinate and color in a standard way
                                gl_TexCoord[0] = gl_MultiTexCoord0;
                                gl_FrontColor = gl_Color;                            }
    );
    
    defaultVert = "#version 120\n" + defaultVert;
    
    
    defaultGeom = STRINGIFY(
                                   // get the variables for speed and phase (mousex and mousey)
                                   uniform float phase = 0.0;		//Phase for "sin" function
                                   uniform float distortAmount = 0.25; 	//Amount of distortion
                                   //the geometry shader has been set to recieve triangles
                                   //therefor it has three points to work with
                                   uniform sampler2D iChannel0;
                                   void main(void)
    {
        vec3 p0 = gl_PositionIn[0].xyz;
        vec3 p1 = gl_PositionIn[1].xyz;
        vec3 p2 = gl_PositionIn[2].xyz;
        vec3 dir0 = p1 - p0;
        vec3 dir1 = p2 - p1;
        
        //	find the normal of the face (direction it is facing)
        vec3 N = normalize(cross(dir1,dir0));
        
        float boomx = smoothstep(0.0,1.0, texture2D(iChannel0, vec2(0.0,p0.y)).x);
        float boomy = smoothstep(0.0,1.0, texture2D(iChannel0, vec2(0.0,p0.y)).x);

        float distort = distortAmount * sin( phase + 0.01 * p0.y ); //deform along y axis
        
		//Move the position
        N.x /= 1.0 + distort +boomx;
        N.y /= 1.0 + distort + boomy;
        N.z /= 1.0 + distort;
        
        //uncomment this section if you would like the deformation to be along two axis instead of just one.
        /*
         float distort2 = distortAmount * sin( phase + 0.01 * p0.x ); //deform along x axis
         //Move the position
         N.x /= 1.0 + distort2;
         N.y /= 1.0 + distort2;
         N.z /= 1.0 + distort2;
         */ 
		//remember there are three points to be moved
        for (int i = 0; i < 3; i++)
        {
            gl_Position = gl_PositionIn[i] + vec4(50*N, 0.0);
            EmitVertex();
        }
        
        EndPrimitive();
        
    }

    );
    
    defaultGeom = "#version 120\n" + defaultGeom;
    
    editor.loadFile(ofToDataPath(mainFrag, true), 1);
    editor.update();
    
    shader.setGeometryInputType(GL_TRIANGLES);
	shader.setGeometryOutputType(GL_TRIANGLES);
	shader.setGeometryOutputCount(4);
    
	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(ofToDataPath(mainFrag, true)));
    //shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
    //shader.setupShaderFromSource(GL_GEOMETRY_SHADER, defaultGeom);
    shader.linkProgram();
    
    
    ofSetFrameRate(60);
    testMesh.setMode(OF_PRIMITIVE_POINTS);

    
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
    
    //sphere.setRadius(300.0);
	// this sets resolution of 'sphere'.
	// if this were an ofIcoSphere then a number such as 2-4 is appropriate
	//sphere.setResolution(25);
	
	// this turns the sphere into a triangle mesh made up of the faces.
	//triangles = sphere.getMesh().getUniqueFaces();
	//testMesh.setFromTriangles(triangles);
    
    
    
    testMesh.setMode(OF_PRIMITIVE_POINTS);
    float intensityThreshold = 150.0;
    int w = image.getWidth();
    int h = image.getHeight();
    for (int x=0; x<w; ++x) {
        for (int y=0; y<h; ++y) {
            ofColor c = image.getColor(x, y);
            float intensity = c.getLightness();
            if (intensity >= intensityThreshold) {
                float saturation = c.getSaturation();
                float z = ofMap(saturation, 0, 255, -100, 100);
                ofVec3f pos(4*x, 4*y, z);
                testMesh.addVertex(pos);
                testMesh.addColor(c);
                offsets.push_back(ofVec3f(ofRandom(0,100000), ofRandom(0,100000), ofRandom(0,100000)));
            }
        }
    }
    
    
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

    if(false){
        shader.setGeometryInputType(GL_TRIANGLES);
        shader.setGeometryOutputType(GL_TRIANGLES);
        shader.setGeometryOutputCount(4);

        string oldShader = shader.getShaderSource(GL_FRAGMENT_SHADER);
        bool r = shader.setupShaderFromSource(GL_FRAGMENT_SHADER, prepareShader(ofToDataPath(mainFrag, true)));
        shader.setupShaderFromSource(GL_VERTEX_SHADER,   defaultVert);
        shader.setupShaderFromSource(GL_GEOMETRY_SHADER, defaultGeom);

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
    
    int numVerts = testMesh.getNumVertices();
    for (int i=0; i<numVerts; ++i) {
        ofVec3f vert = testMesh.getVertex(i);
        
        float time = ofGetElapsedTimef();
        float timeScale = 0.0;
        float displacementScale = 0.75;
        ofVec3f timeOffsets = offsets[i];
        
        // A typical design pattern for using Perlin noise uses a couple parameters:
        // ofSignedNoise(time*timeScale+timeOffset)*displacementScale
        //     ofSignedNoise(time) gives us noise values that change smoothly over time
        //     ofSignedNoise(time*timeScale) allows us to control the smoothness of our noise (smaller timeScale, smoother values)
        //     ofSignedNoise(time+timeOffset) allows us to use the same Perlin noise function to control multiple things and have them look as if they are moving independently
        //     ofSignedNoise(time)*displacementScale allows us to change the bounds of the noise from [-1, 1] to whatever we want
        // Combine all of those parameters together, and you've got some nice control over your noise
        
        vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
        vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
        vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
        testMesh.setVertex(i, vert);
    }
}

void ofApp::draw(){
    fbo.begin();

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
    
    shader.setGeometryOutputCount(1024);

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


    
    ofPushMatrix();	//Store the coordinate system
    ofTranslate( 0.0, 0, 0.5);
    float time = ofGetElapsedTimef();	//Get time in seconds
    float angle = time * 10;			//Compute angle. We rotate at speed 10 degrees per second
    ofRotate(1, 0, 1, 0 );
    ofColor centerColor = ofColor(85, 78, 68);
    ofColor edgeColor(0, 0, 0);
    //ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);
    testMesh.draw();
    ofPopMatrix();


    
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
