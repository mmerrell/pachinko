#include <Adafruit_NeoPixel.h>
#include <cppQueue.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define WAIT 5
#define MAX_BALLS 7


const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char start_button_state = HIGH;


//segments
//segment 0:  {  0,  75} - connects to 1
//segment 1:  {120,  34} - connects to 2
//segment 2:  {210,  30} - connects to 3
//segment 3:  {100,  20} - Terminal

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(240, PIN, NEO_GRB + NEO_KHZ800);


uint32_t colorAry[5] = {
  strip.Color(255, 255, 255),
  strip.Color(255, 0, 255),
  strip.Color(0, 255, 255),
  strip.Color(255, 255, 0),
  strip.Color(0, 255, 0)
};

int path0[][2] = {
  {0, 120}
};

int path1[][2] = {
               {0, 74},
               {170, 69},
               {0, 1},
               {0, 1}
             };

int path2[][2] = {
  {0, 65}
};



class Path{
  private:
    int numSegments;
    int (*segments)[2];
    int *flatPath;
    int flatPathLength;

  public:
    Path(int numSegments, int (*segments)[2]) {
      this->numSegments = numSegments;
      this->segments = segments;

      
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
    Ball() {}
    
    Ball(uint32_t color) {
      this->active = true;
      this->color = color;
      this->currPosition = 0;

      int (*path)[2] = path1;
      int ballPathIdx = random(0, 3);
//
//      Serial.print("Choosing path: path");
//      Serial.println(ballPathIdx);
//      
//      if (ballPathIdx % 3 == 0) {
//        path = path0;
//      }
//      if (ballPathIdx % 3 == 1) {
//        path = path1;
//      }
//      if (ballPathIdx % 3 == 2) {
//        path = path2;
//      }
      
      int tempFlatPathLength = 0;
 
      Serial.print("sizeof *path0: ");
      Serial.println(sizeof((*path0)));
      Serial.print("sizeof *path1: ");
      Serial.println(sizeof(*path1));
      Serial.print("sizeof *path2: ");
      Serial.println(sizeof(*path2));

//      Serial.print("sizeof *path");
//      Serial.print(ballPathIdx);
//      Serial.print(": ");
//      Serial.println(sizeof(*path));

      int cols = sizeof(*path);
      for (int i=0; i<cols; i++) {
        tempFlatPathLength += abs(path[i][1]);
      }
      this->flatPathLength = tempFlatPathLength;

      Serial.print("Segments: ");
      Serial.println(cols);
      Serial.print("Flat path length: ");
      Serial.println(this->flatPathLength);

      int flatPath[sizeof(int) * flatPathLength];

      //Flatten the path for this ball
      // Do it once at the beginning, so nothing needs to be calculated later
      int flatIndex=0;
      
      for (int i=0; i<cols; i++) {
        Serial.print("Segment: ");
        Serial.print(i);
        Serial.print(", Segment First Pixel: ");
        Serial.print(path[i][0]);
        Serial.print(", Segment Length: ");
        Serial.println(path[i][1]);
        for (int index=0; index<path[i][1]; index++) {
          int value = path[i][0] + index;
          flatPath[flatIndex] = value;
//          Serial.print("Flat array index: ");
//          Serial.println(flatIndex);
//          Serial.print("Segment index: ");
//          Serial.print(index);
//          Serial.print(", Pixel value: ");
//          Serial.println(value);
          flatIndex++;
        }
      }
      
      this->flatPath = flatPath;
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
//      Serial.print("Current position: ");
//      Serial.print(currPosition);
//      Serial.print(", ");
//      Serial.println(flatPath[currPosition]);
      currPosition++;
      if (currPosition >= flatPathLength) {
        this->active = false;
//        Serial.print("Setting ball inactive: ");
//        Serial.println(this->active);
//        Serial.print("Last pixel lit: ");
//        Serial.println(flatPath[currPosition - 1]);
        return;
      }
    }
};

//Init all the pins
void init_io_pins() {
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
}

void update_io_pins() {
  start_button_state = digitalRead(START_BUTTON_PIN);
//  Serial.print("Start Button State: ");
//  Serial.println(start_button_state);
}


Queue balls(sizeof(Ball), MAX_BALLS, FIFO, false);

void setup() {
  Serial.begin(9600);

  int cols0 = sizeof(path0) / sizeof(int[2]);
  Serial.print("path0 columns: ");
  Serial.println(cols0);

  int cols1 = sizeof(path1) / sizeof(int[2]);
  Serial.print("path1 columns: ");
  Serial.println(cols1);

  int cols2 = sizeof(path2) / sizeof(int[2]);
  Serial.print("path2 columns: ");
  Serial.println(cols2);

  init_io_pins();


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  ballLaunch(strip.Color(255, 255, 255));
}

double counter = 0;
double ignore_start_button_counter = 0;
boolean start_button_low;

void loop() {
  update_io_pins();
  if (ignore_start_button_counter >= 50) {
    if (start_button_state == LOW) {
      start_button_low = true;
    } else {
      if (start_button_low) {
        start_button_low = false;
        ballLaunch(colorAry[0]);
//        ballLaunch(colorAry[random(0, 5)]);
        ignore_start_button_counter = 0;
      }
    }
  }

  updateBalls();
  updateLights();
//  lightPath(path0, strip.Color(20,20,20));
  counter++;
  ignore_start_button_counter++;
  delay(WAIT);
}

void updateBalls() {
//  Serial.println("UPDATE BALLS");
  if (balls.isEmpty()) {
//    Serial.println("Nothing to do. No balls");
    ballLaunch(colorAry[0]);
    return;
  }
  
  Ball ball;
  for (int i=0; i<balls.getCount(); i++) {
    if (balls.pop(&ball)) {
  //    Serial.print("ball isActive before if statement: ");
  //    Serial.println(ball.isActive());
      if (ball.isActive()) {
        ball.inc();
  //      Serial.print("ball isActive after increment: ");
  //      Serial.println(ball.isActive());
      }
    }
//      Serial.println("Pushing ball back onto queue");
    balls.push(&ball);
  }
}

void updateLights() {
    for (int i=0; i<balls.getCount(); i++) {
      Ball ball;
      if (balls.pop(&ball)) {
        if (ball.isActive()) {
//          Serial.println("Lighting up");
          strip.setPixelColor(ball.getCurrPixel(), ball.getColor());
          strip.setPixelColor(ball.getPrevPixel(), strip.Color(10, 10, 10));
          strip.setPixelColor(ball.getTrailingPixel(), strip.Color(0, 0, 0));
          balls.push(&ball);
        } else {
//          Serial.println("Shutting down");
          strip.setPixelColor(ball.getCurrPixel(), strip.Color(0, 0, 0));
          strip.setPixelColor(ball.getPrevPixel(), strip.Color(0, 0, 0));
          strip.setPixelColor(ball.getTrailingPixel(), strip.Color(0, 0, 0));
        }
      }
    }

    strip.show();
}

void ballLaunch(uint32_t color) {
  Ball ball(color);
  balls.push(&ball);
}

//void lightPath(int path[][2], uint32_t color) {
//    for (int seg=0; seg<1; seg++) {
//      for(uint16_t px=path0[seg][0]; px<(path0[seg][0] + path0[seg][1]); px++) {
//        strip.setPixelColor(px, color);
//      }
//  }
//  strip.show();
//  delay(1000);
//}



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
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}




