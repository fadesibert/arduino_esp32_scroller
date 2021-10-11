#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontMatrise.h>

// Change the next 6 defines to match your matrix type and size

#define LED_PIN         13
#define STATUS_PIN      26
#define COLOR_ORDER     GRB
#define CHIPSET         WS2812B

//#define MATRIX_WIDTH   32
#define MATRIX_WIDTH    96
#define MATRIX_HEIGHT  -8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX

#define WIFI_SSID     "CORNILLON"
#define WIFI_KEY      "garterbeltsandstockings"

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ScrollingMsg;

const char* url  = "http://192.168.102.139:8000/";
DynamicJsonDocument doc(2048);        // reasonably sized Json document buffer


void repeat_flash(int n, int delay_ms){
  for (int i=0; i<=n; i++) {
    flash(delay_ms);
  }
}

void flash(int delay_ms)
{
  int before = digitalRead(STATUS_PIN); // don't just turn the pin off, set back to original state
  digitalWrite(STATUS_PIN, HIGH);       // Flash LED on for delay)ns
  delay(delay_ms);
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(STATUS_PIN, before);     // Restore LED state
}

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(16);            // Could control this with a pot
  FastLED.clear(true);                  // Clear the noise
  FastLED.show();                       // Initialize display
  pinMode(LED_PIN, OUTPUT);             // Set FastLED pin to Output
  pinMode(STATUS_PIN, OUTPUT);          // Set Status LED pin to Output
  digitalWrite(STATUS_PIN, HIGH);       // Light it up
  ScrollingMsg.SetFont(MatriseFontData);  // Set the Matrix font
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0); // Initialize the Matrix data structure

  Serial.begin(115200);                 // Tee up some debugging
  digitalWrite(STATUS_PIN, LOW);        // Switch off the status LED
  WiFi.mode(WIFI_STA);    // Set wifi to Station (Client) mode
  WiFi.disconnect();
  Serial.print("Connecting to wifi");
  WiFi.begin(WIFI_SSID, WIFI_KEY);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(STATUS_PIN, HIGH);     // Flash the Status_LED while connecting to Network
    delay(500);
    digitalWrite(STATUS_PIN, LOW);
    delay(500);
  }
  repeat_flash(3, 50);                  // Flash 3 times in quick succession to indicate connection
  Serial.println("");
  Serial.println("Connected!");
}

unsigned char getMessage() {
  HTTPClient http;
  http.begin(url);                      // Begin HTTP Client
  int httpResponseCode = http.GET();    // Begin GET call
  if (httpResponseCode > 0) {           // Process Response (naive)
    Serial.print("HTTP Response Code: ");   // Debug output
    Serial.println(httpResponseCode);       // Debug output
  }
    else {
      Serial.print("Error code: ");     // Error Code < 0??
      Serial.println(httpResponseCode);
    }

    DeserializationError error = deserializeJson(doc, http.getStream());
    if(error) {
      Serial.print("Deserialization failed: ");
      Serial.println(error.f_str());
    }

    unsigned char message = doc["message"];
    http.end();
    return message;
}

void displayScrollingMessage(unsigned char* message)
{
  ScrollingMsg.SetText((unsigned char *) message, sizeof(message) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x22, 0xff, 0x22);
  EVERY_N_MILLISECONDS(50) {
    if (ScrollingMsg.UpdateText() == -1){
      ScrollingMsg.SetText((unsigned char *)message, sizeof(message) - 1);  
    }
    else{
      FastLED.show();
    }
  }  
}

void loop()
{
    unsigned char message = getMessage();

    if(message){
      displayScrollingMessage(&message);
    }
    delay(10 * 1000);                 // Wait a while before re-requesting
}
