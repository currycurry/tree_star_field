#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    
    ofBackgroundHex(0x000000);
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    
    // load the texure
    ofDisableArbTex();
    ofLoadImage( texture, "star2.png");
    
    // set the camera distance
    camDist  = 1600;
    camera.setDistance(camDist);
    
    // randomly add a point on a sphere
    int   num = 10000;
    //float radius = 1000;
    for(int i = 0; i < num; i++ ) {
        
        float radius = ofRandom( 1000, 7000 );
        
        
        float theta1 = ofRandom(0, TWO_PI);
        float theta2 = ofRandom(0, TWO_PI);
        
        ofVec3f p;
        p.x = cos( theta1 ) * cos( theta2 );
        p.y = sin( theta1 );
        p.z = cos( theta1 ) * sin( theta2 );
        p *= radius;
        
        addPoint(p.x, p.y, p.z);
        
    }
    
    // upload the data to the vbo
    int total = (int)points.size();
    vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
    vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
    
    
    // load the shader
#ifdef TARGET_OPENGLES
    shader.load("shaders_gles/shader");
#else
    shader.load("shaders/shader");
#endif
    
    bFullscreen = false;
    
    // === FBO =============================
    
    fbo.allocate( 1280, 800, GL_RGBA );
    starTexture.allocate( 1280, 800, GL_RGBA );
    
    /*mask_points[ 0 ].x = 0; mask_points[ 0 ].y = ofGetHeight() - 1600;
    mask_points[ 1 ].x = ofGetWidth(); mask_points[ 1 ].y = ofGetHeight() - 1600;
    mask_points[ 2 ].x = ofGetWidth(); mask_points[ 2 ].y = ofGetHeight();
    mask_points[ 3 ].x = 0; mask_points[ 3 ].y = ofGetHeight();*/
    
    mask_points[ 0 ].x = 0; mask_points[ 0 ].y = 0;
    mask_points[ 1 ].x = 1280; mask_points[ 1 ].y = 0;
    mask_points[ 2 ].x = 1280; mask_points[ 2 ].y = 800;
    mask_points[ 3 ].x = 0; mask_points[ 3 ].y = 800;
    
    ofBackground( 0 );
    
    rot_x = .001;
    rot_y = -.001;
    rot_z = .001;
    
    // === OSC =============================
    
    // listen on the given port
    cout << "listening for osc messages on port: " << PORT << "\n";
    receiver.setup(PORT);

    // === Video =============================
    // show movies with alpha channels
    tree_video.setPixelFormat(OF_PIXELS_RGBA);
    tree_video.load("silhouette_2_14.mov");
    tree_video.setLoopState( OF_LOOP_NONE );
    
    tree_opacity = 0;
    tree_opacity_increment = 0.0;
    
    frame_w = tree_video.getWidth() / tree_video.getHeight() * fbo.getHeight();
    frame_h = fbo.getHeight();
    frame_x = ( fbo.getWidth() - frame_w ) / 2;
    frame_y = ( fbo.getHeight() - frame_h );
    
    cout << "( x, y, w, h ): " << "( " << frame_x << ", " << frame_y << ", " << frame_w << ", " << frame_h << " )" << endl;
    
    star_opacity = 0;
    star_opacity_increment = 0.0;
    
    // === Gradient =============================
    topPaletteImage.load( "top_grad_6.png" );
    bottomPaletteImage.load( "bottom_grad_6.png" );
    topPalettePixels = topPaletteImage.getPixels();
    bottomPalettePixels = bottomPaletteImage.getPixels();
    
    //color timers
    startTime = ofGetSystemTime();
    currentTime = startTime;
    lastTime = 0;
    colorTime = 0;
    float minutes = 6.3;
    imgWidth = topPaletteImage.getWidth();
    cycleDuration = 1000 * 60 * minutes;
    colorDuration = cycleDuration / imgWidth;
    colorPosition = imgWidth + 1;
    transSpeed = 0;
    cycles = 1;
    
    currentTopColor = ofColor( 0 );
    currentBottomColor = ofColor( 0 );
    fromTopColor = ofColor( 0 );
    toTopColor = ofColor( 0 );
    fromBottomColor = ofColor( 0 );
    toBottomColor = ofColor( 0 );
    
    bShowPalettes = false;
    bPlaying = true;
    
    // === Audio =============================
    idle_audio.load( "Tree_External_Idle_Norm.wav" );
    idle_audio.setLoop( true );
    idle_volume = 0.0;
    idle_max_volume = 1.0;
    idle_audio.setVolume( idle_volume );
    idle_audio.play();
    
    tree_audio.load( "Tree_External_Main_Norm.wav" );
    tree_audio.setLoop( false );
    tree_volume = 0.0;
    tree_audio.setVolume( tree_volume );
    tree_max_volume = 1.0;

    // === Vignette Mask =============================
    vignette.allocate( 1280, 800, OF_IMAGE_COLOR_ALPHA );
    vignette.load( "vignette_1.png" );


}

