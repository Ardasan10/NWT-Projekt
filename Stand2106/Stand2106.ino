char blueToothVal;  //Werte sollen per Bluetooth gesendet werden
char lastValue;     //speichert den letzten Status der LED (on/off)
//int strasenlampe = 31;
int relay;                  //dient als variable zum wechseln zwischen relais
int relay1 = 32;            //relais 1 Motor
int relay2 = 33;            //relais 2 Pumpe
int wassersensor_wert = 0;  //Wasserlevel wird gemessen
int licht = 0;              //speichert den Wert des Fotowiderstand
int licht_out = 2;          //Pin an dem der Fotowiderstand angebracht ist
int licht_in = A0;          //Anologer Pin zum messen des Fotowiderstand
int trigger = 12;           //Trigger-Pin des Ultraschallsensors an Pin12 des Arduino-Boards
int ps = 8;                 //relais zum an- und ausschalten des gesamten Projektes für ## ps = power supply
int echo = 13;              // Echo-Pim des Ultraschallsensors an Pin13 des Arduino-Boards
int reset = 7;              // Der Pin ist mit dem Reset des Arduinos verbunden.
long dauer = 0;             // Das Wort dauer ist jetzt eine Variable, unter der die Zeit gespeichert wird, die eine Schallwelle bis zur Reflektion und zurück benötigt. Startwert ist hier 0.
long entfernung = 0;        // Das Wort „entfernung“ ist jetzt die variable, unter der die berechnete Entfernung gespeichert wird. Info: Anstelle von „int“ steht hier vor den beiden Variablen „long“. Das hat den Vorteil, dass eine größere Zahl gespeichert werden kann. Nachteil: Die Variable benötigt mehr Platz im Speicher.

#include <Adafruit_NeoPixel.h>  //NeoPixel-Bibiothek hinzufügen
#include <Boards.h>             //Boards-Bibiothek hinzufügen
#include <LiquidCrystal_I2C.h>  //LiquidCrystal_I2C-Bibiothek hinzufügen
#include <Wire.h>               //Wire-Bibiothek hinzufügen
#include <SPI.h>                // SPI-Bibiothek hinzufügen
#include <MFRC522.h>            // RFID-Bibiothek hinzufügen
#define SS_PIN 22               // SDA an Pin 24
#define RST_PIN 23              // RST an Pin 23
#define LED_COUNT 13            //Neopixel Led anzahl
#define LED_PIN 31              //Signal Pin des Led-Streifen

MFRC522 mfrc522(SS_PIN, RST_PIN);                                   // RFID-Empfänger benennen
LiquidCrystal_I2C lcd(0x27, 16, 2);                                 // set the LCD address to 0x27 for a 16 chars and 2 line displayv
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  //erstellt ein Objekt namens strip von der Klasse Adafruit_NeoPixel


void rfid_read() {
  String content = "";  // Eine leere Zeichenkette zum Speichern der Karten-ID wird erstellt
  byte letter;
  if (mfrc522.PICC_IsNewCardPresent()) {  // Überprüft, ob eine neue NFC-Karte erkannt wurde
    mfrc522.PICC_ReadCardSerial();        // Liest die Karten-ID aus der Karte aus

    // Die Karten-ID wird durchlaufen und der Inhalt zur Zeichenkette 'content' hinzugefügt
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();  // Die Karten-ID wird in Großbuchstaben umgewandelt

    // Überprüft, ob die Karten-ID mit einer autorisierten ID übereinstimmt
    if (content.substring(1) == "B0 50 21 32") {
      Serial.println("Authorized access");

      // Überprüft, ob der Pin eingeschaltet ist und schaltet es bei Bedarf aus
      if (digitalRead(ps) == HIGH)
        digitalWrite(ps, LOW);
    } else {
      Serial.println("Access denied");
    }
  }
}




