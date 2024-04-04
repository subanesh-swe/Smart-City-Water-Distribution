//------------------------------------------------------------HTTP
#include <Arduino.h>
//#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h> //https://github.com/jbuszkie/HTTPSRedirect
// Deployment ID from App Script
const char *GScriptId = "AKfycbx3PngS2TesXqjwOAu355e2pJM6rRIFWvXKhFGel48wNJpu7BftprjqVcRcF2WkqqoU1A";

const char* ssid     = "Galaxy S20 FE 5G1B3A";
const char* password = "stmhwfii";

String payload_base =  "{\"command\": \"main_tank\", \"values\": ";
String payload = "";

const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------HTTP

///////////////// Communicate
#include <ArduinoJson.h>

// Declare variables that will be published to Google Sheets
#define BLYNK_TEMPLATE_ID "TMPLlPlZ6osq"
#define BLYNK_TEMPLATE_NAME "H20 USAGE Template"
#define BLYNK_AUTH_TOKEN "jI6gf_svrH39CwQUNUpWHD50BvWk3VPy"
//#define BLYNK_DEVICE_NAME "H20 USAGE Template" //OLD data type
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

///////////////// Wifi 
char swekn[] = BLYNK_AUTH_TOKEN;
char sweid[] = "Galaxy S20 FE 5G1B3A";  // wifi name
char swerd[] =  "stmhwfii";  // wifi password

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
int usage_Tank_Level = 0;
int storage_Tank_Level = 0;
int storage_Tank_Liter = 0;
int avg_con_Level = 0;
int tot_con_Level = 0;
int gar_con_Level = 0;
int inc_con_Level = 0;
char valve_status[5] = "OFF";
char gar_valve_status[5] = "OFF";
////////////// Time From Blynk
int v_hh = 0;
int v_mm = 0;
int g_hh = 0;
int g_mm = 0;
int g_dur = 0;
////////////////////// VALVE GL Sheets
int gls_vlv_mode = 0;
int gls_gar_vlv_mode = 0;
////////////////////// VALVE BLYNK
int blk_vlv_mode = 0;
int blk_gar_vlv_mode = 0;
//////////
#define VALVE_PIN D1
#define GARDEN_VAL_PIN D2

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
  
  pinMode(VALVE_PIN, OUTPUT);           // MOTOR valve
  pinMode(GARDEN_VAL_PIN, OUTPUT);     // Garden Valve
  digitalWrite(VALVE_PIN, LOW);       // MOTOR valve
  digitalWrite(GARDEN_VAL_PIN, LOW); // Garden Valve
  Serial.println("Setup Completed..............>>>>");
  //////////// BLYNk
  Serial.println("Completing to BLYNK...");
  Blynk.begin(swekn, sweid, swerd); // Blynk
  Serial.println("\nConnected to BLYNK!!!");
}

