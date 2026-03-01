#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

// --- Configuration ---
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const int ledPin     = 2; // Change to 12 or 13 if using specific AirM2M GPIOs

WebServer server(80);
bool sosActive = false;

// SOS Timing Constants
const int DOT   = 200;
const int DASH  = 600;
const int GAP   = 200; // Gap between parts of same letter
const int WAIT  = 3000; // 3 second delay between SOS loops

// HTML Interface
void handleRoot() {
  String state = sosActive ? "<span style='color:green'>ACTIVE</span>" : "<span style='color:red'>OFF</span>";
  String page = "<html><head><style>body{text-align:center;font-family:sans-serif;padding-top:50px;} .btn{padding:20px;width:200px;font-size:20px;margin:10px;border-radius:10px;border:none;color:white;cursor:pointer;} .on{background:green;} .off{background:red;}</style></head><body>";
  page += "<h1>AirM2M SOS Beacon</h1><p>Status: <b>" + state + "</b></p>";
  page += "<a href='/on'><button class='btn on'>START</button></a><br>";
  page += "<a href='/off'><button class='btn off'>STOP</button></a></body></html>";
  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  // Server Routes
  server.on("/", handleRoot);
  server.on("/on",  []() { sosActive = true;  handleRoot(); });
  server.on("/off", []() { sosActive = false; digitalWrite(ledPin, LOW); handleRoot(); });
  
  server.begin();
  ArduinoOTA.begin();
  Serial.println("\nReady at: " + WiFi.localIP().toString());
}

// Helper to pulse the LED
void pulse(int duration) {
  if (!sosActive) return;
  digitalWrite(ledPin, HIGH);
  unsigned long start = millis();
  while(millis() - start < duration) { 
    server.handleClient(); 
    ArduinoOTA.handle(); 
  }
  digitalWrite(ledPin, LOW);
  delay(GAP); 
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  if (sosActive) {
    // S (...)
    for(int i=0; i<3; i++) pulse(DOT);
    delay(GAP);
    // O (---)
    for(int i=0; i<3; i++) pulse(DASH);
    delay(GAP);
    // S (...)
    for(int i=0; i<3; i++) pulse(DOT);

    // The 3-second cycle delay (Non-blocking)
    unsigned long startWait = millis();
    while(millis() - startWait < WAIT && sosActive) {
      server.handleClient();
      ArduinoOTA.handle();
      delay(10);
    }
  }
}