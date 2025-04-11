#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>

// TFT wiring for UNO 8-bit parallel
#define LCD_CS   A3
#define LCD_CD   A2
#define LCD_WR   A1
#define LCD_RD   A0
#define LCD_RESET A4

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// SD chip select
#define SD_CS 10

File bmpFile;
char filename[12];

void setup() {
  Serial.begin(9600);
  tft.begin(0x9341);  // Update this if your driver is different
  tft.fillScreen(0x0000);
  tft.setCursor(20, 100);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.println("Send number (1-10)");

  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed!");
    tft.setCursor(20, 130);
    tft.println("SD Init Failed!");
    while (1);
  }
  Serial.println("SD Initialized.");
}

void loop() {
  if (Serial.available()) {
    int input = Serial.parseInt();
    if (input >= 1 && input <= 10) {
      sprintf(filename, "%d.bmp", input);
      Serial.print("Loading ");
      Serial.println(filename);
      drawBMP(filename);
    } else {
      Serial.println("Enter a number 1 to 10");
    }
  }
}

void drawBMP(const char *filename) {
  bmpFile = SD.open(filename);
  if (!bmpFile) {
    Serial.println("File not found!");
    return;
  }

  // BMP header
  if (read16(bmpFile) != 0x4D42) return;

  (void)read32(bmpFile); // file size
  (void)read32(bmpFile); // creator bytes
  uint32_t imageOffset = read32(bmpFile);
  (void)read32(bmpFile); // header size
  int32_t bmpWidth = read32(bmpFile);
  int32_t bmpHeight = read32(bmpFile);
  if (read16(bmpFile) != 1) return;
  uint16_t bmpDepth = read16(bmpFile);
  if (bmpDepth != 24) return;

  (void)read32(bmpFile); // compression

  // Centering
int x_offset = (320 - bmpWidth) / 2;
int y_offset = (240 - bmpHeight) / 2;

bmpFile.seek(imageOffset);

uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;
uint8_t sdbuffer[rowSize];

for (int y = bmpHeight - 1; y >= 0; y--) {
  bmpFile.read(sdbuffer, rowSize);
  for (int x = 0; x < bmpWidth; x++) {
    uint8_t b = sdbuffer[x * 3];
    uint8_t g = sdbuffer[x * 3 + 1];
    uint8_t r = sdbuffer[x * 3 + 2];
    uint16_t color = tft.color565(r, g, b);
    tft.drawPixel(x + x_offset, y + y_offset, color);
  }
}


  bmpFile.close();
  Serial.println("Done!");
}

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); 
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read();
  return result;
}