//--------------------------------------------------------------
void ofApp::update(){
    
    ofSetFullscreen( bFullscreen );
    
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if ( m.getAddress() == "/show" ) {
            showState = m.getArgAsInt32( 0 );
            cout << "showState: " << ofToString( m.getArgAsInt32( 0 )) << endl;
        }
    }
    
    tree_video.update();
    
    ////////Gradient timers
    if ( bPlaying ) {
        if ( colorPosition < imgWidth ) {
            transSpeed = (float) colorTime / colorDuration;
            currentTime = ofGetSystemTime() - startTime;//how long the sketch has been running in m
            
            currentTopColor = fromTopColor.getLerped( toTopColor, transSpeed );
            currentBottomColor = fromBottomColor.getLerped( toBottomColor, transSpeed );
            
            if ( colorTime >= colorDuration ) {
                cycles ++;
                lastColorIndex = nextColorIndex;
                nextColorIndex ++;
                
                if ( nextColorIndex > imgWidth - 1 ) {
                    nextColorIndex = 0;
                }
                
                fromTopColor = topPalettePixels.getColor( lastColorIndex, 0 );
                toTopColor = topPalettePixels.getColor( nextColorIndex, 0 );
                
                fromBottomColor = bottomPalettePixels.getColor( lastColorIndex, 0 );
                toBottomColor = bottomPalettePixels.getColor( nextColorIndex, 0 );
                
                colorPosition ++;
                
            }
            
            
            if ( cycles > 1 ) {
                colorTime = currentTime - ((cycles - 1 ) * colorDuration );
            }
            
            else {
                colorTime = currentTime;
            }
        }
        
        else {
            
            currentTopColor = ofColor( 0 );
            currentBottomColor = ofColor( 0 );
            
        }
    }
    
    else {
        if ( transSpeed < .99 ) {
            //fade it out
            currentTopColor = fromTopColor.getLerped( toTopColor, transSpeed );
            currentBottomColor = fromBottomColor.getLerped( toBottomColor, transSpeed );
            
        }
        else {
            currentTopColor = ofColor( 0 );
            currentBottomColor = ofColor( 0 );
        }
        
    }
    
    /////////
    
    /////////play triggered
    if ( showState == 1 && _showState != 1 ) {
        
        cout << "play triggered" << endl;
        tree_audio.setPosition( 0.0 );
        tree_audio.setVolume( 1.0 );
        tree_audio.play();
        tree_video.setPosition( 0.0 );
        tree_video.play();
        transSpeed = 0.0;
        toTopColor = ofColor( 0 );
        toBottomColor = ofColor( 0 );
        fromTopColor = ofColor( 0 );
        fromBottomColor = ofColor( 0 );
        startTime = ofGetSystemTime();
        currentTime = startTime;
        lastTime = 0;
        colorTime = 0;
        colorPosition = 0;
        cycles = 0;
        lastColorIndex = 0;
        nextColorIndex = 0;
        bPlaying = true;
        
    }
    
    if ( showState == 0 && _showState != 0 ) {
        cout << "reset triggered" << endl;
        bPlaying = false;
        transSpeed = 0.0;
        toTopColor = ofColor( 0 );
        toBottomColor = ofColor( 0 );
        lastTime = 0;
        colorTime = 0;
        colorPosition = imgWidth + 1;
        cycles = 0;
        lastColorIndex = 0;
        nextColorIndex = 0;
        
    }
    
    _showState = showState;
    
    
    if ( showState == 0 ) {
        
        //fade up idle audio
        if ( idle_volume < idle_max_volume ) {
            idle_volume += .003; //10 second fade at 30 fps
            idle_audio.setVolume( idle_volume );
            //cout << "fading in idle audio: " << idle_volume << endl;
        }
        
        //fade in idle video
        if ( star_opacity_increment <= 1.0 ) {
            star_opacity_increment += .005;
            //cout << "fading in idle video: " << star_opacity_increment << endl;
        }
        
        star_opacity = (int) ( star_opacity_increment * 255 );
        //fade down tree audio
        if ( tree_volume > 0 ) {
            tree_volume -= .003;
            tree_audio.setVolume( tree_volume );
            //cout << "fading out tree audio: " << tree_volume << endl;
        }
        
        //fade out tree video
        if ( tree_opacity_increment > 0.0 ) {
            tree_opacity_increment -= .005;
            //cout << "fading out tree video: " << tree_opacity_increment << endl;
            
        }
        tree_opacity = (int) ( tree_opacity_increment * 255 );
        //cout << "tree_opacity: " << tree_opacity << endl;
        
        
        if ( transSpeed <= .99 ) {
            transSpeed += .005;
        }
        else {
            transSpeed = 1;
        }
        
        
    }
    
    if ( showState == 1 ) {
        
        //fade down idle audio
        if ( idle_volume > 0 ) {
            idle_volume -= .003;
            idle_audio.setVolume( idle_volume );
            //cout << "fading out idle audio: " << idle_volume << endl;
        }
        
        //fade out idle video
        if ( star_opacity_increment > 0.0 ) {
            star_opacity_increment -= .005;
            //cout << "fading out idle video: " << star_opacity_increment << endl;
            
        }
        
        star_opacity = (int) ( star_opacity_increment * 255 );
        
        
        //fade in tree video
        if ( tree_opacity_increment < 1.0 ) {
            tree_opacity_increment += .005;
            //cout << "fading in tree video: " << tree_opacity_increment << endl;
            
        }
        
        tree_opacity = (int) ( tree_opacity_increment * 255 );
        //cout << "tree_opacity: " << tree_opacity << endl;
        
        
        //fade in tree audio
        if ( tree_volume < tree_max_volume ) {
            tree_volume += .003; //10 second fade at 30 fps
            tree_audio.setVolume( tree_volume );
            //cout << "fading in tree audio: " << tree_volume << endl;
            
        }
        
        if ( tree_video.getPosition() >= .78 ) {
            
            showState = 2;
            
        }
    }
    
    if ( showState == 2 ) {
        
        //fade in idle video
        if ( star_opacity_increment < 1.0 ) {
            star_opacity_increment += .001;
            //cout << "fading in idle video: " << star_opacity_increment << endl;
            
        }
        
        star_opacity = (int) ( star_opacity_increment * 255 );
        
        //fade up idle audio
        if ( idle_volume < idle_max_volume ) {
            idle_volume += .001;
            idle_audio.setVolume( idle_volume );
            //cout << "fading in idle audio: " << idle_volume << endl;
            
        }
        
        if ( tree_video.getPosition() >= .89 ) {
            
            showState = 3;
            
        }
        
    }
    
    if ( showState == 3 ) {
        //fade out tree video
        if ( tree_opacity_increment >= 0.0 ) {
            tree_opacity_increment -= .002;
            //cout << "fading out tree video: " << tree_opacity_increment << endl;
            
        }
        
        tree_opacity = (int) ( tree_opacity_increment * 255 );
        
        
        if ( tree_video.getIsMovieDone() ) {
            showState = 0;
            //cout << "tree video ended" << endl;
            
        }
        
    }
    
    camera.tilt( rot_x ); // tilt up+down (around local x axis)
    camera.pan( rot_y ); // rotate left+right (around local y axis)
    camera.roll( rot_z );	// roll left+right (around local z axis)
    
    if ( camDist < 8000 && camDist > 500 ) {
        camDist += .05;
        rot_x = .001;
        rot_y = -.001;
        rot_z = .001;
    }
    else {
        camDist -= .05;
        rot_x = -.001;
        rot_y = +.001;
        rot_z = -.001;
    }
    
    camera.setDistance(camDist);

    
    drawFbo();
    

}


