#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
    ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
	// can be OF_WINDOW or OF_FULLSCREEN
	ofRunApp(new ofApp());
}
