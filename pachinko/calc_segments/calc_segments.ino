
#include <Adafruit_NeoPixel.h>


#define NEOPIXEL_PIN        6
#define WAIT                20
#define BRIGHTNESS          123
#define TOTAL_PIXELS        360
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void initLights() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
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

void setup() {
  Serial.begin(9600);
  Serial.println(F("Ready..."));

  initLights();
  readyAnimation();
}

char data[100];
char incomingByte;
String input;
void loop() {
  // put your main code here, to run repeatedly:
  // send data only when you receive data:
  if (Serial.available() > 0) {
    int chars = Serial.available();
    for (int i=0; i<chars; i++) {
      // read the incoming byte:
      incomingByte = Serial.read();
      if (incomingByte != '\n') {
        input += incomingByte;
      }
    }
  }
  
  if (input.length() > 0) {
    int start = input.substring(0, 3).toInt();
    int segLength = input.substring(4).toInt();
    lightSegment(start, segLength);
    
    input = "";
  }
 
  delay(500);
}

uint32_t forward = strip.Color(0, 100, 0);
uint32_t backward = strip.Color(100, 0, 0);
void lightSegment(int start, int segLength) {
  strip.clear();
  sprintf_P(data, PSTR("Segment: { %d, %d }"), start, segLength);
  Serial.println(data);
  
  if (segLength > 0) {
    strip.fill(forward, start, segLength);
  } else {
    strip.fill(backward, start + segLength, abs(segLength));
  }
  strip.show();
}
