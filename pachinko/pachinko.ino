#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN        6
#define WAIT                5
#define MAX_BALLS           5
#define BRIGHTNESS          123
#define TOTAL_PIXELS        360
#define TIME_BETWEEN_LAUNCH 3  //The number of loops, not the actual time
#define MAX_CONNECTIONS     4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static int segments[][2] = {
  {   0,  119 }, //0
  {   0,   73 }, //1
  { 167,  -14 }, //2
  { 174,   27 }, //3
  { 221,   19 }, //4
  { 167,  -47 }, //5
  {   0,   78 }, //6
  {  77,   -5 }, //7
  { 168,   33 }, //8
  { 168,    6 }, //9
  { 153,  -33 }, //10
  { 200,   20 }, //11
  { 264,  -24 }, //12 
  { 265,   49 }, //13
  { 168,   10 }, //14
  { 226,   15 }, //15
  { 265,   25 }  //16
};

//Each element of this array corresponds to a segment above, and lists is "connecting segments"
// -1 is a "non-prize" terminal
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
  { 15,         -1 }  //16
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
int ballEffects[MAX_BALLS];

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

//keeps track of which segment each ball is on
int ballSegments[MAX_BALLS];

//Which segments can be starting points
int startingSegments[3] = { 0, 1, 6 };

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
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
}

void readyAnimation() {
  for (int i=0; i<2; i++) {
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

void updateIoPins() {
  startButtonState = digitalRead(START_BUTTON_PIN);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Ready...");

  initIoPins();
  initArrays();
  initLights();
  readyAnimation();
}

uint32_t getRandomBallColor() {
  return colorAry[random(0, sizeof(colorAry)/sizeof(uint32_t))];
}

void loop() {
  updateIoPins();
  if (ignoreStartButtonCounter >= TIME_BETWEEN_LAUNCH) {
    if (startButtonState == LOW) {
      startButtonLow = true;
    } else {
      if (startButtonLow) {
        startButtonLow = false;
        Serial.println("Launching Ball!!!");
        ballLaunch();
        ignoreStartButtonCounter = 0;
      }
    }
  }

  updateBalls();
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void ballLaunch(uint32_t color, int ballEffect) {
  for (int i=0; i<MAX_BALLS; i++) {
    if (ballPositions[i] == -1) {
      ballSegments[i] = startingSegments[random(0, 3)];
      sprintf(data, "Choosing new segment for ball %d: %d", i, ballSegments[i]);
      Serial.println(data);
      ballPositions[i] = 0;
      ballColors[i] = color;
      ballEffects[i] = ballEffect;
      return;
    }
  }
}

void ballLaunch(uint32_t color) {
  ballLaunch(color, 0);
}

void ballLaunch() {
  ballLaunch(getRandomBallColor(), random(0, 2));
}

void updateBalls() {
  for (int ballIdx=0; ballIdx<MAX_BALLS; ballIdx++) {
    //If the position is -1, the ball is inactive
    if (ballPositions[ballIdx] != -1) {
//      sprintf(data, "Checkpoint 0: ball %d is active in segment %d, position %d, pixel %d", ballIdx, ballSegments[ballIdx], ballPositions[ballIdx], ballPixels[ballIdx]);
//      Serial.println(data);
      
      //before we increment the position in the path, set the trailing pixels
      trailingBallPixels2[ballIdx] = trailingBallPixels1[ballIdx];
      trailingBallPixels1[ballIdx] = ballPixels[ballIdx];

      //increment the ball's position in the path
      ballPositions[ballIdx]++;
      
      int ballEffect = ballEffects[ballIdx];

      if (segments[ballSegments[ballIdx]][1] >= 0) {
        ballPixels[ballIdx] = segments[ballSegments[ballIdx]][0] + ballPositions[ballIdx];
//        sprintf(data, "Incrementing: Ball %d to position %d: %d", ballIdx, ballPositions[ballIdx], ballPixels[ballIdx]);
//        Serial.println(data);
      } else {
        ballPixels[ballIdx] = segments[ballSegments[ballIdx]][0] - ballPositions[ballIdx];
//        sprintf(data, "Decrementing: Ball %d to position %d: %d", ballIdx, ballPositions[ballIdx], ballPixels[ballIdx]);
//        Serial.println(data);
      }

      //We've determined which pixel to light, now determine whether 
      //we're at the end of the segment, and if so, determine the new segment
//      sprintf(data, "Ball %d, Segment %d, Starting pixel %d, Length %d, Position %d", ballIdx, ballSegments[ballIdx], segments[ballSegments[ballIdx]][0], segments[ballSegments[ballIdx]][1], ballPositions[ballIdx]);
//      Serial.println(data);
      if (ballPositions[ballIdx] == abs(segments[ballSegments[ballIdx]][1])) {
        sprintf(data, "Current Segment %d, Position %d", ballSegments[ballIdx], ballPositions[ballIdx]);
        Serial.println(data);
        if (connectionCounts[ballSegments[ballIdx]] == 0) {
          ballPositions[ballIdx] = -1;
          ballSegments[ballIdx] = -1;
          ballColors[ballIdx] = strip.Color(0, 0, 0);
          strip.setPixelColor(ballPixels[ballIdx], ballColors[ballIdx]);
          strip.setPixelColor(trailingBallPixels1[ballIdx], strip.Color(0, 0, 0));
          strip.setPixelColor(trailingBallPixels2[ballIdx], strip.Color(0, 0, 0));
          strip.show();
//          sprintf(data, "Ball %d terminated\n\tPixel: %d\n\tTrailing 1: %d\n\tTrailing 2: %d", ballIdx, ballPixels[ballIdx], trailingBallPixels1[ballIdx], trailingBallPixels2[ballIdx]);
//          Serial.println(data);
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

  strip.show();
}

//void lightPath(int pathIdx) {
//  int index = 0;
//  while (paths[pathIdx][index] != -1) {
//    strip.setPixelColor(paths[pathIdx][index], strip.Color(10, 10, 10));
//    index++;
//  }
//  strip.show();
//  delay(60000);
//}
