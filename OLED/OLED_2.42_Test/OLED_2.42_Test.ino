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

void setup() {
  Serial.begin(115200);

  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED failed to start");
    while (true);
  }

  drawEye();
}

void loop() {
  // Nothing here
}

void drawEye() {
  display.clearDisplay();

  // Smaller outer eye shape - upper eyelid
  display.drawLine(18, 32, 30, 24, SSD1306_WHITE);
  display.drawLine(30, 24, 45, 18, SSD1306_WHITE);
  display.drawLine(45, 18, 64, 15, SSD1306_WHITE);
  display.drawLine(64, 15, 83, 18, SSD1306_WHITE);
  display.drawLine(83, 18, 98, 24, SSD1306_WHITE);
  display.drawLine(98, 24, 110, 32, SSD1306_WHITE);

  // Smaller outer eye shape - lower eyelid
  display.drawLine(18, 32, 30, 40, SSD1306_WHITE);
  display.drawLine(30, 40, 45, 46, SSD1306_WHITE);
  display.drawLine(45, 46, 64, 49, SSD1306_WHITE);
  display.drawLine(64, 49, 83, 46, SSD1306_WHITE);
  display.drawLine(83, 46, 98, 40, SSD1306_WHITE);
  display.drawLine(98, 40, 110, 32, SSD1306_WHITE);

  // Smaller iris
  display.drawCircle(64, 32, 13, SSD1306_WHITE);
  display.drawCircle(64, 32, 12, SSD1306_WHITE);

  // Smaller pupil
  display.fillCircle(64, 32, 6, SSD1306_WHITE);

  // Eye highlight
  display.fillCircle(60, 28, 2, SSD1306_BLACK);

  // Iris detail lines
  display.drawLine(64, 19, 64, 25, SSD1306_WHITE);
  display.drawLine(64, 39, 64, 45, SSD1306_WHITE);
  display.drawLine(51, 32, 58, 32, SSD1306_WHITE);
  display.drawLine(70, 32, 77, 32, SSD1306_WHITE);

  display.drawLine(55, 23, 60, 28, SSD1306_WHITE);
  display.drawLine(73, 23, 68, 28, SSD1306_WHITE);
  display.drawLine(55, 41, 60, 36, SSD1306_WHITE);
  display.drawLine(73, 41, 68, 36, SSD1306_WHITE);

  // Smaller eyelashes
  display.drawLine(35, 22, 30, 14, SSD1306_WHITE);
  display.drawLine(49, 17, 47, 9, SSD1306_WHITE);
  display.drawLine(64, 15, 64, 7, SSD1306_WHITE);
  display.drawLine(79, 17, 81, 9, SSD1306_WHITE);
  display.drawLine(93, 22, 98, 14, SSD1306_WHITE);

  display.display();
}