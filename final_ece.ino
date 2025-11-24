//https://github.com/T-vK/ESP32-BLE-Keyboard/releases/tag/0.3.0
//Sketch -> include library -> add .zip library -> select zip 
//make sure esp32 by espressif systems is version @2.0.13 (newer doesnt work with BLE library)
//yes this means uninstall newer versions, new versions have an error with zigbee mode setting
#include <BleKeyboard.h>


//https://github.com/kj831ca/KasaSmartPlug
#include <WiFi.h>
#include "KasaSmartPlug.h"

String Lamp1ID = "fish_lamp";
String Lamp2ID = "tall_lamp";
String Lamp3ID = "null";


const char *ssid = "";
const char *password = "";


//tools -> partition scheme -> huge APP
//For storage reasons

BleKeyboard bleKeyboard("ESP32 Keyboard");

KASASmartPlug *lamp1 = nullptr;
KASASmartPlug *lamp2 = nullptr;
KASASmartPlug *lamp3 = nullptr;

KASAUtil kasaUtil;

class DebouncedButton {
public:
    int pin;
    unsigned long lastChange = 0;
    bool stableState = HIGH;
    bool lastReading = HIGH;
    const unsigned long debounceDelay = 50;

    DebouncedButton(int p) { pin = p; }

    void begin() {
        pinMode(pin, INPUT_PULLUP);
    }

    // returns true ONCE per actual press
    bool isPressed() {
        bool reading = digitalRead(pin);

        if (reading != lastReading) {
            lastChange = millis();
        }

        if ((millis() - lastChange) > debounceDelay) {
            if (reading != stableState) {
                stableState = reading;
                if (stableState == LOW) {  // bc INPUT_PULLUP
                    lastReading = reading;
                    return true;  // <- press event
                }
            }
        }

        lastReading = reading;
        return false;
    }
};



// BUTTON PINS
const int btn3 = 19;
const int btn2 = 18;
const int btn1 = 5;
const int main_btn = 4;

// RGB LED pins
const int redPin = 27;
const int greenPin = 26;
const int bluePin = 25;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;

// LED PWM config
const int freq = 5000;
const int resolution = 8;

// state
int state = 1;

// debounce
int lastReading = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50UL;

//buttons
DebouncedButton b1(btn1);
DebouncedButton b2(btn2);
DebouncedButton b3(btn3);
DebouncedButton bMain(main_btn);

void setup() {

   int found;
   Serial.begin(115200);


   Serial.printf("Connecting to %s ", ssid);
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED)
     {
       delay(500);
       Serial.print(".");
     }
     Serial.println(" CONNECTED");

      found = kasaUtil.ScanDevices();
      // print out devices name and ip address found
     for (int i = 0; i < found; i++)
     {
       KASASmartPlug *p = kasaUtil.GetSmartPlugByIndex(i);
       Serial.printf("\r\n %d. %s IP: %s Relay: %d", i, p->alias, p->ip_address, p->state);

    if (strcmp(p->alias, Lamp1ID) == 0) {
        lamp1 = p;
    } else if (strcmp(p->alias, Lamp2ID) == 0) {
        lamp2 = p;
    } else if (strcmp(p->alias, Lamp3ID) == 0) {
        lamp3 = p;
    }

     } 
  

 

b1.begin();
b2.begin();
b3.begin();
bMain.begin();



  // attach PWM using the older API 
  // (some newer cores provide ledcAttach(pin,freq,res) rather than attachPin)
  ledcSetup(redChannel, freq, resolution);
  ledcSetup(greenChannel, freq, resolution);
  ledcSetup(blueChannel, freq, resolution);
  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);

  // start with a known color/off
  setColor(0, 0, 0);

  //initialize bt keyboard
   bleKeyboard.begin();
   bleKeyboard.setDelay(100);
}

void setColor(int red, int green, int blue) {
  // ledcWrite(pin, value) for the older API
  ledcWrite(redChannel, red);
  ledcWrite(greenChannel, green);
  ledcWrite(blueChannel, blue);
}

void inc_mode() {
  state++;
  if (state > 3) state = 1;

  switch (state) {
    case 1: setColor(255, 0, 0); break; // Red
    case 2: setColor(0, 255, 0); break; // Green
    case 3: setColor(0, 0, 255); break; // Blue
    default: setColor(255, 255, 255); break; //in case something goes wrong
  }
}


void exec(int button){

switch (state){
  case 1:
   media(button);
    break;
    case 2:
    lights(button);
    break;
    case 3:
    printState(button);
    break;
    default: 
     Serial.println("No valid state detected");
     break;

}

}


void media(int button){
  switch (button){
    case 1: 
     printState(button);
    Serial.println("prev");
    bleKeyboard.press(KEY_MEDIA_PREVIOUS_TRACK);
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    case 2:
     printState(button);
    Serial.println("play/pause");
    bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    case 3:
     printState(button);
    Serial.println("next");
    bleKeyboard.press(KEY_MEDIA_NEXT_TRACK);
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    default:
    Serial.println("invalid button value");
    break;

   }
}


void togglePlug(KASASmartPlug *plug){

    plug->QueryInfo();
    int current = plug->state;
    int newState = (current == 1) ? 0 : 1;
    plug->SetRelayState(newState);

}

void printState(int button){
    Serial.printf("state %d, button %d\n",state,button);
}

void lights(int button){
  switch (button){
    case 1: 
    Serial.printf("toggling lamp 1\n");
    printState(button);
    togglePlug(lamp1); 
    break; 
    case 2:
    Serial.printf("toggling lamp 2\n");
    printState(button);
    togglePlug(lamp2);
    break; 
    case 3:
    Serial.printf("toggling lamp 3\n");
    printState(button);
    togglePlug(lamp3);
    delay(500);
    break; 
    default:
    Serial.println("invalid button value");
    break;

   }
}

void loop() {
  if (bMain.isPressed()) {
    inc_mode();
    Serial.printf("Main button pressed (debounced). State -> %d\n", state);
  }

if (bleKeyboard.isConnected()) {
  //other buttons (also using INPUT_PULLUP)
   if (b1.isPressed()) {
      exec(1);
    }

    if (b2.isPressed()) {
      exec(2);
    }

    if (b3.isPressed()) {
      exec(3);
    }
} else{
    Serial.println("Keyboard not connected, retrying...");
    delay(1000); // short delay to prevent spamming
}
}
