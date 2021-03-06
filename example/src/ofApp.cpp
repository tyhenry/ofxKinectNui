/******************************************************************/
/**
 * @file	ofApp.cpp
 * @brief	Example for ofxKinectNui addon
 * @note
 * @todo
 * @bug	
 *
 * @author	sadmb
 * @date	Oct. 28, 2011
 */
/******************************************************************/
#include "ofApp.h"
#include "ofxKinectNuiDraw.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	ofxKinectNui::InitSetting initSetting;
	initSetting.grabVideo = true;
	initSetting.grabDepth = true;
	initSetting.grabAudio = false;
	initSetting.grabLabel = true;
	initSetting.grabSkeleton = true;
	initSetting.grabCalibratedVideo = true;
	initSetting.grabLabelCv = false;
	initSetting.videoResolution = NUI_IMAGE_RESOLUTION_640x480;
	initSetting.depthResolution = NUI_IMAGE_RESOLUTION_640x480;
	kinect.init(initSetting);
//	kinect.setMirror(false); // if you want to get NOT mirror mode, uncomment here
//	kinect.setNearmode(true); // if you want to set nearmode, uncomment here
	kinect.open();

	kinect.addKinectListener(this, &ofApp::kinectPlugged, &ofApp::kinectUnplugged);
	
#ifdef USE_TWO_KINECTS
	// watch out that only the first kinect can grab label and skeleton.
	kinect2.init(true, true, false, false, false, false, false, true);
	kinect2.open();
#endif
	ofSetVerticalSync(true);

	kinectSource = &kinect;
	angle = kinect.getCurrentAngle();
	bRecord = false;
	bPlayback = false;
	bPlugged = kinect.isConnected();
	nearClipping = kinect.getNearClippingDistance();
	farClipping = kinect.getFarClippingDistance();
	
	bDrawVideo = false;
	bDrawDepthLabel = false;
	bDrawSkeleton = false;
	bDrawCalibratedTexture = false;

	ofSetFrameRate(60);
	
	calibratedTexture.allocate(kinect.getDepthResolutionWidth(), kinect.getDepthResolutionHeight(), GL_RGB);

	videoDraw_ = ofxKinectNuiDrawTexture::createTextureForVideo(kinect.getVideoResolution());
	depthDraw_ = ofxKinectNuiDrawTexture::createTextureForDepth(kinect.getDepthResolution());
	labelDraw_ = ofxKinectNuiDrawTexture::createTextureForLabel(kinect.getDepthResolution());
	skeletonDraw_ = new ofxKinectNuiDrawSkeleton();
	kinect.setVideoDrawer(videoDraw_);
	kinect.setDepthDrawer(depthDraw_);
	kinect.setLabelDrawer(labelDraw_);
	kinect.setSkeletonDrawer(skeletonDraw_);
}