void setup() {
  Serial.begin(9600);                // Startet den seriellen Monitor und setzt die Baudrate auf 9600
  Serial1.begin(4800);               // Startet die Software-Serielle-Kommunikation mit einer Baudrate von 4800 (wird verwendet, weil andere Pins belegt sind)
  digitalWrite(ps, HIGH);            // Schaltet das Relais, um das Projekt vom Stromnetz zu trennen
  SPI.begin();                       // Initialisiert die SPI-Verbindung
  mfrc522.PCD_Init();                // Initialisiert den RFID-Empfänger
  pinMode(ps, OUTPUT);               // Setzt den Pin "ps" als Ausgang
  while (digitalRead(ps) == HIGH) {  // Überprüft, ob die richtige NFC-Karte erkannt wurde, um das Projekt zu aktivieren
    rfid_read();
  }
  pinMode(licht_out, OUTPUT);     // Setzt den Pin "licht_out" als Ausgang
  digitalWrite(licht_out, HIGH);  // Aktiviert die Pullup-Widerstände für "licht_out"
  pinMode(trigger, OUTPUT);       // Der Trigger-Pin ist ein Ausgang
  pinMode(echo, INPUT);           // Der Echo-Pin ist ein Eingang
  strip.begin();                  // Initialisiert die LED-Strip-Bibliothek
  strip.show();                   // Setzt alle Pixel auf "aus"
  strip.setBrightness(64);        // Setzt die Helligkeit des LED-Streifens auf 64
  lcd.init();                     // Initialisiert das LCD-Display
  lcd.init();                     // Initialisiert das LCD-Display erneut (Doppelter Aufruf?)
  lcd.backlight();                // Aktiviert die Hintergrundbeleuchtung des LCD-Displays
  lcd.setCursor(0, 0);            // Setzt den Cursor des LCD-Displays auf die Position (0, 0)
  pinMode(relay, OUTPUT);         // Setzt den Pin "relay" als Ausgang
  digitalWrite(relay1, HIGH);     // Schaltet das Relais "relay1" ein (HIGH)
  digitalWrite(relay2, HIGH);     // Schaltet das Relais "relay2" ein (HIGH)
  Serial1.print("Läuft2");        // Sendet den Text "Läuft2" über die Software-Serielle-Kommunikation
  strip.clear();                  // Löscht alle LEDs auf dem LED-Streifen
  delay(2500);                    // Wartet 2500 Millisekunden (2,5 Sekunden)
}


void loop() {
  rfid_read_off();      // Funktion zum Lesen des RFID-Empfängers wird aufgerufen
  entfernungsmesser();  // Funktion zum Messen der Entfernung wird aufgerufen
  bluetooth();          // Funktion für die Bluetooth-Kommunikation wird aufgerufen
  lichtsensor();        // Funktion zum Auslesen des Lichtsensors wird aufgerufen
  wassersensor();       // Funktion zum Auslesen des Wassersensors wird aufgerufen

  if (blueToothVal == '1') {          // Wenn das Bluetooth-Modul eine '1' empfängt...
    rainbow(10);                      // Funktion zum Anzeigen eines Regenbogenmusters auf dem LED-Streifen wird aufgerufen
    if (lastValue != '1')             // Wenn der letzte empfangene Wert keine '1' war...
      Serial.println(F("Nummer0"));   // ...wird "Nummer0" auf dem seriellen Monitor ausgegeben
    lastValue = blueToothVal;         // Der letzte empfangene Wert wird aktualisiert
  } else if (blueToothVal == '0') {   // Wenn das Bluetooth-Modul eine '0' empfängt...
    strip.Color(255, 255, 255);       // Die Farbe des LED-Streifens wird auf Weiß gesetzt
    if (lastValue != '0')             // Wenn der letzte empfangene Wert keine '0' war...
      Serial.println(F("Nummero1"));  // ...wird "Nummero1" auf dem seriellen Monitor ausgegeben
    lastValue = blueToothVal;         // Der letzte empfangene Wert wird aktualisiert
  } else if (blueToothVal == '2') {   // Wenn das Bluetooth-Modul eine '2' empfängt...
    strip.clear();                    // Alle LEDs auf dem Streifen werden ausgeschaltet
    if (lastValue != '0')             // Wenn der letzte empfangene Wert keine '0' war...
      Serial.println(F("Nummero2"));  // ...wird "Nummero2" auf dem seriellen Monitor ausgegeben
    lastValue = blueToothVal;         // Der letzte empfangene Wert wird aktualisiert
  }

  if (!licht <= 1000) {
    strip.clear();  // Alle LEDs auf dem Streifen werden ausgeschaltet
  } else {
    strip.Color(255, 255, 255);  // Die Farbe des LED-Streifens wird auf Weiß gesetzt
  }

  if (entfernung <= 10) {  // Wenn der Rohwert der Entfernung kleiner oder gleich 10 ist...
    lcd.setCursor(0, 0);
    lcd.print("ANKUNFT GLEICH.");  // "ANKUNFT GLEICH." wird auf dem LCD-Display angezeigt
    lcd.setCursor(0, 3);
    lcd.print("Abstand halten.");  // "Abstand halten." wird auf dem LCD-Display angezeigt
  } else {
    lcd.clear();  // Andernfalls wird der Inhalt des LCD-Displays gelöscht
  }
}


