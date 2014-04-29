#include "ofApp.h"
#include <algorithm>

// demo stuff

float junaX;
float junaStartX;

ofx3DModelLoader junaModel;
ofImage junatex;
ofImage junanormals;

ofImage bgImage;

ofShader shaderBlurX;
ofShader shaderBlurY;
ofShader bumpshader;
ofEasyCam easyCam;

ofFbo bg;
ofFbo fg;

ofxPostProcessing post;

ofMaterial mat;
ofFloatColor diffuse;
ofFloatColor ambient;
ofFloatColor specular;
ofFloatColor emissive;

ofFloatColor diffuseL;
ofFloatColor ambientL;
ofFloatColor specularL;
ofFloatColor emissiveL;

ofLight light;

// tweak stuff

ofxTweakbar* settings;
ofxIniFile* ini_file;
ofxTweakbarINI* ini;
float speed_float;
float blur_float;
float train_rotation[4];
int num_files;
int num_textures;
float tweak;

ofFile sync_file;
ofXml sync_xml;

float millis = 0.0;
bool paused = false;
int currentMaxKeyFrame = 0;
int currentKeyFrame = 0;
int keyFrames = 0;

std::vector<std::pair<int,std::string> > keyFrameData;

std::map<std::string, ofxTweakbarType*> prevFrameTweakVars;

// media

ofSoundPlayer soundPlayer;

ofxFFTFile fftFile;
ofxFFTLive fftLive;

bool mySortFunc(std::pair<int,std::string> i, std::pair<int,std::string> j) {
    int ii = (int)i.first;
    int jj = (int)j.first;
    return (ii<jj);
}

void setupGlLight(){
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    
    GLfloat lmKa[] = {0.8, 0.8, 0.8, 0.8 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmKa);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);
    
    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { +100, 100, 100, 0.0 }; //light directional if w=0 without shader works
    //GLfloat light_position[] = { 220.0, 10.0, 0.0, 1.0 };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);
    
}

void setupGlMaterial(){
    
    GLfloat mat_ambient[] = {0.0f, 0.0f, 0.7f, 1.0f};
    GLfloat mat_diffuse[] = {0.1f, 0.2f, 0.2f, 1.0f};
    GLfloat mat_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT,GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT,GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT,GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT,GL_SHININESS, 60.0);
    
}

void ofApp::setupOFmaterial(){
    
    diffuse.set(0.4f,0.4f,0.4f,1.0f);
    ambient.set(0.4f,0.4f,0.4f,1.0f);
    specular.set(0.01f,0.2f,0.2f,1.0f);
    emissive.set(0.0f,0.0f,0.0f,1.0f);
    mat.setColors(diffuse,ambient,specular,emissive);
    mat.setShininess(120.1f);
    
}

void ofApp::setupOFlight(){
    
    light.enable();
    light.draw();
    light.setDirectional();
    //light.setPointLight();
    
    light.setPosition(+100, 100, 100);
    light.setAttenuation(0.2,0.3,0.4);
    
    diffuseL.set(1.0f,1.0f,1.0f,1.0f);
    ambientL.set(0.0f,0.0f,0.0f,1.0f);
    specularL.set(1.0f,1.0f,1.0f,1.0f);
    
    light.setAmbientColor(diffuseL);
    light.setDiffuseColor(ambientL);
    light.setSpecularColor(specularL);
    
}


