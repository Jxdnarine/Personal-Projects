/*
  ESP32 + 2.8" SPI TFT + 4 Buttons + Joystick RGB Game
  Game: RGB Dodge

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

// ---------------- BUTTON PINS ----------------
#define BTN_A 25
#define BTN_B 26
#define BTN_X 27
#define BTN_Y 14
#define JOY_SW 32

// ---------------- JOYSTICK PINS ----------------
#define JOY_X 34
#define JOY_Y 35

// ---------------- SCREEN SETTINGS ----------------
#define SCREEN_W 320
#define SCREEN_H 240

// ---------------- PLAYER SETTINGS ----------------
int playerX = 150;
int playerY = 200;
int oldPlayerX = 150;
int oldPlayerY = 200;

int playerSize = 16;
int playerSpeed = 4;
int colorMode = 0;

// ---------------- ENEMY SETTINGS ----------------
int enemyX = 40;
int enemyY = -20;
int oldEnemyX = 40;
int oldEnemyY = -20;

int enemyW = 28;
int enemyH = 18;
int enemySpeed = 3;
uint16_t enemyColor = ILI9341_RED;

// ---------------- GAME STATE ----------------
int score = 0;
int highScore = 0;

bool gameOver = false;
bool paused = false;

unsigned long lastFrame = 0;
const int frameDelay = 30;

unsigned long lastButtonTime = 0;
const int debounceDelay = 180;

int bgShift = 0;
int rgbTick = 0;

// ---------------- COLOR HELPERS ----------------

uint16_t rgb565(byte r, byte g, byte b) {
  return tft.color565(r, g, b);
}

uint16_t rainbowColor(int index) {
  index = index % 6;

  if (index == 0) return ILI9341_RED;
  if (index == 1) return ILI9341_ORANGE;
  if (index == 2) return ILI9341_YELLOW;
  if (index == 3) return ILI9341_GREEN;
  if (index == 4) return ILI9341_CYAN;
  return ILI9341_MAGENTA;
}

uint16_t smoothRGB(int step) {
  step = step % 1536;

  byte r = 0;
  byte g = 0;
  byte b = 0;

  if (step < 256) {
    r = 255;
    g = step;
    b = 0;
  } else if (step < 512) {
    r = 511 - step;
    g = 255;
    b = 0;
  } else if (step < 768) {
    r = 0;
    g = 255;
    b = step - 512;
  } else if (step < 1024) {
    r = 0;
    g = 1023 - step;
    b = 255;
  } else if (step < 1280) {
    r = step - 1024;
    g = 0;
    b = 255;
  } else {
    r = 255;
    g = 0;
    b = 1535 - step;
  }

  return rgb565(r, g, b);
}

// ---------------- GAME FUNCTIONS ----------------

bool buttonPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void drawRGBBackground() {
  tft.fillScreen(ILI9341_BLACK);

  for (int x = 0; x < SCREEN_W; x += 20) {
    uint16_t c = rainbowColor((x / 20) % 6);
    tft.drawFastVLine(x, 24, SCREEN_H - 24, c);
  }

  for (int y = 24; y < SCREEN_H; y += 20) {
    uint16_t c = rainbowColor((y / 20) % 6);
    tft.drawFastHLine(0, y, SCREEN_W, c);
  }

  for (int i = 0; i < 25; i++) {
    int sx = random(0, SCREEN_W);
    int sy = random(28, SCREEN_H);
    uint16_t sc = rainbowColor(random(0, 6));
    tft.drawPixel(sx, sy, sc);
  }
}

void drawStartScreen() {
  tft.fillScreen(ILI9341_BLACK);

  for (int y = 0; y < SCREEN_H; y += 8) {
    tft.drawFastHLine(0, y, SCREEN_W, smoothRGB(y * 8));
  }

  tft.fillRect(20, 25, 280, 170, ILI9341_BLACK);
  tft.drawRect(20, 25, 280, 170, ILI9341_WHITE);

  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.setCursor(65, 40);
  tft.print("R");

  tft.setTextColor(ILI9341_GREEN);
  tft.print("G");

  tft.setTextColor(ILI9341_BLUE);
  tft.println("B");

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(95, 75);
  tft.println("DODGE");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setCursor(55, 125);
  tft.println("Joystick: Move");
  tft.setCursor(55, 140);
  tft.println("A: Restart   B: Pause");
  tft.setCursor(55, 155);
  tft.println("X: Dash      Y: RGB Mode");

  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(65, 215);
  tft.println("Get ready...");

  delay(1500);
}

void drawHUD() {
  tft.fillRect(0, 0, SCREEN_W, 24, ILI9341_BLACK);

  tft.drawFastHLine(0, 23, SCREEN_W, smoothRGB(rgbTick * 8));

  tft.setTextSize(1);

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(5, 7);
  tft.print("Score: ");

  tft.setTextColor(ILI9341_WHITE);
  tft.print(score);

  tft.setTextColor(ILI9341_MAGENTA);
  tft.setCursor(105, 7);
  tft.print("High: ");

  tft.setTextColor(ILI9341_WHITE);
  tft.print(highScore);

  tft.setCursor(220, 7);
  if (paused) {
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("PAUSED");
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print("RUNNING");
  }
}

void resetGame() {
  playerX = 150;
  playerY = 200;
  oldPlayerX = playerX;
  oldPlayerY = playerY;

  enemyX = random(10, SCREEN_W - enemyW - 10);
  enemyY = -20;
  oldEnemyX = enemyX;
  oldEnemyY = enemyY;

  enemySpeed = 3;
  score = 0;
  gameOver = false;
  paused = false;

  enemyColor = rainbowColor(random(0, 6));

  drawRGBBackground();
  drawHUD();
}

void drawPlayer() {
  tft.fillRect(oldPlayerX - 2, oldPlayerY - 2, playerSize + 4, playerSize + 4, ILI9341_BLACK);

  uint16_t playerColor;

  if (colorMode == 0) {
    playerColor = smoothRGB(rgbTick * 16);
  } else if (colorMode == 1) {
    playerColor = rainbowColor(score + rgbTick / 10);
  } else {
    playerColor = ILI9341_WHITE;
  }

  tft.fillRect(playerX, playerY, playerSize, playerSize, playerColor);
  tft.drawRect(playerX, playerY, playerSize, playerSize, smoothRGB(rgbTick * 20));

  tft.drawPixel(playerX + 4, playerY + 4, ILI9341_WHITE);
  tft.drawPixel(playerX + 11, playerY + 4, ILI9341_WHITE);

  oldPlayerX = playerX;
  oldPlayerY = playerY;
}

void drawEnemy() {
  tft.fillRect(oldEnemyX - 1, oldEnemyY - 1, enemyW + 2, enemyH + 2, ILI9341_BLACK);

  tft.fillRect(enemyX, enemyY, enemyW, enemyH, enemyColor);
  tft.drawRect(enemyX, enemyY, enemyW, enemyH, ILI9341_WHITE);

  tft.drawFastHLine(enemyX + 3, enemyY + 4, enemyW - 6, smoothRGB(rgbTick * 12));
  tft.drawFastHLine(enemyX + 3, enemyY + enemyH - 5, enemyW - 6, smoothRGB(rgbTick * 18));

  oldEnemyX = enemyX;
  oldEnemyY = enemyY;
}

bool checkCollision() {
  return (
    playerX < enemyX + enemyW &&
    playerX + playerSize > enemyX &&
    playerY < enemyY + enemyH &&
    playerY + playerSize > enemyY
  );
}

void showGameOver() {
  gameOver = true;

  if (score > highScore) {
    highScore = score;
  }

  tft.fillScreen(ILI9341_BLACK);

  for (int i = 0; i < 80; i++) {
    int x = random(0, SCREEN_W);
    int y = random(0, SCREEN_H);
    tft.drawPixel(x, y, rainbowColor(random(0, 6)));
  }

  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.setCursor(55, 50);
  tft.println("GAME OVER");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(90, 105);
  tft.print("Score: ");
  tft.print(score);

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(90, 135);
  tft.print("High: ");
  tft.print(highScore);

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(70, 190);
  tft.println("Press Button A to restart");
}

void handleButtons() {
  if (millis() - lastButtonTime < debounceDelay) {
    return;
  }

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
    colorMode++;

    if (colorMode > 2) {
      colorMode = 0;
    }

    drawHUD();
    return;
  }
}

void handleJoystick() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  int moveX = 0;
  int moveY = 0;

  if (joyX < 1400) {
    moveX = -1;
  } else if (joyX > 2600) {
    moveX = 1;
  }

  if (joyY < 1400) {
    moveY = -1;
  } else if (joyY > 2600) {
    moveY = 1;
  }

  int speed = playerSpeed;

  if (buttonPressed(BTN_X)) {
    speed = 9;
  }

  playerX += moveX * speed;
  playerY += moveY * speed;

  if (playerX < 0) {
    playerX = 0;
  }

  if (playerX > SCREEN_W - playerSize) {
    playerX = SCREEN_W - playerSize;
  }

  if (playerY < 24) {
    playerY = 24;
  }

  if (playerY > SCREEN_H - playerSize) {
    playerY = SCREEN_H - playerSize;
  }
}

void updateEnemy() {
  enemyY += enemySpeed;

  if (enemyY > SCREEN_H) {
    enemyY = -enemyH;
    enemyX = random(10, SCREEN_W - enemyW - 10);
    enemyColor = rainbowColor(random(0, 6));

    score++;

    if (score % 5 == 0 && enemySpeed < 10) {
      enemySpeed++;
    }

    drawHUD();
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

  if (gameOver) {
    return;
  }

  if (paused) {
    return;
  }

  if (millis() - lastFrame < frameDelay) {
    return;
  }

  lastFrame = millis();
  rgbTick++;

  handleJoystick();
  updateEnemy();

  drawPlayer();
  drawEnemy();

  if (checkCollision()) {
    showGameOver();
  }
}