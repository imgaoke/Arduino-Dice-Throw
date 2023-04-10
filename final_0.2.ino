#include <funshield.h>
enum State {BootScreen, NormalMode, ConfigurationMode};

//variables for function displayChar
constexpr byte LETTER_GLYPH[] {
  0b10001000,   // A
  0b10000011,   // b
  0b11000110,   // C
  0b10100001,   // d
  0b10000110,   // E
  0b10001110,   // F
  0b10000010,   // G
  0b10001001,   // H
  0b11111001,   // I
  0b11100001,   // J
  0b10000101,   // K
  0b11000111,   // L
  0b11001000,   // M
  0b10101011,   // n
  0b10100011,   // o
  0b10001100,   // P
  0b10011000,   // q
  0b10101111,   // r
  0b10010010,   // S
  0b10000111,   // t
  0b11000001,   // U
  0b11100011,   // v
  0b10000001,   // W
  0b10110110,   // ksi
  0b10010001,   // Y
  0b10100100,   // Z
};
constexpr byte EMPTY_GLYPH = 0b11111111;

// variables for function windowSlide
unsigned long currentLoopCounter;
unsigned long lastLoopCounter;

//variables for function BootScreenMultiplex
int charactorPosition;
const char *message = "Welcome to Dice Throws";

// the variable for function normalModeMultiplex
int normalModePosition;

// the variable for function ConfigurationModeMultiplex
int configurationModePosition;

// varaibles for functions buttonOneFunction, buttonTwoFunction, buttonThreeFunction
constexpr int button[3] = {button1_pin, button2_pin, button3_pin};
constexpr bool released = true;
constexpr bool pressed = false;
bool lastButtonStatus[3];

// variables for the function buttonOneFunction
unsigned long counter;
int randomNumber;

// the variable for the function buttonTwoFunction
int currentNumberOfThrows;

// variables for the function buttonThreeFunction
constexpr int diceTypes[7] = {4, 6, 8, 10, 12, 20, 100};
int diceTypeIndex;
int currentdiceType;

// general variables
enum State state;
const char *messageItselfStartingLocation = message;
const char *messageWindowStartingLocation = message;




// general helper functions : start
void displayChar(char ch, byte pos)
{
  byte glyph = EMPTY_GLYPH;
  if (isAlpha(ch)) {
    glyph = LETTER_GLYPH[ ch - (isUpperCase(ch) ? 'A' : 'a') ];
  }
  
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << pos);
  digitalWrite(latch_pin, HIGH);
}

void displayDigit(int num, byte pos){
  byte digit = digits[num];
  
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, digit);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << pos);
  digitalWrite(latch_pin, HIGH);
}

// interfaces for global variables in function buttonOneFunction
int retrieveCounter(){
  return counter;
}
int retrieveRandomNumber(){
  return randomNumber;
}

// the interface for the global variable in function buttonTwoFunction
int retrieveCurrentNumberOfThrows(){
  return currentNumberOfThrows;
}

// the interface for the global variable in function buttonThreeFunction
int retrieveCurrentdiceType(){
  return currentdiceType;
}
// general helper functions : end

// helper functions for boot screen mutiplex: start
void setWindowPointer(const char* pointer){
    messageWindowStartingLocation = pointer;
}

const char * retrieveMessageItselfStartingLocation(){
  return messageItselfStartingLocation;
}

const char * retrieveMessageWindowStartingLocation(){
  return messageWindowStartingLocation;
}

void windowSlide(){
  currentLoopCounter = millis() / 300;
  if (currentLoopCounter > lastLoopCounter){
    lastLoopCounter = currentLoopCounter;
    setWindowPointer(retrieveMessageWindowStartingLocation() + 1);
  }
}
// helper functions for boot screen multiplex: end

void BootScreenMultiplex(){
  if (*message != '\0'){
    displayChar(*message, charactorPosition);
    message += 1;
    if (charactorPosition == 3){
      message = retrieveMessageWindowStartingLocation();
    }
    charactorPosition = (charactorPosition + 1) % 4;
  }
  else if (*message == '\0'){
    if (charactorPosition == 0){
      setWindowPointer(retrieveMessageItselfStartingLocation());
    }
    charactorPosition = 0;
    message = retrieveMessageWindowStartingLocation();
  }

}

