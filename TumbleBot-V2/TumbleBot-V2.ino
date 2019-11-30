#include <FirebaseESP32.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <U8x8lib.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define FIREBASE_HOST "tumblebot-d43f6.firebaseio.com"
#define FIREBASE_AUTH "wNVmE1zoQB48UmIKiOKooCp9A820v9QQzIGNgBH3"
#define WIFI_SSID "VladimirRoutin"
#define WIFI_PASSWORD "jn3qvGncNbn8"

//Define FirebaseESP32 data object
FirebaseData firebaseData;

FirebaseJson json;

void printResult(FirebaseData &data);

String path = "/object";

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16); // Create LCD instance

MFRC522 mfrc522(21, 4); // Create MFRCC522 instance

WiFiClient client;
WiFiServer servero(80);
String header;

const char* authKey = "eh7LRYUr0zctT7d7GceXajmodzrcn19H__wbezBKHFs";

// For LEDs
const int redLED = 13;
const int greenLED = 12;
const int blueLED = 14;

// For SW-420
const int sw420 = 35;

// For LDR
const int ldr = 34;
int light_value = 0;
//int light_threshold = 3800;
//int light_threshold = 500;
int light_threshold = 1;



int messageSent = 0;

String user = "";

volatile bool flag = false;
volatile bool dryerStat = false; // For web page to show if its running or idle
volatile bool finished = false; // for web page to show if its finished or in still progress
volatile bool power = false; // for web page to show if dryer is on or off
volatile bool cardScanned = false; // for RFID

AsyncWebServer server(80); // AsyncWebServer object on port 80

String readStatus() {
  if (finished) {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Sleeping");
    return "Sleeping";
  } else if (flag) {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Running");
    return "Running";
  } else if (!flag) {
    Firebase.setString(firebaseData, path + "/dryerStatus", "Waiting");
    return "Waiting";
  }
}

String readUser() {
  Firebase.setString(firebaseData, path + "/user", cardScanned ? user : "Nobody");
  return (cardScanned ? String(user) : "Nobody");
}

String readIfFinished() {
  if (flag) {
    Firebase.setString(firebaseData, path + "/cycleStatus", "In Cycle");
    return "In Cycle";
  } else if (finished) {
    Firebase.setString(firebaseData, path + "/cycleStatus", "Finished");
    return "Finished";
  }
  Firebase.setString(firebaseData, path + "/cycleStatus", "Idle");
  return "Idle";
}

String readSW420() {
  long measurement = pulseIn(sw420, HIGH);
  Serial.println("Reading from SW-420: " + String(measurement));
  Firebase.setInt(firebaseData, path + "/sw420", measurement);
  return String(measurement);
}

String readLDR() {
  light_value = analogRead(ldr);
  Serial.println("Reading from LDR: " + String(light_value));
  Firebase.setInt(firebaseData, path + "/ldr", light_value);
  return String(light_value);
}

void ISR() {
  flag = true;
  dryerStat = true;
  messageSent = 0;
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

const char* index_html = R"rawText(
<!DOCTYPE HTML>
<html>

<head>
  <title>TumbleBot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/eea885c940.js" crossorigin="anonymous"></script>
  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css"
    integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
  <style>
    body {
      background: url(https://hdqwalls.com/wallpapers/blur-background-6z.jpg) no-repeat center center fixed;
    }
  </style>
</head>

<body>
  <div class='mt-5 mx-4'>
    <div class='col-12 col-lg-5 col-md-5 container bg-light p-5 shadow'>
      <div class='text-center'>
        <h3>TumbleBot</h3>
        <h6 class='text-muted font-italic font-weight-normal'>Tumble dryer monitoring system</h6>
      </div>
      <br>
      <div class='pt-4 text-center'>
      <div class='py-2'>
        <i class="fas fa-plug" style="color:purple"></i>
        <span> Dryer status: </span>
        <span id="status" style="color:#004e86">%STATUS%</span>
      </div>

      <div class='py-2'>
        <i class="fas fa-power-off" style="color: purple"></i>
        <span> Cycle status: </span>
        <span id="finished" style="color:#004e86">%FINISHED%</span>
      </div>

      <div class='py-2'>
        <i class="fas fa-user" style="color: purple"></i>
        <span> Being used by: </span>
        <span id="user" style="color:#004e86">%USER%</span>
      </div>
    </div>
    <br><br>

      <div class='row pt-5 text-center'>
        <div class='col-md-6 col-6 text-center'>
          <i class="fas fa-chart-area" style="color:purple"></i>
          <p class='m-0'>LDR:</p>
          <p id="ldr" style="color:#004e86">%LDR%</p>
        </div>

        <div class='col-md-6 col-6 text-center'>
          <i class="fas fa-cogs" style="color:purple"></i>
          <p class='m-0'>SW-420:</p>
          <p id="sw420" style="color:#004e86">%SW420%</p>
        </div>
      </div>
    </div>
  </div>
</body>

<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ldr").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ldr", true);
  xhttp.send();
}, 3000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sw420").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/sw420", true);
  xhttp.send();
}, 3000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("status").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}, 3000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("finished").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/finished", true);
  xhttp.send();
}, 3000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("user").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/user", true);
  xhttp.send();
}, 3000);

</script>
</html>)rawText";

// Replaces placeholder with selected values
String processor(const String& var) {
  if (var == "LDR") {
    return readLDR();
  } else if (var == "SW420") {
    return readSW420();
  } else if (var == "FLAG") {
    return readSW420();
  } else if (var == "STATUS") {
    return readSW420();
  } else if (var == "FINISHED") {
    return readIfFinished();
  } else if (var == "USER") {
    return readUser();
  }
  return String();
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
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " "));
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
  } else {
    user = content;
  }
  cardScanned = true;

}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(sw420, INPUT);
  pinMode(ldr, INPUT);
  attachInterrupt(sw420, ISR, FALLING);
  lcd.begin();
  lcd.setFont(u8x8_font_chroma48medium8_r);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/ldr", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readLDR().c_str());
  });
  server.on("/sw420", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readSW420().c_str());
  });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readStatus().c_str());
  });
  server.on("/finished", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readIfFinished().c_str());
  });
  server.on("/user", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readUser().c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  readRFID();
  if (flag) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
    lcd.drawString(0, 0, "Dryer is on!");
    Serial.println("DRYER IS ON");
    flag = false;
  } else {
    digitalWrite(blueLED, LOW);
  }
  delay(2000);

  if (!flag && (analogRead(ldr) < light_threshold)) {
    digitalWrite(blueLED, LOW);
    digitalWrite(greenLED, HIGH);
    dryerStat = false;
    lcd.clear();
    lcd.drawString(0, 0, "Dryer is done!");
    Serial.println("Tumble Dryer has finished!");
    finished = true;
    if (messageSent == 0 && WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connecting to IFTTT");
      if (client.connect("maker.ifttt.com", 80))
      {
        telegramTrigger();
        Serial.println("Connecting to Amazon Echo Dot");
        notifyMyEcho();
        messageSent++;
      }
    }
    delay(2000);
  }
}
