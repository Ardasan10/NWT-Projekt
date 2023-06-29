char blueToothVal;  //Werte sollen per Bluetooth gesendet werden
char lastValue;     //speichert den letzten Status der LED (on/off)
//int strasenlampe = 31;
int relay;          //dient als variable zum wechseln zwischen relais
int relay1 = 32;    //relais 1 Motor
int relay2 = 33;    //relais 2 Pumpe
int wassersensor_wert = 0;  //Wasserlevel wird gemessen
int licht = 0;      //speichert den Wert des Fotowiderstand
int licht_out = 2;  //Pin an dem der Fotowiderstand angebracht ist
int licht_in = A0;  //Anologer Pin zum messen des Fotowiderstand
int trigger = 12;  //Trigger-Pin des Ultraschallsensors an Pin12 des Arduino-Boards
int ps = 8;       //relais zum an- und ausschalten des gesamten Projektes für ## ps = power supply 
int echo = 13;  // Echo-Pim des Ultraschallsensors an Pin13 des Arduino-Boards
int reset = 7;  // Der Pin ist mit dem Reset des Arduinos verbunden.
long dauer = 0;           // Das Wort dauer ist jetzt eine Variable, unter der die Zeit gespeichert wird, die eine Schallwelle bis zur Reflektion und zurück benötigt. Startwert ist hier 0.
long entfernung_raw = 0;  // Das Wort „entfernung“ ist jetzt die variable, unter der die berechnete Entfernung gespeichert wird. Info: Anstelle von „int“ steht hier vor den beiden Variablen „long“. Das hat den Vorteil, dass eine größere Zahl gespeichert werden kann. Nachteil: Die Variable benötigt mehr Platz im Speicher.

#include <Adafruit_NeoPixel.h>
#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>      // SPI-Bibiothek hinzufügen
#include <MFRC522.h>  // RFID-Bibiothek hinzufügen
#define SS_PIN 22     // SDA an Pin 24
#define RST_PIN 23    // RST an Pin 23
#define LED_COUNT 13    //Neopixel Led anzahl
#define LED_PIN    31     //Signal Pin des Led-Streifen

MFRC522 mfrc522(SS_PIN, RST_PIN);    // RFID-Empfänger benennen
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line displayv
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  //erstellt ein Objekt namens strip von der Klasse Adafruit_NeoPixel

void rfid_read() {
  String content = "";
  byte letter;
  if (mfrc522.PICC_IsNewCardPresent()) {          //Abfrage nach neuer NFC Karte
    mfrc522.PICC_ReadCardSerial();              //Ablesen der Karten-ID
    for (byte i = 0; i < mfrc522.uid.size; i++) {   //
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));  // returns true on success, false on failure (in which case, the string is left unchanged). if the argument is null or invalid, the concatenation is considered unsuccessful.
    }
    content.toUpperCase();
    if (content.substring(1) == "B0 50 21 32") {
      Serial.println("Authorized access");
      if (digitalRead(ps) == HIGH)
        digitalWrite(ps, LOW);
    }

    else {
      Serial.println("Access denied");
    }
  }
}

void rfid_read() {
  String content = "";  // Eine leere Zeichenkette zum Speichern der Karten-ID wird erstellt
  byte letter;
  if (mfrc522.PICC_IsNewCardPresent()) {  // Überprüft, ob eine neue NFC-Karte erkannt wurde
    mfrc522.PICC_ReadCardSerial();  // Liest die Karten-ID aus der Karte aus

    // Die Karten-ID wird durchlaufen und der Inhalt zur Zeichenkette 'content' hinzugefügt
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();  // Die Karten-ID wird in Großbuchstaben umgewandelt

    // Überprüft, ob die Karten-ID mit einer autorisierten ID übereinstimmt
    if (content.substring(1) == "B0 50 21 32") {
      Serial.println("Authorized access");
      
      // Überprüft, ob das Schloss eingeschaltet ist und schaltet es bei Bedarf aus
      if (digitalRead(ps) == HIGH)
        digitalWrite(ps, LOW);
    }
    else {
      Serial.println("Access denied");
    }
  }
}




void setup() {
  Serial.begin(9600);   //serieller Monitor wird gestartet, Baudrate auf 9600 festgelegt
  Serial1.begin(4800);  // ICH BRACH SOFTWARE SERIELLS WEIL NICHT GEHEN MIT NORMALE SERIAL WEIL ANDERE PINS DRAUF
  digitalWrite(ps, HIGH); //Relais wird geschalten damit das projekt keinen Strom hat
  SPI.begin();         // SPI-Verbindung aufbauen
  mfrc522.PCD_Init();  // Initialisierung des RFID-Empfängers
  pinMode(ps, OUTPUT);  //Pin ps wird als Ausgang gesetzt
  while (digitalRead(ps) == HIGH) {       //Abfrage nach richtigen NFC-Karte zum Anschalten des Projektes
    rfid_read();
  }
  pinMode(licht_out, OUTPUT);     // set pin to input
  digitalWrite(licht_out, HIGH);  // turn on pullup resistors-
  pinMode(trigger, OUTPUT);       // Trigger-Pin ist ein Ausgang
  pinMode(echo, INPUT);           // Echo-Pin ist ein Eingang
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64);
  lcd.init();                     // initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  pinMode(relay, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  Serial1.print("Läuft2");
  strip.clear();
  delay(2500);
}

