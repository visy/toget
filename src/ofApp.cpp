#include "ofApp.h"

// scene stuff

GLfloat lightOnePosition[] = {80.0, -10, 60.0, 0.0};
GLfloat lightOneColor[] = {1.99, 1.99, 1.99, 1.0};

GLfloat lightTwoPosition[] = {-40.0, 10, 70.0, 0.0};
GLfloat lightTwoColor[] = {0.99, 0.99, 0.99, 1.0};

// demo stuff

float junaX;
float junaStartX;

ofx3DModelLoader junaModel;
ofImage bgImage;

ofShader shaderBlurX;
ofShader shaderBlurY;

ofFbo bg;
ofFbo fg;

ofxPostProcessing post;

// tweak stuff

ofxTweakbar* settings;
ofxTweakbar* particles;
ofxIniFile* ini_file;
ofxTweakbarINI* ini;
float speed_float;
float blur_float;
float train_rotation[4];
int num_files;
int num_textures;
float tweak;

// media

ofSoundPlayer soundPlayer;

ofxFFTFile fftFile;
ofxFFTLive fftLive;

//--------------------------------------------------------------
void ofApp::setup() {
    // tweakbars
    
    settings = ofxTweakbars::create("settings", "Demo Settings");
	settings->addFloat("speed", &speed_float)->setLabel("train speed")->setMin("0.0")->setMax("50000")->setStep("10");
	settings->addQuat4f("train_rotation", &train_rotation)->setLabel("train rotation");
    
	settings
    ->setSize(500,200)
    ->setColor(44,44,44,180)
    ->setFontSize(1);

    ini_file = new ofxIniFile("tweakbar.ini");
	ini = new ofxTweakbarINI(settings, ini_file);

    settings->load();
	ini->retrieve();
    
    // shaders
    
    shaderBlurX.load("shaders/shaderBlurX");
    
    // set OFX state
    ofHideCursor();
    
    ofEnableAntiAliasing();
    
    ofSetBackgroundAuto(false);
    ofBackground(0,0,0);
		
	ofSetVerticalSync(true);
    
    ofDisableArbTex();
    
    //some model / light stuff
    ofEnableDepthTest();
    glShadeModel (GL_SMOOTH);

    /* initialize lighting */
    glLightfv (GL_LIGHT0, GL_POSITION, lightOnePosition);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, lightOneColor);
    glEnable (GL_LIGHT0);
    glLightfv (GL_LIGHT1, GL_POSITION, lightTwoPosition);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, lightTwoColor);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
    glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable (GL_COLOR_MATERIAL);

    junaModel.loadModel("models/sm2-lowpoly.3ds", 20);

    //you can create as many rotations as you want
    //choose which axis you want it to effect
    //you can update these rotations later on
    junaModel.setRotation(0, 90, 1, 0, 0);
    junaModel.setRotation(1, 270, 0, 0, 1);
    junaModel.setScale(2.0, 2.0, 1.0);
    junaModel.setPosition(ofGetWidth()*2.0, ofGetHeight()/1.02, 0);
    junaStartX = junaModel.pos.x;
    
    bgImage.loadImage("gfx/junabg.png");
    
    bg.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    fg.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    
    // postprocess
    
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<VerticalTiltShifPass>();
    post.createPass<ContrastPass>();
    
    // media
    
    fftFile.setMirrorData(false);
    fftFile.setup();
    fftLive.setMirrorData(false);
    fftLive.setup();
    
    soundPlayer.loadSound("media/capsule455.mp3");
    soundPlayer.setLoop(true);
 //   soundPlayer.play();

}

float delta = 0.0;
float startTime = 0.0;
float endTime = 0.0;
float junaSpeed = 0.0;
//--------------------------------------------------------------
void ofApp::update(){
    // demo
    
    junaSpeed+=speed_float*delta;
    junaX=junaStartX-junaSpeed;
    if (junaX < -ofGetWidth()*3.0) junaStartX+=ofGetWidth()*2.45;
    
    blur_float = speed_float*0.000001;
    
    fftFile.update();
    fftLive.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    startTime = ofGetElapsedTimef();

    ofDisableBlendMode();
    ofDisableAlphaBlending();
    
    bg.begin();
        ofSetColor(255,255,255,64);
        bgImage.draw(0, 0);
    bg.end();
    
    ofEnableDepthTest();

    bg.begin();
    ofBackground(255, 255, 255, 255);
    ofSetColor(255,255,255,255);
    bgImage.draw(0,0);
    bg.end();
    
    fg.begin();
    ofClearAlpha();
    ofClear(0,0,0);
    ofBackground(0, 0, 0, 0);
        glPushMatrix();
            ofVec3f qaxis; float qangle;
            ofQuaternion qr;
            qr.set(train_rotation[0], train_rotation[1], train_rotation[2], train_rotation[3]);
            qr.getRotate(qangle, qaxis);
            glRotatef(qangle, qaxis[0], qaxis[1]+fftLive.getAveragePeak(), qaxis[2]);
    
            junaModel.setPosition(junaX, junaModel.pos.y, junaModel.pos.z);
            junaModel.draw();
        
            junaModel.setPosition((junaX+ofGetWidth()*2.45), junaModel.pos.y, junaModel.pos.z);
            junaModel.draw();
        
            junaModel.setPosition((junaX+ofGetWidth()*2.45*2), junaModel.pos.y, junaModel.pos.z);
            junaModel.draw();

            junaModel.setPosition((junaX+ofGetWidth()*2.45*3), junaModel.pos.y, junaModel.pos.z);
            junaModel.draw();
        glPopMatrix();
    fg.end();

    ofDisableDepthTest();
    ofDisableBlendMode();
    
    // composite
    
    post.begin();
    
    ofSetColor(255, 255, 255, 255);
    bg.draw(0, 0);
    ofEnableAlphaBlending();
 
    shaderBlurX.begin();
    shaderBlurX.setUniform1f("blurAmnt", blur_float);
    fg.draw(0, 0);
    shaderBlurX.end();
    
    post.end();

    // end frame
    
    endTime = ofGetElapsedTimef();
    delta = endTime-startTime;
    
    // fps display
    
    ofSetHexColor(0x000000);
    ofDrawBitmapString("fps: "+ofToString(ofGetFrameRate(), 2), 10, 15);

    // draw tweakbars
    
    ofxTweakbars::draw();
    
    ofDisableAlphaBlending();
    ofSetColor(0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){ 
	if (key == 's') {
		// Store the values into an ini file.
		ini->store();
	}
	else if (key == 'l') {
		// Retrieve the values.
		ini->retrieve();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
