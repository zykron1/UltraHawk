struct Vector3 {
    float x, y, z;

    Vector3 operator*(float s) const { return {x*s, y*s, z*s}; }
    Vector3 operator+(const Vector3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vector3& operator+=(const Vector3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vector3& operator*=(float s) { x*=s; y*=s; z*=s; return *this; }
    Vector3& operator+=(float s) { x+=s; y+=s; z+=s; return *this; }
    Vector3 operator+(float s) const { return {x+s, y+s, z+s}; }
};

class StateEstimator {
public:
    void updateState(Vector3 gyro, Vector3 accel,float altitude, float elapsed_ms);
    Vector3 getOrientation(); // i use radians 
	Vector3 getPosition();
	float getAltitude();
	float getVelocity();
	void resetVelocity();

private:
	// dcm init rows, credit wikipedia
    float R[3][3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

	bool _initialized = false;
	float _lastAltitude = 0.0f;
	float _baro_velocity = 0.0f;

	Vector3 _position;
	Vector3 _velocity;

    void normalizeDCM();
};
