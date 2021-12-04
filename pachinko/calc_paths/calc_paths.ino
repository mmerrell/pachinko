#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN        6
#define WAIT                20
#define BRIGHTNESS          123
#define TOTAL_PIXELS        360
#define TIME_BETWEEN_LAUNCH 2
#define MAX_CONNECTIONS     4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

const int segments[][2] PROGMEM = {
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
  {   0,   16 }, //28
  {  17,  -17 }, //29
};

//Each element of this array corresponds to a segment above, and lists is "connecting segments"
// -1 is a "non-prize" terminal
// -2 is a "tulip prize"
// -3 is the "center prize"
const char connections[][MAX_CONNECTIONS] PROGMEM = {
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
  { 29,         -1 }, //28
  {             -1 }, //29 
};

static int connectionCounts[sizeof(connections)/sizeof(int[MAX_CONNECTIONS])];
static int numSegments = sizeof(segments) / sizeof(int[2]);

char data[100];

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

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
  //build the array that keeps track of how many connections each segment has
  for (int i=0; i<28; i++) {
    for (int j=0; j<MAX_CONNECTIONS; j++) {
      int segment = pgm_read_word(&connections[i][j]);
      if (segment < 0) {
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
  Serial.println(F("Ready..."));

  initIoPins();
  initArrays();
  initLights();
  readyAnimation();
  lightSegment(0, 1);
}

int segDepth = 1;
int segment = 0;
String input = "";
void loop() {
  updateIoPins();
  if (ignoreStartButtonCounter >= TIME_BETWEEN_LAUNCH) {
    if (startButtonState == LOW) {
      startButtonLow = true;
    } else {
      if (startButtonLow) {
        startButtonLow = false;
        strip.clear();
        sprintf_P(data, PSTR("\nBUTTON PUSHED: LIGHTING SEGMENT %d"), segment);
        Serial.println(data);
        if (segment >= numSegments) {
          segment = 0;
        }
        lightSegment(segment, segDepth);
        segment++;

        ignoreStartButtonCounter = 0;
      }
    }
  }

  ignoreStartButtonCounter++;
  delay(WAIT);
}

uint32_t forward = strip.Color(0, 100, 0);
uint32_t backward = strip.Color(100, 0, 0);
void lightSegment(int seg, int segDepth) {
  int start = pgm_read_word(&segments[seg][0]);
  int segLength = pgm_read_word(&segments[seg][1]);
  sprintf_P(data, PSTR("Lighting Segment %d:\n\t{ %d, %d }, depth %d"), seg, start, segLength, segDepth);
  Serial.println(data);
  
  if (segLength > 0) {
    strip.fill(forward + (segDepth + (seg * 10000)), start, segLength);
  } else {
    strip.fill(backward - (segDepth + (seg * 10000)), start + segLength, abs(segLength));
  }
  strip.show();

  if (segDepth > 0) {
    if (segDepth == 0) {
      Serial.println(F("segDepth is 0. I don't know how we made it to this block"));
    } else {
      Serial.println(segDepth);
    }
    int segIdx = 0;
    char segConnection = pgm_read_word(&connections[seg][segIdx]);
    sprintf_P(data, PSTR("Current Segment is %d, Depth is %d. Lighting next segment: idx %d, %d"), seg, segDepth, segIdx, segConnection);
    Serial.println(data);
    while (segConnection >= 0) {
      sprintf_P(data, PSTR("Lighting %d: Segment %d, Depth %d"), segIdx, segConnection, segDepth - 1);
      Serial.println(data);
      lightSegment(segConnection, 0);
      segIdx++;
    }
  } else {
    Serial.println(F("Depth 0: Pathway Terminated"));
  }
}
