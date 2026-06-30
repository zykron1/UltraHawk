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
	t1 = thrust + pitch + roll - yaw;
	t2 = thrust + pitch - roll + yaw;
	t3 = thrust - pitch - roll - yaw;
	t4 = thrust - pitch + roll + yaw;

	t1 = constrain(1000 + t1 * 10, 1000, 2000);
	t2 = constrain(1000 + t2 * 10, 1000, 2000);
	t3 = constrain(1000 + t3 * 10, 1000, 2000);
	t4 = constrain(1000 + t4 * 10, 1000, 2000);

	writeESC(m1, t1);
	writeESC(m2, t2);
	writeESC(m3, t3);
	writeESC(m4, t4);
}