//--------------------------------------------------------------
void ofApp::drawFbo(){
    
    fbo.begin();
    ofClear(255,255,255, 0);
    ofSetColor( 255, 255, 255, tree_opacity );
    ofBackgroundGradient( currentTopColor, currentBottomColor, OF_GRADIENT_LINEAR );

    ofEnableAlphaBlending();
    glDepthMask(GL_FALSE);
    
    ofSetColor(255, 255, 255, star_opacity );
    
    // this makes everything look glowy :)
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnablePointSprites();
    
    // bind the shader and camera
    // everything inside this function
    // will be effected by the shader/camera
    shader.begin();
    camera.begin();
    
    // bind the texture so that when all the points
    // are drawn they are replace with our dot image
    texture.bind();
    vbo.draw(GL_POINTS, 0, (int)points.size());
    texture.unbind();
    
    camera.end();
    shader.end();
    
    ofDisablePointSprites();
    ofDisableBlendMode();
    
    // check to see if the points are
    // sizing to the right size
    ofEnableAlphaBlending();
    camera.begin();
    for (unsigned int i=0; i<points.size(); i++) {
        ofSetColor( 255, 255, 255, 125 );
        ofVec3f mid = points[i];
        mid.normalize();
        mid *= 300;
        //ofDrawLine(points[i], mid);
    }
    camera.end();
    
    glDepthMask(GL_TRUE);
    
    if ( bDebug ) {
        
        ofSetColor(255, 140);
        ofDrawRectangle(0, 0, 250, 120);
        ofSetColor(0);
        string info = "FPS "+ofToString(ofGetFrameRate(), 0) + "\n";
        info += "Total Points "+ofToString((int)points.size())+"\n";
        info += "Show State: " + ofToString( showState ) + "\n";
        info += "Video Position: " + ofToString( tree_video.getPosition()) + "\n";
        info += "Tree Opacity: " + ofToString( tree_opacity ) + "\n";
        info += "Star Opacity: " + ofToString( star_opacity ) + "\n";
        info += "Show Volume: " + ofToString( tree_volume ) + "\n";
        info += "Idle Volume: " + ofToString( idle_volume );
        ofDrawBitmapString(info, 20, 20);
    }
    
    ofEnableAlphaBlending();
    
    //ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    ofSetColor( 255, 255, 255, tree_opacity );
    tree_video.draw( frame_x, frame_y, frame_w, frame_h );
    
    ofSetColor( 255, 255, 255, 255 );
    vignette.draw( 0, 0, fbo.getWidth(), fbo.getHeight() + 300);

    ofDisableAlphaBlending();
    
    if ( bShowPalettes ) {
        ofSetColor( 255, 255, 255 );
        topPaletteImage.draw( 10, 20, 1000, 50 );
        bottomPaletteImage.draw( 10, 90, 1000, 50 );
        
        ofSetColor( currentTopColor );
        ofDrawRectangle( 5 + colorPosition, 10, 10, 10 );
        
        ofSetColor( currentBottomColor );
        ofDrawRectangle( 5 + colorPosition, 80, 10, 10 );
    }
    
    fbo.end();
    
    starTexture = fbo.getTexture();
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    starTexture.draw( mask_points[ 0 ], mask_points[ 1 ], mask_points[ 2 ], mask_points[ 3 ] );
    


}

