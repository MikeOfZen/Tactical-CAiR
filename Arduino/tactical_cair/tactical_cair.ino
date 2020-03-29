/* TACTICAL CAIR
 *  Application to configure and control the Tactical CAIR ventilator
 *  Jonathan Evans
 *  27-3-2020

Sweeper Class controls the Servo
Relay Class controls the Relay
Alarm Class controls the beeper
Any number of class instances can be created for multiple servos, relays etc...
All code is non-blocking with no delays.  
No code is currently in the main loop, the Timer0 interrupt has the program in a loop for testing. 

*/
#include <Servo.h> 
//Relay configuration
int relay_pin=7;  //relay pin
unsigned long relay_open_interval=5000; //ms time interval the relay stays open
unsigned long relay_closed_interval=2000; //ms time interval the relay stays closed

//Servo configuration
int servo_pin=9; //the pin driving the servo
int sweep_angle=90;//degrees
int servo_speed=1; //1=fastest 10=slower, 20=slower etc... the actual speed depends on the servo
unsigned long servo_open_interval=3000;    //ms time interval the servo stays open
unsigned long servo_closed_interval=1000;  //ms time interval the servo stays closed

//Alarm configuration
int beep_interval=500; //interval between alarm beeps
int alarm_pin=11; //the alarm pin

class Sweeper
{
  Servo servo;              // the servo
  int pos;                  // current servo position 
  int increment;            // increment to move for each interval
  unsigned long updateInterval;      // interval between updates
  unsigned long open_interval;
  unsigned long closed_interval;
  unsigned long lastUpdate; // last update of position
  int stop_servo;

public: 
  Sweeper(int interval, unsigned long servo_open_interval, unsigned long servo_closed_interval)
  {
    updateInterval = interval;
    open_interval = servo_open_interval;
    closed_interval = servo_closed_interval;
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
  
  void Update(unsigned long currentMillis)
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
          delay_interval=open_interval;
          }
       else{
          delay_interval=closed_interval;
          }
      if((currentMillis - lastUpdate) > delay_interval){
        stop_servo=false;
      }
    }
  }
};

class Relay
{
	int relayPin;     
	long OnTime;     
	long OffTime;   
	int relayState;             		
	unsigned long previousMillis;  	
 
  public:
  Relay(long on, long off)
  {
	relayPin = relay_pin;
	pinMode(relayPin, OUTPUT);     
	  
	OnTime = on;
	OffTime = off;
	
	relayState = LOW; 
	previousMillis = 0;
  }
 
  void Update(unsigned long currentMillis)
  {
    if((relayState == HIGH) && (currentMillis - previousMillis >= OnTime))
    {
    	relayState = LOW;  
      previousMillis = currentMillis;  
      digitalWrite(relayPin, relayState);  
    }
    else if ((relayState == LOW) && (currentMillis - previousMillis >= OffTime))
    {
      relayState = HIGH;  
      previousMillis = currentMillis;   
      digitalWrite(relayPin, relayState);	  
    }
  }
};

class Alarm
{
  int increment;        
  unsigned long updateInterval;      
  unsigned long lastUpdate; 
  bool toggle;

public: 
  Alarm(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
    
  void Update(unsigned long currentMillis)
  {
    if((currentMillis - lastUpdate) > updateInterval)  
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
  
Sweeper sweeper1(servo_speed, servo_open_interval, servo_closed_interval);
Alarm alarm1(beep_interval);
Relay relay1(relay_open_interval, relay_closed_interval);
 
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
  //sweeper1.Update(currentMillis);
  //alarm1.Update(currentMillis);
  relay1.Update(currentMillis);
} 

void loop()
{

}



