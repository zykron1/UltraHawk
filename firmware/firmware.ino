#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include "MPU6050.h"
#include "StateEstimator.h"

// Must be the same on GCS
enum States {
	IDLE,
	TELEOP,
	AUTON,
};

typedef struct {
	size_t packet_number;
	float missionTime;
	States state;
	float gx, gy, gz, ax, ay, az; // imu data
	float roll, pitch, yaw; // orientation
	float x, y, z; // location
} TelemetryPacket;

typedef struct {
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

void setup () {
	Wire.begin(6, 7);

	imu.init(0x68);
	Serial.begin(9600);

	delay(100);
	imu.calibrate(500, 10);
	Serial.printf("{%f, %f, %f} \n", imu.drift.gx, imu.drift.gy, imu.drift.gz);

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

		i = imu.getData();

		estimate.updateState({i.gx, i.gy, i.gz}, 0.0f, actual_ms); 
		Orientation o = estimate.getOrientation() * RAD_TO_DEG;

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
