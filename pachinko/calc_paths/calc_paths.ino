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

const static int segments[][2] = {
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
};

//Each element of this array corresponds to a segment above, and lists is "connecting segments"
// -1 is a "non-prize" terminal
// -2 is a "tulip prize"
// -3 is the "center prize"
const static int connections[][MAX_CONNECTIONS] = {
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
  {             -1 }, //24
};

static int connectionCounts[sizeof(connections)/sizeof(int[MAX_CONNECTIONS])];
static int numSegments = sizeof(segments) / sizeof(int[2]);

char data[100];

const unsigned char START_BUTTON_PIN = 8;         //Not PWM
unsigned char startButtonState = HIGH;

//Which segments can be starting points
int startingSegments[] = { 0, 1, 6 };

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
  sprintf_P(data, PSTR("Lighting Segment %d of %d"), seg, numSegments);
  Serial.println(data);
  
  if (segments[seg][1] > 0) {
    strip.fill(forward, segments[seg][0], segments[seg][1]);
  } else {
    strip.fill(backward, segments[seg][0] + segments[seg][1], abs(segments[seg][1]));
  }
  strip.show();

  if (segDepth > 0) {
    int segIdx = 0;
    sprintf_P(data, PSTR("Current Segment is %d, Depth is %d. Lighting next segment: idx %d, %d"), seg, segDepth, segIdx, connections[seg][segIdx]);
    Serial.println(data);
    while (connections[seg][segIdx] >= 0) {
      sprintf_P(data, PSTR("Lighting segment idx %d, Segment %d, Depth %d"), segIdx, connections[seg][segIdx], segDepth - 1);
      Serial.println(data);
      lightSegment(connections[seg][segIdx], segDepth - 1);
      segIdx++;
    }
  } else {
    Serial.println(F("Depth 0: Pathway Terminated"));
  }
}
