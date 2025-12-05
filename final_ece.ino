//tools -> partition scheme -> huge APP
//For storage reasons

//https://github.com/T-vK/ESP32-BLE-Keyboard/releases/tag/0.3.0
//Sketch -> include library -> add .zip library -> select zip
//make sure esp32 by espressif systems is version @2.0.13 (newer doesnt work with BLE library)
//yes this means uninstall newer versions, new versions have an error with zigbee mode setting
#include <BleKeyboard.h>

//https://github.com/kj831ca/KasaSmartPlug
#include <WiFi.h>
#include "KasaSmartPlug.h"

//Declare Lamp/Plug IDS
String Lamp1ID = "fish_lamp";
String Lamp2ID = "tall_lamp";
String Lamp3ID = "null";

//Declare Wifi Credentials
const char *ssid = "";
const char *password = "";

//AppGroups for App Volume Control
enum AppGroup {
  GROUP_1 = 1,  // Chrome, Arc, Zen, Edge, Firefox
  GROUP_2 = 2,  // Discord, Zoom, Teams
  GROUP_3 = 3   // Spotify
};
AppGroup currentGroup = GROUP_1;

//Setup Keyboard
BleKeyboard bleKeyboard("ESP32 Keyboard");

//Setup lamp variables for lamps 1-3
KASASmartPlug *lamp1 = nullptr;
KASASmartPlug *lamp2 = nullptr;
KASASmartPlug *lamp3 = nullptr;
KASAUtil kasaUtil;

