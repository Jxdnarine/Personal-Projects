/*
  ESP32 + 2.8" SPI TFT 240x320 Screen Test
  With TFT LED/backlight controlled by GPIO 15 / D15

  Display pins:
  MISO, LED, SCK, MOSI, DC, RESET, CS, GND, VCC

  Wiring:
  Display VCC   -> ESP32 3V3
  Display GND   -> ESP32 GND
  Display SCK   -> ESP32 GPIO 18
  Display MOSI  -> ESP32 GPIO 23
  Display MISO  -> ESP32 GPIO 19
  Display CS    -> ESP32 GPIO 5
  Display DC    -> ESP32 GPIO 2
  Display RESET -> ESP32 GPIO 4
  Display LED   -> ESP32 GPIO 15 / D15

  Required Libraries:
  - Adafruit GFX Library
  - Adafruit ILI9341
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// TFT pin definitions
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_MISO  19
#define TFT_LED   15   // Display LED/backlight pin connected to D15/GPIO15

// Create TFT object using hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(
  TFT_CS,
  TFT_DC,
  TFT_RST
);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting TFT screen test...");

  // Turn on display backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);   // Most TFT boards use HIGH = backlight ON

  // Start SPI manually for ESP32
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  // Start display
  tft.begin();

  // Rotation options:
  // 0 = portrait
  // 1 = landscape
  // 2 = upside-down portrait
  // 3 = upside-down landscape
  tft.setRotation(1);

  // Color test
  tft.fillScreen(ILI9341_RED);
  delay(500);

  tft.fillScreen(ILI9341_GREEN);
  delay(500);

  tft.fillScreen(ILI9341_BLUE);
  delay(500);

  tft.fillScreen(ILI9341_BLACK);
  delay(500);

  // Text test
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("ESP32 TFT Test");

  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(20, 55);
  tft.println("240x320 SPI Display");

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(20, 90);
  tft.println("If you see this,");
  tft.setCursor(20, 115);
  tft.println("screen works!");

  // Shape test
  tft.drawRect(15, 150, 120, 60, ILI9341_WHITE);
  tft.fillCircle(200, 180, 30, ILI9341_MAGENTA);
  tft.drawLine(0, 0, 319, 239, ILI9341_GREEN);

  delay(1500);
}

void loop() {
  // Moving box animation left to right
  for (int x = 0; x < 260; x += 5) {
    tft.fillRect(0, 200, 320, 40, ILI9341_BLACK);

    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 200);
    tft.print("Animation running...");

    tft.fillRect(x, 215, 40, 20, ILI9341_ORANGE);

    delay(30);
  }

  // Moving box animation right to left
  for (int x = 260; x > 0; x -= 5) {
    tft.fillRect(0, 200, 320, 40, ILI9341_BLACK);

    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 200);
    tft.print("Animation running...");

    tft.fillRect(x, 215, 40, 20, ILI9341_ORANGE);

    delay(30);
  }
}