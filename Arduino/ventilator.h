// The MIT License (MIT)
//
// Copyright (c) 2020 Tactical CAIR Ventilator
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ventilator_h
#define ventilator_h

#include "Arduino.h"
#include <Servo.h> 

//Speaker settings for the alarm
#define SOUND_INTERVAL 100 // ms Piezo buzzer sounding interval on/off
#define SPEAKER_PIN 11 // Piezzo buzzer pin
#define AUTO_CLEAR_ALARM_TIME 5000 //(ms) All alarms auto clear after this threshold

//Manometer sensor settings
#define WATER_SENSOR_POLL_INTERVAL 100 //(ms) 
#define ANEODE_OUTPUT_PIN 24 // Pin number associated with the manometer aneode 
#define CATHODE_OUTPUT_PIN 22 // Pin number associated with the manometer cathode
#define DEBOUNCE_THRESHOLD 5 //The number of consecutive readings above/below the threshold
#define M_PIN1 A0 //Pin number associated with the manometer probe 1
#define M_PIN2 A1 //Pin number associated with the manometer probe 2
#define M_PIN3 A2 //Pin number associated with the manometer probe 3
#define M_PIN4 A3 //Pin number associated with the manometer probe 4
#define M_ACTIVATION_THRESHOLD 90//The analog reading value that triggers an immersed state
#define M_DEACTIVATION_THRESHOLD 140 //The analog reading value that clears an immersed state (i.e. sensor not immersed)

//Apnea alarm settings
#define APNEA_THRESHOLD 100 // The water sensor analog value for the apnea alarm
#define APNEA_ALARM_INTERVAL 5000 //(ms) The alarm will trigger if the water sensor has not crossed the threshold
#define APNEA_PIN A0 // The apnea probe pin

//Exhaust valve settings
#define EXHAUST_VALVE_PIN 2  //1=open valve;0=close valve

//Servo configuration 
#define SERVO_START 0 //degrees
#define SERVO_END 90 //degrees
#define SERVO_SPEED 1 //1=fastest 10=slower, 20=slower etc... the actual speed depends on the servo

//Polarity of relay settings
#define EV_OPEN 1
#define EV_CLOSED 0

//Calibration
//Use these settings to calibrate the manometer probes one at a time
//Set the C_PROBE_NO to the probe number (0-3) you want to test
//Set STREAM_PROBE_READING to 1 and this will start readings to be sent to the serial port in real time
//Run the ventilator system and observe the probe readings to determine min/max thresholds
//Repeat above for each probe to check all sensors are behaving in same
//Once complete configure the M_ACTIVATION_THRESHOLD and M_DECTIVATION_THRESHOLD 
//set PRINT_PROBE_STATE_CHANGES to 1 to observe change in state of each probe in real time 
#define STREAM_PROBE_READING 1 //Streams probe reading at 10ms, 0=OFF, 1=ON , NB:switch off for production
#define C_PROBE_NO 1 //The probe pin used for streaming (0-3) being calibrated
#define PRINT_PROBE_STATE_CHANGES 1 //Change in start will print to serial

void startEVCycle();

float get_analog_reading(int pin);
void reset_probes();
void print_probe_status(int i);
int get_probe_status(int pin, int probe);
void read_probes();

class WaterSensor
{
private:
  unsigned long updateInterval; // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;
  int previous_analog_reading=0;
  int pin;
  int debounce_cnt=0;
  float current_reading;
  int probe_no;
  
public: 
  WaterSensor(unsigned long interval, int pin_param, int probe);
  void Update(unsigned long currentMillis);
};

class Alarm
{
private:
  int increment;        // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;

public: 
  Alarm(int interval);
  void Update(unsigned long currentMillis);

};

class Apnea
{
private:
  unsigned long updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;
  int previous_reading=0;
  unsigned long apnea_timer;
  int pin;
  float current_reading;
  int debounce_cnt;
  
public: 
  Apnea(unsigned long interval, int pin_param);
  void Reset(void);
  void Update(unsigned long currentMillis);
};

class Sweeper
{
private:
  Servo servo;              // the servo
  int pos;                  // current servo position 
  int increment;            // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  int stop_servo;
  int i;

public: 
  Sweeper(unsigned long interval);
  void Attach(int pin);
  void Detach();
  void Reset();
  void Update(unsigned long currentMillis, int delay_open_interval, int delay_closed_interval);
};

class SerialInput
{
private:
  byte numChars = 15;
  char receivedChars[15];   // an array to store the received data
  boolean newData = false;

public: 
  SerialInput(void);
  void Update(void);
  void ProcessInput(void);
};


#endif