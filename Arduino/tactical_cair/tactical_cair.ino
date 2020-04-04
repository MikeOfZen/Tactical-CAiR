/* TACTICAL CAIR
 *  Application to configure and control the Tactical CAIR ventilator
 *  Jonathan Evans
 *  27-3-2020
 *  version 4.0
 
Release Notes:
 - Added start stop button fucntionality
 - Hard coded I/E ratio and used iTime & eTime times to drive the EV relay
 
General Notes:
- Sweeper Class controls the Servo
- Relay Class controls the Relay
- Any number of class instances can be created for multiple servos, relays etc...
- All code is non-blocking with no delays.  
- Timer0 - used for millis(), micros(), delay() and PWM on pins 5 & 6
- Timer1 - used for Servos, and PWM on pins 9 & 10
- Timer2 - used by Tone and PWM on pins 11 & 13
 

*/
#include <Servo.h> 
//exhaust_valve configuration
#define EXHAUST_VALVE_PIN 9  //1=open valve;0=close valve
//Servo configuration 
#define SERVO_START 70 //degrees
#define SERVO_END 135  //degrees
#define SERVO_SPEED 3 //1=fastest 10=slower, 20=slower etc... the actual speed depends on the servo
//Polarity of relay settings
#define EV_OPEN 1
#define EV_CLOSED 0

//I/E ratio variables
float iTime; //ms Inspitory time (hardcoded for now)
int bMin; //Respitory rate (hard coded for now)
float cycle_time; //Total cycle time
float eTime; //Explitory time
float ie_ratio; // I/E Ratio (Inspiratory-to-expiratory time)

//UI Inputs:
//To be connected to the UI when it is built
int stop_button_pin = 3; //Start button pin
int start_button_pin = 2; //Stop button pin

int control_ind = false; //flag used to start/stop the EV 1=Start, 0=Stop

void startEVCycle();


class Sweeper
{
  Servo servo;              // the servo
  int pos;                  // current servo position 
  int increment;            // increment to move for each interval
  int  updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  int stop_servo;
  int i;

public: 
  Sweeper(unsigned long interval)
  {
    updateInterval = interval;
    if (SERVO_END>SERVO_START){
        increment = 1;
    }
    else {
        increment = -1;      
    }
  }
  
  void Attach(int pin)
  {
    servo.attach(pin);
    servo.write(SERVO_START);
  }
  
  void Detach()
  {
    servo.write(SERVO_START);
    servo.detach();
  }
  
  void Reset(){
    servo.write(SERVO_START);
    pos=SERVO_START;
    if (SERVO_END>SERVO_START){
      increment = 1;
    }
    else{
      increment = -1;
    }
  }
  
  void Update(unsigned long currentMillis, int delay_open_interval, int delay_closed_interval)
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
};

class Relay
{
	int relayPin;     
	int relayState;             		
	unsigned long previousMillis;  	
 
  public:
  Relay(void)
  {
	  
	relayState = EV_OPEN;
  
	previousMillis = 0;
  }
 
  void Attach(int pin)
  {
	 relayPin = pin;      
   digitalWrite(pin, relayState);
   pinMode(pin, OUTPUT); 
  }
 
 void Reset(void)
  {
  digitalWrite(relayPin, EV_CLOSED);
  previousMillis=0;
  }
 
  void Update(unsigned long currentMillis, unsigned long eTime, unsigned long iTime)
  {
    if (!control_ind){
      relayState=EV_OPEN;
      return;
    }
    if((relayState == EV_OPEN) && (currentMillis - previousMillis >= iTime))
    {
    	relayState = EV_CLOSED;  
      previousMillis = currentMillis;  
      digitalWrite(relayPin, relayState);
      Serial.println("on");      
    }
    else if ((relayState == EV_CLOSED) && (currentMillis - previousMillis >= eTime))
    {
      relayState = EV_OPEN;  
      previousMillis = currentMillis;   
      digitalWrite(relayPin, relayState);	  
      Serial.println("off");      
    }
  }
};

class SerialInput
{
byte numChars = 10;
char receivedChars[10];   // an array to store the received data
boolean newData = false;

public: 
  SerialInput(void)
  {
  newData = false;
  }
    
  void Update(void)
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

 void ProcessInput(void)
  {
    int colon_pos;
    float b;
    float i;
    int found=false;
    if (newData == true) {
      Serial.print("Received:");
      Serial.println(receivedChars);
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
        Serial.println("Error enter iTime,bMin");
      }
      newData = false;
    }
  }
  
};

//Sweeper exhaust_valve;
SerialInput serial_input;
Sweeper exhaust_valve(SERVO_SPEED);

void startEVCycle(){
   exhaust_valve.Reset(); 
   control_ind=true;
}

void setup() 
{ 
  Serial.begin(9600);
  Serial.println("Enter iTime,bMin");
  
  //Set up the start and stop buttons
  //pinMode(start_button_pin, INPUT);    
  //pinMode(stop_button_pin, INPUT);    
  
  //Open exhaust valve on startup
  //digitalWrite(EXHAUST_VALVE_PIN, EV_OPEN);
  exhaust_valve.Attach(EXHAUST_VALVE_PIN);
  //exhaust_valve.Attach(servo_pin);
  
  control_ind = false; //This will force it into a continuous loop. Set to False when buttons installed
  
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();
  if (!control_ind){
    serial_input.Update();
    serial_input.ProcessInput();
    
  }
  exhaust_valve.Update(currentMillis, iTime*1000, eTime*1000);

} 

void loop()
{

//Uncomment this code once the start and stop button are built
// and set "control_ind = false;" in setup()
/*
  if (!digitalRead(start_button_pin) && !control_ind){ //start button pressed and control loop not runnning 
    if (ie_ratio>0.67){
      Serial.println("Cannot have ie ratio > 0.67");//Alarm & Error to display
    }
    else{
      control_ind=true;  // start button is pressed and ventilator control loop begings
      Serial.println("Start Routine");
    }
  }
  
  if (!digitalRead(stop_button_pin) && control_ind){ //stop button pressed and control is running 
    control_ind=false;  // stop button is pressed and ventilator control loop stops
    //digitalWrite(EXHAUST_VALVE_PIN, EV_OPEN);  //stop cycle open EV valve	
    exhaust_valve.Reset();
    Serial.println("Stop Routine"); 
  }
*/

    
}
