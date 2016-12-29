/*
 Web Server
 
 A simple web server visualizing analog input voltages for CONTROLLINO (and other Arduinos)
 Based on script by David A. Mellis and Tom Igoe

 Modifications for CONTROLLINO by Claus Kühnel 2016-12-29 info@ckuehnel.ch
 */
 
#include <Controllino.h>
#include <SPI.h>
#include <Ethernet.h>

const float scale = 5.0/1024; // scale factor for ADC readout
 
// Hier die MAC Adresse des Shields eingeben
// (Aufkleber auf Rückseite)
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x09, 0x70 };
 
// Eine IP im lokalen Netzwerk angeben. Dazu am besten die IP
// des PCs herausfinden (googlen!) und die letzte Zahl abändern 
IPAddress ip(192,168,178,57);
 
// Ethernet Library als Server initialisieren
// Verwendet die obige IP, Port ist per default 80
EthernetServer server(80);
 
void setup() 
{
 // Serielle Kommunikation starten, damit wir auf dem Seriellen Monitor
 // die Debug-Ausgaben mitlesen können.
 Serial.begin(9600);
 while (!Serial) {}; // wait for serial port to connect. Needed for native USB port only
 Serial.println("Initializing...");

 pinMode(CONTROLLINO_A1, INPUT);
 pinMode(CONTROLLINO_A0, INPUT);
 
 // Ethernet Verbindung und Server starten
 Ethernet.begin(mac, ip);
 server.begin();
 Serial.print("Server gestartet. IP: ");
 // IP des Arduino-Servers ausgeben
 Serial.println(Ethernet.localIP());
}
 
void loop() {
 // server.available() schaut, ob ein Client verfügbar ist und Daten
 // an den Server schicken möchte. Gibt dann eine Client-Objekt zurück,
 // sonst false
 EthernetClient client = server.available();
 // Wenn es einen Client gibt, dann...
 if (client) {
 Serial.println("Neuer Client");
 // Jetzt solange Zeichen lesen, bis eine leere Zeile empfangen wurde
 // HTTP Requests enden immer mit einer leeren Zeile 
 boolean currentLineIsBlank = true;
 // Solange Client verbunden 
 while (client.connected()) {
 // client.available() gibt die Anzahl der Zeichen zurück, die zum Lesen
 // verfügbar sind
 if (client.available()) {
 // Ein Zeichen lesen und am seriellen Monitor ausgeben
 char c = client.read();
 Serial.write(c);
 // In currentLineIsBlank merken wir uns, ob diese Zeile bisher leer war.
 // Wenn die Zeile leer ist und ein Zeilenwechsel (das \n) kommt,
 // dann ist die Anfrage zu Ende und wir können antworten
 if (c == '\n' && currentLineIsBlank) {
 // Aanlog input read
 unsigned int adc0 = analogRead(A0);
 unsigned int adc1 = analogRead(A1);
 // HTTP Header 200 an den Browser schicken
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println("Connection: close"); // Verbindung wird nach Antwort beendet
 client.println("Refresh: 2"); // Seite alle 25 Sekunden neu abfragen
 client.println();
 // Ab hier berginnt der HTML-Code, der an den Browser geschickt wird
 client.println("<!DOCTYPE HTML>");
 client.println("<html>");
 client.println("<head><title>CONTROLLINO Analog Input</title></head>");
 client.println("<body><h3>CONTROLLINO</h3>");
 client.println("Connect A0 and/or A1 from pin header X1 to the voltage to be measured.<br>");
 client.println("X1 has GND, 5 V and 3.3 V. You can use them for test.<br><br>");
 client.println("<table style='text-align: left; width: 192px; height: 60px;' border='1' cellpadding='1' cellspacing='1'>");
 client.println("<tbody><tr><td></td><td>A0</td><td>A1</td><td>A2</td><td>A3</td><td>A4</td><td>A5</td><td>A6</td><td>A7</td><td>A8</td><td>A9</td></tr>");
 client.print("<tbody><tr><td>ADC counts</td><td>"); client.print(adc0); client.print("</td><td>"); client.print(adc1); client.println("</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td></tr>");
 client.print("<tbody><tr><td>Volts</td><td>"); client.print(adc0 * scale); client.print("</td><td>"); client.print(adc1 * scale); client.println("</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td></tr>");
 client.println("</tbody></table></body></html>");
 break;
 }
 if (c == '\n') {
 // Zeilenwechsel, also currentLineIsBlack erstmal auf True setzen
 currentLineIsBlank = true;
 } 
 else if (c != '\r') {
 // Zeile enthält Zeichen, also currentLineIsBlack auf False setzen
 currentLineIsBlank = false;
 }
 }
 }
 // Kleine Pause
 delay(1);
 // Verbindung schliessen
 client.stop();
 Serial.println("Verbindung mit Client beendet.");
 Serial.println("");
 }
}
