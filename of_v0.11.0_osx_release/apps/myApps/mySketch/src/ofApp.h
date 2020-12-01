#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxSyphon.h"

struct Content {
    vector<vector<ofPath>> paths;
    vector<int> posX;
    int messageWidth;
    float x;
    int ID;
    string text;
    bool inUse;
    
    void setup(int ID, shared_ptr<string> text, vector<ofTrueTypeFont> * fonts) {
        this->inUse = false;
        this->ID = ID;
        this->text = *text;
        this->messageWidth = fonts->at(0).getStringBoundingBox(this->text, 0, 0).width;
        
        for(int i = 0; i < fonts->size(); i++){
            vector<ofPath> filled = fonts->at(i).getStringAsPoints(this->text, true, true);
            vector<ofPath> outilined = fonts->at(i).getStringAsPoints(this->text, true, false);
            
            this->paths.push_back(filled);
            
            for(int j = 0; j < this->paths[i].size(); j++){
                vector<ofPolyline> p = outilined[j].getOutline();
                int x = p.size() > 0 ? p[0].getCentroid2D().x : 0;
                if(i==0)
                    this->posX.push_back(x);
                this->paths[i][j].translate(glm::vec2(-x, 0));
            }
        }
    }
};

struct Transmission {
    ofFbo * canvas;
    ofxSyphonServer * syphonServer;
    Content * content;
};

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    void exit();
    
    ofXml configXml;
    
    vector<Transmission> transmissions;
    vector<ofTrueTypeFont> fonts;
    vector<Content *> contents;
    
    void keyPressed(int key);
    void receiveMessage(int ID, shared_ptr<string> message);
    
    void loadData();
    void urlResponse(ofHttpResponse & response);
    
    bool hasId(int ID);
    
    float updateInterval;
    float lastUpdateTime = 0;
    
    vector<int> randomPipe;
};

#endif
