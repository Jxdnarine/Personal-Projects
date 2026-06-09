#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED SPI pins
#define OLED_MOSI 23   // OLED SDA
#define OLED_CLK  18   // OLED SCK
#define OLED_DC   16   // ESP32 RX2 label
#define OLED_CS   5
#define OLED_RESET 17  // ESP32 TX2 label

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &SPI,
  OLED_DC,
  OLED_RESET,
  OLED_CS
);

// Eye center
int eyeCenterX = 64;
int eyeCenterY = 32;

// Iris position
int irisX = 64;
int irisY = 32;

// Target position for smooth movement
int targetX = 64;
int targetY = 32;

// Timing
unsigned long lastMoveTime = 0;
unsigned long lastBlinkTime = 0;

void setup() {
  Serial.begin(115200);

  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED failed to start");
    while (true);
  }

  randomSeed(analogRead(0));

  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long now = millis();

  // Choose a new random look direction every 700 ms
  if (now - lastMoveTime > 700) {
    targetX = eyeCenterX + random(-14, 15);
    targetY = eyeCenterY + random(-7, 8);

    lastMoveTime = now;
  }

  // Smoothly move iris toward target
  irisX += (targetX - irisX) / 4;
  irisY += (targetY - irisY) / 4;

  // Blink every few seconds
  if (now - lastBlinkTime > random(2500, 5500)) {
    blinkEye();
    lastBlinkTime = now;
  }

  drawOpenEye(irisX, irisY);

  delay(45);
}

void drawOpenEye(int x, int y) {
  display.clearDisplay();

  // Outer eye shape - upper eyelid
  display.drawLine(18, 32, 30, 24, SSD1306_WHITE);
  display.drawLine(30, 24, 45, 18, SSD1306_WHITE);
  display.drawLine(45, 18, 64, 15, SSD1306_WHITE);
  display.drawLine(64, 15, 83, 18, SSD1306_WHITE);
  display.drawLine(83, 18, 98, 24, SSD1306_WHITE);
  display.drawLine(98, 24, 110, 32, SSD1306_WHITE);

  // Outer eye shape - lower eyelid
  display.drawLine(18, 32, 30, 40, SSD1306_WHITE);
  display.drawLine(30, 40, 45, 46, SSD1306_WHITE);
  display.drawLine(45, 46, 64, 49, SSD1306_WHITE);
  display.drawLine(64, 49, 83, 46, SSD1306_WHITE);
  display.drawLine(83, 46, 98, 40, SSD1306_WHITE);
  display.drawLine(98, 40, 110, 32, SSD1306_WHITE);

  // Iris
  display.drawCircle(x, y, 13, SSD1306_WHITE);
  display.drawCircle(x, y, 12, SSD1306_WHITE);

  // Pupil
  display.fillCircle(x, y, 6, SSD1306_WHITE);

  // Highlight
  display.fillCircle(x - 4, y - 4, 2, SSD1306_BLACK);

  // Iris details, moving with iris
  display.drawLine(x, y - 13, x, y - 7, SSD1306_WHITE);
  display.drawLine(x, y + 7, x, y + 13, SSD1306_WHITE);
  display.drawLine(x - 13, y, x - 7, y, SSD1306_WHITE);
  display.drawLine(x + 7, y, x + 13, y, SSD1306_WHITE);

  display.drawLine(x - 9, y - 9, x - 4, y - 4, SSD1306_WHITE);
  display.drawLine(x + 9, y - 9, x + 4, y - 4, SSD1306_WHITE);
  display.drawLine(x - 9, y + 9, x - 4, y + 4, SSD1306_WHITE);
  display.drawLine(x + 9, y + 9, x + 4, y + 4, SSD1306_WHITE);

  // Eyelashes
  display.drawLine(35, 22, 30, 14, SSD1306_WHITE);
  display.drawLine(49, 17, 47, 9, SSD1306_WHITE);
  display.drawLine(64, 15, 64, 7, SSD1306_WHITE);
  display.drawLine(79, 17, 81, 9, SSD1306_WHITE);
  display.drawLine(93, 22, 98, 14, SSD1306_WHITE);

  display.display();
}

void blinkEye() {
  // Half closed
  drawBlinkFrame(8);
  delay(45);

  // Almost closed
  drawBlinkFrame(4);
  delay(45);

  // Fully closed
  drawBlinkFrame(1);
  delay(90);

  // Open back up
  drawBlinkFrame(4);
  delay(45);

  drawBlinkFrame(8);
  delay(45);
}

void drawBlinkFrame(int openness) {
  display.clearDisplay();

  int centerY = 32;

  // Draw eyelids based on openness
  display.drawLine(18, centerY, 35, centerY - openness, SSD1306_WHITE);
  display.drawLine(35, centerY - openness, 64, centerY - openness - 2, SSD1306_WHITE);
  display.drawLine(64, centerY - openness - 2, 93, centerY - openness, SSD1306_WHITE);
  display.drawLine(93, centerY - openness, 110, centerY, SSD1306_WHITE);

  display.drawLine(18, centerY, 35, centerY + openness, SSD1306_WHITE);
  display.drawLine(35, centerY + openness, 64, centerY + openness + 2, SSD1306_WHITE);
  display.drawLine(64, centerY + openness + 2, 93, centerY + openness, SSD1306_WHITE);
  display.drawLine(93, centerY + openness, 110, centerY, SSD1306_WHITE);

  // If the eye is still open enough, show the iris squished
  if (openness > 4) {
    display.drawCircle(irisX, irisY, 8, SSD1306_WHITE);
    display.fillCircle(irisX, irisY, 4, SSD1306_WHITE);
    display.fillCircle(irisX - 3, irisY - 3, 1, SSD1306_BLACK);
  }

  // Fully closed crease line
  if (openness <= 1) {
    display.drawLine(20, centerY, 40, centerY + 2, SSD1306_WHITE);
    display.drawLine(40, centerY + 2, 64, centerY + 3, SSD1306_WHITE);
    display.drawLine(64, centerY + 3, 88, centerY + 2, SSD1306_WHITE);
    display.drawLine(88, centerY + 2, 108, centerY, SSD1306_WHITE);
  }

  display.display();
}