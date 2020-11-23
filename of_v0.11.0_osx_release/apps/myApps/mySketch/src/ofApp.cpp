#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("Vamoss - Web to Resolume");
    
    ofXml rootXml;
    if(!rootXml.load("config.xml")){
        ofLogError() << "Couldn't load file: config.xml";
    }
    
    configXml = rootXml.getChild("ROOT");
    cout << "FPS........." << configXml.getChild("FPS").getIntValue() << endl;
    cout << "TOTAL......." << configXml.getChild("TOTAL").getIntValue() << endl;
    cout << "FONT_SIZE..." << configXml.getChild("FONT_SIZE").getIntValue() << endl;
    cout << "WIDTH......." << configXml.getChild("WIDTH").getIntValue() << endl;
    cout << "URL........." << configXml.getChild("URL").getValue() << endl;
    
    ofSetFrameRate(configXml.getChild("FPS").getIntValue());
    
    ofDirectory dir("fonts");
    dir.allowExt("ttf");
    dir.listDir();
    for(int i = 0; i < dir.size(); i++){
        ofTrueTypeFont font;
        font.load(dir.getPath(i), configXml.getChild("FONT_SIZE").getIntValue(), true, true, true);
        fonts.push_back(font);
    }
    
    for(int i = 0; i < configXml.getChild("TOTAL").getIntValue(); i++){
        Transmission t;
        
        t.syphonServer = new ofxSyphonServer();
        t.syphonServer->setName("Texture " + ofToString(i));
        
        t.canvas = new ofFbo();
        t.canvas->allocate(configXml.getChild("WIDTH").getIntValue(), configXml.getChild("FONT_SIZE").getIntValue()*1.5, GL_RGBA);
        
        t.x = 0;
        t.messageWidth = 0;
        
        transmissions.push_back(t);
    }
    
    ofRegisterURLNotification(this);
    
    loadData();
}

//--------------------------------------------------------------
void ofApp::exit() {
    ofUnregisterURLNotification(this);
}

//--------------------------------------------------------------
void ofApp::update(){
    float deltaTime = ofGetLastFrameTime();
    int randomIndexToAdd = floor(ofRandom(transmissions.size()));
    for(int k = 0; k < transmissions.size(); k++){
        Transmission * t = &transmissions[k];
        
        if(t->x < -t->messageWidth){
            t->message.clear();
            t->posX.clear();
            
            if(messages.size() > 0 && k == randomIndexToAdd){
                //set message
                string m = *messages[0];
                messages.erase(messages.begin());
                
                for(int i = 0; i < fonts.size(); i++){
                    vector<ofPath> filled = fonts[i].getStringAsPoints(m, true, true);
                    vector<ofPath> outilined = fonts[i].getStringAsPoints(m, true, false);
                    
                    t->message.push_back(filled);
                
                    for(int j = 0; j < t->message[i].size(); j++){
                        vector<ofPolyline> p = outilined[j].getOutline();
                        int x = p.size() > 0 ? p[0].getCentroid2D().x : 0;
                        if(i==0)
                            t->posX.push_back(x);
                        t->message[i][j].translate(glm::vec2(-x, 0));
                    }
                }
                t->messageWidth = fonts[0].getStringBoundingBox(m, 0, 0).width;
                t->x = t->canvas->getWidth();
            }
        }
        
        t->x -= deltaTime * 100;
        t->canvas->begin();
            ofClear(0);
            if(t->message.size() > 0){
                for (int i = 0; i < t->message[0].size(); i++) {
                    int randomFont = floor(ofRandom(t->message.size()));
                    t->message[randomFont][i].draw(t->x + t->posX[i], t->canvas->getHeight() * 3 / 4);
                }
            }
        t->canvas->end();
        t->syphonServer->publishTexture(&t->canvas->getTexture());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    int y = 0;
    for(int i = 0; i < transmissions.size(); i++){
        Transmission t = transmissions[i];
        int w = t.canvas->getWidth() * 0.8;
        int h = t.canvas->getHeight() * 0.8;
        t.canvas->draw(0, y, w, h);
        y += h + 10;
    }
}

//--------------------------------------------------------------
void ofApp::receiveMessage(shared_ptr<string> message) {
    messages.push_back(message);
}

//--------------------------------------------------------------
void ofApp::loadData(){
    ofLoadURLAsync(configXml.getChild("URL").getValue(),"frases");
}

//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response){
    if(response.status==200 && response.request.name == "frases"){
        cout << "loaded " << response.data << endl;
        ofJson json = ofJson::parse(response.data);
        for(auto & message: json){
            if(!message.empty()){
                receiveMessage(make_shared<string>(message["mensagem"]));
                
                struct tm tm;
                istringstream ss = istringstream((string)message["createdAt"]);
                ss >> get_time(&tm, "%Y-%M-%dT%%H:%M:%S");
                //this timestamp is not timezone accurate
                time_t time = mktime(&tm);
                
                cout << "-----" << endl;
                cout << message["createdAt"] << endl;
                cout << time << endl;
            }
        }
    }else{
        cout << response.status << " " << response.error << " for request " << response.request.name << endl;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    if(key == 't'){
        const vector<string> v = {
            u8"ANTIRRACISTA",
            u8"VIDAS NEGRAS IMPORTAM",
            u8"EQUIDADE",
            u8"MARIELLE PRESENTE",
            u8"LUTA",
            u8"DEFENDA O SUS",
            u8"DITADURA NUNCA MAIS",
            u8"FORA BOLSONARO",
            u8"R$89 MIL",
            u8"COVIDA",
            u8"FICA BEM",
            u8"168.687 MORTES"
        };
        
        shared_ptr<string> r = make_shared<string>(v[floor(ofRandom(v.size()))]);
        receiveMessage(r);
    }
}