//Refresher on C++ OOP
//class Segment {
//  private:
//    int firstPixel;
//    int segmentLength;
//    Segment *segments;
//
//    int locatePixelInSegment(int ballPosition) {
//      if ((ballPosition + firstPixel) <= (firstPixel + segmentLength)) {
//        return ballPosition + firstPixel;
//      } else {
//        return -1;
//      }
//    }
//
//  public:
//    Segment(int firstPixel, int segmentLength) {
//      this->firstPixel = firstPixel;
//      this->segmentLength = segmentLength;
//      this->segments = NULL;
//    }   
//     
//    Segment(int firstPixel, int segmentLength, Segment *segment) {
//      this->firstPixel = firstPixel;
//      this->segmentLength = segmentLength;
//      this->segments = &segments[0];
//    }
//
//    void lightPixel(int ballPosition, uint32_t color) {
//      int currPixel = locatePixelInSegment(ballPosition);
//      if (currPixel == -1) {
//        //panic!
//      } else {
//        strip.setPixelColor(locatePixelInSegment(ballPosition), color);
//        strip.setPixelColor(currPixel - 1, strip.Color(0, 0, 0));
//      }
//    }
//
//    int getLength() {
//      return segmentLength;
//    }
//    
//    int getLastPosition() {
//      return firstPixel + segmentLength;
//    }
//
//    Segment getNextSegment() {
//      return segments[0]; //this will eventually be randomized
//    }
//    
//};
//
////Segment SEGMENT_3(130, 15);
//Segment SEGMENT_1(120, 50);
////
////Segment seg_3_connectors[1] = { SEGMENT_1 };
////
////Segment SEGMENT_2(96, 15, seg_3_connectors);
//Segment seg_0_connectors[1] = { SEGMENT_1 };
//
//Segment SEGMENT_0(0, 75, seg_0_connectors);
////class Path {
////  private: 
////    Segment *segments;
////
////  public:
////    Path() {
////    }
////}
//
