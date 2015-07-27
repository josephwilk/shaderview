#pragma once

#include "ofMain.h"
#include "ofxIO.h"
#include "ofxFft.h"
#include "ofxEasyFft.h"
#include "ofxOsc.h"
#include "ofxEditor.h";
#include "ofxPostProcessing.h"

class ofApp : public ofBaseApp{
   
    //vector to hold all triangles
	vector<ofMeshFace> triangles;
	//mesh to hold triangles for drawing as a single unit
	ofMesh testMesh;
    //traditional sphere primitive which will look like strips
	ofSpherePrimitive sphere;
    string defaultGeom;
    
    map<string, float> uniforms;
    
    ofx::IO::DirectoryWatcherManager watcher;
    ofx::IO::HiddenFileFilter fileFilter;
    ofShader shader;
    ofTexture mTexture;
    ofxEasyFft fft;
    int plotHeight, bufferSize, w,h;
    ofFbo fbo;
    vector<ofVec3f> offsets;
    ofImage image;
    
    ofxPostProcessing post;

    ofxOscReceiver receiver;
    
    string defaultVert;
    string mainFrag;
    float currentAmp;
    ofxEditor editor;
    bool editorVisible;
    bool isFullscreen;
    bool shaderErrored;
    bool showFreqGraph;
    int listeningOnPort;
    
    ofEasyCam easyCam;
   
    
public:
    ofApp() : editor(2), editorVisible(false), isFullscreen(false) {}
    bool isShaderDirty;
    void setup();
    void update();
    void draw();
    
    void plot(vector<float>& buffer, float scale);
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void onMessageReceived(ofxOscMessage &msg);
    string prepareShader(string path);
    
    void toggleEditorSave(void);
    
    static void toggleEditor(void *);
    
    
    void onDirectoryWatcherItemModified(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt);
    
    void onDirectoryWatcherItemAdded(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
        ofLogNotice("Added:    " + evt.item.path());
    }
    
    void onDirectoryWatcherItemRemoved(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
        ofLogNotice("Removed:  " + evt.item.path());
    }
    
    void onDirectoryWatcherItemMovedFrom(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
        ofLogNotice("ofApp::onDirectoryWatcherItemMovedFrom") << "Moved From: " << evt.item.path();
    }
    
    void onDirectoryWatcherItemMovedTo(const ofx::IO::DirectoryWatcherManager::DirectoryEvent& evt){
        ofLogNotice("ofApp::onDirectoryWatcherItemMovedTo") << "Moved To: " << evt.item.path();
    }
    
    void onDirectoryWatcherError(const Poco::Exception& exc){
        ofLogError("ofApp::onDirectoryWatcherError") << "Error: " << exc.displayText();
    }
};
