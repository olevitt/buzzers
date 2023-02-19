#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 

const int PIN_LED = PIN_LED;
const int PIN_BOUTON = PIN_BOUTON;

unsigned long stopTime = 0UL;
unsigned long dureeOK = 20000UL;
unsigned long dureeKO = 2000UL;

const char *ssid = "buzzers";
const char *password = "12345678";

const char *ROUGE = "ROUGE";
const char *BLEU = "BLEU";
const char *VERT = "VERT";
const char *JAUNE = "JAUNE";

const char *couleurBuzzer = VERT;

WiFiClient client;  // or WiFiClientSecure for HTTPS
HTTPClient http;

void get() {
  http.begin(client, "http://192.168.4.1/buzz");
  http.addHeader("couleur", couleurBuzzer);
  int code = http.GET();

  // Print the response
  Serial.print(http.getString());
  if (code == 200) {
    // GG !
    on(dureeOK);
  }
  else {
    for (int i = 0; i < 12; i++) {
          digitalWrite(PIN_LED, HIGH); 
          delay(250);
          digitalWrite(PIN_LED, LOW); 
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
  digitalWrite(PIN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  stopTime = millis() + duree;
}

void off() {
  digitalWrite(PIN_LED, LOW);   // turn the LED on (HIGH is the voltage level)
}

void connectToServer() {
    WiFi.begin(ssid, password);             // Connect to the network
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    Serial.println("Still connecting ...");
    digitalWrite(PIN_LED, HIGH); 
    delay(500);
     digitalWrite(PIN_LED, LOW); 
      delay(500);
      }
       digitalWrite(PIN_LED, LOW);  
    Serial.println("Connected !");
    Serial.println(WiFi.localIP()); 

}

void setup() {
   pinMode(PIN_LED, OUTPUT);
   pinMode(PIN_BOUTON, INPUT_PULLUP);
  Serial.begin(115200);
    connectToServer();
}

void loop() {
  if (digitalRead(PIN_BOUTON) == LOW)
  {
    if (stopTime == 0UL) {
      get();
    }
  }
  if ((stopTime != 0UL)  && ((millis() >= stopTime))) {
    digitalWrite(PIN_LED, LOW); 
    stopTime = 0UL;
  }
}