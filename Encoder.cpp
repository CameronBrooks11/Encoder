#include "Encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder(uint8_t pin1, uint8_t pin2) : pin1(pin1), pin2(pin2), position(0), state(0), interrupts_in_use(0) {
    // Setup pins with INPUT_PULLUP or INPUT and HIGH if necessary

    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);

    this->position = 0; // Initialize position to 0
    // allow time for a passive R-C filter to charge
    // through the pullup resistors, before reading
    // the initial state
    delayMicroseconds(2000);
    uint8_t s = 0;
    if (digitalRead(pin1)) s |= 1;
    if (digitalRead(pin2)) s |= 2;
    this->state = s;

    // Setup static instance pointer
    Encoder::instance = this;

    // Attach interrupts using wrapper functions
    attachInterrupt(pin1, isrWrapper1, CHANGE);
    attachInterrupt(pin2, isrWrapper2, CHANGE);
}

//                           _______         _______       
//               Pin1 ______|       |_______|       |______ Pin1
// negative <---         _______         _______         __      -> positive
//               Pin2 __|       |_______|       |_______|   Pin2

		//	new	new	old	old
		//	pin2	pin1	pin2	pin1	Result
		//	----	----	----	----	------
		//	0	0	0	0	no movement
		//	0	0	0	1	+1
		//	0	0	1	0	-1
		//	0	0	1	1	+2  (assume pin1 edges only)
		//	0	1	0	0	-1
		//	0	1	0	1	no movement
		//	0	1	1	0	-2  (assume pin1 edges only)
		//	0	1	1	1	+1
		//	1	0	0	0	+1
		//	1	0	0	1	-2  (assume pin1 edges only)
		//	1	0	1	0	no movement
		//	1	0	1	1	-1
		//	1	1	0	0	+2  (assume pin1 edges only)
		//	1	1	0	1	-1
		//	1	1	1	0	+1
		//	1	1	1	1	no movement
void Encoder::update() {
	uint8_t s = state & 3;
	if (digitalRead(pin1)) s |= 4;
	if (digitalRead(pin2)) s |= 8;
	switch (s) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			position++; break;
		case 2: case 4: case 11: case 13:
			position--; break;
		case 3: case 12:
			position += 2; break;
		default:
			position -= 2; break;
	}
	state = (s >> 2);}

int32_t Encoder::read() {
    noInterrupts();
    int32_t ret = this->position;
    interrupts();
    return ret;
}

int32_t Encoder::readAndReset() {
		int32_t ret = this->position;
		this->position = 0;
		interrupts();
		return ret;}

void Encoder::write(int32_t p) {
		noInterrupts();
		this->position = p;
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