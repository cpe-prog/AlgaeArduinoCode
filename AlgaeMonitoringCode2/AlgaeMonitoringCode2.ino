#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define INNER_FAN_ENA  D1
#define INNER_FAN_IN1  D2
#define INNER_FAN_IN2  D3
#define OUTER_FAN_ENB  D5
#define OUTER_FAN_IN1  D6
#define OUTER_FAN_IN2  D7

#define WIFI_SSID "I'm in!"
#define WIFI_PASSWORD "connected"
#define API_KEY "AIzaSyCrQtoEKFuxzEH9aINfC4QcYn8i1ijTRXk"
#define DATABASE_URL "monitoring-e8ef8-default-rtdb.firebaseio.com/" 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

long outerCo2;
int innerFanValue = 0;
int outerFanValue = 0;


void setup() {
  pinMode(INNER_FAN_ENA, OUTPUT);
  pinMode(INNER_FAN_IN1, OUTPUT);
  pinMode(INNER_FAN_IN2, OUTPUT);
  pinMode(OUTER_FAN_ENB, OUTPUT);
  pinMode(OUTER_FAN_IN1, OUTPUT);
  pinMode(OUTER_FAN_IN2, OUTPUT);
  Serial.begin(9600);
  pinMode(A0, INPUT);
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

void innerSpeedControl()
{
   if (Firebase.RTDB.getInt(&fbdo, "Controls/1/innerFan")){
      if (fbdo.dataType() == "int"){
      innerFanValue = fbdo.intData();
      analogWrite(INNER_FAN_ENA, innerFanValue);
      digitalWrite(INNER_FAN_IN1, HIGH);
      digitalWrite(INNER_FAN_IN2, LOW);
      Serial.print("Inner Fan: ");
      Serial.println(innerFanValue);
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
}

void outerSpeedControl()
{
   if (Firebase.RTDB.getInt(&fbdo, "Controls/1/outerFan")){
      if (fbdo.dataType() == "int"){
      outerFanValue = fbdo.intData();
      analogWrite(OUTER_FAN_ENB, outerFanValue);
      digitalWrite(OUTER_FAN_IN1, HIGH);
      digitalWrite(OUTER_FAN_IN2, LOW);
      Serial.print("Outer Fan: ");
      Serial.println(outerFanValue);
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

}

void loop() {
  if (Firebase.ready() && signupOK && (millis() -  sendDataPrevMillis > 1000 || sendDataPrevMillis == 0 )) {
    sendDataPrevMillis = millis();
    innerSpeedControl();
    outerSpeedControl();

    outerCo2 = analogRead(A0); 

    if (Firebase.RTDB.setFloat(&fbdo, "SENSORS/1/outerCo2", outerCo2)){
       Serial.print("Co2: ");
       Serial.println(outerCo2, DEC);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    Serial.print("");
    Serial.println("_______________________________________");

    Serial.print("");
    Serial.println("_______________________________________");
  }

}
