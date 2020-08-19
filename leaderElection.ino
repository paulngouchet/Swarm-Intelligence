
#include <ArduinoSTL.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <EEPROM.h>
#include <string.h>
using namespace std ;

SoftwareSerial XBee(2, 3); // RX, TX
char oneByte;
int receivedWord = 0;
vector<char> test;
int receivedID;
String temp;
int destination;
String message;
int counter = 0;
vector<int> idTable;
vector<int> localTable;
bool isLeader = true;
bool isFollower = false;
int readCount = 0;
int sendCount = 0;
int clearCount = 0;
int led7 = 8;
int led8 = 9;
int ledPin[] = {4, 5, 6, 7};
int id = -1;
int leader = 1;
int i = 0;
String idString;
// The number of LEDS configured.
int numLeds = (sizeof(ledPin) / sizeof(int));
// Define our input button pin.
int buttonPin = 10;
long lastDebounceTime = 0;
long debounceDelay = 500;
// Calculate the maximum value to be displayed.
int maxValue = 32;
String my_status = "0";
String leader_exist = "0" ;
String identical = "0";
String request_leader = "0";
String state = "";
int current_leader = -1 ;

void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
  randomSeed(analogRead(3));
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);
  pinMode(buttonPin, INPUT);

  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPin[i], OUTPUT);
  }
  
  fastBlink();
}

void loop() {
  if (id == -1){
    generateRandom();
    localTable.push_back(id);
  }
  
  if(clearCount == 800){
    localTable.clear();
    localTable.push_back(id);
    clearCount = 0;
  }
  
  clearCount++;
  buttonPress();
  
  if(readCount > 10){
    readCount = 0;
    readMessage();
  }
  
  readCount++;

  if(sendCount > 30){
    if(isLeader){
      my_status = "1";
    }
    else{
      my_status = "0";
    }
    sendMessage(idString, my_status, request_leader);
    sendCount = 0;
  }
  
  sendCount++;
  Serial.print("Node: ");
  Serial.println(receivedID);
  Serial.print("Me: ");
  Serial.println(idString);

  for(int i = 0; i < localTable.size(); i++){
    Serial.print(localTable[i]);
    Serial.print(",");
  }
  
  Serial.println();

  if(state =="000") {
   electionAlgorithm(localTable);
  }
  else if(state == "001"){
    generateRandom();
    request_leader = "0";
  }
  else if( state == "010"){
    localTable.clear();
    generateRandom();
  }
  else if( state == "011"){
    generateRandom();
    request_leader = "0";
  }
  else if( state == "100"){
    electionAlgorithm(localTable);
  }
  else if( state == "101"){
    generateRandom();
    isLeader = false;
    request_leader = "0";
  }
  else if( state == "110"){
    localTable.clear();  
    if(isLeader == true)
    cout << "Leader is alright" << endl;
    else{
      generateRandom();
    }
  }
  else if( state == "111"){
    generateRandom();
    isLeader = false ;
    request_leader = "0" ;
  }
  
  state = "";
  setLED();
}

void sendMessage(String id, String dest, String payload){
  String sendState = id + "|" + dest + "|" + payload + "&";
  char buf_state[sendState.length() + 1];
  sendState.toCharArray(buf_state,sendState.length() + 1);
  XBee.write(buf_state);
  sendState = "";
}

void buttonPress(){
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (digitalRead(buttonPin) == HIGH) {
        if(isLeader){
          id++;
          if (id > maxValue){
            id = 0;
          }
          idString = (String)id;
          displayBinary(id);
          lastDebounceTime = millis(); //set the current time
          localTable.clear();
          idTable.clear();
          delay(500);
          localTable.push_back(id);
        }
        else if(isLeader == false){
          request_leader = "1";
          for(int i = 0; i < 150; i++){
          sendMessage(idString, my_status,request_leader);
          }
          Serial.print("send request leader");
        }
    }
  }
}

