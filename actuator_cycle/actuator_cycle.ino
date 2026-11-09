// Two VEVOR OK648 linear actuators moving together, endless in/out cycle.
// Shows UP / DOWN / REST on the built-in OLED.
//
// Libraries (Arduino IDE > Library Manager): "Adafruit SSD1306" + "Adafruit GFX Library"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Board: Ideaspark ESP32 with 0.96" OLED (SSD1306, I2C address 0x3C)
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1

// ---------- SETTINGS ----------
const int PIN_RELAY_UP   = 32;   // relay 1 IN -> red actuator wires
const int PIN_RELAY_DOWN = 33;   // relay 2 IN -> black actuator wires
// Full stroke time = stroke / speed, plus ~2 s extra so both actuators
// always reach their internal end stop (keeps them in sync).
// 150 mm @ 14 mm/s ≈ 11 s -> 13 s.   300 mm @ 5 mm/s = 60 s -> 62 s.
const unsigned long MOVE_MS = 13000;

// Actuators are rated 25% duty cycle: rest 3x as long as they move.
const unsigned long REST_MS = MOVE_MS * 0;

// ---------- STATE ----------
enum State { MOVING_UP, REST_AFTER_UP, MOVING_DOWN, REST_AFTER_DOWN };
State state;
unsigned long stateStart;
unsigned long cycles = 0;

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);

// Relay jumper on "L" (low-level trigger): pulling IN to GND = relay on.
// "Off" releases the pin (input / floating) instead of driving 3.3 V, because
// 3.3 V is not high enough to fully switch off a module powered from 5 V.
void setRelay(int pin, bool on) {
  if (on) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  } else {
    pinMode(pin, INPUT);
  }
}

void relaysOff() {
  setRelay(PIN_RELAY_UP, false);
  setRelay(PIN_RELAY_DOWN, false);
}

void enterState(State s) {
  relaysOff();
  delay(200);  // let the contacts open before the other relay closes
  state = s;
  stateStart = millis();
  if (s == MOVING_UP)   setRelay(PIN_RELAY_UP, true);
  if (s == MOVING_DOWN) setRelay(PIN_RELAY_DOWN, true);
}

unsigned long stateLength() {
  return (state == MOVING_UP || state == MOVING_DOWN) ? MOVE_MS : REST_MS;
}

void drawScreen() {
  unsigned long left = (stateLength() - (millis() - stateStart)) / 1000 + 1;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Top band (yellow, y 0-15): small info
  display.setTextSize(1);
  display.setCursor(0, 4);
  display.print("Cycles: ");
  display.print(cycles);
  String t = String(left) + " s";
  display.setCursor(128 - 6 * t.length(), 4);
  display.print(t);

  // Main area (blue, y 16-63)
  display.setTextSize(3);
  display.setCursor(0, 26);
  switch (state) {
    case MOVING_UP:   display.print("UP");   break;
    case MOVING_DOWN: display.print("DOWN"); break;
    default:          display.print("REST"); break;
  }
  // Arrow on the right
  if (state == MOVING_UP)   display.fillTriangle(104, 48, 124, 48, 114, 24, SSD1306_WHITE);
  if (state == MOVING_DOWN) display.fillTriangle(104, 24, 124, 24, 114, 48, SSD1306_WHITE);

  // Progress bar for the current state
  unsigned long len = stateLength();
  int w = len ? (int)(126UL * (millis() - stateStart) / len) : 126;
  if (w > 126) w = 126;
  display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
  display.fillRect(1, 57, w, 6, SSD1306_WHITE);
  display.display();
}

void setup() {
  relaysOff();

  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found - check board selection");
  }

  // Start by going down so both actuators begin from the same end
  enterState(MOVING_DOWN);
}

void loop() {
  if (millis() - stateStart >= stateLength()) {
    switch (state) {
      case MOVING_UP:       enterState(REST_AFTER_UP);   break;
      case REST_AFTER_UP:   enterState(MOVING_DOWN);     break;
      case MOVING_DOWN:     enterState(REST_AFTER_DOWN); break;
      case REST_AFTER_DOWN: cycles++; enterState(MOVING_UP); break;
    }
  }

  static unsigned long lastDraw = 0;
  if (millis() - lastDraw > 200) {
    lastDraw = millis();
    drawScreen();
  }
}
