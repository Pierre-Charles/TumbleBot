#include <FirebaseESP32.h>
#include <WiFi.h>
#include <U8x8lib.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "" // insert FireBase Host ID
#define FIREBASE_AUTH "" // insert FireBase Auth ID
#define WIFI_SSID "" // insert SSID
#define WIFI_PASSWORD "" // insert password
#define authKey "" // insert IFTTT API key

//Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;
String path = "/object";

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(15,4,16); // Create LCD instance

MFRC522 mfrc522(21, 4); // Create MFRCC522 instance

WiFiClient client;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dateTime;


// For LEDs
const int redLED = 13;
const int greenLED = 12;
const int yellowLED = 14;

// For SW-420
const int sw420 = 35;

// For LDR
const int ldr = 34;
int light_threshold = 3800;

int messageSent = 0;


String user = "";

volatile bool startTimeLogged = false;
volatile bool endTimeLogged = false;
volatile bool flag = false;
volatile bool dryerStat = false; // For web page to show if its running or idle
volatile bool finished = false; // for web page to show if its finished or in still progress
volatile bool power = false; // for web page to show if dryer is on or off
volatile bool cardScanned = false; // for RFID
volatile bool hasStarted = false;
volatile bool hasFinished = false;

void readStatus() {
  if (finished) {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Sleeping");
  } else if (flag && !finished) {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Running");
  } else {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Waiting");
  }
}

void readUser() {
  if (cardScanned) {
    Firebase.setString(firebaseData, path + "/user", user);
  } else {
    Firebase.setString(firebaseData, path + "/user", "N/A");
  }
}

void readIfFinished() {
  if (flag && !finished) {
    Firebase.setString(firebaseData, path + "/cycleStatus", "In Cycle");
  } else if (finished) {
    Firebase.setString(firebaseData, path + "/cycleStatus", "Finished");
  }
}

void readSW420() {
  long measurement = pulseIn(sw420, HIGH);
  String vibration = (measurement > 0) ? "ON" : "OFF";
  Serial.println("Reading from SW-420: " + String(measurement));
  Firebase.setString(firebaseData, path + "/sw420", vibration);
}

void readLDR() {
  long light_value = analogRead(ldr);
  String light = (light_value < light_threshold) ? "ON" : "OFF";
  Serial.println("Reading from LDR: " + String(light_value));
  Firebase.setString(firebaseData, path + "/ldr", light);
}

void ISR() {
  flag = true;
}

void initWiFi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void telegramTrigger() {
  client.print("POST /trigger/");
  client.print("tumbleBot");
  client.print("/with/key/");
  client.print(authKey);
  client.println(" HTTP/1.1");
  client.println("Host: maker.ifttt.com");
  client.println("User-Agent: ESP");
  client.println("Connection: close");
  client.println();
}

void notifyMyEcho() {
  HTTPClient http;
  http.begin("https://api.notifymyecho.com/v1/NotifyMe?notification=The%20tumble%20dryer%20is%20done!%20Please%20collect%20your%20clothes%20and%20remember%20to%20clean%20the%20filter%20after%20emptying%20the%20dryer&accessCode=amzn1.ask.account.AFA4JRT4OJ4DPYPL5ND5537QTFREQKIPDKMVVAHSORMMRHSFCD5HVAYH5M2CNKIJ77QFOIPYWTU23U56572JC5SVTLTF5C4JTCWHL4ZROAPWMMKOG52MHDZDOUIQKCYN7ELCTN7D3URL3I7PAUERSWI5Q7JAGRHEAE4A3UUQ4JSV26HAJ4DRJ73SGPUSNKPUNHK2PUL6AXDHM2Y");
  http.GET();
  http.end();
}

void getStartDateTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  //dateTime = formattedDate.substr(0, formattedDate.length() - 1);
  Serial.println(formattedDate);
  Firebase.setString(firebaseData, path + "/cycleStartTime", formattedDate);
}

void getEndDateTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  //dateTime = formattedDate.substr(0, formattedDate.length() - 1);
  Serial.println(formattedDate);
  Firebase.setString(firebaseData, path + "/cycleEndTime", formattedDate);
}

void readRFID() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Show UID on serial monitor
  Serial.print("UID: ");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Message: ");
  content.toUpperCase();

  if (content.substring(1) == "82 BB B9 67") {
    Serial.println("Hello Pierre!");
    user = "Pierre";
  } else if (content.substring(1) == "49 93 05 4F") {
    Serial.println("Hello, Pierre!");
    user = "Pierre";
  } else if (content.substring(1) == "49 D5 CC A3") {
    Serial.println("Hello, Pierre!");
    user = "Pierre";
  } else {
    user = content;
  }
  lcd.clear();
  lcd.drawString(0, 0, "Card scanned!");
  lcd.drawString(0, 4, "User:");
  lcd.drawString(0, 6, user.c_str());

  cardScanned = true;

}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(0);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(sw420, INPUT);
  pinMode(ldr, INPUT);
  attachInterrupt(sw420, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_8x13B_1x2_f);
  lcd.drawString(0, 0, "TumbleBot is on!");
  Firebase.setString(firebaseData, path + "/dryerStatus", "Waiting");
  Firebase.setString(firebaseData, path + "/cycleStatus", "Idle");
  Firebase.setString(firebaseData, path + "/user", "N/A");
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }

  if ( flag && startTimeLogged == false) {
    getStartDateTime();
    startTimeLogged = true;
  }
  readRFID();
  readSW420();
  readLDR();
  readStatus();
  readUser();
  readIfFinished();

  if (flag && !finished) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    lcd.clear();
    lcd.drawString(0, 0, "Dryer is on!");
    Serial.println("DRYER IS ON");
    flag = false;
  } else {
    digitalWrite(yellowLED, LOW);
  }
  delay(2000);

  if (!flag && (analogRead(ldr) < light_threshold)) {
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, HIGH);
    finished = true;
    lcd.clear();
    lcd.drawString(0, 0, "Dryer is done!");
    Serial.println("Tumble Dryer has finished!");
    if (messageSent == 0 && WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connecting to IFTTT");
      if (client.connect("maker.ifttt.com", 80))
      {
        telegramTrigger();
        Serial.println("Connecting to Amazon Echo Dot");
        notifyMyEcho();
        getEndDateTime();
        messageSent++;
      }
    }
  }
}
