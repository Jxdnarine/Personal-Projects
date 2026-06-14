/*
  ESP32 + 2.8" SPI TFT + 4 Buttons + Joystick Game
  Game: Dodge the Falling Blocks

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
// Landscape mode: 320 wide x 240 tall
#define SCREEN_W 320
#define SCREEN_H 240

// ---------------- PLAYER SETTINGS ----------------
int playerX = 150;
int playerY = 200;
int oldPlayerX = 150;
int oldPlayerY = 200;

int playerSize = 16;
int playerSpeed = 4;

uint16_t playerColors[] = {
  ILI9341_CYAN,
  ILI9341_GREEN,
  ILI9341_YELLOW,
  ILI9341_MAGENTA,
  ILI9341_ORANGE
};

int playerColorIndex = 0;

// ---------------- ENEMY SETTINGS ----------------
int enemyX = 40;
int enemyY = -20;
int oldEnemyX = 40;
int oldEnemyY = -20;

int enemyW = 28;
int enemyH = 18;
int enemySpeed = 3;

// ---------------- GAME STATE ----------------
int score = 0;
int highScore = 0;

bool gameOver = false;
bool paused = false;

unsigned long lastFrame = 0;
const int frameDelay = 30;

// Debounce timing
unsigned long lastButtonTime = 0;
const int debounceDelay = 180;

// ---------------- FUNCTIONS ----------------

bool buttonPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void drawStartScreen() {
  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.setCursor(35, 35);
  tft.println("ESP32 GAME");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(35, 85);
  tft.println("Dodge the Blocks");

  tft.setTextSize(1);
  tft.setCursor(45, 130);
  tft.println("Joystick: Move");
  tft.setCursor(45, 145);
  tft.println("A: Restart");
  tft.setCursor(45, 160);
  tft.println("B: Pause");
  tft.setCursor(45, 175);
  tft.println("X: Dash");
  tft.setCursor(45, 190);
  tft.println("Y: Change Color");

  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(65, 215);
  tft.println("Move joystick to start");

  delay(1500);
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

  tft.fillScreen(ILI9341_BLACK);
  drawHUD();
}

void drawHUD() {
  tft.fillRect(0, 0, SCREEN_W, 22, ILI9341_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(5, 7);
  tft.print("Score: ");
  tft.print(score);

  tft.setCursor(110, 7);
  tft.print("High: ");
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

void drawPlayer() {
  tft.fillRect(oldPlayerX, oldPlayerY, playerSize, playerSize, ILI9341_BLACK);

  uint16_t color = playerColors[playerColorIndex];

  tft.fillRect(playerX, playerY, playerSize, playerSize, color);
  tft.drawRect(playerX, playerY, playerSize, playerSize, ILI9341_WHITE);

  oldPlayerX = playerX;
  oldPlayerY = playerY;
}

void drawEnemy() {
  tft.fillRect(oldEnemyX, oldEnemyY, enemyW, enemyH, ILI9341_BLACK);

  tft.fillRect(enemyX, enemyY, enemyW, enemyH, ILI9341_RED);
  tft.drawRect(enemyX, enemyY, enemyW, enemyH, ILI9341_WHITE);

  oldEnemyX = enemyX;
  oldEnemyY = enemyY;
}

bool checkCollision() {
  if (playerX < enemyX + enemyW &&
      playerX + playerSize > enemyX &&
      playerY < enemyY + enemyH &&
      playerY + playerSize > enemyY) {
    return true;
  }
  return false;
}

void showGameOver() {
  gameOver = true;

  if (score > highScore) {
    highScore = score;
  }

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.setCursor(60, 55);
  tft.println("GAME OVER");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(90, 105);
  tft.print("Score: ");
  tft.print(score);

  tft.setCursor(75, 135);
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

  // Button A: restart
  if (buttonPressed(BTN_A)) {
    lastButtonTime = millis();
    resetGame();
    return;
  }

  // Button B: pause/unpause
  if (buttonPressed(BTN_B) && !gameOver) {
    lastButtonTime = millis();
    paused = !paused;
    drawHUD();
    return;
  }

  // Button Y: change player color
  if (buttonPressed(BTN_Y) && !gameOver) {
    lastButtonTime = millis();
    playerColorIndex++;

    if (playerColorIndex >= 5) {
      playerColorIndex = 0;
    }

    drawPlayer();
    return;
  }
}

void handleJoystick() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  int moveX = 0;
  int moveY = 0;

  // Deadzone range.
  // Center is usually around 1800-2200.
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

  // Button X: dash/speed boost
  if (buttonPressed(BTN_X)) {
    speed = 9;
  }

  playerX += moveX * speed;
  playerY += moveY * speed;

  // Keep player inside screen.
  // Top limit is 24 because HUD is at top.
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

    score++;

    // Make game harder every 5 points
    if (score % 5 == 0 && enemySpeed < 10) {
      enemySpeed++;
    }

    drawHUD();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Buttons
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_X, INPUT_PULLUP);
  pinMode(BTN_Y, INPUT_PULLUP);
  pinMode(JOY_SW, INPUT_PULLUP);

  // Start SPI
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  // Start TFT
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

  handleJoystick();
  updateEnemy();

  drawPlayer();
  drawEnemy();

  if (checkCollision()) {
    showGameOver();
  }
}