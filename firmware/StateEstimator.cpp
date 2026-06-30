#include "StateEstimator.h"
#include <math.h>

void StateEstimator::updateState(Vector3 gyro, Vector3 accel, float altitude, float elapsed_ms) {
    float dt = elapsed_ms / 1000.0f;

    if (!_initialized) {
        _position.z = altitude;
        _initialized = true;
        return;
    }

    _baro_velocity = (altitude - _position.z) / dt;

    Vector3 w = gyro * dt;

    float dR[3][3] = {
        { 1,    -w.z,   w.y},
        { w.z,   1,    -w.x},
        {-w.y,   w.x,   1  }
    };

    float newR[3][3] = {};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                newR[i][j] += R[i][k] * dR[k][j];
            }    
        }
    }

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            R[i][j] = newR[i][j];

    normalizeDCM();

    Vector3 acc_world;
    acc_world.x = R[0][0]*accel.x + R[0][1]*accel.y + R[0][2]*accel.z;
    acc_world.y = R[1][0]*accel.x + R[1][1]*accel.y + R[1][2]*accel.z;
    acc_world.z = R[2][0]*accel.x + R[2][1]*accel.y + R[2][2]*accel.z;

    _velocity += acc_world * dt;
    _position += _velocity * dt;

    float z_err = altitude - _position.z;
    //_position.z += z_err * 0.05f;
}

void StateEstimator::normalizeDCM() {
    //extract rows
    float x[3] = {R[0][0], R[0][1], R[0][2]};
    float y[3] = {R[1][0], R[1][1], R[1][2]};

    //error between rows 
    float err = x[0]*y[0] + x[1]*y[1] + x[2]*y[2];

    //correct each row by half the error
    float xo[3] = {x[0] - 0.5f*err*y[0], x[1] - 0.5f*err*y[1], x[2] - 0.5f*err*y[2]};
    float yo[3] = {y[0] - 0.5f*err*x[0], y[1] - 0.5f*err*x[1], y[2] - 0.5f*err*x[2]};

    //z = x cross y
    float zo[3] = {
        xo[1]*yo[2] - xo[2]*yo[1],
        xo[2]*yo[0] - xo[0]*yo[2],
        xo[0]*yo[1] - xo[1]*yo[0]
    };

    //renormalize each row
    auto norm = [](float v[3]) {
        float n = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        v[0]/=n; v[1]/=n; v[2]/=n;
    };
    norm(xo); norm(yo); norm(zo);

    R[0][0]=xo[0]; R[0][1]=xo[1]; R[0][2]=xo[2];
    R[1][0]=yo[0]; R[1][1]=yo[1]; R[1][2]=yo[2];
    R[2][0]=zo[0]; R[2][1]=zo[1]; R[2][2]=zo[2];
}

Vector3 StateEstimator::getOrientation() {
    float roll = atan2f(R[2][1], R[2][2]);
    float pitch = asinf(-R[2][0]);
    float yaw = atan2f(R[1][0], R[0][0]);
    return {roll, pitch, yaw};  // x=roll, y=pitch, z=yaw
}

Vector3 StateEstimator::getPosition(){
	return {_position};
}

float StateEstimator::getAltitude() { return _lastAltitude; }
float StateEstimator::getVelocity() { return _baro_velocity; }

void StateEstimator::resetVelocity() {
    _baro_velocity = 0.0f;
	_initialized = false;
}
