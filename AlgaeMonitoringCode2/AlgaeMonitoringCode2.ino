#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define INNER_FAN_IN1 1 // IN1 pin connected to IN1 of L298N for inner fan
#define INNER_FAN_IN2 2 // IN2 pin connected to IN2 of L298N for inner fan
#define OUTER_FAN_IN3 3 // IN1 pin connected to IN1 of L298N for outer fan
#define OUTER_FAN_IN4 4 // IN2 pin connected to IN2 of L298N for outer fan

#define WIFI_SSID "I'm in!"
#define WIFI_PASSWORD "connected"
#define API_KEY "AIzaSyCrQtoEKFuxzEH9aINfC4QcYn8i1ijTRXk"
#define DATABASE_URL "monitoring-e8ef8-default-rtdb.firebaseio.com/" 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

int innerFanSpeed = 0;
int outerFanSpeed = 0;
long outerCo2;


void setup() {
  Serial.begin(9600);
  pinMode(INNER_FAN_IN1, OUTPUT);
  pinMode(INNER_FAN_IN2, OUTPUT);
  pinMode(OUTER_FAN_IN1, OUTPUT);
  pinMode(OUTER_FAN_IN2, OUTPUT);
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

void setFanSpeed(int IN1, int IN2, int speed) {
  digitalWrite(IN1, speed > 0 ? HIGH : LOW);
  digitalWrite(IN2, speed < 0 ? HIGH : LOW);
  analogWrite(FAN_ENA, abs(speed));
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

  }

}