void bluetooth() {
  if (Serial.available()) {        // Überprüft, ob Daten verfügbar sind
    blueToothVal = Serial.read();  // Liest die verfügbaren Daten aus dem seriellen Eingang
  }
}


void lichtsensor() {
  licht = analogRead(licht_in);  // Liest den Wert des Lichtsensors
}

void entfernungsmesser() {
  digitalWrite(trigger, LOW);          //Hier nimmt man die Spannung für kurze Zeit vom Trigger-Pin, damit man später beim senden des Trigger-Signals ein rauschfreies Signal hat.
  delay(5);                            //Dauer: 5 Millisekunden
  digitalWrite(trigger, HIGH);         //Jetzt sendet man eine Ultraschallwelle los.
  delay(10);                           //Dieser „Ton“ erklingt für 10 Millisekunden.
  digitalWrite(trigger, LOW);          //Dann wird der „Ton“ abgeschaltet.
  dauer = pulseIn(echo, HIGH);         //Mit dem Befehl „pulseIn“ zählt der Mikrokontroller die Zeit in Mikrosekunden, bis der Schall zum Ultraschallsensor zurückkehrt.
  entfernung = (dauer / 2) * 0.03432;  //Nun berechnet man die Entfernung in Zentimetern. Man teilt zunächst die Zeit durch zwei (Weil man ja nur eine Strecke berechnen möchte und nicht die Strecke hin- und zurück). Den Wert multipliziert man mit der Schallgeschwindigkeit in der Einheit Zentimeter/Mikrosekunde und erhält dann den Wert in Zentimetern.
}

void relay_on() {
  digitalWrite(relay, HIGH);  // Schaltet das Relais ein, indem der Pin "relay" auf HIGH gesetzt wird
}

void relay_off() {
  digitalWrite(relay, LOW);  // Schaltet das Relais aus, indem der Pin "relay" auf LOW gesetzt wird
}

void wassersensor() {
  wassersensor_wert = analogRead(A5);  // Liest den Wert des Wassersensors und speichert ihn in der Variablen "wassersensor_wert"
}

void rfid_read_off() {
  String content = "";  // Eine leere Zeichenkette zum Speichern der Karten-ID wird erstellt
  byte letter;
  if (mfrc522.PICC_IsNewCardPresent()) {  // Überprüft, ob eine neue NFC-Karte erkannt wurde
    mfrc522.PICC_ReadCardSerial();        // Liest die Karten-ID aus der Karte aus
    // Die Karten-ID wird durchlaufen und der Inhalt zur Zeichenkette 'content' hinzugefügt
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));  // returns true on success, false on failure (in which case, the string is left unchanged). if the argument is null or invalid, the concatenation is considered unsuccessful.
    }
    content.toUpperCase();                        // Die Karten-ID wird in Großbuchstaben umgewandelt
    if (content.substring(1) == "B0 50 21 32") {  // Überprüft, ob die Karten-ID mit einer autorisierten ID übereinstimmt
      Serial.println("Authorized access");
      delay(1000);
      pinMode(reset, OUTPUT);     //Der Pin der mit dem reset verbunden ist, wird erst hier als Ausgang definiert, da er sonst im Setup das Program zurücksetzt beim starten
      digitalWrite(reset, HIGH);  //Falls die Karten-ID übereinstimmt wird dem Reset Pin am Arduino ein Signal gegeben, dadurch wird das Projekt zurück gesetzt und man gelangt in die ausgeschaltene Situation in der die NFC-Karte abgefragt wird vom Anfang
    }

    else {
      Serial.println("Access denied");
    }
  }
}

void rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    // Schleife, die von 0 bis 5 * 65536 läuft und den Wert von firstPixelHue erhöht

    for (int i = 0; i < strip.numPixels(); i++) {
      // Schleife, die von 0 bis zur Anzahl der LEDs auf dem Streifen läuft

      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // Berechnung des Hue-Werts für jede LED basierend auf firstPixelHue und dem Index i

      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      // Setzt die Farbe der LED i auf den berechneten Hue-Wert mit der gamma32-Funktion
    }

    strip.show();  // Aktualisiert den LED-Streifen mit den neuen Farben
    delay(wait);   // Verzögerung für die angegebene Zeit
  }
}
