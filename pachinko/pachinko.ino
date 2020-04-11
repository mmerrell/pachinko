#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 6
#define WAIT 5
#define MAX_BALLS 5
#define TOTAL_PATHS 5
#define TIME_BETWEEN_LAUNCH 50  //This is the number of loops, not the actual time

int *path0;
int *path1;
int *path2;
int *path3;
int *path4;

int trial[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
  37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
  49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
  61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
  73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 
  85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 
  97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 
  109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, -1 };



const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(240, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//keeps track of which path each ball is on
int ballPaths[MAX_BALLS];

//keeps track of where the ball is in its path
int ballPositions[MAX_BALLS];

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
  }

  static int pathAry0[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 
    85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 
    109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, -1 };

  static int pathAry1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 
    155, 154, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 
    184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 
    196, 197, 198, 199, 200, 221, 222, 223, 224, 225, 226, 227, 
    228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
    -1 };

  static int pathAry2[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 
    155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 
    143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 
    131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, -1 };

  static int pathAry3[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    73, 74, 75, 76, 77, 77, 76, 75, 74, 73, 168, 169, 
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 
    182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 
    194, 195, 196, 197, 198, 199, 200, 221, 222, 223, 224, 225, 
    226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 
    238, 239, -1 };

  static int pathAry4[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    73, 74, 75, 76, 77, 77, 76, 75, 74, 73, 168, 169, 
    170, 171, 172, 173, 153, 152, 151, 150, 149, 148, 147, 146, 
    145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 
    133, 132, 131, 130, 129, 128, 127, 126, 125, 124, -1 };

  path0 = pathAry0;
  path1 = pathAry1;
  path2 = pathAry2;
  path3 = pathAry3;
  path4 = pathAry4;
}

void initLights() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void updateIoPins() {
  startButtonState = digitalRead(START_BUTTON_PIN);
}

int* getFlatPath(int pathIdx) {
  switch(pathIdx) {
    case 0:
      return path0;
      break;
    case 1:
      return path1;
      break;
    case 2:
      return path2;
      break;
//    case 3:
//      return path3;
//      break;
//    case 4:
//      return path4;
//      break;
    default:
      return path0;
      break;
  }
}

void setup() {
//  Serial.begin(9600);
  Serial.println("Ready...");

  initIoPins();
  initArrays();
  initLights();
}

uint32_t getRandomBallColor() {
  uint32_t colorAry[5] = {
    strip.Color(120, 120, 120),
    strip.Color(120, 0, 120),
    strip.Color(0, 120, 120),
    strip.Color(120, 120, 0),
    strip.Color(0, 120, 0)
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
        ballLaunch(getRandomBallColor());
        ignoreStartButtonCounter = 0;
      }
    }
  }

  updateBalls();
//  lightPath(path0, strip.Color(20,20,20));
  counter++;
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void ballLaunch(uint32_t color) {
  Serial.println("Launching!!");
  for (int i=0; i<MAX_BALLS; i++) {
    if (ballPositions[i] == -1) {
      ballPaths[i] = random(0, 5);
      ballPositions[i] = 0;
      ballColors[i] = getRandomBallColor();
      return;
    }
  }
}

void updateBalls() {
  for (int i=0; i<MAX_BALLS; i++) {
    
    //If the position is -1, the ball is inactive
    if (ballPositions[i] != -1) {
      int* path = getFlatPath(ballPaths[i]);
      int positionIdx = ballPositions[i];
      uint32_t color = ballColors[i];

      Serial.print("Ball: ");
      Serial.print(i);
      Serial.print(", Path: ");
      Serial.print(ballPaths[i]);
      Serial.print(", Position: ");
      Serial.print(positionIdx);
      Serial.print(", Pixel: ");
      Serial.println(path[positionIdx]);


      strip.setPixelColor(path[positionIdx], color);
      strip.setPixelColor(path[positionIdx - 1], strip.Color(10, 10, 10));
      strip.setPixelColor(path[positionIdx - 2], strip.Color(0, 0, 0));

      //increment the position index
      ballPositions[i]++;

      //The terminal element of each path should be -1
      // This will set the ball to be 'inactive' and free up a slot for another ball
      if (path[positionIdx] == -1) {
        ballPaths[i] = -1;
        ballPositions[i] = -1;
        ballColors[i] = strip.Color(0, 0, 0);
        strip.setPixelColor(path[positionIdx - 1], strip.Color(0, 0, 0));
        strip.setPixelColor(path[positionIdx - 2], strip.Color(0, 0, 0));
        strip.setPixelColor(path[positionIdx - 3], strip.Color(0, 0, 0));
      }
    }
  }

  strip.show();
}

void lightPath() {
  int index = 0;
  while (path0[index] != -1) {
    strip.setPixelColor(path0[index], strip.Color(10, 10, 10));
    index++;
  }
  strip.show();
  delay(30000);
}
