#pragma once

#include <algorithm>    // std::sort
#include "ofMain.h"
#include "ofx3DModelLoader.h"
#include "ofxTweakbar.h"
#include "ofxIniFile.h"
#include "ofxTweakbarINI.h"
#include "ofxPostProcessing.h"
#include "ofxFFTFile.h"
#include "ofxFFTLive.h"

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
        void setKeyFrameData(std::pair<int,std::string> sync_datas);
        void recordKeyFrame(int index);
        void seekBy(int offset);
};
