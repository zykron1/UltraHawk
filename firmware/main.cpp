#include <Wire.h>
#include <BMI270.h>
#include <BMP581.h>

BMI270 imu;
BMP581 bmp;

float targetAltitude = 2.0f; // meters

float altKp = 120.0f;
float altKi = 10.0f;
float altKd = 40.0f;

float rollKp = 4.0f;
float pitchKp = 4.0f;

float altIntegral = 0.0f;
float prevAltError = 0.0f;

uint32_t lastTime = 0;

float throttleBase = 1400;

float motorFL;
float motorFR;
float motorBL;
float motorBR;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    imu.begin();
    bmp.begin();

    lastTime = micros();
}

void loop() {
    uint32_t now = micros();
    float dt = (now - lastTime) * 1e-6f;
    lastTime = now;

    imu.readSensor();
    bmp.read();

    float altitude = bmp.getAltitude();

    float roll = imu.getRoll();
    float pitch = imu.getPitch();

    float altError = targetAltitude - altitude;

    altIntegral += altError * dt;

    float altDerivative = (altError - prevAltError) / dt;
    prevAltError = altError;

    float throttleCorrection =
        altKp * altError +
        altKi * altIntegral +
        altKd * altDerivative;

    float throttle = throttleBase + throttleCorrection;

    float rollCorrection = rollKp * (-roll);
    float pitchCorrection = pitchKp * (-pitch);

    motorFL = throttle + pitchCorrection + rollCorrection;
    motorFR = throttle + pitchCorrection - rollCorrection;
    motorBL = throttle - pitchCorrection + rollCorrection;
    motorBR = throttle - pitchCorrection - rollCorrection;

    motorFL = constrain(motorFL, 1000, 2000);
    motorFR = constrain(motorFR, 1000, 2000);
    motorBL = constrain(motorBL, 1000, 2000);
    motorBR = constrain(motorBR, 1000, 2000);

    // escFL.writeMicroseconds(motorFL);
    // escFR.writeMicroseconds(motorFR);
    // escBL.writeMicroseconds(motorBL);
    // escBR.writeMicroseconds(motorBR);

    delay(5);
}
