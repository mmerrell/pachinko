#include <Adafruit_NeoPixel.h>
#include <cppQueue.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 6
#define WAIT 5
#define MAX_BALLS 5
#define TOTAL_PATHS 5


const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char start_button_state = HIGH;

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

//      int flatPath[sizeof(int) * flatPathLength];

      //Flatten the path for this ball
      // Do it once at the beginning, so nothing needs to be calculated later
      int flatIndex=0;
      
      for (int i=0; i<numSegments; i++) {
//        Serial.print("Segment: ");
//        Serial.print(i);
//        Serial.print(", Segment First Pixel: ");
//        Serial.print(segments[i][0]);
//        Serial.print(", Segment Length: ");
//        Serial.println(segments[i][1]);
        if (segments[i][1] < 0) {
          for (int index=0; index>segments[i][1]; index--) {
            int value = segments[i][0] + index;
            flatPath[flatIndex] = value;
//            Serial.print("Flat array index: ");
//            Serial.println(flatIndex);
//            Serial.print("Segment index: ");
//            Serial.print(index);
//            Serial.print(", Pixel value: ");
//            Serial.print(value);
//            Serial.print(", Real Pixel value: ");
//            Serial.println(flatPath[flatIndex]);
            flatIndex++;
          }
        } else {
          for (int index=0; index<segments[i][1]; index++) {
            int value = segments[i][0] + index;
            flatPath[flatIndex] = value;
//            Serial.print("Flat array index: ");
//            Serial.println(flatIndex);
//            Serial.print("Segment index: ");
//            Serial.print(index);
//            Serial.print(", Pixel value: ");
//            Serial.print(value);
//            Serial.print(", Real Pixel value: ");
//            Serial.println(flatPath[flatIndex]);
            flatIndex++;
          }
        }
      }
      
//      this->flatPath = flatPath;
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
    int ballPathSelector;
  
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
      this->ballPathSelector = random(0, TOTAL_PATHS);

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
        strip.setPixelColor(getCurrPixel(), strip.Color(0, 0, 0));
        strip.setPixelColor(getPrevPixel(), strip.Color(0, 0, 0));
        strip.setPixelColor(getTrailingPixel(), strip.Color(0, 0, 0));
//        strip.show();
        return;
      }
      strip.setPixelColor(getCurrPixel(), getColor());
      strip.setPixelColor(getPrevPixel(), strip.Color(10, 10, 10));
      strip.setPixelColor(getTrailingPixel(), strip.Color(0, 0, 0));
//      strip.show();
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
void init_io_pins() {
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
}

void update_io_pins() {
  start_button_state = digitalRead(START_BUTTON_PIN);
}


Queue ballQueue(sizeof(Ball), MAX_BALLS, FIFO, true);

void setup() {
  Serial.begin(9600);

  init_io_pins();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

//  lightPath();
}

double counter = 0;
double ignore_start_button_counter = 0;
boolean start_button_low;

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
  update_io_pins();
  if (ignore_start_button_counter >= 50) {
    if (start_button_state == LOW) {
      start_button_low = true;
    } else {
      if (start_button_low) {
        start_button_low = false;
        ballLaunch(getRandomBallColor());
        ignore_start_button_counter = 0;
      }
    }
  }

  updateBalls();
//  lightPath(path0, strip.Color(20,20,20));
  counter++;
  ignore_start_button_counter++;
  strip.show();
  delay(WAIT);
}

void updateBalls() {
  if (ballQueue.isEmpty()) {
    return;
  }
  
  int ballCount = ballQueue.getCount();
  Ball balls[ballCount];
  
  for (int i=0; i<ballCount; i++) {
    ballQueue.pop(&balls[i]);
  }

  for (int i=0; i<ballCount; i++) {
    if (balls[i].isActive()) {
      balls[i].inc();
    }
  }

  //push it back into the queue no matter what--updateLights() has the job of removing balls permanently
  for (int i=0; i<ballCount; i++) {
    ballQueue.push(&balls[i]);
  }
}

void ballLaunch(uint32_t color) {
  Ball ball(color);
  ballQueue.push(&ball);
  Serial.println("Launching!!");
}


//void singleBall(uint32_t c) {
//  for (int seg=0; seg<4; seg++) {
//    for(uint16_t px=path0[seg][0]; px<(path0[seg][0] + path0[seg][1]); px++) {
//      strip.setPixelColor(px, c);
//      strip.setPixelColor(px-1, strip.Color(10, 10, 10));
//      strip.setPixelColor(px-2, strip.Color(0, 0, 0));
//      strip.show();
//      delay(WAIT);
//    }
//    int px = path0[seg][0] + path0[seg][1];
//    strip.setPixelColor(px-1, strip.Color(0, 0, 0));
//    strip.setPixelColor(px-2, strip.Color(0, 0, 0));
//    strip.show();
//  }
//}





//  colorWipe(strip.Color(255, 0, 0), WAIT); // Red
//  colorWipe(strip.Color(0, 255, 0), WAIT); // Green
//  colorWipe(strip.Color(0, 0, 255), WAIT); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), WAIT); // White RGBW
  // Send a theater pixel chase in...
//  theaterChase(strip.Color(127, 127, 127), WAIT); // White
//  theaterChase(strip.Color(127, 0, 0), WAIT); // Red
//  theaterChase(strip.Color(0, 0, 127), WAIT); // Blue

//  rainbow(20);
//  rainbowCycle(20);
//  theaterChaseRainbow(50);

// Fill the dots one after the other with a color
//void colorWipe(uint32_t c, uint8_t wait) {
//  for(uint16_t i=0; i<strip.numPixels(); i++) {
//    strip.setPixelColor(i, c);
//    strip.show();
//    delay(wait);
//  }
//}
//
//void rainbow(uint8_t wait) {
//  uint16_t i, j;
//
//  for(j=0; j<256; j++) {
//    for(i=0; i<strip.numPixels(); i++) {
//      strip.setPixelColor(i, Wheel((i+j) & 255));
//    }
//    strip.show();
//    delay(wait);
//  }
//}
//
//// Slightly different, this makes the rainbow equally distributed throughout
//void rainbowCycle(uint8_t wait) {
//  uint16_t i, j;
//
//  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//    for(i=0; i< strip.numPixels(); i++) {
//      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
//    }
//    strip.show();
//    delay(wait);
//  }
//}
//
////Theatre-style crawling lights.
//void theaterChase(uint32_t c, uint8_t wait) {
//  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
//    for (int q=0; q < 3; q++) {
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, c);    //turn every third pixel on
//      }
//      strip.show();
//
//      delay(wait);
//
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, 0);        //turn every third pixel off
//      }
//    }
//  }
//}
//
////Theatre-style crawling lights with rainbow effect
//void theaterChaseRainbow(uint8_t wait) {
//  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
//    for (int q=0; q < 3; q++) {
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
//      }
//      strip.show();
//
//      delay(wait);
//
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, 0);        //turn every third pixel off
//      }
//    }
//  }
//}
//
//// Input a value 0 to 255 to get a color value.
//// The colours are a transition r - g - b - back to r.
//uint32_t Wheel(byte WheelPos) {
//  WheelPos = 255 - WheelPos;
//  if(WheelPos < 85) {
//    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//  }
//  if(WheelPos < 170) {
//    WheelPos -= 85;
//    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//  }
//  WheelPos -= 170;
//  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
//}
