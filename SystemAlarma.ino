#include <Keypad.h>
#include <Servo.h>

#define PIR_PIN 2
#define RED_LED 13
#define GREEN_LED 12
#define BUZZER 8
#define SERVO_PIN 10

// PIN code
String correctPIN = "1234";
String enteredPIN = "";

// States
bool alarmActive = false;
bool buzzerActive = false;

unsigned long motionTime = 0;  // When motion was first detected

Servo lockServo;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);

  lockServo.attach(SERVO_PIN);
  lockServo.write(0); // locked position

  // Initial state
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  noTone(BUZZER);

  Serial.println("System ready.");
}

void loop() {

  int motion = digitalRead(PIR_PIN);

  // If motion detected and alarm not active yet
  if (motion == HIGH && !alarmActive) {
    alarmActive = true;
    motionTime = millis();
    Serial.println("Motion detected! Countdown started...");
  }

  // Alarm mode
  if (alarmActive) {
    alarmState();
  }
}

void alarmState() {

  // Blink red LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 300) {
    digitalWrite(RED_LED, !digitalRead(RED_LED));
    lastBlink = millis();
  }

  digitalWrite(GREEN_LED, LOW); // green off

  // After 30 seconds, buzzer starts
  if (!buzzerActive && millis() - motionTime >= 10000) {
    buzzerActive = true;
    tone(BUZZER, 1000);
    Serial.println("No PIN entered - ALARM SIREN!");
  }

  // Handle keypad input
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key: ");
    Serial.println(key);

    if (key >= '0' && key <= '9') {
      enteredPIN += key;
    }

    if (key == '#') {  // Confirm PIN
      Serial.print("Entered PIN: ");
      Serial.println(enteredPIN);

      if (enteredPIN == correctPIN) {
        deactivateAlarm();
      } else {
        Serial.println("Wrong PIN.");
      }

      enteredPIN = ""; // reset
    }
  }
}

void deactivateAlarm() {
  Serial.println("Correct PIN! UNLOCKING.");

  alarmActive = false;
  buzzerActive = false;

  noTone(BUZZER);

  // LED states
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  // Unlock
  lockServo.write(90);
  delay(3000);  // keep open
  lockServo.write(0);

  // Reset state
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  Serial.println("System reset. Locked again.");
}
