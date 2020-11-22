#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxSyphon.h"

#define TOTAL 4
#define FONT_SIZE 80
#define FPS 60
#define WIDTH 800

struct Transmission {
    ofFbo * canvas;
    ofxSyphonServer * syphonServer;
    vector<vector<ofPath>> message;
    vector<int> posX;
    int messageWidth;
    float x;
    string text;
};

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    
    vector<Transmission> transmissions;
    vector<ofTrueTypeFont> fonts;
    vector<shared_ptr<string>> messages;
    
    void keyPressed(int key);
    void receiveMessage(shared_ptr<string> message);
};

#endif
