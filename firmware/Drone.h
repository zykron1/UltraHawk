#pragma once

#include <Arduino.h>

class Drone {
	private:
		int m1, m2, m3, m4;
	protected:
		uint16_t pulseToDuty(uint16_t pulse_us);
		void writeESC(int pin, uint16_t pulse_us);
	public:
		Drone(int p1, int p2, int p3, int p4) : m1(p1), m2(p2), m3(p3), m4(p4) {}
		void initMotors();
		void writeMotors(int thrust, int roll, int pitch, int yaw);
};
