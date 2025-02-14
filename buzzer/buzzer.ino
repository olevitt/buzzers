#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <Adafruit_NeoPixel.h>

const int PIN_LED = D2;
const int PIN_BOUTON = D7;

unsigned long stopTime = 0UL;
unsigned long dureeOK = 5000UL;
unsigned long dureeKO = 2000UL;

const char *ssid = "buzzers";
const char *password = "12345678";

const char *ROUGE = "ROUGE";
const char *BLEU = "BLEU";
const char *VERT = "VERT";
const char *JAUNE = "JAUNE";
const char *BLANC = "BLANC";



Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, PIN_LED, NEO_GRB + NEO_KHZ800);

int pixelOff = pixels.Color( 0, 0 , 0 );
int pixelOn = pixels.Color( 255 , 0 , 0 ); // Rouge
//int pixelOn = pixels.Color( 0 , 0 , 255 ); // Bleu
//int pixelOn = pixels.Color( 0 , 255 , 0 ); // Vert
//int pixelOn = pixels.Color( 255 , 255 , 0 ); // Jaune
//int pixelOn = pixels.Color( 255 , 255 , 255 ); // Blanc

const char *couleurBuzzer = ROUGE;

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
          pixels.setPixelColor(0, pixelOn );
          pixels.setPixelColor(1, pixelOn );
          pixels.show();
          delay(250);
          off();
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
  pixels.setPixelColor(0, pixelOn);
  pixels.setPixelColor(1, pixelOn );
  pixels.show();
  stopTime = millis() + duree;
}

void off() {
  pixels.setPixelColor(0, pixelOff);
  pixels.setPixelColor(1, pixelOff);
  pixels.show();
}

void connectToServer() {
    WiFi.begin(ssid, password);             // Connect to the network
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    Serial.println("Still connecting ...");
    pixels.setPixelColor(0, pixelOn);
    pixels.setPixelColor(1, pixelOn);
    pixels.show();
    delay(500);
    off();
    delay(500);
      }
    off();
    Serial.println("Connected !");
    Serial.println(WiFi.localIP()); 

}

void setup() {
    Serial.begin(115200);
   pinMode(PIN_LED, OUTPUT);
   pinMode(PIN_BOUTON, INPUT_PULLUP);
   pixels.begin();

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
    off();
    stopTime = 0UL;
  }
}
