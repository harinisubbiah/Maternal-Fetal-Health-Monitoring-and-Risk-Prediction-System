#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <ESP8266HTTPClient.h>

#define ECG_PIN A0

const char* ssid = "<ssid>";
const char* password = "<password>";

// Use HTTPS URL
const char* serverName = "http://10.36.81.206:5000/heartrate";


unsigned long lastBeat = 0;
float BPM = 0;
int threshold = 558;


void setup() {
  Serial.begin(115200);
  pinMode(ECG_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected!");

  Serial.println(WiFi.localIP());
}

void loop() {
  int ecgValue = analogRead(ECG_PIN);

  if (ecgValue > threshold) {
    unsigned long now = millis();
    if (now - lastBeat > 300) {
      BPM = 60000.0 / (now - lastBeat);
      lastBeat = now;

      float adjustedBPM = BPM ;
      Serial.printf("Heart Rate: %.1f BPM\n", adjustedBPM);

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();  // ⚠️ skip certificate check (good for testing only)

        HTTPClient https;
        if (https.begin(client, serverName)) {
          https.addHeader("Content-Type", "application/json");


          String jsonData = "{\"heart_rate\": " + String(adjustedBPM, 1) + "}";
          int httpResponseCode = https.POST(jsonData);

          if (httpResponseCode > 0) {
            Serial.printf("✅ HTTPS POST OK, code: %d\n", httpResponseCode);
          } else {
            Serial.printf("❌ HTTPS POST failed: %s\n",
                          https.errorToString(httpResponseCode).c_str());
          }

          https.end();
        } else {
          Serial.println("❌ HTTPS begin() failed");
        }
      }
    }
  }

  delay(2);
}
