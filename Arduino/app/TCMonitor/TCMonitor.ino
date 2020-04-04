/* TACTICAL CAIR
 *  Ventilator monitoring system
 *  Jonathan Evans
 *  27-3-2020
 *  version 1.0
 
Release Notes:

Serial Commands:

stop - disables all alarms
start - enables all alarms
start apnea - enables the apnea alarm
stop apnea - disables the apnea alarm
start pressure - enables the pressure alarm
stop pressure - disables the pressure alarm
 
General Notes:


*/

#define SOUND_INTERVAL 500 // ms Piezo buzzer sounding interval on/off
#define SPEAKER_PIN 11 // Piezzo buzzer pin
#define WATER_SENSOR_POLL_INTERVAL 10 //(ms) 
#define ANEODE_OUTPUT_PIN 24
#define CATHODE_OUTPUT_PIN 22
#define DEBOUNCE_THRESHOLD 5
#define HIGH_PRESSURE_INPUT_PIN A0 //High pressure probe pin
#define LOW_PRESSURE_INPUT_PIN A0  // Low pressure proble pin
#define LOW_PRESSURE_THRESHOLD 10 // If the water sensor analog value falls below this then trigger alarm
#define HIGH_PRESSURE_THRESHOLD 400 // If the water sensor analog value is higher than this trigger alarm
#define AUTO_CLEAR_ALARM_TIME 5000 //(ms) All alarms auto clear after this threshold
#define APNEA_THRESHOLD 100 // The water sensor analog value for the the apnea alarm
#define APNEA_ALARM_INTERVAL 5000 //(ms) The alarm will trigger if the water sensor has not crossed the threshold
#define APNEA_PIN A0 // The apnea probe pin
#define HIGH 1
#define LOW 0 

bool stop_all_alarms=false; //all alarms suspended when set 
bool stop_apnea_alarm=false; //suspends the apnea alarm when set
bool stop_pressure_alarms=false; //suspends the pressure alarm when set

int sound_alarm=0; //Alarm sounds when set to 1
unsigned long auto_clear_timer;
int alarm_ind; //used by auto_clear_timer

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
};

class Apnea
{
  unsigned long updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;
  int previous_reading=0;
  unsigned long apnea_timer;
  int pin;
  float current_reading;
  int debounce_cnt;
  
public: 
  Apnea(unsigned long interval, int pin_param)
  {
    updateInterval = interval;
    apnea_timer = millis();
    lastUpdate = millis();
    pin=pin_param;
  }
  
 void Reset(void){
   apnea_timer = millis();
   lastUpdate = millis();
 }
  
  void Update(unsigned long currentMillis)
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
};

class WaterSensorThreshold
{
  unsigned long updateInterval; // interval between updates
  unsigned long lastUpdate; // last update of position
  bool toggle;
  int previous_analog_reading=0;
  int pin;
  int debounce_cnt=0;
  float current_reading;
  int type;
  int limit;
  
public: 
  WaterSensorThreshold(unsigned long interval, int pin_param, int sensor_type, int threshold)
  {
    updateInterval = interval;
    pin = pin_param;
    type=sensor_type;
    limit=threshold;
  }
  

  void Update(unsigned long currentMillis)
  {
    if (sound_alarm || stop_pressure_alarms){
      return;
    }
    current_reading = get_analog_reading(pin);
    if (type==HIGH){
      if (current_reading > limit){
        debounce_cnt++;
      }
      else {
        debounce_cnt=0;
      }
      if (debounce_cnt>=DEBOUNCE_THRESHOLD){
        sound_alarm=true;
        Serial.print("High pressure alarm triggered:");
        Serial.print(current_reading);
        debounce_cnt=0;
      }
    }    
    if (type==LOW){
      if (current_reading < limit){
        debounce_cnt++;
      }
      else {
        debounce_cnt=0;
      }
      if (debounce_cnt>=DEBOUNCE_THRESHOLD){
        sound_alarm=true;
        Serial.println("Low pressure alarm triggered");
        Serial.print(current_reading);
        debounce_cnt=0;
      }
    }
  }
};

class SerialInput
{
byte numChars = 15;
char receivedChars[15];   // an array to store the received data
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
      if (String(receivedChars)=="stop"){
        Serial.println("All alarms stopped");
        stop_all_alarms = true;
      }
      if (String(receivedChars)=="stop apnea"){
        Serial.println("apnea alarms stopped");
        stop_apnea_alarm = true;
      }
      if (String(receivedChars)=="start apnea"){
        Serial.println("apnea alarms started");
        stop_apnea_alarm = false;
      }
      if (String(receivedChars)=="start pressure"){
        Serial.println("pressure alarms started");
        stop_pressure_alarms = false;
      }
      if (String(receivedChars)=="stop pressure"){
        Serial.println("pressure alarms stopped");
        stop_pressure_alarms = true;
      }
      if (String(receivedChars)=="start"){
        Serial.println("All alarms started");
        stop_pressure_alarms = false;
        stop_apnea_alarm = false;
        stop_all_alarms = false;        
      }

      newData = false;
    }
  }
  
};

Alarm alarm1(SOUND_INTERVAL);
WaterSensorThreshold water_sensor1(WATER_SENSOR_POLL_INTERVAL, HIGH_PRESSURE_INPUT_PIN, HIGH, HIGH_PRESSURE_THRESHOLD);
WaterSensorThreshold water_sensor2(WATER_SENSOR_POLL_INTERVAL, LOW_PRESSURE_INPUT_PIN, LOW, LOW_PRESSURE_THRESHOLD);
Apnea apnea(WATER_SENSOR_POLL_INTERVAL, APNEA_PIN);
SerialInput serial_input;

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
  
  if (!stop_all_alarms){
    alarm1.Update(currentMillis);   
  }
  
  if (!stop_apnea_alarm){
    apnea.Update(currentMillis);  
  }
  
  if (!stop_pressure_alarms){
    water_sensor1.Update(currentMillis);
    water_sensor2.Update(currentMillis);
    }
    
  serial_input.Update();
  serial_input.ProcessInput();
  
  if (sound_alarm && !alarm_ind){
    alarm_ind=true;
    auto_clear_timer = millis();
    Serial.println("Alarm started");
  }
  
  if (alarm_ind){
    if (millis()-auto_clear_timer > AUTO_CLEAR_ALARM_TIME){
      sound_alarm=false;
      alarm_ind=false;
      Serial.println("Alarm cleared");
      apnea.Reset();
    }
  }
} 

void loop()
{
  
}
  