void loop() {
  Blynk.run();
  /*currentTime = millis();
  if(currentTime >= (tankLoopTime + 3000))// Every 3 second, calculate quality
  {
    tankLoopTime = currentTime; // Updates cloopTime
    getDistance(); // Get Storage tank data
  }*/
  getDistance(); // Get Storage tank data
  Serial.print("STORAGE Tank Liter : ");
  Serial.print(storage_Tank_Liter);
  Serial.print("STORAGE Tank Level : ");
  Serial.print(storage_Tank_Level);
  Serial.println(" %");
    
  Serial.print("Response >>>>>>>");
  String response_data = send_to_GLS();
  //Serial.print(response_data);
  Serial.println("\n-----------------------------");
  StaticJsonDocument<300> doc;
  //DynamicJsonDocument doc(200);
  DeserializationError err = deserializeJson(doc, response_data);
  if (err == DeserializationError::Ok)
  {
    gls_vlv_mode = doc["auto"].as<int>();
    gls_gar_vlv_mode = doc["auto"].as<int>();

    usage_Tank_Level = doc["d1_lev"].as<int>();
    Serial.print("USAGE Tank Level : ");
    Serial.print(usage_Tank_Level);
    Serial.println(" %");
    avg_con_Level = doc["m_avg_con"].as<float>();
    Serial.print("AVERAGE Consuption Level : ");
    Serial.print(avg_con_Level);
    Serial.println(" %");
    tot_con_Level = doc["m_tot_con"].as<int>();
    Serial.print("TOTAL Consuption Level : ");
    Serial.print(tot_con_Level);
    Serial.println(" %");
    gar_con_Level = doc["m_d1_con"].as<int>();
    Serial.print("Div 1 Consuption Level : ");
    Serial.print(gar_con_Level);
    Serial.println(" %");
    inc_con_Level = doc["d1_rec"].as<int>();
    Serial.print("INCOMMING WATER Level : ");
    Serial.print(inc_con_Level);
    Serial.println(" %");
  } else {
    // Print error to the "debug" serial port
    Serial.print("deserializeJson() returned ");
    Serial.println(err.c_str());
  }

  // if( blk_vlv_mode == 1 || gls_vlv_mode == 1){    // MOTOR valve
  //   digitalWrite(VALVE_PIN, HIGH);
  //   strcpy(valve_status ,"ON");
  // } else {
  //   digitalWrite(VALVE_PIN, LOW);
  //   strcpy(valve_status ,"OFF");
  // }

  // if( blk_gar_vlv_mode == 1 || gls_gar_vlv_mode == 1){    // GARDEN valve
  //   digitalWrite(GARDEN_VAL_PIN, HIGH);
  //   strcpy(gar_valve_status ,"ON");
  // } else {
  //   digitalWrite(GARDEN_VAL_PIN, LOW);
  //   strcpy(gar_valve_status ,"OFF");
  // }

  if (blk_gar_vlv_mode == 1) // automatic
  {
    if(gls_vlv_mode == 1){
      digitalWrite(VALVE_PIN, HIGH);
      strcpy(valve_status ,"ON");
      strcpy(gar_valve_status ,"ON");
    } else {
      digitalWrite(VALVE_PIN, LOW);
      digitalWrite(GARDEN_VAL_PIN, LOW);
      strcpy(valve_status ,"OFF");
      strcpy(gar_valve_status ,"OFF");
    }
  }
  else {
    if(blk_vlv_mode == 1){
      digitalWrite(VALVE_PIN, HIGH);
      strcpy(valve_status ,"ON");
      strcpy(gar_valve_status ,"ON");
    } else {
      digitalWrite(VALVE_PIN, LOW);
      digitalWrite(GARDEN_VAL_PIN, LOW);
      strcpy(valve_status ,"OFF");
      strcpy(gar_valve_status ,"OFF");
    }
  }

  ///////// SEND to BLYNK
  Blynk.virtualWrite(V14, storage_Tank_Liter );
  Blynk.virtualWrite(V0, storage_Tank_Level );
  Blynk.virtualWrite(V1, usage_Tank_Level );
  Blynk.virtualWrite(V2, avg_con_Level );
  Blynk.virtualWrite(V3, tot_con_Level );
  Blynk.virtualWrite(V4, gar_con_Level );
  Blynk.virtualWrite(V5, inc_con_Level );
  Blynk.virtualWrite(V8, valve_status );
  Blynk.virtualWrite(V15, gar_valve_status );
  Serial.println("-----------------------------\n");
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
  String val3=String(v_hh)+":"+String(v_mm);
  String val4=String(g_hh)+":"+String(g_mm);
  // String val5=String(g_dur);
  // String val6=String();
  String values = "\"" + val1 + "," + val2 + "," + val3 + ","+ val4 + "\"}";

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
  Serial.print("Distance : ");
  Serial.println(distanceval);
  if(distanceval>14)  distanceval=14;
  //int tankLevel= (int) map(distanceval, 0.0, 30.00, 100, 0);
  //return distanceval;
  //return tankLevel;
  storage_Tank_Level = (int) map(distanceval, 3, 14, 100, 0);
  storage_Tank_Liter = storage_Tank_Level * 10;
  // storage_Tank_Liter = (int) map(storage_Tank_Level, 0.0, 100.00, 0, 100000);
}

BLYNK_WRITE(V9)
{
  v_hh = param.asInt(); // Get Valve Hour
  //Serial.print("Valve hh -> ");
  //Serial.println(v_hh);
}
BLYNK_WRITE(V10)
{
  v_mm = param.asInt(); // Get Valve Minutes
  //Serial.print("Valve mm -> ");
  //Serial.println(v_mm);
}
BLYNK_WRITE(V11)
{
  g_hh = param.asInt(); // Get Garden Valve Hour
  //Serial.print("Garden hh -> ");
  //Serial.println(g_hh);
}
BLYNK_WRITE(V12)
{
  g_mm = param.asInt(); // Get Valve Minutes
  //Serial.print("Garden hh -> ");
  //Serial.println(g_mm);
}
BLYNK_WRITE(V13)
{
  g_dur = param.asInt(); // Get Valve Minutes
  //Serial.print("Garden dur -> ");
  //Serial.println(g_dur);
}


BLYNK_WRITE(V6)
{
  int value = param.asInt();
  //Serial.println(value);
  if(value == 1)
  {
    blk_vlv_mode = 1;
    //Serial.println("Valve ON");
  }
  if(value == 0)
  {
    blk_vlv_mode = 0;
    //Serial.println("Valve OFF");
  }
}

BLYNK_WRITE(V7)
{
  int value = param.asInt();
  //Serial.println(value);
  if(value == 1)
  {
    blk_gar_vlv_mode = 1;
    //Serial.println("Valve ON");
  }
  if(value == 0)
  {
    blk_gar_vlv_mode = 0;
    //Serial.println("Valve OFF");
  }
}