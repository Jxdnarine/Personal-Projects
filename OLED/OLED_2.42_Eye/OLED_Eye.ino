#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Try 0x3C first. If blank, change to 0x3D.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void drawLeftEye() {
  display.clearDisplay();

  // Eye outline
  display.drawEllipse(64, 32, 55, 22, SSD1306_WHITE);
  display.drawEllipse(64, 32, 54, 21, SSD1306_WHITE);

  // Eyelid shading
  display.drawLine(14, 32, 38, 18, SSD1306_WHITE);
  display.drawLine(38, 18, 64, 12, SSD1306_WHITE);
  display.drawLine(64, 12, 92, 18, SSD1306_WHITE);
  display.drawLine(92, 18, 116, 32, SSD1306_WHITE);

  display.drawLine(14, 32, 38, 46, SSD1306_WHITE);
  display.drawLine(38, 46, 64, 52, SSD1306_WHITE);
  display.drawLine(64, 52, 92, 46, SSD1306_WHITE);
  display.drawLine(92, 46, 116, 32, SSD1306_WHITE);

  // Iris
  display.drawCircle(64, 32, 16, SSD1306_WHITE);
  display.drawCircle(64, 32, 15, SSD1306_WHITE);

  // Pupil
  display.fillCircle(64, 32, 7, SSD1306_WHITE);

  // Eye shine
  display.fillCircle(58, 25, 3, SSD1306_BLACK);
  display.drawCircle(58, 25, 3, SSD1306_WHITE);

  // Iris texture lines
  for (int i = 0; i < 360; i += 20) {
    float angle = i * 3.14159 / 180.0;
    int x1 = 64 + cos(angle) * 9;
    int y1 = 32 + sin(angle) * 9;
    int x2 = 64 + cos(angle) * 15;
    int y2 = 32 + sin(angle) * 15;
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Eyelashes
  for (int x = 25; x <= 105; x += 10) {
    display.drawLine(x, 17, x - 3, 5, SSD1306_WHITE);
  }

  for (int x = 30; x <= 100; x += 12) {
    display.drawLine(x, 47, x - 2, 58, SSD1306_WHITE);
  }

  display.display();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(); // ESP32 default: SDA=21, SCL=22

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found. Try address 0x3D.");
    while (true);
  }

  drawLeftEye();
}

void loop() {
  // Nothing needed
}