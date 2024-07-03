#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DHTPIN D3
#define DHTTYPE DHT11
#define Pump D4
#define Light D5
#define echoPin D7
#define trigPin D6

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);

#define WIFI_SSID "WIFI"
#define WIFI_PASSWORD "WIFI PASSWORD"
#define API_KEY "API"
#define DATABASE_URL "URL" 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

DHT dht(DHTPIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool pumpStatus = false;
bool lightStatus = false;
bool fanStatus = false;

long duration;
long distance; 
long innerCo2;
long vocValue;

void ultrasonic()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2)                                                                                                     ;
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    long formula = 0.017 * duration;
    distance = 100 - formula;
    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/distance", distance)){
    Serial.print("Distance: ");
    Serial.println(distance);
    lcd.setCursor(10, 2);
    lcd.print("Dist:");
    lcd.println(distance);
    }
}

byte Robot[8] = {
0b11111,
0b10101,
0b11111,
0b11011,
0b11111,
0b10001,
0b10001,
0b11111
};

void setup() {
  lcd.begin(20,4);
  lcd.init();        
  lcd.backlight(); 
  lcd.createChar(0, Robot);
  pinMode(Pump, OUTPUT);
  pinMode(Light, OUTPUT);
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(D8, INPUT);
  pinMode(A0, INPUT);
  dht.begin();
  digitalWrite(Pump, LOW);
  digitalWrite(Light, LOW);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
   while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
   Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}



void loop() {
  lcd.setCursor(1, 0);          // Set the cursor on the first column and first row.
  lcd.print("'Algae Monitoring'");
  lcd.setCursor(0,0);
  lcd.write(0);
  lcd.setCursor(19,0);
  lcd.write(0);
  
  if (Firebase.ready() && signupOK && (millis() -  sendDataPrevMillis > 1000 || sendDataPrevMillis == 0 )) {
    sendDataPrevMillis = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    innerCo2 = analogRead(A0); 
    vocValue = analogRead(D8);

    ultrasonic();

    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/innerCo2", innerCo2)){
       Serial.print("InnerCo2: ");
       Serial.println(innerCo2, DEC);
       lcd.setCursor(0, 1);          // Set the cursor on the first column and first row.
       lcd.print("ICo2:");
       lcd.println(innerCo2, DEC);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getInt(&fbdo, "SENSORS/1/outerCo2")){
      if (fbdo.dataType() == "int"){
      int outerCo2 = fbdo.intData();
      Serial.print("OuterCo2: ");
      Serial.println(outerCo2 , DEC);
      lcd.setCursor(10, 1);          // Set the cursor on the first column and first row.
      lcd.print("OCo2:");
      lcd.println(outerCo2, DEC);
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/voc", vocValue / 4)){
       Serial.print("Voc: ");
       Serial.println(vocValue / 4, DEC);
       lcd.setCursor(0, 2);        
       lcd.print("Voc:");
       lcd.println(vocValue / 4, DEC);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/humidity", h)){
       Serial.print("Humidity: ");
       Serial.println(h);
       lcd.setCursor(0, 3);        
       lcd.print("Hum:");
       lcd.println(h);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/temperature", t)){
       Serial.print("temperature: ");
       Serial.println(t);
       lcd.setCursor(10, 3);        
       lcd.print("Temp:");
       lcd.println(t);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    Serial.print("");
    Serial.println("_______________________________________________");
    if (Firebase.RTDB.getBool(&fbdo, "Controls/1/pump")){
      if (fbdo.dataType() == "boolean"){
      pumpStatus = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + pumpStatus + "(" + fbdo.dataType() + ")");
      digitalWrite(Pump, pumpStatus);
      }
      
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getBool(&fbdo, "Controls/1/light")){
      if (fbdo.dataType() == "boolean"){
      lightStatus = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + lightStatus + "(" + fbdo.dataType() + ")");
      digitalWrite(Light, lightStatus);
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    Serial.print("");
    Serial.println("_______________________________________");

  }

}
