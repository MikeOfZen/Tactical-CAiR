/* TACTICAL CAIR
 *  Application to configure and control the Tactical CAIR ventilator
 *  Jonathan Evans
 *  27-3-2020

*/
#include <Servo.h> 
int sweep_angle=90;              //degrees
int servo_speed=1;              //1=fastest 10=slower, 20=slower etc... the actual speed depends on the servo
unsigned long delay_open_interval=3000;    //ms time interval the servo stays open
unsigned long delay_closed_interval=1000;  //ms time interval the servo stays closed
int servo_pin=9;                 //the pin driving the servo
int beep_interval=500;           //interval between alarm beeps
int alarm_pin=11;                //the alarm pin

class Sweeper
{
  Servo servo;              // the servo
  int pos;                  // current servo position 
  int increment;            // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  int stop_servo;

public: 
  Sweeper(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
  
  void Attach(int pin)
  {
    servo.attach(pin);
  }
  
  void Detach()
  {
    servo.detach();
  }
  
  void Update(unsigned long currentMillis, int delay_open_interval, int delay_closed_interval)
  {
    int delay_interval;
    
    if((currentMillis - lastUpdate) > updateInterval && !stop_servo)  // time to update
    {
      lastUpdate = millis();
      pos += increment;
      servo.write(pos);
      if ((pos >= sweep_angle) || (pos <= 0)) // end of sweep
      {
        // reverse direction
        increment = -increment;
        stop_servo=true;
      }
    }
     if (stop_servo){
       if (increment>0){
          delay_interval=delay_open_interval;
          }
       else{
          delay_interval=delay_closed_interval;
          }
      if((currentMillis - lastUpdate) > delay_interval){
        stop_servo=false;
      }
    }
  }
};

class Alarm
{
  int increment;        // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;

public: 
  Alarm(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
    
  void Update(unsigned long currentMillis)
  {
    if((currentMillis - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = millis();
      toggle=!toggle;
    }
    if (toggle){
      tone(alarm_pin,1000);
    }
    else{
      noTone(alarm_pin);
    }
  }
};
  
Sweeper sweeper1(servo_speed);
Alarm alarm1(beep_interval);
 
void setup() 
{ 
  Serial.begin(9600);
  sweeper1.Attach(servo_pin);
  
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();
  sweeper1.Update(currentMillis, delay_open_interval, delay_closed_interval);
  alarm1.Update(currentMillis);
} 

void loop()
{
  int val = analogRead(A5);    // read the value from the sensor
  Serial.println(val);
  delay(50);
}



