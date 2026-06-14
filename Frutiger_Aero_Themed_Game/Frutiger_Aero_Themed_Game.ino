/*
  ESP32 + 2.8" SPI TFT + 4 Buttons + Joystick
  Frutiger Aero Themed Game: Aqua Dodge

  Theme:
  - glossy blue/green background
  - bubbles
  - clouds
  - shiny aqua player orb
  - red/purple "glitch virus" enemies

  TFT Wiring:
  TFT VCC   -> ESP32 5V/VIN
  TFT GND   -> ESP32 GND
  TFT SCK   -> GPIO 18
  TFT MOSI  -> GPIO 23
  TFT MISO  -> GPIO 19
  TFT CS    -> GPIO 5
  TFT DC    -> GPIO 2
  TFT RESET -> GPIO 4
  TFT LED   -> GPIO 15

  Button Wiring:
  Button A -> GPIO 25 -> button -> GND
  Button B -> GPIO 26 -> button -> GND
  Button X -> GPIO 27 -> button -> GND
  Button Y -> GPIO 14 -> button -> GND

  Joystick Wiring:
  Joystick GND -> ESP32 GND
  Joystick VCC -> ESP32 3V3
  Joystick VRx -> GPIO 34
  Joystick VRy -> GPIO 35
  Joystick SW  -> GPIO 32

  Libraries:
  - Adafruit GFX Library
  - Adafruit ILI9341
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ---------------- TFT PINS ----------------
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_MISO  19
#define TFT_LED   15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ---------------- INPUT PINS ----------------
#define BTN_A 25
#define BTN_B 26
#define BTN_X 27
#define BTN_Y 14
#define JOY_SW 32

#define JOY_X 34
#define JOY_Y 35

// ---------------- SCREEN ----------------
#define SCREEN_W 320
#define SCREEN_H 240

// ---------------- COLORS ----------------
#define SKY_TOP       0x5D9F
#define SKY_MID       0x7E7F
#define AQUA          0x07FF
#define DEEP_AQUA     0x04BF
#define LIME          0x87E0
#define GREEN_GLOW    0xAFE5
#define WHITE_SOFT    0xF7DE
#define GLASS_BLUE    0x4DFF
#define WATER_BLUE    0x05FF
#define VIRUS_RED     0xF986
#define VIRUS_PURPLE  0xB81F
#define DARK_BLUE     0x0013

// ---------------- PLAYER ----------------
int playerX = 150;
int playerY = 190;
int oldPlayerX = 150;
int oldPlayerY = 190;
int playerR = 9;
int playerSpeed = 4;

// ---------------- ENEMY ----------------
int enemyX = 40;
int enemyY = -30;
int oldEnemyX = 40;
int oldEnemyY = -30;
int enemySize = 22;
int enemySpeed = 3;

// ---------------- COLLECTIBLE ----------------
int bubbleX = 100;
int bubbleY = 80;
int oldBubbleX = 100;
int oldBubbleY = 80;
bool bubbleActive = true;

// ---------------- GAME STATE ----------------
int score = 0;
int highScore = 0;
int lives = 3;

bool gameOver = false;
bool paused = false;

unsigned long lastFrame = 0;
const int frameDelay = 30;

unsigned long lastButtonTime = 0;
const int debounceDelay = 180;

int animTick = 0;
int themeMode = 0;

// ---------------- HELPERS ----------------

bool buttonPressed(int pin) {
  return digitalRead(pin) == LOW;
}

uint16_t mixColor(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

void glossyCircle(int x, int y, int r, uint16_t outer, uint16_t inner, uint16_t shine) {
  tft.fillCircle(x, y, r, outer);
  tft.fillCircle(x - 2, y - 2, r - 3, inner);
  tft.fillCircle(x - 4, y - 5, r / 3, shine);
  tft.drawCircle(x, y, r, WHITE_SOFT);
}

void drawCloud(int x, int y) {
  tft.fillCircle(x, y + 8, 12, WHITE_SOFT);
  tft.fillCircle(x + 14, y + 4, 16, WHITE_SOFT);
  tft.fillCircle(x + 32, y + 8, 12, WHITE_SOFT);
  tft.fillRoundRect(x - 4, y + 8, 44, 13, 8, WHITE_SOFT);
}

void drawPalmLeaf(int x, int y, int dir) {
  uint16_t c = LIME;
  tft.drawLine(x, y, x + dir * 20, y - 8, c);
  tft.drawLine(x, y, x + dir * 18, y, c);
  tft.drawLine(x, y, x + dir * 15, y + 8, c);
}

void drawFrutigerBackground() {
  tft.fillScreen(SKY_TOP);

  // Gradient-ish horizontal layers
  for (int y = 24; y < SCREEN_H; y += 8) {
    uint8_t g = map(y, 24, SCREEN_H, 160, 230);
    uint8_t b = map(y, 24, SCREEN_H, 255, 180);
    uint16_t c = mixColor(70, g, b);
    tft.fillRect(0, y, SCREEN_W, 8, c);
  }

  // Sun glow
  tft.fillCircle(270, 55, 34, mixColor(180, 255, 220));
  tft.fillCircle(270, 55, 24, mixColor(230, 255, 245));
  tft.fillCircle(260, 45, 8, ILI9341_WHITE);

  // Clouds
  drawCloud(25, 42);
  drawCloud(170, 68);

  // Green hill
  tft.fillCircle(80, 260, 110, GREEN_GLOW);
  tft.fillCircle(240, 265, 120, LIME);

  // Water band
  tft.fillRoundRect(0, 174, 320, 60, 20, mixColor(0, 150, 220));
  tft.fillRoundRect(8, 182, 304, 42, 18, mixColor(60, 220, 255));
  tft.drawFastHLine(20, 192, 270, WHITE_SOFT);
  tft.drawFastHLine(60, 211, 190, mixColor(180, 255, 255));

  // Palm-ish decoration
  tft.fillRect(292, 122, 7, 55, mixColor(150, 95, 40));
  drawPalmLeaf(296, 125, -1);
  drawPalmLeaf(296, 125, 1);

  // Bubbles in background
  for (int i = 0; i < 18; i++) {
    int bx = random(5, SCREEN_W - 5);
    int by = random(30, SCREEN_H - 10);
    int br = random(2, 7);
    tft.drawCircle(bx, by, br, WHITE_SOFT);
    tft.drawPixel(bx - 1, by - 1, ILI9341_WHITE);
  }
}

void drawHUD() {
  tft.fillRoundRect(4, 3, 312, 20, 8, DARK_BLUE);
  tft.drawRoundRect(4, 3, 312, 20, 8, WHITE_SOFT);
  tft.drawFastHLine(18, 7, 280, mixColor(140, 255, 255));

  tft.setTextSize(1);

  tft.setTextColor(AQUA);
  tft.setCursor(12, 9);
  tft.print("AQUA DODGE");

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(105, 9);
  tft.print("Score:");
  tft.print(score);

  tft.setCursor(172, 9);
  tft.print("High:");
  tft.print(highScore);

  tft.setCursor(238, 9);
  tft.print("Lives:");
  tft.print(lives);

  if (paused) {
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(282, 9);
    tft.print("PAU");
  }
}

void drawStartScreen() {
  drawFrutigerBackground();

  tft.fillRoundRect(28, 38, 264, 150, 18, mixColor(0, 80, 140));
  tft.drawRoundRect(28, 38, 264, 150, 18, WHITE_SOFT);
  tft.drawRoundRect(32, 42, 256, 142, 15, AQUA);

  tft.setTextSize(3);
  tft.setTextColor(AQUA);
  tft.setCursor(55, 58);
  tft.print("AQUA");

  tft.setTextColor(LIME);
  tft.setCursor(150, 58);
  tft.print("DODGE");

  glossyCircle(160, 118, 22, DEEP_AQUA, WATER_BLUE, ILI9341_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(68, 150);
  tft.println("Frutiger Aero ESP32 Game");

  tft.setCursor(56, 166);
  tft.println("Joystick: Move   X: Dash");

  tft.setCursor(56, 180);
  tft.println("A: Restart  B: Pause  Y: Theme");

  delay(1800);
}

void resetGame() {
  playerX = 150;
  playerY = 190;
  oldPlayerX = playerX;
  oldPlayerY = playerY;

  enemyX = random(20, SCREEN_W - 40);
  enemyY = -30;
  oldEnemyX = enemyX;
  oldEnemyY = enemyY;

  bubbleX = random(30, SCREEN_W - 30);
  bubbleY = random(45, SCREEN_H - 45);
  oldBubbleX = bubbleX;
  oldBubbleY = bubbleY;
  bubbleActive = true;

  score = 0;
  lives = 3;
  enemySpeed = 3;
  gameOver = false;
  paused = false;

  drawFrutigerBackground();
  drawHUD();
}

void erasePlayer() {
  tft.fillRect(oldPlayerX - playerR - 3, oldPlayerY - playerR - 3, playerR * 2 + 6, playerR * 2 + 6, mixColor(60, 210, 250));
}

void drawPlayer() {
  // Erase with local aqua-ish color. Not perfect, but fast.
  tft.fillRect(oldPlayerX - playerR - 4, oldPlayerY - playerR - 4, playerR * 2 + 8, playerR * 2 + 8, mixColor(60, 210, 250));

  uint16_t outer = DEEP_AQUA;
  uint16_t inner = WATER_BLUE;

  if (themeMode == 1) {
    outer = LIME;
    inner = GREEN_GLOW;
  } else if (themeMode == 2) {
    outer = ILI9341_MAGENTA;
    inner = AQUA;
  }

  glossyCircle(playerX, playerY, playerR, outer, inner, ILI9341_WHITE);

  // Tiny sparkle
  if ((animTick / 8) % 2 == 0) {
    tft.drawPixel(playerX + playerR + 4, playerY - 6, ILI9341_WHITE);
    tft.drawPixel(playerX + playerR + 5, playerY - 6, ILI9341_WHITE);
    tft.drawPixel(playerX + playerR + 4, playerY - 7, ILI9341_WHITE);
    tft.drawPixel(playerX + playerR + 4, playerY - 5, ILI9341_WHITE);
  }

  oldPlayerX = playerX;
  oldPlayerY = playerY;
}

void drawEnemy() {
  tft.fillRect(oldEnemyX - 3, oldEnemyY - 3, enemySize + 6, enemySize + 6, SKY_MID);

  uint16_t body = (animTick / 10) % 2 == 0 ? VIRUS_RED : VIRUS_PURPLE;

  tft.fillRoundRect(enemyX, enemyY, enemySize, enemySize, 5, body);
  tft.drawRoundRect(enemyX, enemyY, enemySize, enemySize, 5, ILI9341_WHITE);

  // Spikes/glitch pixels
  tft.drawLine(enemyX - 4, enemyY + 4, enemyX, enemyY + 7, body);
  tft.drawLine(enemyX + enemySize, enemyY + 5, enemyX + enemySize + 4, enemyY + 1, body);
  tft.drawLine(enemyX + 5, enemyY + enemySize, enemyX + 2, enemyY + enemySize + 4, body);
  tft.drawLine(enemyX + 17, enemyY - 4, enemyX + 13, enemyY, body);

  tft.fillCircle(enemyX + 7, enemyY + 8, 2, ILI9341_WHITE);
  tft.fillCircle(enemyX + 15, enemyY + 8, 2, ILI9341_WHITE);
  tft.drawFastHLine(enemyX + 7, enemyY + 16, 9, ILI9341_BLACK);

  oldEnemyX = enemyX;
  oldEnemyY = enemyY;
}

void drawCollectibleBubble() {
  if (!bubbleActive) return;

  tft.fillRect(oldBubbleX - 10, oldBubbleY - 10, 20, 20, mixColor(80, 220, 250));

  tft.drawCircle(bubbleX, bubbleY, 8, ILI9341_WHITE);
  tft.drawCircle(bubbleX, bubbleY, 6, AQUA);
  tft.drawPixel(bubbleX - 3, bubbleY - 4, ILI9341_WHITE);
  tft.drawPixel(bubbleX - 2, bubbleY - 4, ILI9341_WHITE);

  oldBubbleX = bubbleX;
  oldBubbleY = bubbleY;
}

bool rectCircleCollision(int rx, int ry, int rw, int rh, int cx, int cy, int cr) {
  int closestX = constrain(cx, rx, rx + rw);
  int closestY = constrain(cy, ry, ry + rh);

  int dx = cx - closestX;
  int dy = cy - closestY;

  return (dx * dx + dy * dy) < (cr * cr);
}

bool playerHitsEnemy() {
  return rectCircleCollision(enemyX, enemyY, enemySize, enemySize, playerX, playerY, playerR);
}

bool playerGetsBubble() {
  if (!bubbleActive) return false;

  int dx = playerX - bubbleX;
  int dy = playerY - bubbleY;
  int distSq = dx * dx + dy * dy;

  return distSq < (playerR + 8) * (playerR + 8);
}

void showGameOver() {
  gameOver = true;

  if (score > highScore) {
    highScore = score;
  }

  drawFrutigerBackground();

  tft.fillRoundRect(35, 45, 250, 145, 16, DARK_BLUE);
  tft.drawRoundRect(35, 45, 250, 145, 16, WHITE_SOFT);
  tft.drawRoundRect(40, 50, 240, 135, 13, AQUA);

  tft.setTextSize(3);
  tft.setTextColor(VIRUS_RED);
  tft.setCursor(62, 65);
  tft.println("SYSTEM");

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(72, 95);
  tft.println("ERROR");

  tft.setTextSize(2);
  tft.setTextColor(AQUA);
  tft.setCursor(82, 132);
  tft.print("Score: ");
  tft.print(score);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(78, 170);
  tft.println("Press A to restore aqua mode");
}

void handleButtons() {
  if (millis() - lastButtonTime < debounceDelay) return;

  if (buttonPressed(BTN_A)) {
    lastButtonTime = millis();
    resetGame();
    return;
  }

  if (buttonPressed(BTN_B) && !gameOver) {
    lastButtonTime = millis();
    paused = !paused;
    drawHUD();
    return;
  }

  if (buttonPressed(BTN_Y) && !gameOver) {
    lastButtonTime = millis();
    themeMode++;
    if (themeMode > 2) themeMode = 0;

    drawFrutigerBackground();
    drawHUD();
    return;
  }
}

void handleJoystick() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  int moveX = 0;
  int moveY = 0;

  if (joyX < 1400) moveX = -1;
  else if (joyX > 2600) moveX = 1;

  if (joyY < 1400) moveY = -1;
  else if (joyY > 2600) moveY = 1;

  int speed = playerSpeed;

  if (buttonPressed(BTN_X)) {
    speed = 9;
  }

  playerX += moveX * speed;
  playerY += moveY * speed;

  if (playerX < playerR) playerX = playerR;
  if (playerX > SCREEN_W - playerR) playerX = SCREEN_W - playerR;

  if (playerY < 30 + playerR) playerY = 30 + playerR;
  if (playerY > SCREEN_H - playerR) playerY = SCREEN_H - playerR;
}

void updateEnemy() {
  enemyY += enemySpeed;

  if (enemyY > SCREEN_H + enemySize) {
    enemyY = -enemySize;
    enemyX = random(20, SCREEN_W - enemySize - 20);

    score++;

    if (score % 5 == 0 && enemySpeed < 11) {
      enemySpeed++;
    }

    drawHUD();
  }
}

void updateBubble() {
  if (!bubbleActive) {
    if (random(0, 80) == 1) {
      bubbleX = random(30, SCREEN_W - 30);
      bubbleY = random(45, SCREEN_H - 45);
      oldBubbleX = bubbleX;
      oldBubbleY = bubbleY;
      bubbleActive = true;
    }
    return;
  }

  // Gentle floating motion
  if (animTick % 5 == 0) {
    bubbleY -= 1;
    if (bubbleY < 35) bubbleY = SCREEN_H - 35;
  }

  if (playerGetsBubble()) {
    score += 3;
    bubbleActive = false;

    tft.fillRect(bubbleX - 12, bubbleY - 12, 24, 24, mixColor(80, 220, 250));
    drawHUD();
  }
}

void takeDamage() {
  lives--;

  // Flash effect
  tft.fillScreen(ILI9341_WHITE);
  delay(60);
  drawFrutigerBackground();
  drawHUD();

  enemyY = -enemySize;
  enemyX = random(20, SCREEN_W - enemySize - 20);

  playerX = 150;
  playerY = 190;

  if (lives <= 0) {
    showGameOver();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_X, INPUT_PULLUP);
  pinMode(BTN_Y, INPUT_PULLUP);
  pinMode(JOY_SW, INPUT_PULLUP);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(1);

  randomSeed(analogRead(JOY_X));

  drawStartScreen();
  resetGame();
}

void loop() {
  handleButtons();

  if (gameOver) return;
  if (paused) return;

  if (millis() - lastFrame < frameDelay) return;
  lastFrame = millis();

  animTick++;

  handleJoystick();
  updateEnemy();
  updateBubble();

  drawPlayer();
  drawEnemy();
  drawCollectibleBubble();

  if (playerHitsEnemy()) {
    takeDamage();
  }
}