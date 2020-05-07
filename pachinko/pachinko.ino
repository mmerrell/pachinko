#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN        6
#define WAIT                5
#define MAX_BALLS           3
#define BRIGHTNESS          123
#define TOTAL_PIXELS        360
#define TIME_BETWEEN_LAUNCH 30  //The number of loops, not the actual time
#define MAX_CONNECTIONS     4

#define POSITION_0_PIN      10  //Hall effect sensor, under handle at REST position
#define POSITION_1_PIN      11  //Hall effect 1
#define POSITION_2_PIN      12  //Hall effect 2
#define POSITION_3_PIN      13  //Hall effect 3
#define POSITION_4_PIN      8   //The actual switch, indicating MAX POWER

Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static int segments[][2] = {
  {   0,  119 }, //0
  {   0,   73 }, //1
  { 167,  -14 }, //2
  { 174,   29 }, //3
  { 223,   17 }, //4
  { 167,  -47 }, //5
  {   0,   78 }, //6
  {  77,   -5 }, //7
  { 168,   33 }, //8
  { 168,    6 }, //9
  { 154,  -34 }, //10
  { 201,   22 }, //11
  { 265,  -25 }, //12 
  { 265,   48 }, //13
  { 168,   10 }, //14
  { 227,   14 }, //15
  { 265,   26 }, //16
  {  77,  -13 }, //17
  { 359,   -8 }, //18
  { 268,   23 }, //19
  { 181,    1 }, //20
  { 342,    8 }, //21
  {   0,   50 }, //22
  { 182,   13 }, //23
  { 324,   -9 }, //24
  { 206,   17 }, //25
  {   0,   41 }, //26
  { 333,  -18 }, //27
};

//Each element of this array corresponds to a segment above, and lists is "connecting segments"
// -1 is a "non-prize" terminal (or another segment)
// -2 is a "tulip prize"
// -3 is the "center prize"
static int connections[][MAX_CONNECTIONS] = {
  {             -1 }, //0
  { 2, 5,       -1 }, //1
  { 3,          -1 }, //2
  { 4,          -1 }, //3
  {             -1 }, //4
  {             -1 }, //5
  { 7,          -1 }, //6
  { 8, 9, 14,   -1 }, //7
  { 4, 11,      -1 }, //8
  { 10,         -1 }, //9
  {             -1 }, //10
  { 4,          -1 }, //11
  {             -1 }, //12 
  {             -1 }, //13
  { 12, 13, 16, -1 }, //14
  {             -1 }, //15
  { 15,         -1 }, //16
  { 18,         -1 }, //17
  { 20,         -1 }, //18
  { 15,         -1 }, //19
  { 19, 23,     -1 }, //20
  { 20,         -1 }, //21
  { 21,         -1 }, //22
  { 24,         -1 }, //23
  { 25,         -1 }, //24
  { 4,          -1 }, //25
  { 27,         -1 }, //26
  { 25,         -1 }, //27
};

static int connectionCounts[sizeof(connections)/sizeof(int[MAX_CONNECTIONS])];

static uint32_t colorAry[] = {
  strip.Color(255, 255, 255),
  strip.Color(255, 0, 255),
  strip.Color(0, 255, 255),
  strip.Color(255, 255, 0),
  strip.Color(0, 120, 0),
  strip.Color(123, 255, 123),
  strip.Color(255, 123, 0)
};

char data[100];

//Ball Effects:
// 0: Just the specified color
// 1: Cycle through the palette, starting with the specifed color and going around the wheel
byte ballEffects[MAX_BALLS];

//Each ball has its own speed, which will (eventually) determine its path selection
int ballSpeeds[MAX_BALLS];

//keeps track of which segment each ball is on
int ballSegments[MAX_BALLS];

//Which segments can be starting points
byte startingSegments[] = { 0, 1, 6, 22, 26 };

