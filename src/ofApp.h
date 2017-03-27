#pragma once

#include "ofMain.h"
#include "ofxIO.h"
#include "ofxFft.h"
#include "ofxEasyFft.h"
#include "ofxOsc.h"
#include "ofxEmacsEditor.h"
#include "ofxPostProcessing.h"

class ofApp : public ofBaseApp{
    
    map<string, float> uniforms;
    map<string, float> tickingUniforms;
    map<string, float> growingUniforms;
    map<string, float> decayRate;
    map<string, float> growthRate;
    
    int vertexType;
    int vertexCount;
    ofx::IO::DirectoryWatcherManager watcher;
    ofx::IO::HiddenFileFilter fileFilter;
    ofShader shader;
    ofTexture mTexture;
    ofxEasyFft fft;
    int plotHeight, bufferSize, w,h;
    ofFbo fbo;
    
    ofxPostProcessing post;
    
    ofxOscReceiver receiver;
    
    ofTrueTypeFont renderFont;
    ofTrueTypeFont renderSmallFont;
    
    string defaultVert;
    string mainFrag;
    string mainVert;
    float currentAmp;
    ofxEmacsEditor editor;
    bool editorVisible;
    bool isFullscreen;
    bool shaderErrored;
    bool showFreqGraph;
    bool postFxMode;
    int listeningOnPort;

    int prevKey;

    int rBackground;
    int gBackground;
    int bBackground;
    
    string textString;
    string textSmallString;

    string oldShader;
    string oldVert;

    int textStringWidth;
    int textStringHeight;
    
    
    bool cameraMode;
    ofVideoGrabber cam;
    
public:
    ofApp() : editor(3), editorVisible(false), isFullscreen(true) {}    bool isShaderDirty;
    bool isVertexDirty;
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
    string prepareVertex(string path);
    string loadFileShader(string path);
    
    void toggleEditorSave(void);
    void clearErrorLog(void);
    
    static void toggleEditor(void *);
    static void toggleErrors(void *);
    
    
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
    
private:
    int toVertexType(string thing);
};
