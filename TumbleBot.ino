#include <U8x8lib.h> // OLED Display for ESP
#include <Adafruit_PN532.h> // RFID Scanner


U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const int ledPin = 19;
const int vibrationSensor = 35;
const int buttonPin = 0;
const int ldr = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(vibrationSensor, INPUT);
  pinMode(ldr, INPUT);
  attachInterrupt(vibrationSensor, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_chroma48medium8_r);
}

volatile bool flag = false;

void ISR() {
  flag = true;
}

void loop() {
  Serial.println(analogRead(ldr));
  if (flag) {
    lcd.clear();
    digitalWrite (ledPin, HIGH);
    lcd.drawString(0, 0, "DRYER IS ON");
    Serial.println("DRYER IS ON");
    flag = false;
  } else {
    lcd.drawString(0, 0, "DRYER IS DONE");
    digitalWrite (ledPin, LOW);
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