//--------------------------------------------------------------
void ofApp::update() {
	kinectSource->update();
	if(bRecord){
		kinectRecorder.update();
	}
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(100, 100, 100);
	// Draw video only
	if(bDrawVideo){
		// draw video images from kinect camera
		kinect.drawVideo(0, 0, 1024, 768);
	// Draw depth + users label only
	}else if(bDrawDepthLabel){
		ofEnableAlphaBlending();

		// draw depth images from kinect depth sensor
		kinect.drawDepth(0, 0, 1024, 768);
		// draw players' label images on video images
		kinect.drawLabel(0, 0, 1024, 768);
		ofDisableAlphaBlending();
	// Draw skeleton only
	}else if(bDrawSkeleton){
		kinect.drawSkeleton(0, 0, 1024, 768);	// draw skeleton images on video images
	// Draw calibrated image only
	}else if(bDrawCalibratedTexture){
		ofPushMatrix();
		drawCalibratedTexture();	// draw calibrated images coodinates to depth images
		ofPopMatrix();
	}else{
		if(!bPlayback){
			// draw video images from kinect camera
			kinect.drawVideo(20, 20, 400, 300);
			ofEnableAlphaBlending();
			// draw depth images from kinect depth sensor
			kinect.drawDepth(20, 340, 400, 300);
			// draw players' label images on video images
			kinect.drawLabel(20, 340, 400, 300);
			ofDisableAlphaBlending();
			// draw skeleton images on video images
			kinect.drawSkeleton(20, 20, 400, 300);

#ifdef USE_TWO_KINECTS
			kinect2.draw(440, 20, 400, 300);
			kinect2.drawDepth(440, 340, 400, 300);
#endif
		}else{
			kinectPlayer.draw(20, 20, 400, 300);
			ofEnableAlphaBlending();
			kinectPlayer.drawDepth(20, 340, 400, 300);
			kinectPlayer.drawLabel(20, 340, 400, 300);

			ofDisableAlphaBlending();
			kinectPlayer.drawSkeleton(20, 20, 400, 300);
		}
	}

	ofPushMatrix();
	ofTranslate(35, 35);
	ofFill();
	if(bRecord) {
		ofSetColor(255, 0, 0);
		ofCircle(0, 0, 10);
	}
	if(bPlayback) {
		ofSetColor(0, 255, 0);
		ofTriangle(-10, -10, -10, 10, 10, 0);
	}
	ofPopMatrix();

	stringstream kinectReport;
	if(bPlugged && !kinect.isOpened() && !bPlayback){
		ofSetColor(0, 255, 0);
		kinectReport << "Kinect is plugged..." << endl;
		ofDrawBitmapString(kinectReport.str(), 200, 300);
	}else if(!bPlugged){
		ofSetColor(255, 0, 0);
		kinectReport << "Kinect is unplugged..." << endl;
		ofDrawBitmapString(kinectReport.str(), 200, 300);
	}

	// draw instructions
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "fps: " << ofGetFrameRate() << "  Kinect Nearmode: " << kinect.isNearmode() << endl
				 << "press 'c' to close the stream and 'o' to open it again, stream is: " << kinect.isOpened() << endl
				 << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
				 << "press LEFT and RIGHT to change the far clipping distance: " << farClipping << " mm" << endl
				 << "press '+' and '-' to change the near clipping distance: " << nearClipping << " mm" << endl
				 << "press 'r' to record and 'p' to playback, record is: " << bRecord << ", playback is: " << bPlayback << endl
				 << "press 'v' to show video only: " << bDrawVideo << ",      press 'd' to show depth + users label only: " << bDrawDepthLabel << endl
				 << "press 's' to show skeleton only: " << bDrawSkeleton << ",   press 'q' to show point cloud sample: " << bDrawCalibratedTexture;
	ofDrawBitmapString(reportStream.str(), 20, 648);
	
}

//--------------------------------------------------------------
void ofApp::drawCalibratedTexture(){
	ofPixels calibpix = kinect.getCalibratedVideoPixels();
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	int w = kinect.getDepthResolutionWidth();
	int h = kinect.getDepthResolutionHeight();
	ofShortPixels dis = kinect.getDistancePixels();
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			int dIndex = y * w + x;
			short distance = dis[dIndex];
			if(distance > 500 && distance < 3000){
				ofFloatColor c = ofFloatColor(calibpix[3 * dIndex] / 255.f, calibpix[3 * dIndex + 1] / 255.f, calibpix[3 * dIndex + 2] / 255.f);
				mesh.addColor(c);

				// z-position is just proper value
				mesh.addVertex(ofPoint(x, y, (1500 - distance) * 0.5f));
				mesh.addIndex(y * w + x);
			}else{
				ofFloatColor c = ofFloatColor(calibpix[3 * dIndex] / 255.f, calibpix[3 * dIndex + 1] / 255.f, calibpix[3 * dIndex + 2] / 255.f, 0.f);
				mesh.addColor(c);
				mesh.addVertex(ofPoint(x, y, 0));
				mesh.addIndex(y * w + x);
			}
		}
	}

	for(int y = 0; y < h - 1; y += 2){
		for(int x = 0; x < w - 1; x += 2){
			int pos1 = w * y + x;
			int pos2 = w * y + x + 1;
			int pos3 = w * (y + 1) + x;
			int pos4 = w * (y + 1) + x + 1;
			mesh.addTriangle(pos1, pos2, pos3);
			mesh.addTriangle(pos2, pos3, pos4);
		}
	}

	ofRotateY(mRotationY);
	ofRotateX(mRotationX);
	ofEnableAlphaBlending();
	mesh.drawVertices();
	ofDisableAlphaBlending();
}


