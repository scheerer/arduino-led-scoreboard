#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Bounce2.h>        // Button Debouncer

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

// Hardware setup
#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);


#define BUTTON_PIN_1 10
#define BUTTON_PIN_2 11
#define BUTTON_PIN_3 12
#define BUTTON_PIN_4 13

Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 
Bounce debouncer3 = Bounce(); 
Bounce debouncer4 = Bounce();

// Program variables
       
const int WINNING_SCORE = 21;
int homeScore = 0;
int awayScore = 0;
bool updateDisplay = true;

/****************************************
 * Program Setup
 ****************************************/
void setup() {
  // Setup the first button with an internal pull-up :
  pinMode(BUTTON_PIN_1,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(250); // interval in ms

  pinMode(BUTTON_PIN_2,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(250); // interval in ms

  pinMode(BUTTON_PIN_3,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer3.attach(BUTTON_PIN_3);
  debouncer3.interval(250); // interval in ms

  pinMode(BUTTON_PIN_4,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer4.attach(BUTTON_PIN_4);
  debouncer4.interval(250); // interval in ms

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
  check_switches();

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
  // Update display
  matrix.swapBuffers(false);
  //textHue += 7;
  //if(textHue >= 1536) textHue -= 1536;
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
  return debouncer1.fell();
//  return debouncer1.read() == LOW;
//  return buttonPressed(homeAddButton);
}

bool awayScored() {
  return debouncer2.fell();
//  return debouncer2.read() == LOW;
//  return buttonPressed(awayAddButton);
}

bool homeScoreCorrection() {
  return debouncer3.fell();
//  return debouncer3.read() == LOW;
//  return buttonPressed(homeSubButton);
}

bool awayScoreCorrection() {
  return debouncer4.fell();
//  return debouncer4.read() == LOW;
//  return buttonPressed(awaySubButton);
}

bool scoreResetPressed() {
  return homeScoreCorrection() && awayScoreCorrection();
}

//bool buttonPressed(int buttonIndex) {
//  return justpressed[buttonIndex];
////  return digitalRead(buttonPin) == HIGH;
//}

/****************************************
 * Display Related
 ****************************************/
void displayScoreBoardScreen() {
    // fill the screen with 'black'
  //matrix.fillScreen(0);
  
  //displayScrollingText();
  
  //if (gameOver()) {
  //  blinkScores();
  //} else {
    displayScores();
  //}
  updateDisplay = false;
}

void displayScores() {
  if (updateDisplay) {
    matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));
  }
  matrix.setCursor(1, 4);   // bottom line

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
  clearScoresDisplay();
  delay(500);
}

void clearScoresDisplay() {
  matrix.fillRect(0, 4, 32, 10, matrix.Color333(0, 0, 0));
}

// Scrolls text from right to left with changing colors
//void displayScrollingText() {
//  // Draw big scrolly text on top
//  matrix.setTextColor(matrix.ColorHSV(textHue, 255, 255, true));
//  matrix.setCursor(textX, 0); // top line
//  matrix.print(F2(str));
//
//  // Move text left (w/wrap), increase hue
//  if((--textX) < textMin) textX = matrix.width();
//  textHue += 7;
//  if(textHue >= 1536) textHue -= 1536;
//}

void check_switches()
{
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();
  debouncer4.update();
}
