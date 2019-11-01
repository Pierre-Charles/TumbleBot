// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

const char* ssid = "VM3156656";
const char* password = "jn3qvGncNbn8";

const int ldr = 34; // Pin used for LDR sensor
const int sw420 = 35;
int light_value = 0;

AsyncWebServer server(80); // AsyncWebServer object on port 80

String readLDR() {
  light_value = analogRead(ldr);
  Serial.println("Reading from LDR: ");
  Serial.println(light_value);
  return String(light_value);
}

String readSW420() {
  long measurement = pulseIn(sw420, HIGH);
  Serial.println("Reading from SW-420");
  Serial.println(measurement);
  return String(measurement);
}

const char index_html[] PROGMEM = R"rawliteral(
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
      background: url(https://hdqwalls.com/wallpapers/blur-background-6z.jpg) no-repeat center center fixed;
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
      <i class="fas fa-microchip" style="color:#581845;"></i>
      <span class="dht-labels">LDR Reading: </span>
      <span class="readings" id="ldr">%LDR%</span>
    </p>
    <p>
      <i class="fas fa-memory" style="color:#581845;"></i>
      <span class="dht-labels">SW420 Reading: </span>
      <span class="readings" id="sw420">%SW420%</span>
      </span>
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
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sw420").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/sw420", true);
  xhttp.send();
}, 1000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "LDR") {
    return readLDR();
  } else if (var == "SW420") {
    return readSW420();
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ldr, INPUT);
  pinMode(sw420, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

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

  // Start server
  server.begin();
}

void loop() {

}
