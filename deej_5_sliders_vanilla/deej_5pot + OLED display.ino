#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int NUM_SLIDERS = 4;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3};
const char* knobNames[NUM_SLIDERS] = {"Master", "Alexa.exe", "Headphones", "Current"};

int rawValues[NUM_SLIDERS];
int displayValues[NUM_SLIDERS];
int prevDisplayValues[NUM_SLIDERS];
int lastMovedIndex = 0;
unsigned long lastActivityTime = 0;
const unsigned long DISPLAY_TIMEOUT = 1000;

void setup() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
    while(1);
  }
  display.setTextSize(1.5);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  displayDefaultScreen();
}

void showActiveKnob() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(knobNames[lastMovedIndex]);
  
  display.setTextSize(2); // Changed from 3 to 2
  display.setCursor(0, 25);
  display.print(displayValues[lastMovedIndex]);
  display.print("%");
  
  // Progress bar
  int barWidth = map(displayValues[lastMovedIndex], 0, 100, 0, SCREEN_WIDTH);
  display.drawRect(0, 54, SCREEN_WIDTH, 10, WHITE);
  display.fillRect(0, 54, barWidth, 10, WHITE);
  
  display.display();
}

void displayDefaultScreen() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(knobNames[0]);
    
    display.setTextSize(2); // Changed from 3 to 2
    display.setCursor(0, 25);
    display.print(displayValues[0]);
    display.print("%");
    
    // Progress bar
    int barWidth = map(displayValues[0], 0, 100, 0, SCREEN_WIDTH);
    display.drawRect(0, 54, SCREEN_WIDTH, 10, WHITE);
    display.fillRect(0, 54, barWidth, 10, WHITE);
    
    display.display();
    lastUpdate = millis();
  }
}

void loop() {
  updateValues();
  checkForMovement();
  updateDisplay();
  sendSerialData();
  delay(10);
}

void updateValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    rawValues[i] = analogRead(analogInputs[i]);
    int processed = (i >= 1) ? 1023 - rawValues[i] : rawValues[i];
    displayValues[i] = map(processed, 0, 1023, 0, 100);
  }
}

void checkForMovement() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    if (abs(displayValues[i] - prevDisplayValues[i]) > 2) {
      lastMovedIndex = i;
      lastActivityTime = millis();
      prevDisplayValues[i] = displayValues[i];
    }
  }
}

void updateDisplay() {
  if (millis() - lastActivityTime < DISPLAY_TIMEOUT) {
    showActiveKnob();
  } else {
    displayDefaultScreen();
  }
}

void sendSerialData() {
  String dataString = "";
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int serialValue = (i >= 1) ? 1023 - rawValues[i] : rawValues[i];
    dataString += String(serialValue);
    
    if (i < NUM_SLIDERS - 1) dataString += "|";
  }
  Serial.println(dataString);
}
