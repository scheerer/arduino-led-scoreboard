#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Bounce2.h>        // Button Debouncer

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

/////// Hardware setup ////////

// Button Pin Setup
#define BUTTON_PIN_1 10
#define BUTTON_PIN_2 11
#define BUTTON_PIN_3 12
#define BUTTON_PIN_4 13

// 16x32 LED Pin Setup
#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
// last param indicates 'double buffering'
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

const int BUTTON_DEBOUNCE_MS = 150;
Bounce homeAddButton = Bounce(); 
Bounce awayAddButton = Bounce(); 
Bounce homeSubButton = Bounce(); 
Bounce awaySubButton = Bounce();

// Program variables       
const int WINNING_SCORE = 21;
int homeScore = 0;
int awayScore = 0;
bool updateDisplay = true;
int hue = 0;

/****************************************
 * Program Setup
 ****************************************/
void setup() {
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

  // initialize display
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(1);    // size 1 == 8 pixels high

  Serial.begin(9600);
}

/****************************************
 * Main Program Loop
 ****************************************/
void loop() {
  updateButtons();

  int currentHomeScore = homeScore;
  int currentAwayScore = awayScore;
  
  if (scoreResetPressed()) {
    Serial.println("reset pressed"); 
    resetScores();
  } else {
    if (homeScored()) {
      Serial.println("home add"); 
      homeScore = increaseScore(homeScore);
    } else if (homeScoreCorrection()) {
      Serial.println("home sub"); 
      homeScore = decreaseScore(homeScore);
    } else if (awayScored()) {
      Serial.println("away add"); 
      awayScore = increaseScore(awayScore);
    } else if (awayScoreCorrection()) {
      Serial.println("away sub"); 
      awayScore = decreaseScore(awayScore);
    }
  }

  if (currentHomeScore != homeScore || currentAwayScore != awayScore) {
    updateDisplay = true;
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
  return buttonJustPressed(homeAddButton);
}

bool awayScored() {
  return buttonJustPressed(awayAddButton);
}

bool homeScoreCorrection() {
  return buttonJustPressed(homeSubButton);
}

bool awayScoreCorrection() {
  return buttonJustPressed(awaySubButton);
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
  // fill the screen with 'black'
  //matrix.fillScreen(0);
  
  if (gameOver()) {
    drawGameOverBorder();
  }
  displayScores();

  updateDisplay = false;
}

void drawGameOverBorder() {
  matrix.drawRect(0, 0, 32, 16, matrix.ColorHSV(hue, 255, 255, true));
  hue += 7;
  if(hue >= 1536) hue -= 1536;
}

void displayScores() {
  if (updateDisplay) {
    clearDisplay();
  }
  matrix.setCursor(1, 4);

  matrix.setTextColor(matrix.Color333(7,0,0));  
  matrix.print(displayableScore(homeScore));
  matrix.setTextColor(matrix.Color333(1,1,1));
  matrix.print('v');
  matrix.setTextColor(matrix.Color333(0,7,0));
  matrix.print(displayableScore(awayScore));
}

String displayableScore(int score) {
  String scoreString = String(score);
  if (scoreString.length() == 1) {
    scoreString = "0" + scoreString;
  }
  return scoreString;
}

void blinkScores() {
  displayScores();
  delay(500);
  clearDisplay();
  delay(500);
}

void clearDisplay() {
  matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));
}

void updateButtons()
{
  homeAddButton.update();
  awayAddButton.update();
  homeSubButton.update();
  awaySubButton.update();
}
