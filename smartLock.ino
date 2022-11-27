/**********************************************************************
  Filename    : ButtonAndLed
  Description : Control led by button.
  Auther      : www.freenove.com
  Modification: 2022/09/21 - PATELs@up.edu
**********************************************************************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <Wire.h>
//const char * ssid = "UPIoT";
//const char * password = "";
const char* ssid = "Ayurveda";
const char* password = "Sarinivi1";
const char* FetchConfiguredPasscode = "http://pranavrajan568.pythonanywhere.com/FetchConfiguredPasscode";
const char * FetchUnlockedStatus = "http://pranavrajan568.pythonanywhere.com/FetchUnlockedStatus";
const char * serverName = "http://pranavrajan568.pythonanywhere.com";
String myDeviceId = "94:E6:86:E1:20:DC ";

unsigned long lastTime = 0;
unsigned long timerDelay = 3000;
unsigned long lastMotionTime = 0;
unsigned long motionTimerDelay = 500;

String finalUrl;



#define PIN_BUTTON 4
#define TRIG 5
#define ECHO 18
float distance;
float duration;
float timeUltrasonic =  200 * 60;


#define ledPin 2


String enteredPasscode = "";
String configuredPasscode;
String current_unlocked_status = "Lock";


// the setup function runs once when you press reset or power the board

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {14,27,26, 25}; // GIOP18, GIOP5, GIOP17, GIOP16 connect to the row pins
byte pin_column[COLUMN_NUM] = {13,21,22,23};  // GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );







void setup() {
  
  // initialize dpin as an INPUT
  pinMode(ledPin,OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
   pinMode(TRIG, OUTPUT);
   pinMode(ECHO,INPUT);
  Serial.begin(115200); //Set the baud rate to 115200


  // #start the wifi by starting it by passing the wifi name and the password to connect to your network
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    // #while it waiting to connect it will print a dot every half second
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // # notice how we build the string here in pieces, it's something that will be critical to your learning here
  Serial.println("Timer set to " + String(timerDelay) + " miliseconds (timerDelay variable), it will take that long before publishing the first reading.");

  
}

void myButtonPushed(){
 if (digitalRead(PIN_BUTTON) == HIGH) {
    Serial.println("YOU ARE PUSHING MY BUTTON");
    delay(333);
  }else{
    delay(777);
    Serial.print("+.+.");
  }
 
}


String httpGETRequest(const char* serverName) {
  HTTPClient http;


  finalUrl = serverName; 

//  Serial.println("Before:  " + finalUrl);
//  finalUrl = finalUrl + myDeviceId + "/" +"aSDFasdhfka" + "/";
  finalUrl = finalUrl;
//  Serial.println("After:  " + finalUrl);
  
  http.begin(finalUrl);
  
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 

  
  if (httpResponseCode>0) {
//    Serial.print("This the get request HTTP Response code: ");
//    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
//    Serial.print("Error code: ");
//    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}

void sendPasscode(String entered_passcode,String correct_boolean ,const char * serverLink){
HTTPClient http;
String urlLink = serverLink;

urlLink = urlLink + "/" + String(entered_passcode)+"/"+String(correct_boolean);
//Serial.println("This is the url link sent to " + urlLink);
http.begin(urlLink);
//break;
http.addHeader("Content-Type","text/plain");
String requestData = "Post";
int responseCode = http.POST(requestData);

Serial.println("This is reponse code for sending passcode "+ String(responseCode));

http.end();
  
}

void sendMotionData(String doorbell_distance,const char * serverLink){
HTTPClient http;
String urlLink = serverLink;

urlLink = urlLink + "/" + String(doorbell_distance);
Serial.println("This is the url link sent to " + urlLink);
http.begin(urlLink);
//break;
http.addHeader("Content-Type","text/plain");
String requestData = "Post";
int responseCode = http.POST(requestData);

Serial.println("This is reponse code for sending Motion "+ String(responseCode));

http.end();
  
  
}



float ultrasonicSensor(){
    digitalWrite(TRIG,LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG,HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG,LOW);
    duration = pulseIn(ECHO,HIGH);
    distance = duration * .034 / 2;
    if(distance !=0){
      Serial.println("The distance away  in cm is : " + String(distance));
    }

    return distance;


  
}

void keypadEnterPasscode(){
     char typedChar = keypad.getKey();

     if(typedChar){

         if(typedChar != 'A'){
         enteredPasscode += typedChar;
         Serial.println("what you entered so far = " + enteredPasscode);
        

         }

         else if(typedChar == 'A'){
             if(enteredPasscode.equals(configuredPasscode)){
                Serial.println("Accepted Access, entered Passcode =" + enteredPasscode  + " configured Passcode = " + configuredPasscode  );
                
                sendPasscode(enteredPasscode, "Yes", serverName);
                enteredPasscode = "";
                digitalWrite(ledPin,HIGH);
                delay(1000);
                digitalWrite(ledPin,LOW);

             }
    
             else if(!(enteredPasscode.equals(configuredPasscode))){
              Serial.println("Denied Access , entered Passcode =" + enteredPasscode  + " configured Passcode = " + configuredPasscode);
                
              sendPasscode(enteredPasscode,"No",serverName);
              enteredPasscode = "";
             }
    
         }
         

     }

  
}





// the loop function runs over and ovser again forever
void loop() {
  


      if ((millis() - lastTime) > timerDelay) {
        
          //Check WiFi connection status
          if(WiFi.status()== WL_CONNECTED){
            
            configuredPasscode = httpGETRequest(FetchConfiguredPasscode);
            String temp_unlocked_status = current_unlocked_status;
            current_unlocked_status = httpGETRequest(FetchUnlockedStatus);
            Serial.println("temp_unlocked_status = " + temp_unlocked_status + " current_unlocked_status = " + current_unlocked_status); 
            
            if(temp_unlocked_status.equals("Lock") && current_unlocked_status.equals("Unlock")){
              digitalWrite(ledPin,HIGH);
              delay(1000);
              digitalWrite(ledPin,LOW);
            }

            if(temp_unlocked_status.equals("Unlock") && current_unlocked_status.equals("Lock")){
              digitalWrite(ledPin,HIGH);
              delay(1000);
              digitalWrite(ledPin,LOW);
            }
            
      //      Serial.println("Configured Passcode"+ String(configuredPasscode));
      //      sendPasscode("12543","yes",serverName);      
           
          }
      
          else {
            Serial.println("WiFi Disconnected");
          }
          lastTime = millis();
        }
      
      
      
       if((millis() - lastMotionTime) > motionTimerDelay ){
           float dist = ultrasonicSensor();
             if(dist < 5 && dist !=0){
              sendMotionData(String(dist),serverName);
             }
           
      
        lastMotionTime = millis();
       }

      keypadEnterPasscode();
  
 
}
