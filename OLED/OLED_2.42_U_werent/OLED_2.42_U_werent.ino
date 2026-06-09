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

// Song info
const char songTitle[] = "U Weren't Here I Really Miss You";
const char artist[] = "Cult Member";

// Song length: 2 min 41 sec
const int SONG_LENGTH_SECONDS = 161;

// Animation
int scrollX = 128;
unsigned long startTime = 0;
unsigned long lastFrame = 0;

void setup() {
  Serial.begin(115200);

  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED failed to start");
    while (true);
  }

  display.setTextWrap(false);  // Keeps scrolling text on one line

  startTime = millis();

  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long now = millis();

  if (now - lastFrame > 140) {
    drawMusicScreen();
    lastFrame = now;
  }
}

void drawMusicScreen() {
  display.clearDisplay();

  unsigned long elapsedSeconds = (millis() - startTime) / 1000;

  if (elapsedSeconds >= SONG_LENGTH_SECONDS) {
    startTime = millis();
    elapsedSeconds = 0;
    scrollX = 128;
  }

  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Clean outer border
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);

  // Header
  display.setCursor(30, 4);
  display.print("NOW PLAYING");

  display.drawLine(8, 14, 120, 14, SSD1306_WHITE);

  // Small music note
  drawSmallMusicNote(13, 25);

  // Artist name
  display.setCursor(31, 22);
  display.print(artist);

  // Bouncing 3-bar equalizer
  drawBouncingBars(105, 31);

  // Clear song-title strip so it stays clean
  display.fillRect(4, 38, 120, 10, SSD1306_BLACK);

  // ONE-LINE scrolling title, no box
  display.setCursor(scrollX, 40);
  display.print(songTitle);

  // Slow scrolling
  scrollX -= 1;

  int titlePixelLength = strlen(songTitle) * 6;

  if (scrollX < -titlePixelLength) {
    scrollX = 128;
  }

  // Bottom time label: elapsed/total
  display.setCursor(5, 55);
  printTime(elapsedSeconds);
  display.print("/");
  printTime(SONG_LENGTH_SECONDS);

  // Progress bar to the right of the numbers
  int barX = 62;
  int barY = 57;
  int barW = 49;
  int barH = 4;

  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);

  int fillW = map(elapsedSeconds, 0, SONG_LENGTH_SECONDS, 0, barW - 2);
  display.fillRect(barX + 1, barY + 1, fillW, barH - 2, SSD1306_WHITE);

  // Tiny play icon
  display.fillTriangle(116, 55, 116, 62, 123, 58, SSD1306_WHITE);

  display.display();
}

void drawSmallMusicNote(int x, int y) {
  display.fillCircle(x, y + 6, 2, SSD1306_WHITE);
  display.drawLine(x + 2, y + 6, x + 2, y - 5, SSD1306_WHITE);
  display.drawLine(x + 2, y - 5, x + 9, y - 3, SSD1306_WHITE);
  display.drawLine(x + 9, y - 3, x + 9, y + 6, SSD1306_WHITE);
  display.fillCircle(x + 7, y + 6, 2, SSD1306_WHITE);
}

void drawBouncingBars(int x, int baseY) {
  int t = millis() / 120;

  int h1 = 4 + ((t * 3) % 9);
  int h2 = 5 + ((t * 5 + 4) % 11);
  int h3 = 3 + ((t * 7 + 2) % 8);

  display.fillRect(x,      baseY - h1, 3, h1, SSD1306_WHITE);
  display.fillRect(x + 6,  baseY - h2, 3, h2, SSD1306_WHITE);
  display.fillRect(x + 12, baseY - h3, 3, h3, SSD1306_WHITE);
}

void printTime(int totalSeconds) {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  display.print(minutes);
  display.print(":");

  if (seconds < 10) {
    display.print("0");
  }

  display.print(seconds);
}