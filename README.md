# Tactical-CAiR
The main repo for code and experiments for the Tacitcal CAir project

Arduino installation:
 - Download the zip
 - copy /Arduino/ventilator.h to your Arduino libraries TacticalCair directory (usually Arduino/libraries)
 - copy /Arduino/ventilator.cpp to your Arduino libraries TacticalCair directory (usually Arduino/libraries)
 - copy /Arduino/app/TCCombined durectory to your Arduino home directory
 - Close down all instances of the Ariono IDE and restart 
 - Navigate to TCCombined directory and open TCCombined.ino
 - Upload script to the Arduino (see configurtaion seetings below first)
 
 Configuration:
 - Settings are configure in the ventilator.h file

//Spreaker settings for the alarm
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
#define M_ACTIVATION_THRESHOLD 200 //The analog reading value that triggers an immersed state
#define M_DEACTIVATION_THRESHOLD 800 //The analog reading value that clears an immersed state (i.e. sensor not immersed)

//Apnea alarm settings
#define APNEA_THRESHOLD 100 // The water sensor analog value for the the apnea alarm
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
#define STREAM_PROBE_READING 0 //Streams probe reading at 10ms, 0=OFF, 1=ON , NB:switch off for production
#define C_PROBE_NO 0 //The probe pin used for streaming (0-3) being calibrated
#define PRINT_PROBE_STATE_CHANGES 1 //Change in start will print to serial
