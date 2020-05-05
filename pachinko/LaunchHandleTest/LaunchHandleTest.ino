#define POSITION_0_PIN 10
#define POSITION_1_PIN 11
#define POSITION_2_PIN 12
#define POSITION_3_PIN 13

void setup() {
  Serial.begin(115200);
  Serial.println(F("Ready..."));
  pinMode(POSITION_0_PIN, INPUT_PULLUP);
  pinMode(POSITION_1_PIN, INPUT_PULLUP);
  pinMode(POSITION_2_PIN, INPUT_PULLUP);
  pinMode(POSITION_3_PIN, INPUT_PULLUP);
}

int offPosition = LOW;
int prevOffPosition = LOW;
bool launching = false;
byte ballSpeed = 0;

void loop() {
  offPosition=digitalRead(POSITION_0_PIN);

  //if the handle is pulled back, measure the highest Hall sensor it's triggered
  if (launching) {
    if (offPosition == LOW) {
      //Using the "highest Hall triggered" number, launch the ball at a certain velocity
      Serial.print(F("Ball Speed: "));
      Serial.print(ballSpeed, BIN);
      Serial.println(F("\nLAUNCHING!!!"));
      //ballLaunch(ballSpeed);//color, effect, speed
      ballSpeed = 0;
      launching = false;
    }
    //Only need to check these switches if we're "underway"
    int position1 = digitalRead(POSITION_1_PIN);
    int position2 = digitalRead(POSITION_2_PIN);
    int position3 = digitalRead(POSITION_3_PIN);

    if (position3 == LOW) {
      ballSpeed |= 4; //00000100
      Serial.println(F("POSITION 3"));
    }
    if (position2 == LOW) {
      ballSpeed |= 2; //00000010
      Serial.println(F("POSITION 2"));
    }
    if (position1 == LOW) {
      ballSpeed |= 1; //00000001
      Serial.println(F("POSITION 1"));
    }
  } else {
    if (offPosition == LOW) { //offPosition LOW means the handle is AT REST
      if (prevOffPosition == HIGH) {
        //The handle has been brought back to the lowest position
        Serial.println(F("OFF"));
        prevOffPosition = LOW;
      }
      //Do nothing--we're waiting for the handle to be pulled back
    } else {
      //Launching - if they let go of the handle before it hits another magnet, it will be the slowest speed
      Serial.println(F("POSITION 0"));
      prevOffPosition = HIGH;
      launching = true;
    }
  }

  
  
  delay(10);
}
