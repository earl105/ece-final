//https://github.com/T-vK/ESP32-BLE-Keyboard/releases/tag/0.3.0
//Sketch -> include library -> add .zip library -> select zip 
//make sure esp32 by espressif systems is version @2.0.13 (newer doesnt work with BLE library)
//yes this means uninstall newer versions, new versions have an error with zigbee mode setting
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Keyboard");


// BUTTONS
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

// LED PWM config (used with ledcAttach / ledcWrite API)
const int freq = 5000;
const int resolution = 8;

// state
int state = 1;

// debounce
int lastReading = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50UL;



void setup() {
  Serial.begin(115200);

  // Use internal pull-ups so pins are not floating.
  // Wire your buttons between pin and GND for this to work.
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(main_btn, INPUT_PULLUP);

  // Attach PWM using the older API you have working
  // (some cores provide ledcAttach(pin,freq,res))
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
    Serial.printf("state %d, button %d\n",state,button);
     Serial.printf("Button %d Pressed\n",button);
    delay(200); // optional simple debounce for these prints
    break;
    default: 
     Serial.println("No valid state detected");
     break;

}

}


void media(int button){
  switch (button){
    case 1: 
    Serial.printf("state %d, button %d\n",state,button);
    Serial.println("prev");
    bleKeyboard.press(KEY_MEDIA_PREVIOUS_TRACK);
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    case 2:
    Serial.printf("state %d, button %d\n",state,button);
    Serial.println("play/pause");
    bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    case 3:
    Serial.printf("state %d, button %d\n",state,button);
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



void lights(int button){
  switch (button){
    case 1: 
    Serial.printf("toggling tall lamp\n");
    Serial.printf("state %d, button %d\n",state,button);
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press('T');
    delay(500);
    bleKeyboard.releaseAll();
    break; 
    case 2:
    Serial.printf("state %d, button %d\n",state,button);
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    delay(500);
    break; 
    case 3:
    Serial.printf("state %d, button %d\n",state,button);
    bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
    delay(500);
    break; 
    default:
    Serial.println("invalid button value");
    break;

   }
}




void loop() {
  // -------- debounced rising-edge detection for main_btn ----------
  int reading = digitalRead(main_btn); // note: using INPUT_PULLUP -> LOW when pressed

  // Because we're using INPUT_PULLUP, pressed == LOW. We'll detect
  // a falling edge (HIGH -> LOW) as a press.
  if (reading != lastReading) {
    lastDebounceTime = millis(); // reset debounce timer when reading changes
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // stable reading
    static int stableState = HIGH; // remember stable (not volatile)
    if (reading != stableState) {
      // state changed
      stableState = reading;
      if (stableState == LOW) { // button pressed (since INPUT_PULLUP)
        inc_mode();
        Serial.println("Main button pressed (debounced)");
      }
    }
  }

  lastReading = reading;
if (bleKeyboard.isConnected()) {
  // -------- other buttons (also using INPUT_PULLUP) ----------
  if (digitalRead(btn1) == LOW) {
    exec(1);
  } else if (digitalRead(btn2) == LOW) {
    exec(2);
  } else if (digitalRead(btn3) == LOW) {
    exec(3);
  } else {
    delay(10);
  }
} else{
    Serial.println("Keyboard not connected, retrying...");
    delay(1000); // short delay to prevent spamming
}
}
