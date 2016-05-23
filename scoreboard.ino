#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Bounce2.h>        // Button Debouncer
#include <Arduino.h>

#include "fontBig.h"
#include "BluefruitConfig.h"

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

/////// Hardware setup ////////

// Button Pin Setup
#define BUTTON_PIN_1 36
#define BUTTON_PIN_2 37
#define BUTTON_PIN_3 38
#define BUTTON_PIN_4 39

#define REMOTE_BUTTON_A 7
#define REMOTE_BUTTON_B 6
#define REMOTE_BUTTON_C 5
#define REMOTE_BUTTON_D 4
#define POWERPIN   3
#define GROUNDPIN  2

// 16x32 LED Pin Setup
#define CLK 50  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A4
#define OE  51
#define A   A0
#define B   A1
#define C   A2
// last param indicates 'double buffering'
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

const int BUTTON_DEBOUNCE_MS = 50;
Bounce homeAddButton = Bounce(); 
Bounce awayAddButton = Bounce(); 
Bounce homeSubButton = Bounce(); 
Bounce awaySubButton = Bounce();

Bounce remoteHomeAddButton = Bounce(); 
Bounce remoteAwayAddButton = Bounce(); 
Bounce remoteHomeSubButton = Bounce(); 
Bounce remoteAwaySubButton = Bounce();

// Program variables       
const int WINNING_SCORE = 21;
int homeScore = 0;
int awayScore = 0;
int hue = 0;


byte combination[] = "1332";
byte userInput[4];


unsigned long currentTime;

/****************************************
 * Program Setup
 ****************************************/
void setup() {
  while (!Serial);
  delay(500);
  Serial.begin(19200);
  
  pinMode(BUTTON_PIN_1,INPUT_PULLUP);
  homeAddButton.attach(BUTTON_PIN_1);
  homeAddButton.interval(BUTTON_DEBOUNCE_MS);

  pinMode(BUTTON_PIN_2,INPUT_PULLUP);
  awayAddButton.attach(BUTTON_PIN_2);
  awayAddButton.interval(BUTTON_DEBOUNCE_MS);

  pinMode(BUTTON_PIN_3,INPUT_PULLUP);
  homeSubButton.attach(BUTTON_PIN_3);
  homeSubButton.interval(BUTTON_DEBOUNCE_MS);

  pinMode(BUTTON_PIN_4,INPUT_PULLUP);
  awaySubButton.attach(BUTTON_PIN_4);
  awaySubButton.interval(BUTTON_DEBOUNCE_MS);


  // Set the ground pin to a Low output
  pinMode(GROUNDPIN, OUTPUT);
  digitalWrite(GROUNDPIN, LOW);
   
  // Set the +5v pin to a High output
  pinMode(POWERPIN,OUTPUT);
  digitalWrite(POWERPIN, HIGH);

  pinMode(REMOTE_BUTTON_A, INPUT);
  remoteHomeAddButton.attach(REMOTE_BUTTON_A);
  remoteHomeAddButton.interval(0);
  pinMode(REMOTE_BUTTON_B, INPUT);
  remoteAwayAddButton.attach(REMOTE_BUTTON_B);
  remoteAwayAddButton.interval(0);
  pinMode(REMOTE_BUTTON_C, INPUT);
  remoteHomeSubButton.attach(REMOTE_BUTTON_C);
  remoteHomeSubButton.interval(0);
  pinMode(REMOTE_BUTTON_D, INPUT);
  remoteAwaySubButton.attach(REMOTE_BUTTON_D);
  remoteAwaySubButton.interval(0);
  
  // initialize display
  matrix.begin();

  displayText("Shall We Play A Game?");
}

/****************************************
 * Main Program Loop
 ****************************************/
void loop() {
  currentTime = millis();

  updateButtons();

  int currentHomeScore = homeScore;
  int currentAwayScore = awayScore;
  
  if (scoreResetPressed()) {
    resetScores();
  } else {
    if (homeScored()) {
      homeScore = increaseScore(homeScore);
      Serial.println("Home scored: " + displayableScore(homeScore) + " v " + displayableScore(awayScore));
    } else if (homeScoreCorrection()) {
      homeScore = decreaseScore(homeScore);
      Serial.println("Home sub: " +  displayableScore(homeScore) + " v " + displayableScore(awayScore));
    } else if (awayScored()) {
      awayScore = increaseScore(awayScore);
      Serial.println("Away scored: " + displayableScore(homeScore) + " v " + displayableScore(awayScore));
    } else if (awayScoreCorrection()) {
      awayScore = decreaseScore(awayScore);
      Serial.println("Away sub: " + displayableScore(homeScore) + " v " + displayableScore(awayScore));
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

/****************************************
 * BUTTONS
 ****************************************/
bool homeScored() {
  return buttonJustPressed(homeAddButton) || remoteHomeAddButton.rose();
}

bool awayScored() {
  return buttonJustPressed(awayAddButton) || remoteAwayAddButton.rose();
}

bool homeScoreCorrection() {
  return buttonJustPressed(homeSubButton) || remoteHomeSubButton.rose();
}

bool awayScoreCorrection() {
  return buttonJustPressed(awaySubButton) || remoteAwaySubButton.rose();
}

bool scoreResetPressed() {
  return buttonisHeld(homeSubButton) && buttonisHeld(awaySubButton);
}

bool buttonJustPressed(Bounce button) {
  return button.fell();
}

bool buttonisHeld(Bounce button) {
  return button.read() == LOW;
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
  putDigitLarge(0, 2, '0' + homeScore / 10, 0, 0, 7);
  putDigitLarge(6, 2, '0' + homeScore % 10, 0, 0, 7);

  matrix.setCursor(13, 4);
  matrix.setTextColor(matrix.Color333(1,1,1));
  matrix.print('v');

  putDigitLarge(18, 2, '0' + awayScore / 10, 0, 7, 0);
  putDigitLarge(24, 2, '0' + awayScore % 10, 0, 7, 0);
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

  remoteHomeAddButton.update();
  remoteAwayAddButton.update();
  remoteHomeSubButton.update();
  remoteAwaySubButton.update();
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
