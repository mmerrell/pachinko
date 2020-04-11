#define MAX_PATHS 5
#define MAX_SEGMENTS 5

//GOOD
int paths[MAX_PATHS][5][2] = { 
  {
    {0, 120}
  },
  {
    {0, 73},
    {167, -14},
    {174, 27},
    {221, 19}
  },
  {
    {0, 73},
    {167, -47}
  },
  {
    {0, 78},
    {77, -5},
    {168, 33},
    {221, 19}
  },
  {
    {0, 78},
    {77, -5},
    {168, 6},
    {153, -30}
  } 
};



void setup() {
  Serial.begin(9600);

  for (int pathIdx=0; pathIdx<MAX_PATHS; pathIdx++) {
    int newlineCounter = 0;
    Serial.print("  int pathAry");
    Serial.print(pathIdx);
    Serial.print("[] = { ");
    for (int segIdx=0; segIdx<MAX_SEGMENTS; segIdx++) {
      if (paths[pathIdx][segIdx][1] < 0) {
        for (int index=0; index>paths[pathIdx][segIdx][1]; index--) {
          int value = paths[pathIdx][segIdx][0] + index;
          Serial.print(value);
          Serial.print(", ");
          if (newlineCounter>=12) {
            Serial.println("");
            Serial.print("    ");
            newlineCounter = 0;
          }
          newlineCounter++;

        }
      } else {
        for (int index=0; index<paths[pathIdx][segIdx][1]; index++) {
          int value = paths[pathIdx][segIdx][0] + index;
          Serial.print(value);
          Serial.print(", ");
          if (newlineCounter>=12) {
            Serial.println("");
            Serial.print("    ");
            newlineCounter = 0;
          }
          newlineCounter++;
        }        
      }
    }
    Serial.println("-1 };");
    Serial.println("");
  }
}

void loop() {
  delay(50000);
}