//--------------------------------------------------------------
void ofApp::addPoint(float x, float y, float z) {
    ofVec3f p(x, y, z);
    points.push_back(p);
    
    // we are passing the size in as a normal x position
    float size = ofRandom( 2, 40 );
    sizes.push_back(ofVec3f(size));
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch ( key ) {
            
        case 'f':
            bFullscreen = !bFullscreen;
            if ( bFullscreen ) {
                ofHideCursor();
            }
            else {
                ofShowCursor();
            }
            break;
            
        case 'd':
            bDebug = !bDebug;
            break;
            
        case '1':
            cornerIndex = 0;
            break;
            
        case '2':
            cornerIndex = 1;
            break;
            
        case '3':
            cornerIndex = 2;
            break;
            
        case '4':
            cornerIndex = 3;
            break;
            
        case '9':
            showState = 1;
            break;
            
        case '0':
            showState = 0;
            break;

        case OF_KEY_LEFT:
            mask_points[ cornerIndex ].x -= 5;
            cout <<"mask_points[ " << cornerIndex << " ].x = " << mask_points[ cornerIndex ].x << endl;
            break;
            
        case OF_KEY_RIGHT:
            mask_points[ cornerIndex ].x += 5;
            cout <<"mask_points[ " << cornerIndex << " ].x = " << mask_points[ cornerIndex ].x << endl;
            break;
            
        case OF_KEY_UP:
            mask_points[ cornerIndex ].y -= 5;
            cout <<"mask_points[ " << cornerIndex << " ].y = " << mask_points[ cornerIndex ].y << endl;
            break;
            
        case OF_KEY_DOWN:
            mask_points[ cornerIndex ].y += 5;
            cout <<"mask_points[ " << cornerIndex << " ].y = 3" << mask_points[ cornerIndex ].y << endl;
            break;
            
            // add crazy amount
        case 'a':
            float theta1 = ofRandom(0, TWO_PI);
            float theta2 = ofRandom(0, TWO_PI);
            ofVec3f p;
            p.x = cos( theta1 ) * cos( theta2 );
            p.y = sin( theta1 );
            p.z = cos( theta1 ) * sin( theta2 );
            p  *= 800;
            addPoint(p.x, p.y, p.z);
            int total = (int)points.size();
            vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
            vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
            break;
            
            
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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
