#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

// Hardware setup
#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

// Button input pins
const int homeAddButton = 2;
const int homeSubButton = 3;
const int awayAddButton = 4;
const int awaySubButton = 5;

// Program variables
const char str[] PROGMEM = "Cornhole-io";
int    textX   = matrix.width(),
       textMin = sizeof(str) * -12,
       textHue = 0;
       
const int WINNING_SCORE = 21;
int homeScore = 0;
int awayScore = 0;

/****************************************
 * Program Setup
 ****************************************/
void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(homeAddButton, INPUT);
  pinMode(homeSubButton, INPUT);
  pinMode(awayAddButton, INPUT);
  pinMode(awaySubButton, INPUT);

  // initialize display
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(1);    // size 1 == 8 pixels high
  
  displayScoreBoardScreen();
  // Update display
  matrix.swapBuffers(false);
}

/****************************************
 * Main Program Loop
 ****************************************/
void loop() {
  if (scoreResetPressed()) {
    resetScores();
  } else {
    if (homeScored()) {
      homeScore = increaseScore(homeScore);
    } else if (homeScoreCorrection()) {
      homeScore = decreaseScore(homeScore);
    } else if (awayScored()) {
      awayScore = increaseScore(awayScore);
    } else if (awayScoreCorrection()) {
      awayScore = decreaseScore(awayScore);
    }
  }
  
  displayScoreBoardScreen();
  
  // Update display
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
  return buttonPressed(homeAddButton);
}

bool awayScored() {
  return buttonPressed(awayAddButton);
}

bool homeScoreCorrection() {
  return buttonPressed(homeSubButton);
}

bool awayScoreCorrection() {
  return buttonPressed(awaySubButton);
}

bool scoreResetPressed() {
  return homeScoreCorrection() && awayScoreCorrection();
}

bool buttonPressed(int buttonPin) {
  return digitalRead(buttonPin) == HIGH;
}

/****************************************
 * Display Related
 ****************************************/
void displayScoreBoardScreen() {
    // fill the screen with 'black'
  matrix.fillScreen(0);
  
  displayScrollingText();
  
  if (gameOver()) {
    blinkScores();
  } else {
    displayScores();
  }
}

void displayScores() {
  matrix.setCursor(1, 9);   // bottom line
  matrix.setTextColor(matrix.Color333(0,7,7));
  // TODO: add 0 prefix to scores if less than 10
  String homeScoreString = String(homeScore);
  
  matrix.print(displayableScore(homeScore));
  matrix.print('v');
  matrix.print(displayableScore(awayScore));
}

String displayableScore(int score) {
  String scoreString = String(homeScore);
  if (scoreString.length() == 1) {
    scoreString = "0" + scoreString;
  }
  return scoreString;
}

void blinkScores() {
  displayScores();
  delay(500);
  clearScoresDisplay();
  delay(500);
}

void clearScoresDisplay() {
  matrix.fillRect(0, 8, 32, 16, matrix.Color333(0, 0, 0));
}

// Scrolls text from right to left with changing colors
void displayScrollingText() {
  // Draw big scrolly text on top
  matrix.setTextColor(matrix.ColorHSV(textHue, 255, 255, true));
  matrix.setCursor(textX, 0); // top line
  matrix.print(F2(str));

  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) textX = matrix.width();
  textHue += 7;
  if(textHue >= 1536) textHue -= 1536;
}

