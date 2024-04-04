//------------------------------------------------------------HTTP
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h> //https://github.com/jbuszkie/HTTPSRedirect
// Deployment ID from App Script
const char *GScriptId = "AKfycbx3PngS2TesXqjwOAu355e2pJM6rRIFWvXKhFGel48wNJpu7BftprjqVcRcF2WkqqoU1A";

const char* ssid     = "Galaxy S20 FE 5G1B3A";
const char* password = "stmhwfii";

String payload_base =  "{\"command\": \"div_1_tank\",  \"values\": ";
String payload = "";

const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------HTTP

///////////////// Communicate
#include <ArduinoJson.h>

///////////////// Ultrasonic
#define TRIGPIN D6 //12 Arduino 2
#define ECHOPIN D5 //14 Arduino 3
/////////////// Calculation Delay
unsigned long currentTime;        // 
unsigned long flowLoopTime;      //every 1 sec for flow meter
unsigned long tankLoopTime;     //every 3 sec for quality

//SoftwareSerial zigbee(13,12); RX TX
// Rx -> D7 ,, Tx -> D6

/////////////// values
int storage_Tank_Level = 0;
int storage_Tank_Liter = 0;

void setup() {
  //----------------------------------------------------------
  Serial.begin(115200);        
  delay(10);
  Serial.println('\n');
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
  }
  delete client;
  client = nullptr; 
  
  pinMode(ECHOPIN,INPUT_PULLUP); // ultrasonic
  pinMode(TRIGPIN, OUTPUT);     // ultrasonic
  digitalWrite(ECHOPIN, HIGH); // ultrasonic
}

void loop() {
  // currentTime = millis();
  // if(currentTime >= (tankLoopTime + 3000))// Every 3 second, calculate quality
  // {
  //   tankLoopTime = currentTime; // Updates cloopTime
  //   getDistance(); // Get Storage tank data
  // }
  getDistance(); // Get Storage tank data
  String response_data = send_to_GLS();
  Serial.print("Response >>>>>>>");
  Serial.print(response_data);
}

String send_to_GLS(){
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){  if (!client->connected()) { client->connect(host, httpsPort); }  }
  else{Serial.println("Error creating client object!");}

  String val1=String(storage_Tank_Level),val2=String(storage_Tank_Liter);

  String values = "\"" + val1 + "," + val2 + "\"}";

  // Create json object string to send to Google Sheets
  // values = "\"" + value0 + "," + value1 + "," + value2 + "\"}"
  payload = payload_base + values;
  Serial.print("Publishing data... -> ");
  String response_data = "";
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    Serial.println("Succesfully Uploaded");
    response_data = String(client->getResponseBody());
    //Serial.print(">>>>>>>");
    //Serial.print(response_data);
    //Serial.println("-<>-<>-<>-<>-");
  }
  else{
    Serial.println("Error while connecting");
    delay(5000);
  }
  //delay(10000);
  return response_data;
}

void getDistance(){
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(15);
  digitalWrite(TRIGPIN, LOW);
  int dist = pulseIn(ECHOPIN, HIGH, 26000);
  int distanceval=dist/58;
  Serial.print("Diatance : ");
  Serial.println(distanceval);
  if(distanceval<3)  distanceval=3;
  if(distanceval>14)  distanceval=14;
  storage_Tank_Level = (int) map(distanceval, 3, 14, 100, 0);
  storage_Tank_Liter = storage_Tank_Level * 10;
  // int tankLevel= (int) map(distanceval, 0.0, 30.00, 100, 0);
  // storage_Tank_Level = (int) map(distanceval, 0.0, 30.00, 100, 0);
  // storage_Tank_Liter = (int) map(storage_Tank_Level, 0.0, 100.00, 0, 100000);
  // return distanceval;
  // return tankLevel;
}