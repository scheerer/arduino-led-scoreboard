#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Bounce2.h>        // Button Debouncer

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

/////// Hardware setup ////////


// Button Pin Setup
#define BUTTON_PIN_1 36
#define BUTTON_PIN_2 37
#define BUTTON_PIN_3 38
#define BUTTON_PIN_4 39

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
  Serial.begin(115200);
  
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
    } else if (homeScoreCorrection()) {
      homeScore = decreaseScore(homeScore);
    } else if (awayScored()) {
      awayScore = increaseScore(awayScore);
    } else if (awayScoreCorrection()) {
      awayScore = decreaseScore(awayScore);
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