void displayBinary(byte numToShow){
  // Check each of the bits in our number.
  for (int i = 0; i < numLeds; i++){ 
    // Is the bit "on"?
    if (bitRead(numToShow, i) == 1){
      // Turn the LED on.
      digitalWrite(ledPin[i], HIGH);
    }
    else{
      // Turn the LED off.
      digitalWrite(ledPin[i], LOW);
    }
  }
}

void fastBlink(){
  for (int i = (numLeds - 1); i >= 0; i--) {
    digitalWrite(ledPin[i], HIGH);
    delay(50);
    digitalWrite(ledPin[i], LOW);
  }
}

void readMessage(){
  int countSlash = 0;
  int countAmp = 0;
  
  while(XBee.available() > 0) {
    oneByte = XBee.read();
    test.push_back(oneByte);
    if(oneByte == '&'){
      for(int i = 0; i < test.size(); i++){
        if(test[i] == '|'){
          countSlash++;
        }
        if(test[i] == '&'){
          countAmp++;
        }
      }
      if((countSlash == 2) && (countAmp== 1) && (test[0] != '|' && test[0] != '&') ){
        receivedWord = 1;
        break;
      }
      else{
        receivedWord = 0;
        test.clear();
        break;
      }
    }
  }
  
  int flag = 1;
  //print packet when it was recieved. then set tag to 0 so don't print it again.
  
  if(receivedWord == 1){
    Serial.println("print--------------");
    for (int i = 0; i < test.size(); i++){
      Serial.print(test[i]);
    }
    
    Serial.println();
    
    for (int i = 0; i < test.size(); i++){
      if(test[i] != '|'){
        temp = temp + test[i];
      }
      if(counter == 0 && test[i] == '|'){
        receivedID = temp.toInt();
        // Determining if the id received is identical to mine
        if(id == receivedID){
          generateRandom();
        }
        else{
          identical = "0";
          counter++;
          temp = "";
        }
      }
      else if(counter == 1 && test[i] == '|' ){
        destination = temp.toInt();
        //Checking if there is a leader
        if(destination == 1 ){
          leader_exist = "1";
          current_leader = receivedID ;
        }
        else if( isLeader == true ){
          leader_exist = "1";
          current_leader = id ;
        }
        else{
          leader_exist = "0";
          current_leader = -1;
        }
        counter++;
        temp = "";
      }
      else if(test[i] == '&'){
        message = String(temp[0]);
        // Check request new leader message      
        if(message == "1"){
          request_leader = "1";
          }
        else if( message == "0"){
          request_leader = "0";
        }
        counter++;
        temp = "";
      }
    }
    
    Serial.println();
    receivedWord = 0;
    test.clear();
    
    if (flag == 1){
      updateTable();
    }
    counter = 0;
  } 
  state = leader_exist + identical + request_leader;
}

void updateTable(){
  String temp2 = "";
  
  for(i = 0; i < message.length(); i++){
    if(message.charAt(i) == ',') {
      idTable.push_back(temp2.toInt());
      temp2 = "";
    }
    else{
      temp2 += String(message.charAt(i));
    } 
    idTable.push_back(temp2.toInt());
    
    if(receivedID == id){
      generateRandom();
      localTable.clear();
      localTable.push_back(idString.toInt());
    }
    else{
      int exist = 0; 
      for(int i = 0; i< localTable.size(); i++){
        if(localTable[i] == receivedID)
          exist = 1;
      }
      if(exist == 0){
        localTable.push_back(receivedID);
      }
    }
  }
  
  void electionAlgorithm(vector<int> listId){
    int currentSmallest = 32 ;
    
    for(int i = 0 ; i < listId.size() ; i++){
      if(listId[i] < currentSmallest){
        currentSmallest = listId[i] ;
      }
      if(currentSmallest == id){
        isLeader = true ;
      }
      else{
        isLeader = false ;
      }
    
    }
    
void generateRandom(){
  id = abs(random(32));
  byte idByte = (byte) id;
  idString = (String) id;
  displayBinary(idByte);
}

void setLED(){
  if(isLeader == true){
    digitalWrite(9, HIGH);
    digitalWrite(8, LOW);
  }
  else if(isLeader == false){
    digitalWrite(8, HIGH);
    digitalWrite(9, LOW);
  }
