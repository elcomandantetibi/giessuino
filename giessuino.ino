#define RELAY_PIN 9
#define BUTTON_PIN 8
#define LED_PIN 13

enum State {
  STANDBY=0,
  WATERING=1
};

State state;
unsigned long s_lastLogStateMillis=0;
unsigned long s_lastStateChangeMillis=0;
const unsigned long STANDBY_TIME_MILLIS=48L*3600L*1000L; // postfix all long literals with L to avoid overflow
const unsigned long WATERING_TIME_MILLIS=45L*1000L;

int lastButtonState=LOW;
int buttonState=LOW;
long lastDebounceTime=0;
long DEBOUNCE_DELAY_MS=50;

void setup() {
  pinMode(RELAY_PIN,OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  changeState(State::STANDBY);
}

bool buttonPressed() {
  int reading=digitalRead(BUTTON_PIN);
  
  if (reading!=lastButtonState) {
    lastDebounceTime=millis();
  }

  boolean fPressed=false;

  if ((millis()-lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (reading!=buttonState) {
      buttonState=reading;
      fPressed=(buttonState == HIGH);
    }
  }

  lastButtonState=reading;
  return fPressed;
}

void changeState(const State &s) {
  state=s;
  s_lastStateChangeMillis=millis();

  if (state==State::STANDBY) 
  {
    Serial.print("Entering STANDBY for ");
    Serial.print(STANDBY_TIME_MILLIS/1000L);
    Serial.print(" seconds\n");
  } 
  else
  {
    Serial.print("Entering WATERING for ");
    Serial.print(WATERING_TIME_MILLIS/1000L);
    Serial.print(" seconds\n");
  }
}

void logState() {
  if (millis()-s_lastLogStateMillis < 1000L)
    return;
  s_lastLogStateMillis=millis();
 
  Serial.print(state==State::WATERING? "Watering": "Waiting");
  Serial.print(" since ");
  Serial.print((millis()-s_lastStateChangeMillis)/1000L);

  unsigned long timeLeftMillis=(state==State::WATERING)? WATERING_TIME_MILLIS: STANDBY_TIME_MILLIS;
  Serial.print(" seconds ago, left ");
  Serial.print((timeLeftMillis-(millis()-s_lastStateChangeMillis))/1000L);
  Serial.println(" seconds");
}

void loop() {  
  if(buttonPressed()) {
    switch(state) {
    case State::STANDBY:
      changeState(State::WATERING);
      break;
    case State::WATERING:
      changeState(State::STANDBY);
      state=State::STANDBY;
      break;
    }
  }

  if (state==State::WATERING && (millis()-s_lastStateChangeMillis) >= WATERING_TIME_MILLIS) {
    changeState(State::STANDBY);
  } else if (state==State::STANDBY && (millis()-s_lastStateChangeMillis) >= STANDBY_TIME_MILLIS) {
    changeState(State::WATERING);
  }

  if (state==State::WATERING) {   
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(50);
}
   
