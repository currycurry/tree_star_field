#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

// listen on port 12345
#define PORT 12345
#define NUM_MSG_STRINGS 20

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
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void addPoint(float x, float y, float z);

    
        // vector to store all values
        vector <ofVec3f> points;
        vector <ofVec3f> sizes;
        
        ofVbo vbo;
        ofShader shader;
        ofEasyCam camera;
        
        float camDist;
        
        ofTexture texture;
        
        bool bFullscreen;
        int showState, _showState;
        bool bPlaying;
        bool bDebug;
    
        // === FBO =============================
        
        void drawFbo();
        
        ofTexture starTexture;
        
        ofPoint mask_points[ 4 ];
        int cornerIndex = 0;
        
        ofFbo fbo;
        
        float rot_x, rot_y, rot_z;
    
        // === OSC =============================

        ofTrueTypeFont font;
        ofxOscReceiver receiver;

        int current_msg_string;
        string msg_strings[NUM_MSG_STRINGS];
        float timers[NUM_MSG_STRINGS];
    
        // === Video =============================
        
        ofVideoPlayer tree_video;
        
        int frame_x, frame_y, frame_w, frame_h;
        int tree_opacity;
        float tree_opacity_increment;
        float video_speed;
    
        int star_opacity;
        float star_opacity_increment;
    
        // === Gradient =============================
        
        ofImage bottomPaletteImage;
        ofPixels bottomPalettePixels;
        ofColor currentBottomColor;
        ofColor fromBottomColor;
        ofColor toBottomColor;
        
        int imgWidth;
        int lastColorIndex;
        int nextColorIndex;
        
        ofImage topPaletteImage;
        ofPixels topPalettePixels;
        ofColor currentTopColor;
        ofColor fromTopColor;
        ofColor toTopColor;
        
        float transSpeed; // moves 0 - 1 to transition fromColor toColor
        
        //color timers
        long startTime;
        long currentTime;
        long lastTime;
        long colorTime;
        int colorDuration; //how long each color lasts in ms
        int cycles;
        
        int cycleDuration;
        int colorPosition;
        
        bool bShowPalettes;

    
        // === Audio =============================
    
        ofSoundPlayer idle_audio;
        ofSoundPlayer tree_audio;
        float min_volume;
        float idle_volume;
        float idle_max_volume;
        float tree_volume;
        float tree_max_volume;

    
        // === Vignette Mask =============================
        ofImage vignette;


		
};