//--------------------------------------------------------------
void ofApp::setup() {
    // tweakbars
    
    settings = ofxTweakbars::create("settings", "Demo Settings");
	settings->addFloat("speed", &speed_float)->setLabel("train speed")->setMin("0.0")->setMax("10")->setStep("0.01");
	settings->addQuat4f("train_rotation", &train_rotation)->setLabel("train rotation");
    
	settings
    ->setSize(500,200)
    ->setColor(44,44,44,180)
    ->setFontSize(1);

    ini_file = new ofxIniFile("tweakbar.ini");
	ini = new ofxTweakbarINI(settings, ini_file);

    settings->load();
	ini->retrieve();
    
    // sync
    
    sync_xml.load("sync.xml");
    
    int children = sync_xml.getNumChildren();
    
    string sync_data = "derp";
    int i = 0;
    while (sync_data != "") {
        int sync_millis = sync_xml.getValue<int>("millis"+ofToString(i));
        sync_data = sync_xml.getValue<string>("data"+ofToString(i));
        if (sync_data != "") keyFrameData.push_back(std::pair<int, std::string>(sync_millis,sync_data));
        i++;
    }
    
//    std::sort(keyFrameData.begin(), keyFrameData.end(), mySortFunc);
    
    currentMaxKeyFrame = i-1;
    keyFrames = currentMaxKeyFrame;
    currentKeyFrame = 0;
    
    // shaders
    
    shaderBlurX.load("shaders/shaderBlurX");
    bumpshader.load("shaders/NormalMap");

    // set OFX state
    ofHideCursor();
    
    ofEnableLighting();
    light.enable();

    ofEnableAntiAliasing();
    
    ofSetBackgroundAuto(false);
    ofBackground(0,0,0);
		
	ofSetVerticalSync(true);
    
    ofDisableArbTex();
    
    ofEnableDepthTest();
    glShadeModel (GL_SMOOTH);

    setupGlLight();
    setupGlMaterial();

    
    // models
    
    junaModel.loadModel("models/sm2_rev2.3DS", 20);

    junatex.loadImage("models/sm2_tex.png");
    junanormals.loadImage("models/sm2_normal.png");
    
    //you can create as many rotations as you want
    //choose which axis you want it to effect
    //you can update these rotations later on
    junaModel.setRotation(0, 90, 1, 0, 0);
    junaModel.setRotation(1, 270, 0, 0, 1);
    junaModel.setScale(2.0, 2.0, 1.0);
    junaStartX = junaModel.pos.x;
    
    bgImage.loadImage("gfx/junabg.png");
    
    bg.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    fg.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    
    // postprocess
    
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<HorizontalTiltShifPass>();
    post.createPass<ContrastPass>();
    
    // media
    
    fftFile.setMirrorData(false);
    fftFile.setup();
    fftLive.setMirrorData(false);
    fftLive.setup();
    
    soundPlayer.loadSound("media/capsule455.mp3");
    soundPlayer.setLoop(false);
    soundPlayer.play();

}

float delta = 0.0;
float startTime = 0.0;
float endTime = 0.0;
float junaSpeed = 0.0;
//--------------------------------------------------------------
void ofApp::update(){
    // demo
    millis = soundPlayer.getPositionMS();
    
    if (keyFrames > 0 && currentKeyFrame < currentMaxKeyFrame && currentKeyFrame < keyFrameData.size()) setKeyFrameData(keyFrameData.at(currentKeyFrame));
    
    junaSpeed+=delta*400.5*speed_float;
    junaX=junaStartX-junaSpeed;
//    if (junaX < -ofGetWidth()*3.0) junaStartX+=ofGetWidth()*2.45;
    
    blur_float = speed_float*0.001;
    
    fftFile.update();
    fftLive.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
    prevFrameTweakVars = settings->getVariables();

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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
    
    glShadeModel(GL_SMOOTH);
    ofEnableLighting();

    easyCam.disableMouseInput();
    easyCam.begin();
    
    mat.begin();

        bumpshader.begin();

    model3DS* jm = (model3DS*)junaModel.model;
        bumpshader.setUniformTexture("normalMap",junanormals.getTextureReference(),junanormals.getTextureReference().getTextureData().textureID);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glPushMatrix();
                ofVec3f qaxis; float qangle;
                ofQuaternion qr;
                qr.set(train_rotation[0], train_rotation[1], train_rotation[2], train_rotation[3]);
                qr.getRotate(qangle, qaxis);
                glRotatef(qangle, qaxis[0], qaxis[1]+fftFile.getAveragePeak(), qaxis[2]);
    
                float offx = ofGetWidth();
    
                ofScale(1, -1, 1);
                ofTranslate(ofGetWidth()/2+offx, ofGetHeight()/3, -500);

                junaModel.setPosition(junaX+offx, junaModel.pos.y, junaModel.pos.z);
                junaModel.draw();
            
                junaModel.setPosition((junaX+ofGetWidth()*2.45+offx), junaModel.pos.y, junaModel.pos.z);
                junaModel.draw();

                for (int i = 0; i < 20; i++) {
                    junaModel.setPosition((junaX+ofGetWidth()*2.45*(2+i)+offx), junaModel.pos.y, junaModel.pos.z);
                    junaModel.draw();
                }

            glPopMatrix();
        bumpshader.end();
    mat.end();
    
    easyCam.end();


    fg.end();
    
    ofDisableLighting();

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
    ofDrawBitmapString("fps: "+ofToString(ofGetFrameRate(), 2) + " / millis: " + ofToString(millis, 0), 10, 15);

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
	else if (key == 'r') {
		// Retrieve the values.
		ini->retrieve();
	}
	else if (key == 'p') {
		// Toggle play/pause
        if (!paused) { soundPlayer.setPaused(true); paused = true; }
        else { soundPlayer.setPaused(false); paused = false; }
	}
	else if (key == 'q') {
		// rewind a lot
        seekBy(-1000);
	}
	else if (key == 'e') {
		// ff a lot
        seekBy(1000);
	}
	else if (key == 'a') {
		// rewind
        seekBy(-100);
	}
	else if (key == 'd') {
		// ff
        seekBy(100);
	}
	else if (key == 'z') {
		// rewind tiny
        seekBy(-10);
	}
	else if (key == 'c') {
		// ff tiny
        seekBy(10);
	}
}

