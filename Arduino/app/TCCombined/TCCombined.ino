/* TACTICAL CAIR
 *  Combined Controller and Monitor for the Tactical CAIR Ventilator 
 *  Jonathan Evans
 *  27-3-2020
 *  version 1.0
 
Release Notes:

Serial Commands:

stop alarm - disables audible alarm
start alarm - enables audible alarm
start apnea - enables the apnea alarm
stop apnea - disables the apnea alarm
start probe - enables the probe alarm
stop probe - disables the probe alarm
 
General Notes:


*/
#include "ventilator.h"

//Configure the application seetings in "ventilator.h"

//Global variables uses to communicate between each class
//Do not change these variables
//Alarm variables
bool stop_audible_alarm=true; //all alarms suspended when set 
bool stop_apnea_alarm=true; //suspends the apnea alarm when set
bool stop_probe_alarms=true; //suspends the probe alarm when set
int sound_alarm=0; //Alarm sounds when set to 1
unsigned long auto_clear_timer;
int alarm_ind; //used by auto_clear_timer

//Manometer variables
int probe_status[4]; //global variable to hold the status of each probe
float probe_reading[4]; //global variable to hold the analog reading of the sensor when it changes state

//I/E ratio variables
float iTime; //ms Inspitory time (hardcoded for now)
int bMin; //Respitory rate (hard coded for now)
float cycle_time; //Total cycle time
float eTime; //Explitory time
float ie_ratio; // I/E Ratio (Inspiratory-to-expiratory time)

//Exhaust valve variables
int control_ind = false; //flag used to start/stop the EV 1=Start, 0=Stop

//Definition of the class instances uses in this application
Sweeper exhaust_valve(SERVO_SPEED);
Alarm alarm1(SOUND_INTERVAL);
WaterSensor water_sensor1(WATER_SENSOR_POLL_INTERVAL, M_PIN1, 0);
WaterSensor water_sensor2(WATER_SENSOR_POLL_INTERVAL, M_PIN2, 1);
WaterSensor water_sensor3(WATER_SENSOR_POLL_INTERVAL, M_PIN3, 2);
WaterSensor water_sensor4(WATER_SENSOR_POLL_INTERVAL, M_PIN4, 3);
//Apnea apnea(WATER_SENSOR_POLL_INTERVAL, APNEA_PIN);
SerialInput serial_input;

void startEVCycle(){
   exhaust_valve.Reset(); 
   control_ind=true;
}

void setup() 
{ 
  Serial.begin(9600);
  
  //Set up an interrupt to fire every 1ms
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  //Initialize variables
  probe_status[0]=false; //Top probe should be exposed on startup
  probe_status[1]=false;
  probe_status[2]=false;
  probe_status[3]=true; //Bottomprobe should be immersed on startup

  for (int i=0;i<4;i++){
    probe_reading[i]=0;
  }
  
  //Attach the exhaust valve relay
  exhaust_valve.Attach(EXHAUST_VALVE_PIN);
  
  Serial.println("stop alarm - disables audible alarms");
  Serial.println("start alarm - enables audible alarms");
  Serial.println("start apnea - enables the apnea alarm");
  Serial.println("stop apnea - disables the apnea alarm");
  Serial.println("start probe - enables the probe alarm");
  Serial.println("stop probe - disables the probe alarm");
  Serial.println("To start the ventilator valve enter iTime,bMin (e.g. 1,12)");
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();
  
  //This routine will service every class based on round robin multi processing technique
  //Each class has a service interval setting appropriate for the class   
  
  if (!stop_audible_alarm){
    alarm1.Update(currentMillis);   
  }
  
  if (!stop_apnea_alarm){
    //apnea.Update(currentMillis);  
  }
    
  serial_input.Update();
  serial_input.ProcessInput();
  
  if (sound_alarm && !alarm_ind && !stop_audible_alarm){
    auto_clear_timer = millis();
    alarm_ind=true;
    Serial.println("Alarm started");      
  }
  
  if (alarm_ind){
    if (millis()-auto_clear_timer > AUTO_CLEAR_ALARM_TIME){
      sound_alarm=false;
      alarm_ind=false;
      Serial.println("Alarm cleared");
      //apnea.Reset();
    }
  

  }
  exhaust_valve.Update(currentMillis, iTime*1000, eTime*1000);
} 

void loop()
{
  
  bool prev_status[4];
  int i;
  unsigned int currentMillis;
  
  //The main loop monitors the manometer probe global variables
  //and print change in state to the screen if PRINT_PROBE_STATE_CHANGES if set to 1
  if (PRINT_PROBE_STATE_CHANGES){
    prev_status[0]=false; //Top probe should be exposed on startup
    prev_status[1]=false;
    prev_status[2]=false;
    prev_status[3]=true; //Bottomprobe should be immersed on startup
    while (true){
      currentMillis = millis();
      for (i=0;i<4;i++){
        if (prev_status[i]!=probe_status[i]){
          Serial.print("Probe:");
          Serial.print(i);
          if (probe_status[i]==true) Serial.print(",IMMERSED,");
          if (probe_status[i]==false) Serial.print(",EXPOSED,");
          Serial.println(probe_reading[i]);
          prev_status[i]=probe_status[i];
        }
        //Temporary alarm rules, TBD 
        if ((i==0 and probe_status[i]==true) or //Alarm if the top probe is immersed 
            (i==3 and probe_status[i]==false)){ //Alarm if bottom sensor is exposed
          sound_alarm=true;
          //prev_status[0]=false;
          //prev_status[3]=true;
        }
      } 
      if (!stop_probe_alarms){
        water_sensor1.Update(currentMillis);
        water_sensor2.Update(currentMillis);
        water_sensor3.Update(currentMillis);
        water_sensor4.Update(currentMillis);
      } 
    }    
  }
}
  
