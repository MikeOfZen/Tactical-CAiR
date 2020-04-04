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

#include "ventilator.h"

extern int sound_alarm; //Alarm sounds when set to 1
extern unsigned long auto_clear_timer;
extern int alarm_ind; //used by auto_clear_timer
extern int probe_status[4]; //global variable to hold the status of each probe
extern float probe_reading[4]; //global variable to hold the analog reading of the sensor when it changes state
//I/E ratio variables
extern float iTime; //ms Inspitory time (hardcoded for now)
extern int bMin; //Respitory rate (hard coded for now)
extern float cycle_time; //Total cycle time
extern float eTime; //Explitory time
extern float ie_ratio; // I/E Ratio (Inspiratory-to-expiratory time)
extern bool stop_audible_alarm; //all alarms suspended when set 
extern bool stop_apnea_alarm; //suspends the apnea alarm when set
extern bool stop_probe_alarms; //suspends the probe alarm when set
extern int control_ind; //flag used to start/stop the EV 1=Start, 0=Stop

float get_analog_reading(int pin){
  float analog;
  
  pinMode(ANEODE_OUTPUT_PIN, OUTPUT);
  pinMode(CATHODE_OUTPUT_PIN, OUTPUT);
  pinMode (pin, INPUT);
  digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
  analog = analogRead(pin);
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, HIGH);  
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);
  return(analog);
}

WaterSensor:: WaterSensor(unsigned long interval, int pin_param, int probe)
{
  updateInterval = interval;
  pin = pin_param;
  probe_no=probe;
}
  
void WaterSensor::Update(unsigned long currentMillis)
{
  if((currentMillis - lastUpdate) > updateInterval){  // time to update  
    current_reading = get_analog_reading(pin);
    if (STREAM_PROBE_READING){
      if (probe_no==C_PROBE_NO){
        Serial.println(current_reading);    
      }
    }

    if (probe_status[probe_no]==false){
      if (current_reading > M_ACTIVATION_THRESHOLD){
        debounce_cnt++;
      }
      else {
        debounce_cnt=0;
      }      
    }
    if (probe_status[probe_no]==true){
      if (current_reading < M_DEACTIVATION_THRESHOLD){
        debounce_cnt++;
      }
      else {
        debounce_cnt=0;
      }      
    }
    if (debounce_cnt>=DEBOUNCE_THRESHOLD){
      probe_status[probe_no]^=1;  //toggle status
      probe_reading[probe_no]=current_reading;
      debounce_cnt=0;
    }
  }
}

