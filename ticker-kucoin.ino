#include <stdio.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
 
/* Set these to your desired credentials. */
const char *ssid = "WIFI NAME";
const char *password = "WIFI PASSWORD";

const char *host = "api.kucoin.com";
const int httpsPort = 443; //HTTPS= 443 and HTTP = 80
const String path = "/api/v1/market/orderbook/level1?symbol=BTC-USDT";
 
//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "E6 C4 91 4F 3A E0 FF 74 38 83 E1 D4 89 C7 EF 03 09 81 A9 EE";

char line1[17] = {};
LiquidCrystal_I2C lcd(0x27,16,2);

DynamicJsonDocument doc(255);
String jsonResponse;

float rawPrice = 0;

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  lcd.init();
  lcd.backlight();
}
 
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient
 
  Serial.println(host);
 
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
 
  Serial.print("requesting URL: ");
  Serial.println(host + path);
  
  httpsClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");
 
  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
 
  Serial.println("reply was:");
  Serial.println("==========");
  while(httpsClient.available()){        
    jsonResponse = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(jsonResponse); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

  DeserializationError error = deserializeJson(doc, jsonResponse.c_str());
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  rawPrice = doc["data"]["price"];
  sprintf(line1, "%s %.1f USD", "BTC:", rawPrice);


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("KuCoin");
  lcd.setCursor(0,1);
  lcd.print(line1);
    
  delay(60000);  //GET Data at every 2 seconds
}
