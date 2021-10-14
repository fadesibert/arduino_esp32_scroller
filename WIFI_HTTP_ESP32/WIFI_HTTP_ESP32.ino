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
#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  -8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX

#define WIFI_SSID     "UPDATE_ME"
#define WIFI_KEY      "UPDATE_ME"

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;
cLEDText ScrollingMsg;

#define DEBUG           false
#define DELAY_SECONDS   10
#define DELAY_MILLIS    1000 * DELAY_SECONDS

#define uS_TO_S_FACTOR	1000000
#define TIME_TO_SLEEP	10

RTC_DATA_ATTR int bootCount = 0;

const char* url  = "http://192.168.102.139:8000/";
DynamicJsonDocument doc(256);        // reasonably sized Json document buffer


void repeat_flash(int n, int delay_ms){
  for (int i=0; i<=n; i++) {
    flash(delay_ms);
  }
}

void flash(int delay_ms)
{
  int before = digitalRead(STATUS_PIN); // don't just turn the pin off, set back to original state
  digitalWrite(STATUS_PIN, HIGH);       // Flash LED on for delay_ms
  delay(delay_ms);
  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(STATUS_PIN, before);     // Restore LED state
}

void setup()
{
  ++bootCount;
  #if(DEBUG)
    Serial.println("Boot number: " + String(bootCount));
  #endif
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
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
  #if(DEBUG)
    Serial.println("");
    Serial.println("Connected!");
  #endif

  
}


void displayScrollingMessage(String message)
{
  char msg_c[254];
  unsigned char msg_uc[256] = {EFFECT_SCROLL_LEFT};                             // Prepare for an array of unsigned char

  strncpy(msg_c, message.c_str(), 254);
  
  std::copy(msg_c+2, msg_c + 256, msg_uc);                                      // Copy the char array to unsigned char, correctly converting
  const unsigned char * TextMessage[] = { msg_uc };                              // Compose the message String into an LED Text Message
  ScrollingMsg.SetText(msg_uc, 255);                                            // Finally, pass to ScrollingMsg
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0x22, 0xff, 0x22);    // Set some basic colour options. We can get FANCY later
  EVERY_N_MILLISECONDS(50) {
    if (ScrollingMsg.UpdateText() == -1){                                       // Scroll through the message every 50ms
      //ScrollingMsg.SetText((unsigned char *)message, sizeof(message) - 1);
      return;                                                                   // Only display the message once  
    }
    else{
      FastLED.show();                                                           // Display the updated frame
    }
  }  
}

void loop()
{
  String message;
  
  HTTPClient http;
  http.begin(url);                      // Begin HTTP Client
  int httpResponseCode = http.GET();    // Begin GET call
  if (httpResponseCode > 0) {           // Process Response (naive)
    #if(DEBUG)
      Serial.print("HTTP Response Code: ");   // Debug output
      Serial.println(httpResponseCode);       // Debug output
    #endif
//    if (httpResponseCode == 200 || httpResponseCode == 404){
//      //proceed
//    }
  }

    else {
      Serial.print("Error code: ");     // Error Code < 0??
      Serial.println(httpResponseCode);
    }
    
    DeserializationError error = deserializeJson(doc, http.getStream());
    if(error) {
      #if(DEBUG)
        Serial.print("Deserialization failed: ");
        Serial.println(error.f_str());
      #endif
    }
    
    if(doc.containsKey("message")){
      message = doc["message"].as<String>();
      #if(DEBUG)
        Serial.println(message);
        Serial.flush();
        Serial.println("Displaying message...");
      #endif
      repeat_flash(5, 50);
      displayScrollingMessage(message);
    }
    else {
      #if(DEBUG)
        Serial.println("document does not contain a message");
      #endif
    }

    http.end();
    Serial.flush();
    esp_deep_sleep_start();
}
