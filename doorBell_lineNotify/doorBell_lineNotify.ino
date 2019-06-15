/* 
 * Copyright (c) 2019 Sitinut Waisara
 * magiapp.me | github.com/maxmacstn
 * 
 * For sending notification + IP Cam snapshot image to Line Notify when Doorbell was pressed.
 * 
 * Hardware : Generic doorbell with active high or you can connect with button + generic Chinese P2P IP Cam
 * 
 */

#include <TridentTD_LineNotify.h>
#include <FS.h>
#include <ESP8266HTTPClient.h>

//Setup WiFi
#define SSID        "YOUR-WIFI-NAME"
#define PASSWORD    "YOUR-WIFI-PASSWORD"

//Setup IP Camera (Generic P2P IP Camera)
String IPCAM_IP  =  "YOUR-IP-CAM-IP-ADDRESS";
String IPCAM_USERNAME = "YOUR-IP-CAM-IP-USERNAME";
String IPCAM_PASSWORD = "YOUR-IP-CAM-IP-PASSWORD";


#define TOKENCOUNT  1  //How many token that you want to send? (Size of array below)
String tokens[TOKENCOUNT] = {"YOUR-LINE-TOKEN"};

int ledPin = D0;
int signalPin = D1;
bool ipCameraEnabled = false;     //Use IP Camera or just send the message?


void downloadAndSaveFile(String fileName, String  url){
  
  HTTPClient http;

  Serial.println("[HTTP] begin...\n");
  Serial.println(fileName);
  Serial.println(url);
  http.begin(url);
  
  Serial.printf("[HTTP] GET...\n", url.c_str());
  // start connection and send HTTP header
  int httpCode = http.GET();
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      Serial.printf("[FILE] open file for writing %d\n", fileName.c_str());
      
      File file = SPIFFS.open(fileName, "w");

      // file found at server
      if(httpCode == HTTP_CODE_OK) {

          // get lenght of document (is -1 when Server sends no Content-Length header)
          int len = http.getSize();

          // create buffer for read
          uint8_t buff[128] = { 0 };

          // get tcp stream
          WiFiClient * stream = http.getStreamPtr();

          // read all data from server
          while(http.connected() && (len > 0 || len == -1)) {
              // get available data size
              size_t size = stream->available();
              if(size) {
                  // read up to 128 byte
                  int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                  // write it to Serial
                  //Serial.write(buff, c);
                  file.write(buff, c);
                  if(len > 0) {
                      len -= c;
                  }
              }
              delay(1);
          }

          Serial.println();
          Serial.println("[HTTP] connection closed or file end.\n");
          Serial.println("[FILE] closing file\n");
          file.close();
          
      }
      
  }
  http.end();
}



void sendLineNotify(){
  digitalWrite(D0,LOW);

  

  for (int i = 0; i < TOKENCOUNT; i++){
      Serial.println("Send message " + i +1);
      LINE.setToken(tokens[i]);
      LINE.notify("Someone at frontdoor");
  }

  if(ipCameraEnabled){
     downloadAndSaveFile("/snapshot.jpg","http://"+ IPCAM_IP +"/snapshot.cgi?user="+ IPCAM_USERNAME+"&pwd="+ IPCAM_PASSWORD);
    //listFiles();  
    for (int i = 0; i < TOKENCOUNT; i++){
      Serial.println("Send image " + i +1);
      LINE.setToken(tokens[i]);
      LINE.notifyPicture("Camera snapshot", SPIFFS, "/snapshot.jpg");  
      }
    
  }
  
   digitalWrite(D0,HIGH);
  
}


//List files in SPIFFS (For debugging)
void listFiles(void) {
  Serial.println();
  Serial.println("SPIFFS files found:");

  Dir dir = SPIFFS.openDir("/"); // Root directory
  String  line = "=====================================";

  Serial.println(line);
  Serial.println("  File name               Size");
  Serial.println(line);

  while (dir.next()) {
    String fileName = dir.fileName();
    Serial.print(fileName);
    int spaces = 25 - fileName.length(); // Tabulate nicely
    while (spaces--) Serial.print(" ");
    File f = dir.openFile("r");
    Serial.print(f.size()); Serial.println(" bytes");
  }

  Serial.println(line);
  Serial.println();
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(signalPin,INPUT);
  pinMode(D0,OUTPUT);
  digitalWrite(D0,HIGH);
  
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  delay(1000);

  
  if(ipCameraEnabled){
    //Initialize File System
    if(SPIFFS.begin()){
      Serial.println("SPIFFS Initialize....ok");
    }else{
      Serial.println("SPIFFS Initialization...failed");
    }
   
    //Format File System
    if(SPIFFS.format()){
      Serial.println("File System Formated");
    }else{
      Serial.println("File System Formatting Error");
    }
  }

  Serial.println("-- Doorbell Line notify READY --");


}

void loop() {
  if(digitalRead(signalPin) == HIGH){
    Serial.println("Doorbell Pressed");
    sendLineNotify();
    delay(2000);
  }

}
