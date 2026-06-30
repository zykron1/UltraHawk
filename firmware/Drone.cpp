#include <esp32-hal-ledc.h>
#include "Drone.h"

void Drone::initMotors() {
	ledcAttach(this->m1, 50, 16);
	ledcAttach(this->m2, 50, 16);
	ledcAttach(this->m3, 50, 16);
	ledcAttach(this->m4, 50, 16);
}

uint16_t Drone::pulseToDuty(uint16_t pulse_us) {
    constexpr uint32_t MAX_DUTY = (1 << 16) - 1;
    constexpr uint32_t PERIOD_US = 20000; // 50 Hz
    return (pulse_us * MAX_DUTY) / PERIOD_US;
}

void Drone::writeESC(int pin, uint16_t pulse_us) {
    ledcWrite(pin, pulseToDuty(pulse_us));
}

void Drone::writeMotors(int thrust, int roll, int pitch, int yaw) {
	int m1Out = thrust + pitch + roll - yaw;
	int m2Out = thrust + pitch - roll + yaw;
	int m3Out = thrust - pitch - roll - yaw;
	int m4Out = thrust - pitch + roll + yaw;

	m1Out = constrain(1000 + m1Out * 10, 1000, 2000);
	m2Out = constrain(1000 + m2Out * 10, 1000, 2000);
	m3Out = constrain(1000 + m3Out * 10, 1000, 2000);
	m4Out = constrain(1000 + m4Out * 10, 1000, 2000);

	writeESC(m1, m1Out);
	writeESC(m2, m2Out);
	writeESC(m3, m3Out);
	writeESC(m4, m4Out);
}