void ofApp::seekBy(int offset) {
    int currentMS = soundPlayer.getPositionMS();
    int seekToMS = currentMS+offset;
    if (seekToMS < 0) seekToMS = 0;
    soundPlayer.setPositionMS(seekToMS);
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int str2int (const string &str) {
    stringstream ss(str);
    int num;
    if((ss >> num).fail())
    {
        //ERROR
    }
    return num;
}

float str2float (const string &str) {
    stringstream ss(str);
    float num;
    if((ss >> num).fail())
    {
        //ERROR
    }
    return num;
}

int ofApp::getChangedControlIndex() {

    return -1;
}

void ofApp::setKeyFrameData(std::pair<int,std::string> sync_datas) {
    int sync_millis = sync_datas.first;
    
    if (millis < sync_millis) return;
    
    currentKeyFrame++;
    // time to autotweak mofo
    
    std::map<std::string, ofxTweakbarType*> vars = settings->getVariables();
    std::map<std::string, ofxTweakbarType*>::iterator it = vars.begin();
    ofxTweakbarType* type = NULL;
    OFX_TW_TYPE tw_type;
    std::string section = settings->getName();
    
    std::string sync_datum = sync_datas.second;
    std::string sync_data;

    bool allChange = false;
    
    int changeIndex = 0;
    
    if (sync_datum.at(0) == 'X') {
        allChange = true;
        sync_data = sync_datum.substr(1);
    }
    else {
        std::vector<std::string> datas = split(sync_datum, 'Z');
        changeIndex = str2int(datas.at(0));
        sync_data = datas.at(1);
    }
    
    std::vector<std::string> xmlVars = split(sync_data, '|');

    int typeCount = 0;
    // count types
    while(it != vars.end()) {
        type = it->second;
        tw_type = type->getType();
        if(tw_type == OFX_TW_TYPE_INT32) {
            ofxTweakbarInt* type_impl = static_cast<ofxTweakbarInt*>(it->second);
            typeCount++;
        }
        else if (tw_type == OFX_TW_TYPE_FLOAT) {
            ofxTweakbarFloat* type_impl = static_cast<ofxTweakbarFloat*>(it->second);
            typeCount++;
        }
        else if (tw_type == OFX_TW_TYPE_COLOR3F) {
            ofxTweakbarColor3f* type_impl = static_cast<ofxTweakbarColor3f*>(it->second);
            typeCount++;
        }
        else if (tw_type == OFX_TW_TYPE_VEC3F) {
            ofxTweakbarVec3f* type_impl = static_cast<ofxTweakbarVec3f*>(it->second);
            typeCount++;
        }
        else if (tw_type == OFX_TW_TYPE_QUAT4F) {
            ofxTweakbarQuat4f* type_impl = static_cast<ofxTweakbarQuat4f*>(it->second);
            typeCount++;
        }
        ++it;
    }
    
    if (xmlVars.size() != typeCount) cout << "DANGER DANGER" << endl;
    
    it = vars.begin();
    int i = 0;
    // set types from string
    while(it != vars.end()) {
        type = it->second;
        tw_type = type->getType();
        if(tw_type == OFX_TW_TYPE_INT32) {
            ofxTweakbarInt* type_impl = static_cast<ofxTweakbarInt*>(it->second);
            if (i == changeIndex || allChange)
            type_impl->setValue(str2int(xmlVars.at(i).substr(1)));
            ++i;
        }
        else if (tw_type == OFX_TW_TYPE_FLOAT) {
            ofxTweakbarFloat* type_impl = static_cast<ofxTweakbarFloat*>(it->second);
            if (i == changeIndex || allChange)
                type_impl->setValue(str2float(xmlVars.at(i).substr(1)));
            ++i;
        }
        else if (tw_type == OFX_TW_TYPE_COLOR3F) {
            ofxTweakbarColor3f* type_impl = static_cast<ofxTweakbarColor3f*>(it->second);
            if (i == changeIndex || allChange) {
                std::vector<std::string> splitVecf = split(xmlVars.at(i).substr(1), ',');
            float q0 = 0.0;
            float q1 = 0.0;
            float q2 = 0.0;
            
            q0 = str2float(splitVecf.at(0));
            q1 = str2float(splitVecf.at(1));
            q2 = str2float(splitVecf.at(2));
            type_impl->setValue(q0, q1, q2);
            }
            ++i;
        }
        else if (tw_type == OFX_TW_TYPE_VEC3F) {
            ofxTweakbarVec3f* type_impl = static_cast<ofxTweakbarVec3f*>(it->second);
            if (i == changeIndex || allChange) {
                
            std::vector<std::string> splitVecf = split(xmlVars.at(i).substr(1), ',');
            float q0 = 0.0;
            float q1 = 0.0;
            float q2 = 0.0;
            
            q0 = str2float(splitVecf.at(0));
            q1 = str2float(splitVecf.at(1));
            q2 = str2float(splitVecf.at(2));
            type_impl->setValue(q0, q1, q2);
            }
            ++i;
        }
        else if (tw_type == OFX_TW_TYPE_QUAT4F) {
            ofxTweakbarQuat4f* type_impl = static_cast<ofxTweakbarQuat4f*>(it->second);
            if (i == changeIndex || allChange) {
                
            std::vector<std::string> splitQuatf = split(xmlVars.at(i).substr(1), ',');
            float q0 = 0.0;
            float q1 = 0.0;
            float q2 = 0.0;
            float q3 = 0.0;
            
            q0 = str2float(splitQuatf.at(0));
            q1 = str2float(splitQuatf.at(1));
            q2 = str2float(splitQuatf.at(2));
            q3 = str2float(splitQuatf.at(3));
            type_impl->setValue(q0, q1, q2, q3);
            }
            ++i;
        }
        ++it;
    }
    
    settings->refresh();
    if (keyFrames > 0 && currentKeyFrame < currentMaxKeyFrame && currentKeyFrame < keyFrameData.size()) setKeyFrameData(keyFrameData.at(currentKeyFrame));
}

void ofApp::recordKeyFrame(int index) {
    std::map<std::string, ofxTweakbarType*> vars = settings->getVariables();
    std::map<std::string, ofxTweakbarType*>::iterator it = vars.begin();
    ofxTweakbarType* type = NULL;
    OFX_TW_TYPE tw_type;
    std::string section = settings->getName();
    
    int changedControlIndex = getChangedControlIndex();

    string sync_data = "" + (changedControlIndex == -1 ? "X" : ofToString(changedControlIndex) + "Z");
    
    while(it != vars.end()) {
        type = it->second;
        tw_type = type->getType();
        if(tw_type == OFX_TW_TYPE_INT32) {
            ofxTweakbarInt* type_impl = static_cast<ofxTweakbarInt*>(it->second);
            sync_data += "I";
            sync_data += ofToString(type_impl->getValue());
        }
        else if (tw_type == OFX_TW_TYPE_FLOAT) {
            ofxTweakbarFloat* type_impl = static_cast<ofxTweakbarFloat*>(it->second);
            sync_data += "F";
            sync_data += ofToString(type_impl->getValue());
        }
        else if (tw_type == OFX_TW_TYPE_COLOR3F) {
            ofxTweakbarColor3f* type_impl = static_cast<ofxTweakbarColor3f*>(it->second);
            IniVec3f v(type_impl->getX(), type_impl->getY(), type_impl->getZ());
            sync_data += "C";
            sync_data += ofToString(type_impl->getX());
            sync_data += ",";
            sync_data += ofToString(type_impl->getY());
            sync_data += ",";
            sync_data += ofToString(type_impl->getZ());
        }
        else if (tw_type == OFX_TW_TYPE_VEC3F) {
            ofxTweakbarVec3f* type_impl = static_cast<ofxTweakbarVec3f*>(it->second);
            IniVec3f v(type_impl->getX(), type_impl->getY(), type_impl->getZ());
            sync_data += "V";
            sync_data += ofToString(type_impl->getX());
            sync_data += ",";
            sync_data += ofToString(type_impl->getY());
            sync_data += ",";
            sync_data += ofToString(type_impl->getZ());
        }
        else if (tw_type == OFX_TW_TYPE_QUAT4F) {
            ofxTweakbarQuat4f* type_impl = static_cast<ofxTweakbarQuat4f*>(it->second);
            IniQuat4f v(type_impl->getX(), type_impl->getY(), type_impl->getZ(), type_impl->getS());
            sync_data += "Q";
            sync_data += ofToString(type_impl->getX());
            sync_data += ",";
            sync_data += ofToString(type_impl->getY());
            sync_data += ",";
            sync_data += ofToString(type_impl->getZ());
            sync_data += ",";
            sync_data += ofToString(type_impl->getS());
        } else {
            ++it;
            continue;
        }
        
        ++it;
        if (it != vars.end()) sync_data += "|";
    }
    
    sync_xml.setTo("syncdata");
    sync_xml.addValue("millis"+ofToString(index), millis);
    sync_xml.addValue("data"+ofToString(index), sync_data);
    sync_xml.save("sync.xml");
    currentMaxKeyFrame++;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    recordKeyFrame(currentMaxKeyFrame);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    recordKeyFrame(currentMaxKeyFrame);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    recordKeyFrame(currentMaxKeyFrame);
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
