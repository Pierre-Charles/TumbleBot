/*
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
*/

#include <Arduino.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const int ledPin = 19;
const int vibrationSensor = 35;
const int buttonPin = 0;

void setup(){
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(vibrationSensor, INPUT);
  attachInterrupt(vibrationSensor, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_chroma48medium8_r);
}

volatile bool flag = false;

void ISR(){
  flag = true;
}

void loop(){
  digitalWrite (ledPin, HIGH);
 if (flag) {
  lcd.drawString(0,0,"DRYER = ON");
  Serial.println(" DRYER = ON ");
  flag = false;
 } else {
  lcd.drawString(0,0, "FINISHED!");
 }
}

// store a aggref=gator (process start or process end)

// bigger resistor less power it wastes but less range to measure over

// ldr is 3kohms