//keeps track of where the ball is in its path, as well as the trailing pixels
int ballPositions[MAX_BALLS];
int ballPixels[MAX_BALLS];
int trailingBallPixels1[MAX_BALLS];
int trailingBallPixels2[MAX_BALLS];

//keeps track of the color of the balls
uint32_t ballColors[MAX_BALLS];

double ignoreStartButtonCounter = 0;
boolean startButtonLow;

//Init all the pins
void initIoPins() {
  pinMode(POSITION_0_PIN, INPUT_PULLUP);
  pinMode(POSITION_1_PIN, INPUT_PULLUP);
  pinMode(POSITION_2_PIN, INPUT_PULLUP);
  pinMode(POSITION_3_PIN, INPUT_PULLUP);
  pinMode(POSITION_4_PIN, INPUT_PULLUP);
}

void readyAnimation() {
  for (byte i=0; i<2; i++) {
    strip.fill(strip.Color(50, 0, 50), 0);
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
  strip.clear();
  strip.show();
}

void initArrays() {
  //Initialize all balls to "inactive" (-1 is inactive for path and position)
  for (int i=0; i<MAX_BALLS; i++) {
    ballSegments[i] = -1;
    ballPositions[i] = -1;
    ballColors[i] = strip.Color(0, 0, 0);
    trailingBallPixels1[i] = -1;
    trailingBallPixels2[i] = -1;
    ballEffects[i] = 0;
  }

  //build the array that keeps track of how many connections each segment has
  for (int i=0; i<(sizeof(segments)/sizeof(int[2])); i++) {
    for (int j=0; j<MAX_CONNECTIONS; j++) {
      if (connections[i][j] < 0) {
        connectionCounts[i] = j;
      }
    }
  }
}

void initLights() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
}

//Launch variables
int offPosition = LOW;     //TODO - change to byte during optimization - this could potentially be a local
int prevOffPosition = LOW; //TODO - change to byte during optimization
bool launching = false;
byte ballSpeed = 0;

void checkLaunch() {
  offPosition = digitalRead(POSITION_0_PIN);

  //if the handle is pulled back, measure the highest Hall sensor it's triggered
  if (launching) {
    if (offPosition == LOW) {
      //Using the "highest Hall triggered" number, launch the ball at a certain velocity
      Serial.print(F("Ball Speed: "));
      Serial.print(ballSpeed, BIN);
      Serial.println(F("\nLAUNCHING!!!"));
      ballLaunch();//speed, color, effect
      ballSpeed = 0;
      launching = false;
    }
    //Only need to check these switches if we're "underway"
    int position1 = digitalRead(POSITION_1_PIN);
    int position2 = digitalRead(POSITION_2_PIN);
    int position3 = digitalRead(POSITION_3_PIN);

    if (position3 == LOW) {
      ballSpeed |= 4; //00000100
      Serial.println(F("POSITION 3"));
    }
    if (position2 == LOW) {
      ballSpeed |= 2; //00000010
      Serial.println(F("POSITION 2"));
    }
    if (position1 == LOW) {
      ballSpeed |= 1; //00000001
      Serial.println(F("POSITION 1"));
    }
  } else {
    if (offPosition == LOW) { //offPosition LOW means the handle is AT REST
      if (prevOffPosition == HIGH) {
        //The handle has been brought back to the lowest position
        Serial.println(F("OFF"));
        prevOffPosition = LOW;
      }
      //Do nothing--we're waiting for the handle to be pulled back
    } else {
      //Launching - if they let go of the handle before it hits another magnet, it will be the slowest speed
      Serial.println(F("POSITION 0"));
      prevOffPosition = HIGH;
      launching = true;
    }
  }

}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Ready..."));

  initIoPins();
  initArrays();
  initLights();
  readyAnimation();
}

uint32_t getRandomBallColor() {
  return colorAry[random(0, sizeof(colorAry)/sizeof(uint32_t))];
}


