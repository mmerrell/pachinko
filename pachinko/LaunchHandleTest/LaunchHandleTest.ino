void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
}

int val2 = 0;
int val3 = 0;
int offPosition = LOW;
int prevOffPosition = LOW;
bool launching = false;
byte highestLaunchPosition = 0;

void loop() {
  val2 = digitalRead(2);
  val3 = digitalRead(3);
  offPosition= digitalRead(4);
  
  if (val2 == LOW) {
    Serial.println(F("LEFT"));
  }
  
  if (val3 == LOW) {
    Serial.println(F("CENTER"));
  }

  //if the handle is pulled back, measure the highest Hall sensor it's triggered
  if (launching) {

    if (
    launching = false;
    //Using the "highest Hall triggered" number, launch the ball at a certain velocity
    //ballLaunch();//color, effect, speed
    highestLaunchPosition = 0;
    if (offPosition == LOW) {
    }
  } else {
    if (offPosition == LOW) { // offPosition LOW means that the handle is DOWN
      if (prevOffPosition == HIGH) {
        //The handle has been brought back to the lowest position
        Serial.println(F("OFF"));
        prevOffPosition = LOW;
      }
    } else {
      //Launching
      Serial.println(F("LAUNCH"));
      prevOffPosition = HIGH;
      launching = true;
    }
  }

  
  
  delay(10);
}
