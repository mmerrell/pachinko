#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 6
#define WAIT 2
#define MAX_BALLS 8
#define MAX_SEGMENTS 6
#define TOTAL_PATHS 5
#define BRIGHTNESS 123
#define TOTAL_PIXELS 360
#define TIME_BETWEEN_LAUNCH 20  //This is the number of loops, not the actual time

Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static int segments[][2] = {
  {   0,  120 }, //0
  {   0,   73 }, //1
  { 167,  -14 }, //2
  { 174,   27 }, //3
  { 221,   19 }, //4
  { 167,  -47 }, //5
  {   0,   78 }, //6
  {  77,   -5 }, //7
  { 168,   33 }, //8
  { 168,    6 }, //9
  { 153,  -34 }, //10
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
static int connections[][4] = {
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

static int paths[][MAX_SEGMENTS] = {
  { 0,                 -1 }, //0
  { 1,  2,  3,  4,     -1 }, //1
  { 1,  5,             -1 }, //2
  { 6,  7,  8,  4,     -1 }, //3
  { 6,  7,  9, 10,     -1 }, //4
  { 6,  7,  8, 11,  4, -1 }, //5
  { 6,  7, 14, 12,     -1 }, //6
  { 6,  7, 14, 13,     -1 }, //7
  { 6,  7, 14, 16, 15, -1 }  //8
};

static uint32_t colorAry[] = {
  strip.Color(255, 255, 255),
  strip.Color(255, 0, 255),
  strip.Color(0, 255, 255),
  strip.Color(255, 255, 0),
  strip.Color(0, 120, 0),
  strip.Color(123, 255, 123),
  strip.Color(255, 123, 0)
};

//Ball Effects:
// 0: Just the specified color
// 1: Cycle through the palette, starting with the specifed color and going around the wheel
int ballEffects[MAX_BALLS];

const int numPaths = sizeof(paths)/sizeof(int[MAX_SEGMENTS]);
int pathLengths[numPaths];

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

//keeps track of which path each ball is on
int ballPaths[MAX_BALLS];

//keeps track of where the ball is in its path, as well as the trailing pixels
int ballPositions[MAX_BALLS];
int ballPixels[MAX_BALLS];
int trailingBallPixels1[MAX_BALLS];
int trailingBallPixels2[MAX_BALLS];

//keeps track of the color of the balls
uint32_t ballColors[MAX_BALLS];

double counter = 0;
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
  //calculate path lengths
  for (int path=0; path<numPaths; path++) {
    int pixelCount = 0;
    int segment = 0;
    while(paths[path][segment] != -1) {
      pixelCount += abs(segments[paths[path][segment]][1]);
      segment++;
    }
    pathLengths[path] = pixelCount;
  }

  //Initialize all balls to "inactive" (-1 is inactive for path and position)
  for (int i=0; i<MAX_BALLS; i++) {
    ballPaths[i] = -1;
    ballPositions[i] = -1;
    ballColors[i] = strip.Color(0, 0, 0);
    trailingBallPixels1[i] = -1;
    trailingBallPixels2[i] = -1;
    ballEffects[i] = 0;
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

//int pathIdx = 7;

void loop() {
  updateIoPins();
  if (ignoreStartButtonCounter >= TIME_BETWEEN_LAUNCH) {
    if (startButtonState == LOW) {
      startButtonLow = true;
    } else {
      if (startButtonLow) {
        startButtonLow = false;
        ballLaunch();
//        ballLaunch(pathIdx % numPaths, strip.Color(255, 255, 255), 0);
//        Serial.print("Path: ");
//        Serial.println(pathIdx % numPaths);
        ignoreStartButtonCounter = 0;
//        pathIdx++;
      }
    }
  }

  updateBalls();
  counter++;
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void ballLaunch(int path, uint32_t color, int ballEffect) {
  for (int i=0; i<MAX_BALLS; i++) {
    if (ballPositions[i] == -1) {
      ballPaths[i] = path;
      ballPositions[i] = 0;
      ballColors[i] = color;
      ballEffects[i] = ballEffect;
      return;
    }
  }
}

void ballLaunch(uint32_t color) {
  ballLaunch(random(0, numPaths), color, 0);
}

void ballLaunch() {
  ballLaunch(random(0, numPaths), getRandomBallColor(), random(0, 2));
}

//Given a path and a position within the path, calculate which pixel to light up
int calcPathPixel(int ballPath, int ballPosition) {
  //which segment are we in?
  int minSegIndex = 0;
  int maxSegIndex = 0;
  int segment = -1;
  
  for (int i=0; i<MAX_SEGMENTS; i++) {
    if (paths[ballPath][i] == -1) {
      break;
    }
    maxSegIndex += abs(segments[paths[ballPath][i]][1]);
    if (ballPosition >= minSegIndex && ballPosition <= maxSegIndex) {
      segment = i;
      break;
    }
    minSegIndex = maxSegIndex + 1;
  }
  if (segment == -1) {
    return -1;
  }

  int offset = ballPosition - minSegIndex;
  int pixel;
  int segLength = segments[paths[ballPath][segment]][1];
  int segStartingPoint = segments[paths[ballPath][segment]][0];
  
  if (segLength >= 0) {
    pixel = segStartingPoint + offset;
  } else {
    pixel = segStartingPoint - offset;
  }
  return pixel;
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
      int ballPosition = ballPositions[ballIdx];
      int ballPath = ballPaths[ballIdx];
      int ballEffect = ballEffects[ballIdx];
      int pixel = calcPathPixel(ballPath, ballPosition);
      ballPixels[ballIdx] = pixel; //this has to be done after incrementing and calculating the pixel, so the trailing pixels can be set correctly

      //If pixel has been calculated as '-1', the ball has been terminated
      if (pixel == -1) {
        ballPaths[ballIdx] = -1;
        ballPositions[ballIdx] = -1;
        ballColors[ballIdx] = strip.Color(0, 0, 0);
        strip.setPixelColor(pixel, ballColors[ballIdx]);
        strip.setPixelColor(trailingBallPixels1[ballIdx], strip.Color(0, 0, 0));
        strip.setPixelColor(trailingBallPixels2[ballIdx], strip.Color(0, 0, 0));
        return;
      }
      
      uint32_t color;
      switch(ballEffect) {
        case 1:
          uint32_t mappedColor = map(ballPositions[ballIdx], 0, pathLengths[ballPaths[ballIdx]], ballColors[ballIdx], ballColors[ballIdx] + 65535);
          int hue = 255;
          color = strip.ColorHSV(mappedColor, hue, BRIGHTNESS);
          break;
        case 0:
        default:
          color = ballColors[ballIdx];
      };
      
      strip.setPixelColor(pixel, color);
      strip.setPixelColor(trailingBallPixels1[ballIdx], strip.ColorHSV(color, 123, 10));
      strip.setPixelColor(trailingBallPixels2[ballIdx], strip.Color(0, 0, 0));
    }
  }

  strip.show();
}

void lightPath(int pathIdx) {
  int index = 0;
  while (paths[pathIdx][index] != -1) {
    strip.setPixelColor(paths[pathIdx][index], strip.Color(10, 10, 10));
    index++;
  }
  strip.show();
  delay(60000);
}
