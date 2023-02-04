#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 

unsigned long stopTime = 0UL;
unsigned long dureeOK = 20000UL;
unsigned long dureeKO = 2000UL;

const char *ssid = "buzzers";
const char *password = "12345678";

WiFiClient client;  // or WiFiClientSecure for HTTPS
HTTPClient http;

void get() {
  http.begin(client, "http://192.168.4.1/buzz");
  int code = http.GET();

  // Print the response
  Serial.print(http.getString());
  if (code == 200) {
    // GG !
    on(dureeOK);
  }
  else {
    for (int i = 0; i < 12; i++) {
          digitalWrite(4, HIGH); 
          delay(250);
          digitalWrite(4, LOW); 
          delay(250);
    }
  }

  // Disconnect
  http.end();
}

void hello() {
  Serial.print(http.GET());
}

void on(int duree) {
  digitalWrite(4, HIGH);   // turn the LED on (HIGH is the voltage level)
  stopTime = millis() + duree;
}

void off() {
  digitalWrite(4, LOW);   // turn the LED on (HIGH is the voltage level)
}

void connectToServer() {
    WiFi.begin(ssid, password);             // Connect to the network
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    Serial.println("Still connecting ...");
    digitalWrite(4, HIGH); 
    delay(500);
     digitalWrite(4, LOW); 
      delay(500);
      }
       digitalWrite(4, LOW);  
    Serial.println("Connected !");
    Serial.println(WiFi.localIP()); 

}

void setup() {
   pinMode(4, OUTPUT);
   pinMode(13, INPUT_PULLUP);
  Serial.begin(115200);
    connectToServer();
}

void loop() {
  if (digitalRead(13) == LOW)
  {
    if (stopTime == 0UL) {
      get();
    }
  }
  if ((stopTime != 0UL)  && ((millis() >= stopTime))) {
    digitalWrite(4, LOW); 
    stopTime = 0UL;
  }
}