void loop() {
  rfid_read_off();
  entfernungsmesser();
  bluetooth();
  lichtsensor();
  wassersensor();
  if (blueToothVal == '1')  //wenn das Bluetooth Modul eine „1“ empfängt..  ##
  {
    rainbow(10);
    digitalWrite(13, HIGH);          //...soll die LED leuchten
    if (lastValue != '1')            //wenn der letzte empfangene Wert keine „1“ war...
      Serial.println(F("Nummer0"));  //..soll auf dem Seriellen Monitor „LED is on“ angezeigt werden
    lastValue = blueToothVal;
  } else if (blueToothVal == '0')  //wenn das Bluetooth Modul „0“ empfängt...
  {
    strip.Color(255, 255, 255);
    strip.clear();
    digitalWrite(13, LOW);            //..soll die LED nicht leuchten
    if (lastValue != '0')             //wenn der letzte empfangene Wert keine „0“ war...
      Serial.println(F("Nummero1"));  //..soll auf dem seriellen Monitor „LED is off“ angezeigt werden
    lastValue = blueToothVal;
  } else if (blueToothVal == '2')  //wenn das Bluetooth Modul „0“ empfängt...
  {
    digitalWrite(13, LOW);            //..soll die LED nicht leuchten
    if (lastValue != '0')             //wenn der letzte empfangene Wert keine „0“ war...
      Serial.println(F("Nummero2"));  //..soll auf dem seriellen Monitor „LED is off“ angezeigt werden
    lastValue = blueToothVal;
  }

  if (licht <= 1000) {
    //digitalWrite(strasenlampe, HIGH);
  } else {
    //digitalWrite(strasenlampe, LOW);
  }

  if (entfernung_raw <= 10) {  //## ZU PROZENTANGABE ÄNDERN
    lcd.setCursor(0, 0);
    lcd.print("ANKUNFT GLEICH.");  //## CHANGE AFTER MEETUP
    lcd.setCursor(0, 3);
    lcd.print("Abstand halten.");
  } else {
    lcd.clear();
  }
  if (wassersensor_wert <= 570) {
    relay = relay2;
    relay_on();
  } else {
    relay = relay2;
    relay_off();
  }
}

void bluetooth() {
  if (Serial.available())  //wenn Daten empfangen werden...
  {
    blueToothVal = Serial.read();  //..sollen diese ausgelesen werden
  }
  if (blueToothVal == '1')  //wenn das Bluetooth Modul eine „1“ empfängt..
  {
    digitalWrite(13, HIGH);            //...soll die LED leuchten
    if (lastValue != '1')              //wenn der letzte empfangene Wert keine „1“ war...
      Serial.println(F("LED is on"));  //..soll auf dem Seriellen Monitor „LED is on“ angezeigt werden
    lastValue = blueToothVal;
  } else if (blueToothVal == '0')  //wenn das Bluetooth Modul „0“ empfängt...
  {
    digitalWrite(13, LOW);              //..soll die LED nicht leuchten
    if (lastValue != '0')               //wenn der letzte empfangene Wert keine „0“ war...
      Serial.println(F("LED is off"));  //..soll auf dem seriellen Monitor „LED is off“ angezeigt werden
    lastValue = blueToothVal;
  }
  //else if (blueToothVal == '')  //wenn das Bluetooth Modul „“ empfängt...
  //{

  //}
}

void lichtsensor() {
  licht = analogRead(licht_in);
}

void entfernungsmesser() {
  digitalWrite(trigger, LOW);              //Hier nimmt man die Spannung für kurze Zeit vom Trigger-Pin, damit man später beim senden des Trigger-Signals ein rauschfreies Signal hat.
  delay(5);                                //Dauer: 5 Millisekunden
  digitalWrite(trigger, HIGH);             //Jetzt sendet man eine Ultraschallwelle los.
  delay(10);                               //Dieser „Ton“ erklingt für 10 Millisekunden.
  digitalWrite(trigger, LOW);              //Dann wird der „Ton“ abgeschaltet.
  dauer = pulseIn(echo, HIGH);             //Mit dem Befehl „pulseIn“ zählt der Mikrokontroller die Zeit in Mikrosekunden, bis der Schall zum Ultraschallsensor zurückkehrt.



  //if (entfernung >= 500 || entfernung <= 0) //Wenn die gemessene Entfernung über 500cm oder unter 0cm liegt,…
  //{
  //  Serial.println("Kein Messwert"); //dann soll der serial monitor ausgeben „Kein Messwert“, weil Messwerte in diesen Bereichen falsch oder ungenau sind.
  //}
  //else //  Ansonsten…
  //{
  //  Serial.print(entfernung); //…soll der Wert der Entfernung an den serial monitor hier ausgegeben werden.
  //  Serial.println(" cm"); // Hinter dem Wert der Entfernung soll auch am Serial Monitor die Einheit "cm" angegeben werden.
  //}
  //delay(1000); //Das delay von einer Sekunde sorgt in ca. jeder neuen Sekunde für einen neuen Messwert.
}

void Lcd_print() {
  lcd.setCursor(0, 0);
  lcd.print("text1");
  lcd.setCursor(0, 3);
  lcd.print("text2");
}

void relay_on() {
  digitalWrite(relay, HIGH);
}

void relay_off() {
  digitalWrite(relay, LOW);
}

void wassersensor() {
  wassersensor_wert = analogRead(A5);  //Die Spannung an dem Sensor wird ausgelesen und unter der Variable „messwert“ gespeichert.
}

void rfid_read_off() {
  String content = "";
  byte letter;
  if (mfrc522.PICC_IsNewCardPresent()) {
    mfrc522.PICC_ReadCardSerial();
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));  // returns true on success, false on failure (in which case, the string is left unchanged). if the argument is null or invalid, the concatenation is considered unsuccessful.
    }
    content.toUpperCase();
    if (content.substring(1) == "B0 50 21 32") {
      Serial.println("Authorized access");
      delay(1000);
      pinMode(reset, OUTPUT);
      digitalWrite(reset, HIGH);
    }

    else {
      Serial.println("Access denied");
    }
  }
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

