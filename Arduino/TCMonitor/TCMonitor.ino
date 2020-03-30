/* TACTICAL CAIR
 *  Ventilator monitoring system
 *  Jonathan Evans
 *  27-3-2020
 *  version 1.0
 
Release Notes:

 
General Notes:


*/
#define SOUND_INTERVAL 500 //ms
#define SPEAKER_PIN 4
#define MANOMETER_POLL_INTERVAL 10 //ms
#define ANEODE_OUTPUT_PIN 24
#define CATHODE_OUTPUT_PIN 22
#define MANOMETER_INPUT_PIN A0
#define HIGH_PRESSURE_SETTING 800
#define LOW_PRESSURE_SETTING 200

int sound_alarm=0; //Alarm sounds when set to 1

class Manometer
{
  int value;
  int dummy_value;
  unsigned long updateInterval;
  unsigned long lastUpdate;

public: 
  Manometer(unsigned long interval)
  {
    value=0;
    dummy_value=0;
    updateInterval = interval;  
  }
  
  void Attach(void){
    pinMode(ANEODE_OUTPUT_PIN, OUTPUT);
    pinMode(CATHODE_OUTPUT_PIN, OUTPUT);
    }
  
  void Poll(unsigned long currentMillis)
  {
   if((currentMillis - lastUpdate) > updateInterval){
     
    lastUpdate = millis();
    digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
    digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
    
    value = analogRead(MANOMETER_INPUT_PIN);

    digitalWrite(ANEODE_OUTPUT_PIN, LOW);
    digitalWrite(CATHODE_OUTPUT_PIN, HIGH);  
    Serial.println(value);
    
    if (value < LOW_PRESSURE_SETTING){
      sound_alarm=true;
      }
    else if (value > HIGH_PRESSURE_SETTING){
      sound_alarm=true;
      }
    else{
      sound_alarm=false;
      }

    //disable circuit while waiting
    digitalWrite(ANEODE_OUTPUT_PIN, LOW);
    digitalWrite(CATHODE_OUTPUT_PIN, LOW);
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
    if((currentMillis - lastUpdate) > updateInterval && sound_alarm)  // time to update
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
};


Alarm alarm1(SOUND_INTERVAL);
Manometer manometer(MANOMETER_POLL_INTERVAL);

void setup() 
{ 
  Serial.begin(9600);
 
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();
  alarm1.Update(currentMillis);
  manometer.Poll(currentMillis);
} 

void loop()
{
 
}
  