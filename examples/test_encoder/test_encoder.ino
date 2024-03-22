#include <Arduino.h>

void isrWrapper1();
void isrWrapper2();

class Encoder {
public:
    Encoder(uint8_t pin1, uint8_t pin2);
    void update();
    int32_t read();
    int32_t readAndReset();
    void write(int32_t p);

    static Encoder* instance; // Static instance pointer for ISR access

private:
    uint8_t pin1, pin2;
    volatile int32_t position;
    uint8_t state;
};

Encoder* Encoder::instance = nullptr;

Encoder::Encoder(uint8_t pin1, uint8_t pin2) : pin1(pin1), pin2(pin2), position(0), state(0) {
    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);

    // Initial state setup
    delayMicroseconds(2000); // Allow for stabilization
    uint8_t s = 0;
    if (digitalRead(pin1)) s |= 1;
    if (digitalRead(pin2)) s |= 2;
    this->state = s;

    Encoder::instance = this; // Set static instance pointer
}

void Encoder::update() {
    uint8_t s = state & 3;
    if (digitalRead(pin1)) s |= 4;
    if (digitalRead(pin2)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15: break;
        case 1: case 7: case 8: case 14: position++; break;
        case 2: case 4: case 11: case 13: position--; break;
        case 3: case 12: position += 2; break;
        default: position -= 2; break;
    }
    state = (s >> 2);
}

int32_t Encoder::read() {
    noInterrupts();
    int32_t ret = position;
    interrupts();
    return ret;
}

int32_t Encoder::readAndReset() {
    noInterrupts();
    int32_t ret = position;
    position = 0;
    interrupts();
    return ret;
}

void Encoder::write(int32_t p) {
    noInterrupts();
    position = p;
    interrupts();
}

void isrWrapper1() {
    if (Encoder::instance) {
        Encoder::instance->update();
    }
}

void isrWrapper2() {
    if (Encoder::instance) {
        Encoder::instance->update();
    }
}


Encoder myEncoder(0, 1);

void setup(){
  Serial.begin(115200);
      // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(0), isrWrapper1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(1), isrWrapper2, CHANGE);
}

unsigned long previousMillis = 0; // Stores the last time the encoder was read
const long interval = 50; // Interval at which to read the encoder (milliseconds)

void loop() {
  unsigned long currentMillis = millis(); // Grab the current time
  static int32_t lastPosition = -999; // Variable to store the last position

  // Check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save the last time you read the encoder

    int32_t newPosition = myEncoder.read(); // Read the current position of the encoder
      lastPosition = newPosition;
      Serial.print("New position: ");
      Serial.println(newPosition); // Print the new position to the Serial Monitor
  }
}
