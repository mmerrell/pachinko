#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 6
#define WAIT 5
#define MAX_BALLS 8
#define MAX_SEGMENTS 6
#define TOTAL_PATHS 5
#define TIME_BETWEEN_LAUNCH 20  //This is the number of loops, not the actual time

int segments[][2] = {
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
  { 200,   20 }  //11
};

int paths[][MAX_SEGMENTS] = {
  { 0,              -1 },
  { 1, 2, 3, 4,     -1 },
  { 1, 5,           -1 },
  { 6, 7, 8, 4,     -1 },
  { 6, 7, 9, 10,    -1 },
  { 6, 7, 8, 11, 4, -1 }
};

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(240, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//keeps track of which path each ball is on
int ballPaths[MAX_BALLS];

//keeps track of where the ball is in its path
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

void initArrays() {
  for (int i=0; i<MAX_BALLS; i++) {
    ballPaths[i] = -1;
    ballPositions[i] = -1;
    ballColors[i] = strip.Color(0, 0, 0);
    trailingBallPixels1[i] = -1;
    trailingBallPixels2[i] = -1;
  }
}

void initLights() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void updateIoPins() {
  startButtonState = digitalRead(START_BUTTON_PIN);
}

void setup() {
//  Serial.begin(9600);
//  Serial.println("Ready...");

  initIoPins();
  initArrays();
  initLights();
}

uint32_t getRandomBallColor() {
  uint32_t colorAry[] = {
    strip.Color(120, 120, 120),
    strip.Color(120, 0, 120),
    strip.Color(0, 120, 120),
    strip.Color(120, 120, 0),
    strip.Color(0, 120, 0),
    strip.Color(50, 120, 50),
    strip.Color(255, 120, 0)
  };

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
        ballLaunch();
//        ballLaunch(5, strip.Color(120, 120, 120));
        ignoreStartButtonCounter = 0;
      }
    }
  }

  updateBalls();
  counter++;
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void ballLaunch(int path, uint32_t color) {
  for (int i=0; i<MAX_BALLS; i++) {
    if (ballPositions[i] == -1) {
      ballPaths[i] = path;
      ballPositions[i] = 0;
      ballColors[i] = color;
      return;
    }
  }
}

void ballLaunch(uint32_t color) {
  int numPaths = sizeof(paths) / sizeof(int[MAX_SEGMENTS]);
  ballLaunch(random(0, numPaths), color);
}

void ballLaunch() {
  int numPaths = sizeof(paths) / sizeof(int[MAX_SEGMENTS]);
  ballLaunch(random(0, numPaths), getRandomBallColor());
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
  for (int i=0; i<MAX_BALLS; i++) {
    //If the position is -1, the ball is inactive
    if (ballPositions[i] != -1) {

      trailingBallPixels2[i] = trailingBallPixels1[i];
      trailingBallPixels1[i] = ballPixels[i];

      ballPositions[i]++;
      int ballPosition = ballPositions[i];
      int ballPath = ballPaths[i];
      uint32_t color = ballColors[i];
      int pixel = calcPathPixel(ballPath, ballPosition);
      ballPixels[i] = pixel;
      if (pixel == -1) {
        ballPaths[i] = -1;
        ballPositions[i] = -1;
        ballColors[i] = strip.Color(0, 0, 0);
        strip.setPixelColor(pixel, strip.Color(0, 0, 0));
        strip.setPixelColor(trailingBallPixels1[i], strip.Color(0, 0, 0));
        strip.setPixelColor(trailingBallPixels2[i], strip.Color(0, 0, 0));
        return;
      }
      strip.setPixelColor(pixel, color);
      strip.setPixelColor(trailingBallPixels1[i], strip.Color(10, 10, 10));
      strip.setPixelColor(trailingBallPixels2[i], strip.Color(0, 0, 0));
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
