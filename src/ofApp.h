#pragma once

#include "ofMain.h"
#include "ofxBeat.h"
#include "ofxIO.h"

class ofApp : public ofBaseApp{

    ofxBeat beat;
    ofx::IO::DirectoryWatcherManager watcher;
    ofx::IO::HiddenFileFilter fileFilter;
    std::deque<std::string> messages;
    ofShader shader;
    
	public:
        bool isShaderDirty;
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void audioReceived(float*, int, int);
    
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
