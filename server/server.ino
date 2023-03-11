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
const int PIN_BOUTON_MAUVAISEREPONSE = D1;

unsigned long resetTime = 0UL;
unsigned long tempoReset = 5000UL;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, PIN_LED, NEO_GRB + NEO_KHZ800);

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
  bool peutRepondre;
} buzzer;

ESP8266WebServer server(80);

const buzzer ROUGE = {"ROUGE", pixels.Color( 255 , 0 , 0 ),0, true};
const buzzer JAUNE = {"JAUNE", pixels.Color( 255 , 255 , 0 ),0, true};
const buzzer BLEU = {"BLEU", pixels.Color( 0 , 0 , 255 ),0, true};
const buzzer VERT = {"VERT", pixels.Color( 0 , 255 , 0 ),0, true};
const buzzer EN_ATTENTE = {"EN_ATTENTE", pixels.Color( 255 , 255 , 255 ),0, true};
const buzzer EN_RESET = {"EN_RESET", pixels.Color( 255,0,255 ),0, true};
const buzzer PRET_A_RESET = {"PRET_A_RESET", pixels.Color( 150,0,0 ),0, true};
const buzzer INCONNU = {"INCONNU", pixels.Color( 0 , 0 , 0 ),0, true};

#define NB_BUZZERS 4
buzzer buzzers[] = {
  ROUGE, JAUNE, BLEU, VERT
};   

buzzer buzzerActif = EN_ATTENTE;

void setBuzzerActif(buzzer buzzer) {
  buzzerActif = buzzer;
  pixels.setPixelColor(0, buzzerActif.couleurRGB );
  pixels.setPixelColor(1, buzzerActif.couleurRGB );
  pixels.show();
}

bool isEnAttente() {
  return strcmp(buzzerActif.couleurStr,EN_ATTENTE.couleurStr) == 0;
}

void buzz() {
  const char * couleurBuzzerStr = server.header("couleur").c_str();
  int idBuz = findBuzzerByCouleur(couleurBuzzerStr);
  if (idBuz == -1) {
    server.send(400, "text/plain", "COULEUR INVALIDE");
    return;
  }
  if (isEnAttente()) {
    if (buzzers[idBuz].peutRepondre) {
      buzzer buzTemp = buzzers[idBuz];
      buzTemp.peutRepondre = false;
      buzzers[idBuz] = buzTemp;
      setBuzzerActif(buzzers[idBuz]);
      server.send(200, "text/plain", "GO");
    }
    else {
      server.send(429, "text/plain", "DEJA REPONDU");
    }
  }
  else {
    server.send(404, "text/plain", "TROP TARD");
  }
}

void scorePlusUn(int idBuz) {
  buzzer leBuzzer = buzzers[idBuz];
  leBuzzer.score = leBuzzer.score + 1;
  buzzers[idBuz] =  leBuzzer;
}

int findBuzzerByCouleur(const char* couleurBuzzer) {
  for (int i = 0; i < NB_BUZZERS; i++) {
      if (strcmp(couleurBuzzer,buzzers[i].couleurStr) == 0) {
        return i;
      }
   }
  return -1;
}

void resetScores() {
  for (int i = 0; i < NB_BUZZERS; i++) {
    buzzer buzzerCourant = buzzers[i];
    buzzerCourant.score = 0;
    buzzers[i] = buzzerCourant;
   }
}

void resetQuestion() {
  for (int i = 0; i < NB_BUZZERS; i++) {
    buzzer buzzerCourant = buzzers[i];
    buzzerCourant.peutRepondre = true;
    buzzers[i] = buzzerCourant;
   }
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
  Serial.println(WiFi.softAP(ssid,password,6,false, 8) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  server.on("/buzz", HTTP_GET, buzz); 
  server.on("/scores", HTTP_GET, getScores);
  server.on("/", HTTP_GET, getScores);
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
  if (digitalRead(PIN_BOUTON_RESET) == LOW)
  {
    if (resetTime == 0UL) { 
      resetTime = millis() + tempoReset;
      setBuzzerActif(EN_RESET);
    }
    if (resetTime - millis() > tempoReset) {
      setBuzzerActif(PRET_A_RESET);
    }
  }
  else {
    if (resetTime != 0UL) {
      if (millis() > resetTime) {
          resetScores();
      }
      else {
          resetQuestion();    
      }
      resetTime = 0UL;
      setBuzzerActif(EN_ATTENTE);
    } 
  }
  if (digitalRead(PIN_BOUTON_BONNEREPONSE) == LOW && !isEnAttente())
  {
    scorePlusUn(findBuzzerByCouleur(buzzerActif.couleurStr));
    resetQuestion();
    setBuzzerActif(EN_ATTENTE);
  }
  if (digitalRead(PIN_BOUTON_MAUVAISEREPONSE) == LOW && !isEnAttente())
  {
    setBuzzerActif(EN_ATTENTE);
  }
}