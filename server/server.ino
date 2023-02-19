#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h> 
#include <LinkedList.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

const int PIN_LED = D2;
const int PIN_BOUTON_RESET = D5;
const int PIN_BOUTON_ADEFINIR = D6;
const int PIN_BOUTON_BONNEREPONSE = D7;
const int PIN_BOUTON_MAUVAISEREPONSE = D3;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN_LED, NEO_GRB + NEO_KHZ800);

const char *ssid = "buzzers";
const char *password = "12345678";

WiFiClient client;  // or WiFiClientSecure for HTTPS
HTTPClient http;

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,254);
IPAddress subnet(255,255,255,0);

typedef struct { 
  const char *couleurStr; 
  int couleurRGB;
  int score;
} buzzer;

ESP8266WebServer server(80);

const buzzer ROUGE = {"ROUGE", pixels.Color( 255 , 0 , 0 ),0};
const buzzer JAUNE = {"JAUNE", pixels.Color( 255 , 255 , 0 ),0};
const buzzer BLEU = {"BLEU", pixels.Color( 0 , 0 , 255 ),0};
const buzzer VERT = {"VERT", pixels.Color( 0 , 255 , 0 ),0};
const buzzer EN_ATTENTE = {"EN_ATTENTE", pixels.Color( 255 , 255 , 255 ),0};
const buzzer INCONNU = {"INCONNU", pixels.Color( 0 , 0 , 0 ),0};

#define NB_BUZZERS 4
buzzer buzzers[] = {
  ROUGE, JAUNE, BLEU, VERT
};   

buzzer buzzerActif = EN_ATTENTE;

void setBuzzerActif(buzzer buzzer) {
  buzzerActif = buzzer;
  pixels.setPixelColor(0, buzzerActif.couleurRGB );
  pixels.show();
}

bool isEnAttente() {
  return strcmp(buzzerActif.couleurStr,EN_ATTENTE.couleurStr) == 0;
}

void buzz() {
  const char * couleurBuzzerStr = server.header("couleur").c_str();
  Serial.println(couleurBuzzerStr);
  int idBuz = findBuzzerByCouleur(couleurBuzzerStr);
  Serial.println(idBuz);
  if (idBuz == -1) {
    server.send(400, "text/plain", "COULEUR INVALIDE");
    return;
  }
  if (isEnAttente()) {
    setBuzzerActif(buzzers[idBuz]);
    server.send(200, "text/plain", "GG");
  }
  else {
    server.send(404, "text/plain", "PERDU");
  }
}

void scorePlusUn(int idBuz) {
  buzzer leBuzzer = buzzers[idBuz];
  leBuzzer.score = leBuzzer.score + 1;
  buzzers[idBuz] =  leBuzzer;
}

int findBuzzerByCouleur(const char* couleurBuzzer) {
  for (int i = 0; i < NB_BUZZERS; i++) {
      Serial.println(couleurBuzzer);
      Serial.println(buzzers[i].couleurStr);
      if (strcmp(couleurBuzzer,buzzers[i].couleurStr) == 0) {
        return i;
      }
   }
  return -1;
}

void getScores() {
  DynamicJsonDocument doc(1024);
   for (int i = 0; i < NB_BUZZERS; i++) {
     doc["scores"][buzzers[i].couleurStr] = buzzers[i].score;
   }
    String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output.c_str());
}

void setupServer() {
Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  server.on("/buzz", HTTP_GET, buzz); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.on("/scores", HTTP_GET, getScores); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.enableCORS(true);
    const char *headerKeys[] = {"couleur"};
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char *);
    server.collectHeaders(headerKeys, headerKeysSize);
  server.begin();
}

void setup() {
  Serial.begin(115200);
   pinMode(PIN_LED, OUTPUT);
   pinMode(PIN_BOUTON_ADEFINIR, INPUT_PULLUP);
   pinMode(PIN_BOUTON_RESET, INPUT_PULLUP);
   pinMode(PIN_BOUTON_MAUVAISEREPONSE, INPUT_PULLUP);
   pinMode(PIN_BOUTON_BONNEREPONSE, INPUT_PULLUP);
   pixels.begin();
  setBuzzerActif(EN_ATTENTE);
  
  setupServer();
}

void loop() {
  server.handleClient();
  if (digitalRead(PIN_BOUTON_RESET) == LOW && !isEnAttente())
  {
    setBuzzerActif(EN_ATTENTE);
  }
  if (digitalRead(PIN_BOUTON_BONNEREPONSE) == LOW && !isEnAttente())
  {
    scorePlusUn(findBuzzerByCouleur(buzzerActif.couleurStr));
    setBuzzerActif(EN_ATTENTE);
  }
  if (digitalRead(PIN_BOUTON_MAUVAISEREPONSE) == LOW && !isEnAttente())
  {
    setBuzzerActif(EN_ATTENTE);
  }
}