#include <QTRSensors.h>

const int m11Pin = 7;
const int m12Pin = 6;
const int m21Pin = 5;
const int m22Pin = 4;
const int m1Enable = 11;
const int m2Enable = 10;

const int minSpeed = -255;
const int maxSpeed = 255;
const int baseSpeed = 255;
const int calibrationSpeed = 100;

const int sensorCount = 6;

const int blackLineDetectionThresholdValue = 600;
const int otherThanBlackLineDetectionValue = 500;

const int minSensorValue = 0;
const int maxSensorValue = 5000;
const int minErrorValue = -45;
const int maxErrorValue = 45;

const int calibrationSteps = 250;

int sensorValues[sensorCount];
int sensors[sensorCount] = {0, 0, 0, 0, 0, 0};

int m1Speed = 0;
int m2Speed = 0;
int motorSpeed;

int p = 0;
int i = 0;
int d = 0;

int error = 0;
int lastError = 0;

float kp = 30;
float ki = 0.08;
float kd = 150;

QTRSensors qtr;

void setup()
{
    pinMode(m11Pin, OUTPUT);
    pinMode(m12Pin, OUTPUT);
    pinMode(m21Pin, OUTPUT);
    pinMode(m22Pin, OUTPUT);
    pinMode(m1Enable, OUTPUT);
    pinMode(m2Enable, OUTPUT);

    qtr.setTypeAnalog();
    qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5}, sensorCount);

    delay(500);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    setMotorSpeed(calibrationSpeed, -calibrationSpeed);
    for (uint16_t i = 0; i < calibrationSteps; ++i)
    {
        qtr.calibrate();

        qtr.read(sensorValues);

        // The robot sensor has reached the rightmost position, (only the first sensor is detecting the black line) so the robot has to turn to the left
        if (sensorValues[0] > blackLineDetectionThresholdValue && sensorValues[1] < otherThanBlackLineDetectionValue)
        {
            setMotorSpeed(-calibrationSpeed, calibrationSpeed);
        }
        // The robot sensor has reached the leftmost position, (only the last sensor is detecting the black line) so the robot has to turn to the right
        if (sensorValues[sensorCount - 1] > blackLineDetectionThresholdValue && sensorValues[sensorCount - 2] < otherThanBlackLineDetectionValue)
        {
            setMotorSpeed(calibrationSpeed, -calibrationSpeed);
        }
    }

    digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
    pidControl(kp, ki, kd);
    motorPidControl();
    setMotorSpeed(m1Speed, m2Speed);
}

void pidControl(float kp, float ki, float kd)
{
    error = map(qtr.readLineBlack(sensorValues), minSensorValue, maxSensorValue, minErrorValue, maxErrorValue);

    p = error;
    i = i + error;
    d = error - lastError;
    lastError = error;

    motorSpeed = kp * p + ki * i + kd * d;
}

void motorPidControl()
{
    m1Speed = baseSpeed;
    m2Speed = baseSpeed;

    if (error < 0)
    {
        m1Speed += motorSpeed;
    }
    else if (error > 0)
    {
        m2Speed -= motorSpeed;
    }

    m1Speed = constrain(m1Speed, minSpeed, maxSpeed);
    m2Speed = constrain(m2Speed, minSpeed, maxSpeed);
}

void setMotorSpeed(int motor1Speed, int motor2Speed)
{
    motor1Speed = -motor1Speed;
    motor2Speed = -motor2Speed;
    if (motor1Speed == 0)
    {
        digitalWrite(m11Pin, LOW);
        digitalWrite(m12Pin, LOW);
        analogWrite(m1Enable, motor1Speed);
    }
    else
    {
        if (motor1Speed > 0)
        {
            digitalWrite(m11Pin, HIGH);
            digitalWrite(m12Pin, LOW);
            analogWrite(m1Enable, motor1Speed);
        }
        if (motor1Speed < 0)
        {
            digitalWrite(m11Pin, LOW);
            digitalWrite(m12Pin, HIGH);
            analogWrite(m1Enable, -motor1Speed);
        }
    }
    if (motor2Speed == 0)
    {
        digitalWrite(m21Pin, LOW);
        digitalWrite(m22Pin, LOW);
        analogWrite(m2Enable, motor2Speed);
    }
    else
    {
        if (motor2Speed > 0)
        {
            digitalWrite(m21Pin, HIGH);
            digitalWrite(m22Pin, LOW);
            analogWrite(m2Enable, motor2Speed);
        }
        if (motor2Speed < 0)
        {
            digitalWrite(m21Pin, LOW);
            digitalWrite(m22Pin, HIGH);
            analogWrite(m2Enable, -motor2Speed);
        }
    }
}