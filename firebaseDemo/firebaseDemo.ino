#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST "tumblebot-d43f6.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "wNVmE1zoQB48UmIKiOKooCp9A820v9QQzIGNgBH3"
#define WIFI_SSID "VladimirRoutin"
#define WIFI_PASSWORD "jn3qvGncNbn8"


//Define FirebaseESP32 data object
FirebaseData firebaseData;

FirebaseJson json;

void printResult(FirebaseData &data);

String path = "/object";


void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);



}

void loop()
{
  Serial.println("------------------------------------");
  Serial.println("Set Timestamp test...");

  if (Firebase.pushTimestamp(firebaseData, path))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());

    //Timestamp saved in millisecond, get its seconds from intData()
    Serial.print("TIMESTAMP (Seconds): ");
    Serial.println(firebaseData.intData());
  }

  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.setInt(firebaseData, path + "/integer", 123);

  delay(5000);
}
