
#ifndef Encoder_h_
#define Encoder_h_

#include "Arduino.h"

class Encoder {
public:
    Encoder(uint8_t pin1, uint8_t pin2);
    int32_t read();
    int32_t readAndReset();
    void write(int32_t p);
    void update();

    static Encoder* instance; // Static instance pointer

private:
    uint8_t pin1, pin2;
    volatile int32_t position;
    uint8_t state;
    int interrupts_in_use = 0;
};

extern void isrWrapper1();
extern void isrWrapper2();

#endif // Encoder_h_
