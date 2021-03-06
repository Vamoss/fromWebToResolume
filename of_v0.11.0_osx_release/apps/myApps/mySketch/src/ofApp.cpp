#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("Vamoss - Web to Resolume");
    
    ofXml rootXml;
    if(!rootXml.load("config.xml")){
        ofLogError() << "Couldn't load file: config.xml";
    }
    
    configXml = rootXml.getChild("ROOT");
    cout << "FPS.............." << configXml.getChild("FPS").getIntValue() << endl;
    cout << "TOTAL............" << configXml.getChild("TOTAL").getIntValue() << endl;
    cout << "FONT_SIZE........" << configXml.getChild("FONT_SIZE").getIntValue() << endl;
    cout << "WIDTH............" << configXml.getChild("WIDTH").getIntValue() << endl;
    cout << "URL.............." << configXml.getChild("URL").getValue() << endl;
    cout << "UPDATE_INTERVAL.." << configXml.getChild("UPDATE_INTERVAL").getFloatValue() << endl;
    
    ofSetFrameRate(configXml.getChild("FPS").getIntValue());
    
    ofDirectory dir("fonts");
    dir.allowExt("ttf");
    dir.listDir();
    dir.sort();
    for(int i = 0; i < dir.size(); i++){
        cout << "Font: " << dir.getPath(i) << endl;
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
        
        t.content = NULL;
        
        transmissions.push_back(t);
    }
    
    ofRegisterURLNotification(this);
    updateInterval = configXml.getChild("UPDATE_INTERVAL").getFloatValue();
}

//--------------------------------------------------------------
void ofApp::exit() {
    ofUnregisterURLNotification(this);
}

//--------------------------------------------------------------
void ofApp::update(){
    float deltaTime = ofGetLastFrameTime();
    float time = ofGetElapsedTimef();
    
    if(time - lastUpdateTime > updateInterval){
        lastUpdateTime = time;
        loadData();
    }
    
    //sort pipe
    if (randomPipe.size() == 0) {
        for (int i = 0; i < contents.size(); i++) {
            randomPipe.push_back(i);
        }
        random_shuffle(randomPipe.begin(), randomPipe.end());
    }
    
    for(int k = 0; k < transmissions.size() && randomPipe.size() > 0; k++){
        Transmission * t = &transmissions[k];
        
        //exclude content
        if(t->content){
            if(t->content->x < -t->content->messageWidth){
                t->content->inUse = false;
                t->content = NULL;
            }
        }
        
        //attach content
        if(t->content == NULL){
            int index = randomPipe.size()-1;
            while(index > 0 && contents[randomPipe[index]]->inUse){
                index--;
            }
            
            Content * content = contents[randomPipe[index]];
            randomPipe.pop_back();
            
            t->content = content;
            content->inUse = true;
            t->content->x = t->canvas->getWidth();
        }
        
        //draw content
        t->content->x -= deltaTime * 100;
        t->canvas->begin();
            ofClear(0);
            ofSetColor(0);
            if(t->content->paths.size() > 0){
                for (int i = 0; i < t->content->paths[0].size(); i++) {
                    float noise = ofNoise((float)i / 20.0f, (float)k/10.0f, time);
                    int randomFont = floor(noise*t->content->paths.size());
                    t->content->paths[randomFont][i].draw(t->content->x + t->content->posX[i], t->canvas->getHeight() * 3 / 4);
                }
            }
        t->canvas->end();
        t->syphonServer->publishTexture(&t->canvas->getTexture());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(255);
    int y = 0;
    for(int i = 0; i < transmissions.size(); i++){
        Transmission t = transmissions[i];
        int w = t.canvas->getWidth() * 0.8;
        int h = t.canvas->getHeight() * 0.8;
        t.canvas->draw(0, y, w, h);
        y += h + 10;
    }
    
    
    int x = ofGetWidth()-200;
    ofDrawBitmapStringHighlight(ofToString(contents.size()) + " mensagens recebidas", x, 15);
    ofDrawBitmapStringHighlight(ofToString(randomPipe.size()) + " na fila", x, 36);
    
    //draw in reverse, as the last is the first message to appear
    ofSetColor(0);
    for(int i = randomPipe.size()-1; i >= 0; i--){
        int y = 55 + (randomPipe.size() - 1 - i) * 20;
        ofDrawBitmapString(ofToString(randomPipe.size() - i) + " " + contents[randomPipe[i]]->text, x, y);
    }
}

void ofApp::reset() {
    for(int i = 0; i < transmissions.size(); i++){
        transmissions[i].content = NULL;
        transmissions[i].canvas->begin();
            ofClear(0);
        transmissions[i].canvas->end();
    }
    for(int i = 0; i < contents.size(); i++){
        delete contents[i];
    }
    contents.clear();
    randomPipe.clear();
}

//--------------------------------------------------------------
void ofApp::receiveMessage(int ID, shared_ptr<string> message) {
    Content * content = new Content();
    content->setup(ID, message, &fonts);
    contents.push_back(content);
    randomPipe.push_back(contents.size()-1);
}

//--------------------------------------------------------------
void ofApp::loadData(){
    ofLoadURLAsync(configXml.getChild("URL").getValue(),"frases");
}

//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response){
    if(response.status==200 && response.request.name == "frases"){
        cout << "loaded " << response.data << endl;
        
        for (int i = 0; i < contents.size(); i++) {
            contents[i]->shouldDelete = true;
        }
        
        ofJson json = ofJson::parse(response.data);
        for(auto & message: json){
            if(!message.empty()){
                int ID = message["id"].get<int>();
                vector<Content *>::iterator foundIterator = findId(ID);
                if(foundIterator == contents.end()){
                    shared_ptr<string> text = make_shared<string>(message["mensagem"]);
                    receiveMessage(ID, text);
                    
                    cout << "Loaded: " << ID << " " << *text << endl;
                }else{
                    (*foundIterator)->shouldDelete = false;
                }
            }
        }
        
        //if any message was deleted, reset everything
        bool shoudReset = false;
        for (int i = contents.size()-1; i>=0; i--)
            shoudReset |= contents[i]->shouldDelete;
        if(shoudReset)
            reset();
    }else{
        cout << response.status << " " << response.error << " for request " << response.request.name << endl;
    }
}

vector<Content *>::iterator ofApp::findId(int ID){
    return find_if(contents.begin(), contents.end(), [ID](const Content * c){
        return c->ID == ID;
    });
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
        receiveMessage(floor(ofRandom(10000, 1000000)), r);
    }
}
