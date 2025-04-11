#include <Adafruit_GFX.h>    // Core graphics lib
#include <Adafruit_TFTLCD.h> // Hardware-specific lib

// Pin definitions for 8-bit parallel TFT
#define LCD_CS   A3
#define LCD_CD   A2
#define LCD_WR   A1
#define LCD_RD   A0
#define LCD_RESET A4

// Color definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Initialize TFT
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

void setup() {
  Serial.begin(9600);
  tft.begin(0x9341); // Change if your LCD uses a different driver
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Press H / N / S");

  drawNeutral(); // Default face
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();
    if (key == 'h' || key == 'H') {
      drawHappy();
    } else if (key == 'n' || key == 'N') {
      drawNeutral();
    } else if (key == 's' || key == 'S') {
      drawSad();
    }
  }
}

// ---------- FACE DRAWINGS ----------

void drawFaceBase(uint16_t color) {
  tft.fillScreen(BLACK);
  // Head
  tft.fillCircle(160, 120, 80, color);
  // Eyes
  tft.fillCircle(130, 100, 8, BLACK);
  tft.fillCircle(190, 100, 8, BLACK);
}

void drawHappy() {
  drawFaceBase(YELLOW);
  drawArc(160, 130, 30, 30, 150, RED); // Smile
}

void drawNeutral() {
  drawFaceBase(YELLOW);
  tft.drawLine(130, 150, 190, 150, RED); // Flat mouth
}

void drawSad() {
  drawFaceBase(YELLOW);
  drawArc(160, 170, 30, 210, 330, RED); // Frown
}

// ---------- SIMPLE ARC DRAWING ----------

void drawArc(int x, int y, int r, int startAngle, int endAngle, uint16_t color) {
  for (int angle = startAngle; angle <= endAngle; angle++) {
    float rad = angle * 0.0174533; // Convert to radians
    int xx = x + cos(rad) * r;
    int yy = y + sin(rad) * r;
    tft.drawPixel(xx, yy, color);
  }
}
