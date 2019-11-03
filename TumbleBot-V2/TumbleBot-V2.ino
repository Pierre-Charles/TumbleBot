// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "U8x8lib.h"

U8X8_SSD1306_128X64_NONAME_SW_I2C lcd(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

WiFiClient client;
WiFiServer servero(80);
String header;

const char* ssid = "VladimirRoutin";
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
int light_threshold = 3700;

int messageSent = 0;

volatile bool flag = false;
volatile bool dryerStat = false; // For web page to show if its running or idle
volatile bool finished = false; // for web page to show if its finished or in still progress
volatile bool power = false; // for web page to show if dryer is on or off

AsyncWebServer server(80); // AsyncWebServer object on port 80

String readStatus() {
  return (flag ? "Running" : "Idle");

String readIfFinished() {
  return (finished ? "Finished" : "In cycle");

String readSW420() {
  long measurement = pulseIn(sw420, HIGH);
  Serial.println("Reading from SW-420: " + String(measurement));
  return String(measurement);
}

String readLDR() {
  light_value = analogRead(ldr);
  Serial.println("Reading from LDR: " + String(light_value));
  return String(light_value);
}

void ISR() {
  flag = true;
  dryerStat = true;
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

const char* index_html = R"rawText(
<!DOCTYPE HTML>
<html>

<head>
  <title>TumbleBot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
    integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
    @import url('https://fonts.googleapis.com/css?family=Fira+Mono|Lato|Roboto|Montserrat|Open+Sans|Ubuntu|Ubuntu+Mono&display=swap');

    html {
      font-family: 'Roboto';
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }

    body {
      background: url('https://hdqwalls.com/wallpapers/blur-background-6z.jpg') no-repeat center center fixed;
    }

    .dht-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }

    .readings {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
      color: purple
    }

    .container {
      font-family: 'Roboto';
      width: 600px;
      margin: 70px auto 0;
      display: block;
      background: #ececec;
      padding: 10px 50px 50px;
      border: 5px solid #ebebeb;
      box-shadow: 0 19px 38px rgba(0, 0, 0, 0.30), 0 15px 12px rgba(0, 0, 0, 0.22);
    }

    h2 {
      text-align: center;
      margin-bottom: 50px;
    }

    h2 small {
      padding-top: 10px;
      font-weight: normal;
      color: #888;
      display: block;
    }

    .footer {
      padding-top: 100px;
      text-align: center;
    }
  </style>
</head>

<body>
<div class="container">
    <h2>TumbleBot<small>Tumbledryer Monitoring System</small></h2>
    <br>
    <p>
      <i class="fas fa-plug" style="color:#581845;"></i>
      <span class="dht-labels"> Power stat: </span>
      <span class="readings" id="status">%STATUS%</span>
    </p>
    <p>
      <i class="fas fa-power-off" style="color:#581845;"></i>
      <span class="dht-labels"> Cycle stat: </span>
      <span class="readings" id="finished">%FINISHED%</span>
    </p>
    <p>
      <i class="fas fa-microchip" style="color:#581845;"></i>
      <span class="dht-labels">LDR Reading: </span>
      <span class="readings" id="ldr">%LDR%</span>
    </p>
    <p>
      <i class="fas fa-memory" style="color:#581845;"></i>
      <span class="dht-labels">SW420 Reading: </span>
      <span class="readings" id="sw420">%SW420%</span>
    </p>
      <br>
      <p class="footer">An IoT device by Pierre Charles</p>
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
}, 5000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sw420").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/sw420", true);
  xhttp.send();
}, 5000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("status").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}, 5000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("finished").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/finished", true);
  xhttp.send();
}, 5000);

</script>
</html>)rawText";

// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
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
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
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

  // Start server
  server.begin();
}

void loop() {
  if (flag) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
    finished = false;
    lcd.clear();
    lcd.drawString(0, 0, "Dryer is on!");
    Serial.println("DRYER IS ON");
    flag = false;
  } else {
    digitalWrite(blueLED, LOW);
    lcd.clear();
  }

  delay(2000);

  if (!flag && (analogRead(ldr) < light_threshold)) {
    digitalWrite(blueLED, LOW);
    digitalWrite(greenLED, HIGH);
    dryerStat = false;
    lcd.drawString(0, 0, "Dryer is finished!");
    Serial.println("Tumble Dryer has finished!");
    finished = true;
    if (messageSent == 0 && WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connecting to IFTTT");
      if (client.connect("maker.ifttt.com", 80))
      {
        telegramTrigger();
        messageSent++;
      }
    }
    delay(2000);
  }
}
