#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ESP8266FtpServer.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// #include <SoftwareSerial.h>

enum SERIAL_MODE {FREE, COMMAND, DATA_IN, DATA_OUT, FILES_OUT};

const byte LF=10;
const byte CR=13;
const byte EF=26;

String IpAddr = "";

// FTP credentials
const char* ftpuser = "esp8266";
const char* ftppswd = "esp8266";

#define LED_BUILTIN 2



// WiFi credentials
const char* ssid = "ssid";
const char* password = "password";
 
ESP8266WebServer server(80);
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial
 
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32); // 128 x 32 Oled
 
 
void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
 
}
 
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}



//// start David Stephenson code

class CardReader {
 private:
  SERIAL_MODE eSerialMode = FREE;
  //DRIVE_COMMAND eDriveMode = NONE;

  File MyFile;
  
  String sFileName;
  String sInString;
  
  unsigned long TimeLastUpdate = 0;
  
  // Class Private Functions
  void LoadBas() {
   
   char cRead, cLast;

   if(eSerialMode == DATA_OUT){
    MyFile = SD.open(sFileName);

    if(MyFile){
     while(MyFile.available()){
      cRead=MyFile.read();
      if (cRead== LF){
       if (cLast != CR){
        Serial.write(CR);
       }
       Serial.write(LF);
      } else {
       Serial.write(cRead);
       cLast = cRead;
      }
     }
    }
    MyFile.close();
    Serial.write(EF);
    eSerialMode = FREE;
   }
  }

  void SaveBas(char cInChar) {
   
   if(eSerialMode == DATA_IN){
       OledShow("Loading ...", String(cInChar));
       delay(5);
    if(cInChar != EF){
     MyFile.print(cInChar);
    } else { 
     MyFile.close();
     eSerialMode = FREE;
    }
   }  
  }
  
  void FilesOut() {
   
  }
  
  
  void commandIn(){
   String KEYWORDS[7] = {"LOAD","SAVE","KILL","FILES","MOVE","CP2SD","CP2R"};
   String sInSubString;

   sInString.trim();

   // Serial.print(sInString);

   if (sInString.length() >= 3){
    for (byte bKeyword = 0 ; bKeyword < 8 ; bKeyword++){
     sInSubString = sInString.substring(0,KEYWORDS[bKeyword].length() );
     sInSubString.toUpperCase();
     if (sInSubString.indexOf(KEYWORDS[bKeyword])!=-1){
      if (KEYWORDS[bKeyword] == "LOAD") {
       digitalWrite(LED_BUILTIN, LOW);
       OledShow("Loading ...", "");
       sFileName = sInString.substring(4);
       sFileName.trim();
       OledShow("Loading ...", sFileName);
       delay(500);
       eSerialMode = DATA_OUT;
       LoadBas();
       delay(500);
       digitalWrite(LED_BUILTIN, HIGH);
       delay(500);
      }

      else if (KEYWORDS[bKeyword] == "SAVE" || KEYWORDS[bKeyword] == "CP2D") {
        digitalWrite(LED_BUILTIN, LOW);
       OledShow("Saving ...", "");
       sFileName = sInString.substring(4);
       sFileName.trim();
       OledShow("Saving ...", sFileName);
       if (SD.exists(sFileName)) {
        SD.remove(sFileName);
       }
       MyFile = SD.open(sFileName, FILE_WRITE);
       eSerialMode = DATA_IN;
       delay(500);
       digitalWrite(LED_BUILTIN, HIGH);
       delay(500);
      }
      
      else if (KEYWORDS[bKeyword] == "FILES") {
       eSerialMode = FILES_OUT;
       FilesOut();
      }
     }
    }
   }

   //sInCommand = false;
   sInString = "";
   
  }
 
 public:
 
 void SerialIn(char cInChar){
  switch(eSerialMode) {
   case FREE:
    if(cInChar == CR){
     eSerialMode = COMMAND;
     commandIn();
    } else {
     sInString += cInChar;
    }
    break;
   case DATA_IN:
     SaveBas(cInChar);
    break;
  }
 }

};

CardReader MyCard;


//// end David Stephenson code




void OledShow(String text, String text2){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 1, text);
  display.drawString(0, 16, text2);
  display.display();
}









 
void setup(void){
  Serial.begin(9600);

 
  delay(50);
  pinMode(LED_BUILTIN, OUTPUT);
  
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  // IpAddr = String(WiFi.localIP())

 
  IpAddr = WiFi.localIP().toString();
  Serial.println(IpAddr);
  // Serial.println(WiFi.localIP());
 
 
  server.on("/", handleRoot);
 
  server.onNotFound(handleNotFound);
 
  server.begin();
  Serial.println("HTTP server started");

  display.init();
  display.flipScreenVertically();
  Serial.println("display init done");

  ////

 
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
  
  //if (SD.begin(4,48000000)) 
  if (SD.begin(SS)) 
  {
      Serial.println("my SD opened!");
      //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
      ftpSrv.begin(ftpuser,ftppswd);    

  }    
  Serial.println("changing to 1200");
  Serial.println();
  delay(500);
   Serial.end();
   delay(500);
   Serial.begin(1200);
   delay(500);
   Serial.println();
   Serial.println("now at 1200");
    //57I1D
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  /////////////// set the data rate for the SoftwareSerial port
  // mySerial.begin(1200);
  
 /* if (SPIFFS.begin()) {
      Serial.println("SPIFFS opened!");
      //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
      ftpSrv.begin("esp8266","esp8266");    
  } */
}
 
void loop(void){

 char cInChar;
 if (Serial.available()) {
  cInChar = (char)Serial.read();
  //sInString += cInChar;
  MyCard.SerialIn(cInChar);
  // Serial.write(cInChar);
 }else{
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
  server.handleClient();
  OledShow("waiting", IpAddr);
 }
  

 
}
