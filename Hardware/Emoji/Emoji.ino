#include <SD.h>
#include <SPI.h>
#include <LCDWIKI_GUI.h> // Core graphics library
#include <LCDWIKI_KBV.h> // Hardware-specific library

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

LCDWIKI_KBV my_lcd(240, 320, LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BLACK   0x0000

#define FILE_NUMBER 3
#define FILE_NAME_SIZE_MAX 20
#define PIXEL_NUMBER 60  // 240 (width) / 4

char file_name[FILE_NUMBER][FILE_NAME_SIZE_MAX] = {
  "2.bmp", "3.bmp", "4.bmp"
};

uint32_t bmp_offset = 0;
uint16_t s_width;
uint16_t s_heigh;

// Read 16-bit integer from file
uint16_t read_16(File fp) {
  uint8_t low = fp.read();
  uint8_t high = fp.read();
  return (high << 8) | low;
}

// Read 32-bit integer from file
uint32_t read_32(File fp) {
  uint16_t low = read_16(fp);
  uint16_t high = read_16(fp);
  return (high << 16) | low;
}

// Check BMP header
bool analysis_bmp_header(File fp) {
  if (read_16(fp) != 0x4D42) return false;
  read_32(fp); // file size
  read_32(fp); // reserved
  bmp_offset = read_32(fp); // pixel data offset
  read_32(fp); // DIB header size
  uint32_t bmp_width = read_32(fp);
  uint32_t bmp_height = read_32(fp);
  if ((bmp_width != s_width) || (bmp_height != s_heigh)) return false;
  if (read_16(fp) != 1) return false;
  read_16(fp); // bits per pixel
  if (read_32(fp) != 0) return false; // compression
  return true;
}

// Draw the BMP image on screen
void draw_bmp_picture(File fp) {
  uint8_t bmp_data[PIXEL_NUMBER * 3] = {0};
  uint16_t bmp_color[PIXEL_NUMBER];

  fp.seek(bmp_offset);
  for (uint16_t i = 0; i < s_heigh; i++) {
    for (uint16_t j = 0; j < s_width / PIXEL_NUMBER; j++) {
      fp.read(bmp_data, PIXEL_NUMBER * 3);
      for (uint16_t k = 0, m = 0; k < PIXEL_NUMBER; k++, m += 3) {
        bmp_color[k] = my_lcd.Color_To_565(bmp_data[m + 2], bmp_data[m + 1], bmp_data[m + 0]);
      }
      for (uint16_t l = 0; l < PIXEL_NUMBER; l++) {
        my_lcd.Set_Draw_color(bmp_color[l]);
        my_lcd.Draw_Pixel(j * PIXEL_NUMBER + l, i);
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(3);  // landscape
  s_width = my_lcd.Get_Display_Width();
  s_heigh = my_lcd.Get_Display_Height();
  my_lcd.Fill_Screen(BLACK);

  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    my_lcd.Set_Text_Back_colour(BLACK);
    my_lcd.Set_Text_colour(0xFFFF);
    my_lcd.Set_Text_Size(2);
    my_lcd.Print_String("SD Card Init Fail!", 10, 10);
    while (1);
  }
}

void loop() {
  for (int i = 0; i < FILE_NUMBER; i++) {
    File bmp_file = SD.open(file_name[i]);
    if (!bmp_file) {
      Serial.print("Could not find file: ");
      Serial.println(file_name[i]);
      continue;
    }

    if (!analysis_bmp_header(bmp_file)) {
      Serial.print("Bad BMP header in: ");
      Serial.println(file_name[i]);
      bmp_file.close();
      continue;
    }

    draw_bmp_picture(bmp_file);
    bmp_file.close();
    delay(3000); // Display each image for 3 seconds
  }
}
