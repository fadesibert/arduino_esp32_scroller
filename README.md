# arduino_esp32_scroller
Code for pulling text from an API endpoint and scrolling it across a matrix

* Configure the defines for your chosen LED pin, Matrix dimensions colour mode, an optional status LED pin
* Configure a variable for the HTTP API URL you want to call
* Tweak the code if you want to display a different key than "message"

This works on an ESP32 with Wifi running Arduino. Works (barely) on a 32x8 matrix (may have to shave a bit and go for 30x7) on an Arduino Nano or similar