//--------------------------------------------------------------
void ofApp::exit() {
	if(calibratedTexture.bAllocated()){
		calibratedTexture.clear();
	}

	if(videoDraw_) {
		videoDraw_->destroy();
		videoDraw_ = NULL;
	}
	if(depthDraw_) {
		depthDraw_->destroy();
		depthDraw_ = NULL;
	}
	if(labelDraw_) {
		labelDraw_->destroy();
		labelDraw_ = NULL;
	}
	if(skeletonDraw_) {
		delete skeletonDraw_;
		skeletonDraw_ = NULL;
	}
	kinect.setAngle(0);
	kinect.close();
	kinect.removeKinectListener(this);
	kinectPlayer.close();
	kinectRecorder.close();


#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch(key){
	case 'v': // draw video only
	case 'V':
		bDrawVideo = !bDrawVideo;
		if(bDrawVideo){
			bDrawCalibratedTexture = false;
			bDrawSkeleton = false;
			bDrawDepthLabel = false;
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case 'd': // draw depth + users label only
	case 'D':
		bDrawDepthLabel = !bDrawDepthLabel;
		if(bDrawDepthLabel){
			bDrawCalibratedTexture = false;
			bDrawVideo = false;
			bDrawSkeleton = false;
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case 's': // draw skeleton only
	case 'S':
		bDrawSkeleton = !bDrawSkeleton;
		if(bDrawSkeleton){
			bDrawCalibratedTexture = false;
			bDrawVideo = false;
			bDrawDepthLabel = false;
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case 'q': // draw point cloud example
	case 'Q':
		bDrawCalibratedTexture = !bDrawCalibratedTexture;
		if(bDrawCalibratedTexture){
			bDrawVideo = false;
			bDrawDepthLabel = false;
			bDrawSkeleton = false;
			glEnable(GL_DEPTH_TEST);
		}
		break;
	case 'o': // open stream
	case 'O':
		kinect.open();
		break;
	case 'c': // close stream
	case 'C':
		kinect.close();
		break;
	case 'r': // record stream
	case 'R':
		if(!bRecord){
			startRecording();
		}else{
			stopRecording();
		}
		break;
	case 'p': // playback recorded stream
	case 'P':
		if(!bPlayback){
			startPlayback();
		}else{
			stopPlayback();
		}
		break;
	case OF_KEY_UP: // up the kinect angle
		angle++;
		if(angle > 27){
			angle = 27;
		}
		kinect.setAngle(angle);
		break;
	case OF_KEY_DOWN: // down the kinect angle
		angle--;
		if(angle < -27){
			angle = -27;
		}
		kinect.setAngle(angle);
		break;
	case OF_KEY_LEFT: // increase the far clipping distance
		if(farClipping > nearClipping + 10){
			farClipping -= 10;
			kinectSource->setFarClippingDistance(farClipping);
		}
		break;
	case OF_KEY_RIGHT: // decrease the far clipping distance
		if(farClipping < 4000){
			farClipping += 10;
			kinectSource->setFarClippingDistance(farClipping);
		}
		break;
	case '+': // increase the near clipping distance
		if(nearClipping < farClipping - 10){
			nearClipping += 10;
			kinectSource->setNearClippingDistance(nearClipping);
		}
		break;
	case '-': // decrease the near clipping distance
		if(nearClipping >= 10){
			nearClipping -= 10;
			kinectSource->setNearClippingDistance(nearClipping);
		}
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	mRotationY = (x - 512) / 5;
	mRotationX = (384 - y) / 5;
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
void ofApp::kinectPlugged(){
	bPlugged = true;
}

//--------------------------------------------------------------
void ofApp::kinectUnplugged(){
	bPlugged = false;
}

//--------------------------------------------------------------
void ofApp::startRecording(){
	if(!bRecord){
		// stop playback if running
		stopPlayback();

		kinectRecorder.setup(kinect, "recording.dat");
		bRecord = true;
	}
}

//--------------------------------------------------------------
void ofApp::stopRecording(){
	if(bRecord){
		kinectRecorder.close();
		bRecord = false;
	}
}

//--------------------------------------------------------------
void ofApp::startPlayback(){
	if(!bPlayback){
		stopRecording();
		kinect.close();

		// set record file and source
		kinectPlayer.setup("recording.dat");
		kinectPlayer.loop();
		kinectPlayer.play();
		kinectSource = &kinectPlayer;
		bPlayback = true;
	}
}

//--------------------------------------------------------------
void ofApp::stopPlayback(){
	if(bPlayback){
		kinectPlayer.close();
		kinect.open();
		kinectSource = &kinect;
		bPlayback = false;
	}
}
