#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h> 
#include <LinkedList.h>

const char *ssid = "buzzers";
const char *password = "12345678";

const char MAIN_page[] PROGMEM = R"=====(
<HTML>
	<HEAD>
			<TITLE>My first web page</TITLE>
	</HEAD>
<BODY>
	<CENTER>
			<B>Hello World.... </B>
	</CENTER>	
</BODY>
</HTML>
)=====";

WiFiClient client;  // or WiFiClientSecure for HTTPS
HTTPClient http;

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,254);
IPAddress subnet(255,255,255,0);

typedef struct { 
  uint32_t ip;
  int score;
} buzzer;

LinkedList<buzzer> buzzers = LinkedList<buzzer>();
buzzer NO_BUZZER = {0,0};

ESP8266WebServer server(80);

bool questionActive = true;

void buzz() {
  int idBuz = findBuzzerByIP(server.client().remoteIP());
  if (idBuz == -1) {
    buzzer newBuz = {server.client().remoteIP(),0};
    buzzers.add(newBuz);
    idBuz = findBuzzerByIP(server.client().remoteIP());
  }
  buzzer leBuzzer = buzzers.get(idBuz);
  leBuzzer.score = leBuzzer.score + 1;
  buzzers.set(idBuz, leBuzzer);
  scores();
  if (questionActive) {
    questionActive = false;
    server.send(200, "text/plain", "GG");
  }
  else {
    server.send(404, "text/plain", "PERDU");
  }
}

int findBuzzerByIP(uint32_t ip) {
  for (int i = 0; i < buzzers.size(); i++) {
      if (ip == buzzers[i].ip) {
        return i;
      }
   }
  return -1;
}

void getScores() {
  server.send(200, "text/html", MAIN_page);
}

void scores() {
   for (int i = 0; i < buzzers.size(); i++) {
      Serial.print(buzzers.get(i).ip);
      Serial.print(" ");
      Serial.println(buzzers.get(i).score);
   }
}



void on() {
  digitalWrite(4, HIGH);   // turn the LED on (HIGH is the voltage level)
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
  server.begin();
}

void setup() {
   pinMode(4, OUTPUT);
   pinMode(13, INPUT_PULLUP);
  Serial.begin(115200);
  setupServer();
}

void loop() {
  server.handleClient();
  if (digitalRead(13) == LOW)
  {
    questionActive = true;
  }
}