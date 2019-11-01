#include <U8x8lib.h> // OLED Display for ESP
#include <WiFi.h>
#include <WiFiClientSecure.h>

WiFiClient client;
WiFiServer server(80);
String header;

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const char* ssid = "VM3156656";
const char* password = "jn3qvGncNbn8";

const char* authKey = "eh7LRYUr0zctT7d7GceXajmodzrcn19H__wbezBKHFs";

// For LEDs
const int blueLED = 18;
const int redLED = 19;
const int greenLED = 23;

// For SW-420
const int sw420 = 35;

// For LDR
const int ldr = 34;
int light_value = 0;
int light_threshold = 1500;

volatile bool flag = false;
volatile bool dryerStat = false;

void ISR() {
  flag = true;
  dryerStat = true;
}

long TP_init() {
  long measurement = pulseIn(sw420, HIGH);
  return measurement;
}

void initWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
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

void setup() {
  Serial.begin(115200);
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(sw420, INPUT);
  pinMode(ldr, INPUT);
  attachInterrupt(sw420, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_chroma48medium8_r);
  initWiFi();
}

void loop() {
  long measurement = TP_init();
  Serial.println("Readings from SW-420");
  Serial.println(measurement);

  light_value = analogRead(ldr);
  Serial.println("Readings from LDR");
  Serial.println(light_value);

  if (flag) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
    lcd.clear();
    lcd.drawString(0, 0, "DRYER IS ON");
    Serial.println("DRYER IS ON");
    flag = false;
  } else {
    lcd.clear();
    digitalWrite(blueLED, LOW);
  }

  delay(2000);

  if (!flag && (analogRead(ldr) < light_threshold)) {
    digitalWrite(greenLED, HIGH);
    dryerStat = false;
    lcd.drawString(0, 0, "DRYER IS DONE!");
    Serial.println("Tumble Dryer has finished!");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connecting to IFTTT");
      if (client.connect("maker.ifttt.com", 80))
      {
        telegramTrigger();
      }
    }
    delay(2000);
  }
}
