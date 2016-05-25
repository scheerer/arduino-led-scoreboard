#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Arduino.h>

#include "Button.h"
#include "fontBig.h"
#include "BluefruitConfig.h"

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

/////// Hardware setup ////////

// Physical Button Pin Setup
#define BUTTON_PIN_1 36
#define BUTTON_PIN_2 37
#define BUTTON_PIN_3 38
#define BUTTON_PIN_4 39

// RF Button Pin Setup
#define RF_BUTTON_A  7
#define RF_BUTTON_B  6
#define RF_BUTTON_C  5
#define RF_BUTTON_D  4
#define RF_POWERPIN  3
#define RF_GROUNDPIN 2

// 16x32 LED Pin Setup
#define CLK 50  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A4
#define OE  51
#define A   A0
#define B   A1
#define C   A2

//////// Program variables ////////
// last param indicates 'double buffering'
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

Button homeAddButton = Button(BUTTON_PIN_1, RF_BUTTON_A); 
Button awayAddButton = Button(BUTTON_PIN_2, RF_BUTTON_B); 
Button homeSubButton = Button(BUTTON_PIN_3, RF_BUTTON_C); 
Button awaySubButton = Button(BUTTON_PIN_4, RF_BUTTON_D);
       
const int WINNING_SCORE = 21;
int homeScore = 0;
int awayScore = 0;
int hue = 0;

/****************************************
 * Program Setup
 ****************************************/
void setup() {
  while (!Serial);
  delay(500);
  Serial.begin(19200);
  
  // Set the ground pin to a Low output
  pinMode(RF_GROUNDPIN, OUTPUT);
  digitalWrite(RF_GROUNDPIN, LOW);
   
  // Set the +5v pin to a High output
  pinMode(RF_POWERPIN, OUTPUT);
  digitalWrite(RF_POWERPIN, HIGH);

  // initialize buttons
  homeAddButton.init();
  awayAddButton.init();
  homeSubButton.init();
  awaySubButton.init();
  
  // initialize display
  matrix.begin();
  displayText("#Cornholio");
  serialPrintScores();
}

/****************************************
 * Main Program Loop
 ****************************************/
void loop() {
  int currentHomeScore = homeScore;
  int currentAwayScore = awayScore;
  
  updateButtons();

  if (scoreResetPressed()) {
    Serial.println("Reset scores");
    resetScores();
  } else {
    if (homeAddButton.justPressed()) {
      homeScore = increaseScore(homeScore);
    } else if (homeSubButton.justPressed()) {
      homeScore = decreaseScore(homeScore);
    } else if (awayAddButton.justPressed()) {
      awayScore = increaseScore(awayScore);
    } else if (awaySubButton.justPressed()) {
      awayScore = decreaseScore(awayScore);
    }
  }

  if (currentHomeScore != homeScore || currentAwayScore != awayScore) {
    serialPrintScores();
    if (gameOver()) {
      Serial.println("Winner, winner, chicken dinner!");
    }
  }
  
  displayScoreBoardScreen();
  matrix.swapBuffers(false);
}

/****************************************
 * Scoring
 ****************************************/
int increaseScore(int score) {
  if (score == 21) {
    return 13;
  }
  return score + 1;
}

int decreaseScore(int score) {
  if (score == 0) {
    return 0;
  }
  return score - 1;
}

int resetScores() {
  homeScore = 0;
  awayScore = 0;
}

bool gameOver() {
  return homeScore == WINNING_SCORE || awayScore == WINNING_SCORE;
}

bool scoreResetPressed() {
  return homeSubButton.isHeld() && awaySubButton.isHeld();
}

void serialPrintScores() {
  Serial.println("Current scores: " + displayableScore(homeScore) + " v " + displayableScore(awayScore));
}

/****************************************
 * Display Related
 ****************************************/
void displayScoreBoardScreen() {
  matrix.setTextSize(1);
  // fill the screen with 'black'
  clearDisplay();
  
  if (gameOver()) {
    drawGameOverBorder();
  }
  displayScores();
}

void drawGameOverBorder() {
  matrix.drawRect(0, 0, 32, 16, matrix.ColorHSV(hue, 255, 255, true));
  hue += 7;
  if(hue >= 1536) hue -= 1536;
}

void displayScores() {
  putDigitLarge(1, 2, '0' + homeScore / 10, 7, 0, 0);
  putDigitLarge(7, 2, '0' + homeScore % 10, 7, 0, 0);

  matrix.setCursor(14, 4);
  matrix.setTextColor(matrix.Color333(1,1,1));
  matrix.print('v');

  putDigitLarge(19, 2, '0' + awayScore / 10, 0, 7, 0);
  putDigitLarge(25, 2, '0' + awayScore % 10, 0, 7, 0);
}

String displayableScore(int score) {
  String scoreString = String(score);
  if (scoreString.length() == 1) {
    scoreString = "0" + scoreString;
  }
  return scoreString;
}

void displayText(String text) {
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);    // size 1 == 8 pixels high

  int textX   = matrix.width();
  int textMin = text.length() * -12;

  while ((--textX) >= textMin) {
    //Serial.println("TextX: " + textX);
    matrix.fillScreen(0);
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    matrix.setCursor(textX, 1);
    matrix.print(text);
    
    hue += 7;
    if(hue >= 1536) hue -= 1536;
    matrix.swapBuffers(true);
    delay(10);
  }
}

void blinkScores() {
  displayScores();
  delay(500);
  clearDisplay();
  delay(500);
}

void clearDisplay() {
  matrix.fillScreen(0);
}

void updateButtons()
{
  homeAddButton.update();
  awayAddButton.update();
  homeSubButton.update();
  awaySubButton.update();
}

void putDigitLarge(uint8_t x, uint8_t y, char c, uint8_t r, uint8_t g, uint8_t b)
{
  // fonts defined for ascii 32 and beyond (index 0 in font array is ascii 32);
  byte charIndex = c - '0' + 22;
  putChar(x, y, bigFont[charIndex], 12, 6, r, g, b);
}

void putChar(uint8_t x, uint8_t y, const unsigned char * c, uint8_t h, uint8_t w, uint8_t r, uint8_t g, uint8_t b) {
  for (byte row = 0; row<h; row++)
  {
    byte rowDots = pgm_read_byte_near(&c[row]);
    for (byte col = 0; col<w; col++)
    {
      if (rowDots & (1 << (w - col - 1)))
        matrix.drawPixel(x + col, y + row, matrix.Color333(r,g,b));
      else
        matrix.drawPixel(x+col, y+row, matrix.Color333(0,0,0));
    }
  }
}