// return the value of what 'base' is raised to 'power'
// the helper function for function ConfigurationModeMultiplex and function NormalModeMultiplex
int power(int base, int power){  
  int result = 1;
  if (power == 0){
    return result;
  }
  else{
    for (int i = 1; i <= power; ++i) {
     result *= base;
    }
    return result;
  }
}

void ConfigurationModeMultiplex(){
  int numToDisplay;
  if (configurationModePosition == 1){
    displayChar('d', configurationModePosition); 
  }
  else{
    if (configurationModePosition == 0){
      numToDisplay = retrieveCurrentNumberOfThrows();
    }
    else{
      numToDisplay = retrieveCurrentdiceType() / power(10, (3 - configurationModePosition)) % 10;
    }
    displayDigit(numToDisplay, configurationModePosition); 
  }
  configurationModePosition = (configurationModePosition + 1) % 4; // configurationModePosition is 0,1,2,3 in which 0 is the leftmost position
}

void NormalModeMultiplex(){
  int numToDisplay = retrieveRandomNumber() / power(10, (3 - normalModePosition)) % 10;  
  bool isCurrentDigitMeaningless = (retrieveRandomNumber() / power(10, (3 - normalModePosition)) == 0);
  if (isCurrentDigitMeaningless == false){
    displayDigit(numToDisplay, normalModePosition);
  }
  normalModePosition = (normalModePosition + 1) % 4; // normalModePosition is 0,1,2,3 in which 0 is the leftmost position
}

// the helper function for function buttonOneFunction
int randomNumberGenerator(){
  randomSeed(retrieveCounter());
  int randomSum = 0;
  for (int i = 0; i < retrieveCurrentNumberOfThrows(); ++i){
    randomSum += random(1, retrieveCurrentdiceType() + 1);
  }
  return randomSum;
}

void buttonOneFunction(){
  int currentButtonStatus = digitalRead(button[0]);
  if (currentButtonStatus == released){
    if (lastButtonStatus[0] == pressed){
      // After button_one's firstly press and later release
      randomNumber = randomNumberGenerator();
    }
    lastButtonStatus[0] = released;
  }
  else if (currentButtonStatus == pressed){
    state = NormalMode;
    lastButtonStatus[0] = pressed;
    counter += 1;
    randomNumber = randomNumberGenerator();
  }
}

void buttonTwoFunction(){
  int currentButtonStatus = digitalRead(button[1]);
  if (currentButtonStatus == released){
    if (lastButtonStatus[1] == pressed){     
      currentNumberOfThrows = currentNumberOfThrows % 9 + 1;
    }
    lastButtonStatus[1] = released;
  }
  else if (currentButtonStatus == pressed){
    lastButtonStatus[1] = pressed;
    state = ConfigurationMode;
  }
}

void buttonThreeFunction(){
  int currentButtonStatus = digitalRead(button[2]);
  if (currentButtonStatus == released){
    if (lastButtonStatus[2] == pressed){ 
      diceTypeIndex = (diceTypeIndex + 1) % 7;
      currentdiceType = diceTypes[diceTypeIndex];
    }
    lastButtonStatus[2] = released;
  }
  else if (currentButtonStatus == pressed){
    lastButtonStatus[2] = pressed;
    state = ConfigurationMode;
  }
}

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i <= 2; ++i) {
  pinMode(button[i], INPUT);
  lastButtonStatus[i] = released;
  }
  pinMode(latch_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  counter = 0;
  configurationModePosition = 0;
  normalModePosition = 0;
  currentNumberOfThrows = 5;
  currentdiceType = diceTypes[4];
  randomNumber = 1;
  diceTypeIndex = 0;
  state = BootScreen;
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonOneFunction();
  buttonTwoFunction();
  buttonThreeFunction();
  if (state == BootScreen){
    windowSlide();
    BootScreenMultiplex();
  }
  else if (state == NormalMode){
    NormalModeMultiplex();
  }
  else if (state == ConfigurationMode){
    ConfigurationModeMultiplex();
  }
}