void loop() {
  //Don't even check for a launch if it hasn't been long enough
  if (ignoreStartButtonCounter >= TIME_BETWEEN_LAUNCH) {
    checkLaunch();
  }
  //End of Launch Sequence


  updateBalls();
  strip.show();
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void ballLaunch(byte ballSpeed, uint32_t color, int ballEffect) {
  for (int ballIdx=0; ballIdx<MAX_BALLS; ballIdx++) {
    if (ballPositions[ballIdx] == -1) {
      ballSegments[ballIdx] = startingSegments[random(0, sizeof(startingSegments))];
      sprintf_P(data, PSTR("Choosing initial segment for ball %d: %d"), ballIdx, ballSegments[ballIdx]);
      Serial.println(data);
      ballPositions[ballIdx] = 0;
      ballSpeeds[ballIdx] = ballSpeed;
      ballColors[ballIdx] = color;
      ballEffects[ballIdx] = ballEffect;
      return;
    }
  }
}

void ballLaunch(byte ballSpeed, uint32_t color) {
  ballLaunch(ballSpeed, color, 0);
}

void ballLaunch(byte ballSpeed) {
  ballLaunch(ballSpeed, getRandomBallColor(), random(0, 2));
}

void ballLaunch() {
  ballLaunch(0, getRandomBallColor(), random(0, 2));
}

void updateBalls() {
  for (int ballIdx=0; ballIdx<MAX_BALLS; ballIdx++) {
    //If the position is -1, the ball is inactive
    if (ballPositions[ballIdx] != -1) {
      
      //before we increment the position in the path, set the trailing pixels
      trailingBallPixels2[ballIdx] = trailingBallPixels1[ballIdx];
      trailingBallPixels1[ballIdx] = ballPixels[ballIdx];

      //increment the ball's position in the path
      ballPositions[ballIdx]++;
      
      byte ballEffect = ballEffects[ballIdx];

      if (segments[ballSegments[ballIdx]][1] >= 0) {
        ballPixels[ballIdx] = segments[ballSegments[ballIdx]][0] + ballPositions[ballIdx];
      } else {
        ballPixels[ballIdx] = segments[ballSegments[ballIdx]][0] - ballPositions[ballIdx];
      }

      //We've determined which pixel to light, now determine whether 
      //we're at the end of the segment, and if so, determine the new segment
      if (ballPositions[ballIdx] == abs(segments[ballSegments[ballIdx]][1])) {
        sprintf_P(data, PSTR("Current Segment %d, Position %d"), ballSegments[ballIdx], ballPositions[ballIdx]);
        Serial.println(data);
        if (connectionCounts[ballSegments[ballIdx]] == 0) {
          ballPositions[ballIdx] = -1;
          ballSegments[ballIdx] = -1;
          ballColors[ballIdx] = strip.Color(0, 0, 0);
          strip.setPixelColor(ballPixels[ballIdx], ballColors[ballIdx]);
          strip.setPixelColor(trailingBallPixels1[ballIdx], ballColors[ballIdx]);
          strip.setPixelColor(trailingBallPixels2[ballIdx], ballColors[ballIdx]);
          return;
        } else {
          ballPositions[ballIdx] = 0;
          int rndSeg = random(0, connectionCounts[ballSegments[ballIdx]]);
          ballSegments[ballIdx] = connections[ballSegments[ballIdx]][rndSeg];
//          sprintf(data, "%d, %d, %d, %d", rndSeg, connectionCounts[ballSegments[ballIdx]], connections[ballSegments[ballIdx]][rndSeg], ballSegments[ballIdx]);
//          Serial.println(data);
        }
      }
      strip.setPixelColor(ballPixels[ballIdx], ballColors[ballIdx]);
      strip.setPixelColor(trailingBallPixels1[ballIdx], strip.ColorHSV(ballColors[ballIdx], 50, 10));
      strip.setPixelColor(trailingBallPixels2[ballIdx], strip.Color(0, 0, 0));
    }
  }
}
