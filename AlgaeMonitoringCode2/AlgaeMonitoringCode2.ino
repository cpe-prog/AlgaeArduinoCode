#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define INNER_FAN_ENA 1
#define INNER_FAN_PIN 2

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


void setup() {
  Serial.begin(9600);
  pinMode(INNER_FAN_ENA, OUTPUT);
  pinMode(INNER_FAN_PIN, OUTPUT);
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


void loop() {
  if (Firebase.ready() && signupOK && (millis() -  sendDataPrevMillis > 1000 || sendDataPrevMillis == 0 )) {
    sendDataPrevMillis = millis();
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


    if (Firebase.RTDB.getInt(&fbdo, "Controls/1/innerFanValue")){
       if (fbdo.dataType() == "int") {
        innerFanValue = fbdo.intData();
        if (innerFanValue == 0) {
          analogWrite(INNER_FAN_ENA, 0); // Stop the fan
          digitalWrite(INNER_FAN_PIN, LOW);
        } else if (innerFanValue >= 1 && innerFanValue <= 125) {
          analogWrite(INNER_FAN_ENA, 128); // Set low speed
          digitalWrite(INNER_FAN_PIN, HIGH);
        } else if (innerFanValue >= 126 && innerFanValue <= 255) {
          analogWrite(INNER_FAN_ENA, 255); // Set high speed
          digitalWrite(INNER_FAN_PIN, HIGH);
        }
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

  }

}
