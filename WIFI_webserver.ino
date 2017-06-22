#include <ESP8266WiFi.h>

const char *ssid = "";
const char *password = "";
#define RELAY 12
#define LED 13
#define DELAY 500
unsigned long TIMER[] = {1800000, 3600000, 7200000};
unsigned long currentTime;
int state = 0;
String header, footer = "";
char* current[] = {"Off", "On", "Timer 30", "Timer 60", "Timer 120"};

WiFiServer server(80);
WiFiClient client;

void toggleState(bool state);
void flashLED(int flashDelay);
void connectWifi();

void setup() {
  // Pin Setup
  pinMode(RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  // Initial State set to Off
  digitalWrite(LED, HIGH); // LED Off
  digitalWrite(RELAY, LOW); // RELAY Off
  // Flash LED to show startup complete
  flashLED(DELAY);
  // Connect to Wifi
  connectWifi();
  // Webserver Begin
  server.begin();
  // HTML Header String
  header += "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html>\n";
  header += "<html>\n<head>\n<title>Wifi Fan Switch</title>\n<style>table { border:1; }</style>\n</head>\n";
  header += "<h1>Wifi Switch - Fan</h1>\n<br>\n<p>\n<b>Current State :</b>";
  // HTML Footer String
  footer += "</p>\n<table>\n<tr>\n";
  footer += "<td><a href=\"/off\">* Relay off *</a></td>\n";
  footer += "<td><a href=\"/on\">* Relay on  *</a></td>\n";
  footer += "<td><a href=\"/\">* Refresh   *</a></td>\n</tr>\n<tr>\n";
  footer += "<td><a href=\"/timer1\">* Timer 30  *</a></td>\n";
  footer += "<td><a href=\"/timer2\">* Timer 60  *</a></td>\n";
  footer += "<td><a href=\"/timer3\">* Timer 120 *</a></td>\n</tr>\n</table>\n</body>\n</html>";
  // Three long flashes to show setup complete
  for (int i = 0; i < 3; i++){
    flashLED(DELAY*2);  
  }
}

void toggleState(bool state) {
  // if turning on relay
  if (state){
    analogWrite(RELAY, 1023);
    analogWrite(LED, 820); 
    // reduce relay current usage
    delay(DELAY);
    analogWrite(RELAY, 580);
  }
  // if turning off relay
  if (!state){
    analogWrite(RELAY, 0);
    analogWrite(LED, 1023);
  }
}

void flashLED(int flashDelay){
  digitalWrite(LED, !digitalRead(LED)); // LED toggle
  delay (flashDelay);
  digitalWrite(LED, !digitalRead(LED)); // LED toggle  
  delay(flashDelay);  
}

void connectWifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    flashLED(DELAY/2);
  }
}

void loop() {
  // For Timer finish
  if ( state > 1) {
    if ( millis() > currentTime + TIMER[state - 2]) {
      state = 0;
      toggleState(LOW);
    }
  }
  // Checks that the Wifi is Still Connected
  if ( WiFi.status() != WL_CONNECTED ) {
    connectWifi();
  }
  // checks if a client is available
  client = server.available();
  if (!client) {
    return;
  }
  // Get HMTL Inputs
  String req = client.readStringUntil('\r');
  client.flush();
  // If off requested update status and relay
  if (req.indexOf("/off") != -1) {
    state = 0;
    toggleState(LOW);
    client.print(header + current[state] + footer);
  }
  // If on requested update status and relay
  else if (req.indexOf("/on") != -1) {
    state = 1;
    toggleState(HIGH);
    client.print(header + current[state] + footer);
  }
  // If timer1 requested update status and relay
  else if (req.indexOf("/timer1") != -1) {
    state = 2;
    currentTime = millis();
    toggleState(HIGH);
    client.print(header + current[state] + footer);
  }
  // If timer2 requested update status and relay
  else if (req.indexOf("/timer2") != -1) {
    state = 3;
    currentTime = millis();
    toggleState(HIGH);
    client.print(header + current[state] + footer);
  }
  // If timer3 requested update status and relay
  else if (req.indexOf("/timer3") != -1) {
    state = 4;
    currentTime = millis();
    toggleState(HIGH);
    client.print(header + current[state] + footer);
  }
  // If the webpage is loaded / reloaded
  else if (req.indexOf("/") != -1) {
    client.print(header + current[state] + footer);
  }
  // If error occurs
  else {
    client.stop();
    return;
  }
  // flush and kill connection to client
  client.flush();
}
