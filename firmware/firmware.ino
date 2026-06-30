#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include "Drone.h"
#include "MPU6050.h"
#include "StateEstimator.h"
#include "PID.h"

// Must be the same on GCS
enum States {
	IDLE,
	TELEOP,
	AUTON,
};

typedef struct __attribute__((packed)) {
	size_t packet_number;
	float missionTime;
	States state;
	float gx, gy, gz, ax, ay, az; // imu data
	float roll, pitch, yaw; // orientation
	float x, y, z; // location
} TelemetryPacket;

typedef struct __attribute__((packed)) {
	States state;
	float thrust, roll, pitch, yaw; // if teleop
	float xpos, ypos, zpos; // if auton
} Command;

uint8_t broadcastAddress[] = {0xFC, 0x01, 0x2C, 0xD0, 0x77, 0x5C};

MPU6050 imu;
StateEstimator estimate;
MPU6050Data i;

uint32_t startTime = 0;
uint32_t lastTime = 0;

Command currentCommand = {IDLE, 0,0,0,0,0,0,0};

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
	memcpy(&currentCommand, incomingData, sizeof(currentCommand));
}

Drone drone(2, 3, 10, 9);
PID rollController(5, 1, 2);
PID pitchController(5, 1, 2);
PID yawController(5, 1, 2);

void setup () {
	Wire.begin(6, 7);

	imu.init(0x68);
	Serial.begin(9600);

	delay(1000);
	imu.calibrate(500, 10);
	Serial.printf("{%f, %f, %f} \n", imu.drift.gx, imu.drift.gy, imu.drift.gz);

	drone.initMotors();

	WiFi.mode(WIFI_STA);
	esp_now_init();
	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
	peerInfo.channel = 0;  
	peerInfo.encrypt = false;
	esp_now_add_peer(&peerInfo);

	startTime = micros();
	lastTime = startTime;
}

const uint32_t PERIOD_US = 10000; 
size_t packet_number = 0;
void loop () {
	uint32_t now = micros();
	float t = (now - startTime) / 1000000.0f;
	if (now - lastTime >= PERIOD_US) {
		float actual_ms = (now - lastTime) / 1000.0f;
		lastTime += PERIOD_US;

		// State management
		i = imu.getData();

		estimate.updateState({i.gx, i.gy, i.gz}, 0.0f, actual_ms); 
		Orientation o = estimate.getOrientation() * RAD_TO_DEG;
		
		// Handle drone states
		switch (currentCommand.state) {
			case IDLE: {
				break;
			}
			case TELEOP: {
				float rollSetPoint = currentCommand.roll;
				float pitchSetPoint = currentCommand.pitch;
				float yawSetPoint = currentCommand.yaw;
				float thrust = currentCommand.thrust;

				float rollCmd = rollController.updateLoop(o.x, rollSetPoint, 0.01);
				float pitchCmd = pitchController.updateLoop(o.y, pitchSetPoint, 0.01);
				float yawCmd = yawController.updateLoop(o.z, yawSetPoint, 0.01);

				drone.writeMotors(thrust, rollCmd, pitchCmd, yawCmd);
				break;
			}
			case AUTON: {
				break;
			}
		}

		// Telemetry
		TelemetryPacket telem = {
			packet_number,
			t,
			currentCommand.state,
			i.gx,
			i.gy,
			i.gz,
			i.ax,
			i.ay,
			i.az,
			o.x,
			o.y,
			o.z,
			0,0,0
		};
		esp_now_send(broadcastAddress, (uint8_t*)&telem, sizeof(telem));
		packet_number++;
	}
}
