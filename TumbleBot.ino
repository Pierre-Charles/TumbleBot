#include <U8x8lib.h> // OLED Display for ESP
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Vladimir_Routin";
const char* passwd = "Ad%eliePen%guin5";

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

AsyncWebServer server(80);

// For LEDs
const int blueLED = 18;
const int redLED = 19;
const int greenLED = 23;

// For SW-420
const int vibrationSensor = 35;

// For LDR
const int ldr = 2;
int light_value = 0;
int light_threshold = 900;

volatile bool flag = false;
volatile bool dryerStat = false;

void setup() {
  Serial.begin(115200);
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(vibrationSensor, INPUT);
  pinMode(ldr, INPUT);
  attachInterrupt(vibrationSensor, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_chroma48medium8_r);
}

void ISR() {
  flag = true;
  dryerStat = true;
}

void loop() {

  light_value = analogRead(ldr);
  Serial.println("Readings from LDR");
  Serial.println(light_value);

  if (flag) {
    lcd.clear();
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
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
  } else {
    //    lcd.clear();
    //    digitalWrite(greenLED, LOW);
  }
  delay(2000);
}

// Store a aggregator (process start or process end)

// Breadborad pin 13 for input
// Purple wire goes to pin 1
// Grey wire goes to pin 2

// Bigger resistor less power it wastes but less range to measure over

// LDR is 3kohms

// LED off
// LED on
// Fallen off the tumble dryer

// Minify html file and progmem it to program memory. Puts it iiin program memory. Then do client.write(variable name)

// Check when the tumble dryer was last on, then if it has been more than 10 seconds, then tell me that the dryer was off