Alarm::Alarm(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
    
void Alarm::Update(unsigned long currentMillis)
  {
    if (sound_alarm){
      if((currentMillis - lastUpdate) > updateInterval)  // time to update
      {
        lastUpdate = millis();
        toggle=!toggle;
      }
      if (toggle){
        tone(SPEAKER_PIN,1000);
      }
      else{
        noTone(SPEAKER_PIN);
      }      
    }
    else{
      noTone(SPEAKER_PIN);
    }
  }

Apnea:: Apnea(unsigned long interval, int pin_param)
  {
    updateInterval = interval;
    apnea_timer = millis();
    lastUpdate = millis();
    pin=pin_param;
  }
  
void Apnea:: Reset(void){
   apnea_timer = millis();
   lastUpdate = millis();
 }
  
void Apnea:: Update(unsigned long currentMillis)
  {
    if (sound_alarm || stop_apnea_alarm){
      return;
    }
    current_reading = get_analog_reading(pin);
    if((currentMillis - lastUpdate) > updateInterval)  // check manometer reading
    {
      if (previous_reading == 0){
        previous_reading = current_reading; 
      }
      
      if (toggle){
        if (current_reading > APNEA_THRESHOLD){
          debounce_cnt++;
        }
        else {
          debounce_cnt=0;
        }
        if (debounce_cnt>=DEBOUNCE_THRESHOLD){
          Serial.println("Reset Apnea");
          toggle=0;
          apnea_timer=millis();
          previous_reading = current_reading; 
          debounce_cnt=0;
        }
      }    
      if (!toggle){
        if (current_reading < APNEA_THRESHOLD){
          debounce_cnt++;
        }
        else {
          debounce_cnt=0;
        }
        if (debounce_cnt>=DEBOUNCE_THRESHOLD){
          Serial.println("Reset Apnea");
          toggle=1;
          apnea_timer=millis();
          previous_reading = current_reading; 
          debounce_cnt=0;
        }
      }
    lastUpdate = millis();
    }
   
    if((currentMillis - apnea_timer) > APNEA_ALARM_INTERVAL)  // check manometer reading
    {
      Serial.println("Apnea alarm triggered");
      sound_alarm=true;   
    }   
  }

 
Sweeper:: Sweeper(unsigned long interval)
  {
    updateInterval = interval;
    if (SERVO_END>SERVO_START){
        increment = 1;
    }
    else {
        increment = -1;      
    }
  }
  
void Sweeper:: Attach(int pin)
  {
    servo.attach(pin);
    servo.write(SERVO_START);
  }
  
void Sweeper:: Detach()
  {
    servo.write(SERVO_START);
    servo.detach();
  }
  
void Sweeper:: Reset(){
    servo.write(SERVO_START);
    pos=SERVO_START;
    if (SERVO_END>SERVO_START){
      increment = 1;
    }
    else{
      increment = -1;
    }
  }
  
void Sweeper:: Update(unsigned long currentMillis, int delay_open_interval, int delay_closed_interval)
  {
    int delay_interval;
    if (!control_ind){
      return;
    }
    
    if((currentMillis - lastUpdate) > updateInterval && !stop_servo)  // time to update
    {
      lastUpdate = millis();
      pos += increment;
      servo.write(pos);
      if (SERVO_END>SERVO_START){
        if ((pos >= SERVO_END) || (pos <= SERVO_START)) // end of sweep
        {
          // reverse direction
          increment = -increment;
          stop_servo=true;
          i=1;
        }
      }
      else{
        if ((pos <= SERVO_END) || (pos >= SERVO_START)) // end of sweep
        {
          // reverse direction
          increment = -increment;
          stop_servo=true;
          i=-1;
        }        
      }
    }
     if (stop_servo){
       if (increment==i){
          delay_interval=delay_closed_interval;
          }
       else{
          delay_interval=delay_open_interval;
          }
      if((currentMillis - lastUpdate) > delay_interval){
        stop_servo=false;
      }
    }
  }


SerialInput:: SerialInput(void)
  {
  newData = false;
  }
    
void SerialInput:: Update(void)
  {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && newData == false) {
      rc = Serial.read();

      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        ndx = 0;
        newData = true;
      }
    }
  }

void SerialInput:: ProcessInput(void)
  {
    int colon_pos;
    float b;
    float i;
    int found=false;
    if (newData == true) {
      Serial.print("Received:");
      Serial.println(receivedChars);
      if (String(receivedChars)=="stop alarm"){
        Serial.println("Audible alarms stopped");
        stop_audible_alarm = true;
        found=true;
      }
      if (String(receivedChars)=="stop apnea"){
        Serial.println("apnea alarms stopped");
        stop_apnea_alarm = true;
        found=true;
      }
      if (String(receivedChars)=="start apnea"){
        Serial.println("apnea alarms started");
        stop_apnea_alarm = false;
        found=true;
      }
      if (String(receivedChars)=="start probe"){
        Serial.println("probe alarms started");
        stop_probe_alarms = false;
        found=true;
      }
      if (String(receivedChars)=="stop probe"){
        Serial.println("probe alarms stopped");
        stop_probe_alarms = true;
        found=true;
      }
      if (String(receivedChars)=="start alarm"){
        Serial.println("Audible alarms started");
        stop_audible_alarm = false; 
        found=true;        
      }
      char * token = strtok(receivedChars, ",");
      // loop through the string to extract all other tokens
      if ( token != NULL ) {
        i = atof(token);
        if (i) {
          token = strtok(NULL, ",");
          if ( token != NULL ) {
            b = atof(token);
            if (b) {              
              //I/E ratio variables
              iTime=i; //ms Inspitory time (hardcoded for now)
              bMin=b; //Respitory rate (hard coded for now)
              if (bMin>0 && bMin<60){
                cycle_time = 60/bMin; //Total cycle time
                eTime = cycle_time - iTime; //Explitory time
                if (eTime > 0){
                  if (ie_ratio<=0.67){
                  ie_ratio = iTime/eTime; // I/E Ratio (Inspiratory-to-expiratory time)
                  found=true;
                  Serial.println(ie_ratio); 
                  startEVCycle();
                  Serial.print("Inspitory time=");
                  Serial.println(iTime);
                  Serial.print("Explitory time=");
                  Serial.println(eTime);
                  Serial.print("IE Ratio=");
                  Serial.println(ie_ratio);
                  Serial.print("Cycle time=");
                  Serial.println(cycle_time);  
                  }
                }
              }
            }
          }
        }
      }
      if (!found){
        Serial.println("Command error - Try Again!");
      }
      newData = false;
    }
  }
  
