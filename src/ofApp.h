#pragma once

#include "ofMain.h"
#include "ofx3DModelLoader.h"
#include "ofxTweakbar.h"
#include "ofxIniFile.h"
#include "ofxTweakbarINI.h"
#include "ofxPostProcessing.h"

class ofApp : public ofBaseApp{

	public:
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
        static void TW_CALL buttonCallback(void* pApp);
};
