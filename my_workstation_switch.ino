
const int led = LED_BUILTIN;
const int buttonPin = 7;
const int relay_count = 2;
const int relays[relay_count] = {8, 9};
int relays_output_state[relay_count] = {1, 1};

void pciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin)); // enable pin
  PCIFR |= bit(digitalPinToPCICRbit(pin));                   // clear any outstanding interrupt
  PCICR |= bit(digitalPinToPCICRbit(pin));                   // enable interrupt for the group
}

void relaySetup(){
  for (int i = 0; i < relay_count; i++){
    pinMode(relays[i], OUTPUT);
  }
}

void updateRelayOutput(){
  for (int i = 0; i < relay_count; i++){
    digitalWrite(relays[i], relays_output_state[i]);
  }
}

// volatile int buttonPressed = 0;
const unsigned long debounceTime = 60;
volatile int pressedCount = 0;
unsigned long buttonCheckMillis = 0;

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here
 {
    static unsigned long previousStateChangeMillis = 0;
    static int previousPinState = HIGH;

    int pinState = digitalRead(buttonPin);
    digitalWrite(led, pinState);
    if (pinState != previousPinState) { // ignore pin changes of pins other than SELECTOR_BTN
      if (pinState == LOW) { // only falling events
        if ((millis() - previousStateChangeMillis) > debounceTime) { // debounce
          // buttonPressed = 1;
          buttonCheckMillis = millis();
          pressedCount++;
        }
      }
      previousPinState = pinState;
      previousStateChangeMillis = millis();
    }
 }  

void setup()
{
	pinMode(led, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(6, OUTPUT); //10K Pull up resistor on pin 6
  digitalWrite(6, HIGH);
  Serial.begin(115200);
  pciSetup(buttonPin);
  relaySetup();
  updateRelayOutput();
}

void loop()
{
  if (pressedCount > 0){
    if (millis() - buttonCheckMillis >= 1000){
      buttonCheckMillis = millis();
      Serial.print("Pressed count: ");
      Serial.println(pressedCount);
      
      switch (pressedCount)
      {
      case 1:
        relays_output_state[1] = !relays_output_state[1];
        break;
      case 2:
        relays_output_state[0] = !relays_output_state[0];
        break;
      default:
        break;
      }
      pressedCount = 0;

      updateRelayOutput();
    }
  }
  
  // delay(100);
}
