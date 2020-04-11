#include <Adafruit_NeoPixel.h>
#include <LinkedList.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 6
#define WAIT 5
#define MAX_BALLS 7
#define TOTAL_PATHS 5
#define TIME_BETWEEN_LAUNCH 50  //This is the number of loops, not the actual time

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(240, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//GOOD
int path0[][2] = {
  {0, 120}
};

//GOOD
int path1[][2] = {
  {0, 73},
  {167, -14},
  {174, 27},
  {221, 19}
};

//GOOD
int path2[][2] = {
  {0, 73},
  {167, -47}
};

//GOOD
int path3[][2] = {
  {0, 78},
  {77, -5},
  {168, 33},
  {221, 19}
};

//GOOD
int path4[][2] = {
  {0, 78},
  {77, -5},
  {168, 6},
  {153, -30}
};

class Path {
  private:
    int numSegments;
    int (*segments)[2];
    int flatPath[200];
    int flatPathLength;

  public:
    Path(int numSegments, int (*segments)[2]) {
      this->numSegments = numSegments;
      this->segments = segments;

      flatPathLength = 0;
      for (int i=0; i<numSegments; i++) {
        flatPathLength += abs(segments[i][1]);
      }
      this->flatPathLength = flatPathLength;

      Serial.print("Segments: ");
      Serial.println(numSegments);
      Serial.print("Flat path length: ");
      Serial.println(this->flatPathLength);

      //Flatten the path for this ball
      // Do it once at the beginning, so nothing needs to be calculated later
      int flatIndex=0;
      
      for (int i=0; i<numSegments; i++) {
        if (segments[i][1] < 0) {
          for (int index=0; index>segments[i][1]; index--) {
            int value = segments[i][0] + index;
            flatPath[flatIndex] = value;
            flatIndex++;
          }
        } else {
          for (int index=0; index<segments[i][1]; index++) {
            int value = segments[i][0] + index;
            flatPath[flatIndex] = value;
            flatIndex++;
          }
        }
      }
    }

    int* getFlatPath() {
      return flatPath;
    }

    int getFlatPathLength() {
      return flatPathLength;
    }

};

class Ball {
  private:
    int currPosition;                        //The index of the segment this ball in on
    uint32_t color = strip.Color(0, 0, 255); //The (initial) color of the "ball"
    int effect;                              //This will be some color-changing effect
    int *flatPath;                           //
    int flatPathLength;                      //
    const int numSegments = 1;
    boolean active;                          //Whether or not this ball is currently active
  
  public:
    Ball() {
      this->currPosition = 0;  
    }
    
    Ball(uint32_t color) {
      this->active = true;
      this->color = color;
      this->currPosition = 0;


      
      //This is all embarrassing. I need to learn how to work with pointers
      int (*pathPtr)[2];
      int pathSegments;
//      int ballPathSelector = 3;
      int ballPathSelector = random(0, TOTAL_PATHS);

      Serial.print("Choosing path: path");
      Serial.println(ballPathSelector);
      if (ballPathSelector % TOTAL_PATHS == 0) {
        pathPtr = path0;
        pathSegments = sizeof(path0) / sizeof(int[2]);
      }
      if (ballPathSelector % TOTAL_PATHS == 1) {
        pathPtr = path1;
        pathSegments = sizeof(path1) / sizeof(int[2]);
      }
      if (ballPathSelector % TOTAL_PATHS == 2) {
        pathPtr = path2;
        pathSegments = sizeof(path2) / sizeof(int[2]);
      }

      if (ballPathSelector % TOTAL_PATHS == 3) {
        pathPtr = path3;
        pathSegments = sizeof(path3) / sizeof(int[2]);
      }

      if (ballPathSelector % TOTAL_PATHS == 4) {
        pathPtr = path4;
        pathSegments = sizeof(path4) / sizeof(int[2]);
      }

      //Embarassing part over
//        pathPtr = path1;
//        pathSegments = sizeof(path1) / sizeof(int[2]);


      
      
      Path path(pathSegments, pathPtr);
      this->flatPath = path.getFlatPath();
      this->flatPathLength = path.getFlatPathLength();
//      Serial.print("INSIDE BALL: Flat path length: ");
//      Serial.println(path.getFlatPathLength());
    }

    boolean isActive() {
      return active;
    }

    int getCurrPosition() {
      return this->currPosition;
    }

    int getCurrPixel() {
      return this->flatPath[currPosition];
    }

    int getPrevPixel() {
      return this->flatPath[currPosition - 1];
    }

    int getTrailingPixel() {
      return this->flatPath[currPosition - 2];
    }

    uint32_t getColor() {
      return color;
    }

    void printVitals() {
      Serial.print("Current position: ");
      Serial.print(currPosition);
      Serial.print(", ");
      Serial.println(flatPath[currPosition]);
    }
    
    void printFlatPath() {
      Serial.print("flatPathLength: ");
      Serial.println(flatPathLength);
      for (int i=0; i<flatPathLength; i++) {
        Serial.print("index: ");
        Serial.print(i);
        Serial.print(", value: ");
        Serial.println(flatPath[i]);
      }
    }


    void inc() {
      currPosition++;
      if (currPosition >= flatPathLength) {
        this->active = false;
        return;
      }
    }
};


void lightPath() {
  Path pathObj(sizeof(path4)/sizeof(int[2]), path4);
  int *flatPath = pathObj.getFlatPath();
  for(int i=0; i<pathObj.getFlatPathLength(); i++) {
    strip.setPixelColor(flatPath[i], strip.Color(10, 10, 10));
  }
  strip.show();
  delay(30000);
}

//Init all the pins
void initIoPins() {
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
}

void initLights() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void updateIoPins() {
  startButtonState = digitalRead(START_BUTTON_PIN);
}


LinkedList<Ball*> ballList = LinkedList<Ball*>();

void setup() {
//  Serial.begin(9600);

  initIoPins();
  initLights();
}

double counter = 0;
double ignoreStartButtonCounter = 0;
boolean startButtonLow;

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
  updateLights();
//  lightPath(path0, strip.Color(20,20,20));
  counter++;
  ignoreStartButtonCounter++;
  delay(WAIT);
}

void updateBalls() {
  if (!ballList.size()) {
    return;
  }

  Ball *ball;
  int ballCount = ballList.size();
  for (int i=0; i<ballCount; i++) {
    ball = ballList.get(i);
    ball->printVitals();

    if (ball->isActive()) {
      ball->inc();
    }
  }
}


void updateLights() {
  int ballCount = ballList.size();
  Ball *ball;

  for (int i=0; i<ballCount; i++) {
    ball = ballList.get(i);
    if (ball->isActive()) {
      strip.setPixelColor(ball->getCurrPixel(), ball->getColor());
      strip.setPixelColor(ball->getPrevPixel(), strip.Color(10, 10, 10));
      strip.setPixelColor(ball->getTrailingPixel(), strip.Color(0, 0, 0));
    } else {
      Serial.println("Shutting down the last few pixels");
      strip.setPixelColor(ball->getCurrPixel(), strip.Color(0, 0, 0));
      strip.setPixelColor(ball->getPrevPixel(), strip.Color(0, 0, 0));
      strip.setPixelColor(ball->getTrailingPixel(), strip.Color(0, 0, 0));
      ballList.remove(i);
      delete(ball);
    }
  }

  strip.show();
}

void ballLaunch(uint32_t color) {
  Ball *ball = new Ball(color);
  ballList.add(ball);
  Serial.println("Launching!!");
}