//Setup Debounced button class to debounce all buttons
class DebouncedButton {
public:
  int pin;
  unsigned long lastChange = 0;
  bool stableState = HIGH;
  bool lastReading = HIGH;
  const unsigned long debounceDelay = 50;
  DebouncedButton(int p) {
    pin = p;
  }
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

const int main_btn = 4;  //encoder button
const int CLK_PIN = 32;      //encoder clock
const int DT_PIN = 33;       

// Variables to track the encoder state
int counter = 0;
int clkLastState;
int encoderPos;


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

// mode
int state = 1;




//buttons
DebouncedButton b1(btn1);
DebouncedButton b2(btn2);
DebouncedButton b3(btn3);
DebouncedButton bMain(main_btn);


//--------------------
//SETUP
//--------------------

void setup() {

  int found;
  Serial.begin(115200);
  delay(10000);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  found = kasaUtil.ScanDevices();
  // print out devices name and ip address found
  for (int i = 0; i < found; i++) {
    KASASmartPlug *p = kasaUtil.GetSmartPlugByIndex(i);
    Serial.printf("\r\n %d. %s IP: %s Relay: %d", i, p->alias, p->ip_address, p->state);

    if (strcmp(p->alias, Lamp1ID.c_str()) == 0) {
      lamp1 = p;
    } else if (strcmp(p->alias, Lamp2ID.c_str()) == 0) {
      lamp2 = p;
    } else if (strcmp(p->alias, Lamp3ID.c_str()) == 0) {
      lamp3 = p;
    }
  }

  //start debounced buttons
  b1.begin();
  b2.begin();
  b3.begin();
  bMain.begin();

  //rotary
pinMode(CLK_PIN, INPUT);
pinMode(DT_PIN, INPUT);


  



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

   clkLastState = digitalRead(CLK_PIN);

  //initialize bt keyboard
  bleKeyboard.begin();
  bleKeyboard.setDelay(100);
}



//--------------------
//MODE 1: Media control
//--------------------


void media(int button) {
  switch (button) {
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



//--------------------
//MODE 2: LIGHT CONTROL
//--------------------

void lights(int button) {
  switch (button) {
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

void togglePlug(KASASmartPlug *plug) {

  plug->QueryInfo();
  int current = plug->state;
  int newState = (current == 1) ? 0 : 1;
  plug->SetRelayState(newState);
}


//--------------------
//MODE 3: App volume control
//--------------------


void selectGroup(AppGroup g) {
  currentGroup = g;
  Serial.print("Selected Group: ");
  Serial.println((int)g);
}

void groupVolume(int button) {
  switch (button) {
    case 1: selectGroup(GROUP_1); break;  // Chrome/etc
    case 2: selectGroup(GROUP_2); break;  // Discord/Zoom/Teams
    case 3: selectGroup(GROUP_3); break;  // Spotify
    default: Serial.println("Invalid group button"); break;
  }
}


void sendVolumeUp() {
  switch (currentGroup) {
    case GROUP_1:
      bleKeyboard.press(KEY_F13);
      bleKeyboard.release(KEY_F13);
      break;
    case GROUP_2:
      bleKeyboard.press(KEY_F15);
      bleKeyboard.release(KEY_F15);
      break;
    case GROUP_3:
      bleKeyboard.press(KEY_F17);
      bleKeyboard.release(KEY_F17);
      break;
  }
}

void sendVolumeDown() {
  switch (currentGroup) {
    case GROUP_1:
      bleKeyboard.press(KEY_F14);
      bleKeyboard.release(KEY_F14);
      break;
    case GROUP_2:
      bleKeyboard.press(KEY_F16);
      bleKeyboard.release(KEY_F16);
      break;
    case GROUP_3:
      bleKeyboard.press(KEY_F18);
      bleKeyboard.release(KEY_F18);
      break;
  }
}

//--------------------
//HELPERS
//--------------------

//print current state (useful for debug)
void printState(int button) {
  Serial.printf("state %d, button %d\n", state, button);
}

//pick what mode to execute
void exec(int button) {
  switch (state) {
    case 1:
      media(button);
      break;
    case 2:
      lights(button);
      break;
    case 3:
      groupVolume(button);
      break;
    default:
      Serial.println("No valid state detected");
      break;
  }
}

//increment mode and led display for mode
void inc_mode() {
  state++;
  if (state > 3) state = 1;

  switch (state) {
    case 1: setColor(255, 0, 0); break;       // Red
    case 2: setColor(0, 255, 0); break;       // Green
    case 3: setColor(0, 0, 255); break;       // Blue
    default: setColor(255, 255, 255); break;  //in case something goes wrong
  }
}

//set color of R G B values
void setColor(int red, int green, int blue) {
  // ledcWrite(pin, value) for the older API
  ledcWrite(redChannel, red);
  ledcWrite(greenChannel, green);
  ledcWrite(blueChannel, blue);
}

// Handle Rotary Encoder logic based on current state
void handleEncoderRotation() {
  if (encoderPos == 0) return;

  switch (state) {
    case 1:  // Mode 1: Master Volume
      if (encoderPos > 0) {
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        Serial.println("Encoder CW -> Master Volume Up");
      } else {
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        Serial.println("Encoder CCW -> Master Volume Down");
      }
      break;

    case 2:  // Mode 2: Undefined/Placeholder
      Serial.println("Encoder -> Light Mode (Placeholder/Undefined)");
      // TODO Add logic here later for light brightness or selection
      break;

    case 3:  // Mode 3: App Volume
      if (encoderPos > 0) {
        sendVolumeUp();
        Serial.println("Encoder CW -> App Volume Up");
      } else {
        sendVolumeDown();
        Serial.println("Encoder CCW -> App Volume Down");
      }
      break;
  }

  // Reset encoder position after handling
  encoderPos = 0;
}


void check_rotary(){

 int clkCurrentState = digitalRead(CLK_PIN);

 
  if (clkCurrentState != clkLastState) {
  
    if (clkCurrentState == HIGH) {
     
      if (digitalRead(DT_PIN) == HIGH) {
        encoderPos = -1; //ccw
      } else {
        encoderPos = 1; // Cw
      }
      Serial.print("Encoder Value: ");
      Serial.println(encoderPos);
    }
    clkLastState = clkCurrentState; // Update the last state
  }

}



//--------------------
//MAIN LOOP
//--------------------


void loop() {
  check_rotary();
  if (encoderPos != 0) {
    handleEncoderRotation();
  }



  //if main button pressed, change mode
  if (bMain.isPressed()) {
    inc_mode();
    Serial.printf("Main button pressed (debounced). State -> %d\n", state);
  }

  //as long as BLE connected, execute based on mode
  if (bleKeyboard.isConnected()) {
    //other buttons (also using INPUT_PULLUP)
    if (b1.isPressed()) { exec(1); }
    if (b2.isPressed()) { exec(2); }
    if (b3.isPressed()) { exec(3); }
  } else {
    Serial.println("Keyboard not connected, retrying...");
    delay(1000);  // short delay to prevent spamming
  }
}