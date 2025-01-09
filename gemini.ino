#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Wi-Fi Configuration
const char* ssid = "OnePlus 5G";               // Replace with your Wi-Fi SSID
const char* password = "e6h5jthc";       // Replace with your Wi-Fi Password

// OpenAI Gemini API Configuration
const char* Gemini_Token = "AIzaSyCr5dLoC01T9wq6o6BbgXBfZrDD7ufkznc";          // Replace with your API key
const char* Gemini_Max_Tokens = "400";              // Maximum tokens for the response

String res = "";

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
    delay(1000);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to Wi-Fi. Restarting...");
    ESP.restart();  // Restart ESP if Wi-Fi connection fails
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void askQuestion() {
  Serial.println("\nAsk your Question:");

  unsigned long startTime = millis();
  while (!Serial.available() && millis() - startTime < 30000) {  // Timeout after 30 seconds
    delay(10);
  }

  if (!Serial.available()) {
    Serial.println("No input detected.");
    return;
  }

  res = Serial.readStringUntil('\n');  // Read user input
  res.trim();                          // Remove extra whitespace
  res = "\"" + res + "\"";             // Format for JSON

  Serial.print("Asking Your Question: ");
  Serial.println(res);

  // HTTPS client setup
  WiFiClientSecure client;
  client.setInsecure();  // Use only for testing purposes
  HTTPClient http;

  String apiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + String(Gemini_Token);

  if (http.begin(client, apiUrl)) {
    http.addHeader("Content-Type", "application/json");

    String payload = String("{\"contents\": [{\"parts\":[{\"text\":" + res + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + String(Gemini_Max_Tokens) + "}}");

    int httpCode = http.POST(payload);

    if (httpCode == HTTP_CODE_OK) {
      String responsePayload = http.getString();

      // Parse JSON response
      DynamicJsonDocument doc(responsePayload.length() * 1.1);
      DeserializationError error = deserializeJson(doc, responsePayload);

      if (error) {
        Serial.print("JSON Parsing Failed: ");
        Serial.println(error.c_str());
        return;
      }

      String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];
      if (!Answer.isEmpty()) {
        Serial.println("\nHere is your Answer:\n");
        Serial.println(Answer);
      } else {
        Serial.println("Error: Unable to extract the answer.");
      }
    } else {
      Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }

  res = "";  // Clear the user input for the next iteration
}

void setup() {
  Serial.begin(115200);
  connectWiFi();  // Connect to Wi-Fi
}

void loop() {
  askQuestion();  // Prompt the user for a question and fetch the response
  delay(500);     // Small delay to prevent rapid loops
}