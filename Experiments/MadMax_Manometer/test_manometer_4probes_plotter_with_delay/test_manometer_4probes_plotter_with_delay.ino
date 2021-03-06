int ANEODE_OUTPUT_PIN=24;
int CATHODE_OUTPUT_PIN=22;

int MANOMETER_INPUT_PINS[6]={A0,A1,A2,A3};

int value=0;
int meas1=0,meas2=0,meas3=0;
int dummy_value=0;

int THERSHOLD=80;
long zero_time = 0;
long cur_time=0;

void setup() {
  // put your setup code here, to run once:
pinMode(ANEODE_OUTPUT_PIN, OUTPUT);
pinMode(CATHODE_OUTPUT_PIN, OUTPUT);
Serial.begin(115200);
//no need for the analog setup
zero_time =millis();
}

void loop() {
  for(int i=0;i<4;i++){
  // real read

  value=sense_probe(MANOMETER_INPUT_PINS[i]);
  Serial.print("P");
  Serial.print(i);
  Serial.print(" ");
  Serial.print(value);
  if(i<3)Serial.print(" ");
 
  //delay(1);
  }
  Serial.println();
  delay(10);
}

int sense_probe(int probe_pin){
    digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
delay(3);
  meas1 = analogRead(probe_pin);
  meas2 = analogRead(probe_pin);
  meas3 = analogRead(probe_pin);
  value=(meas1+meas2+meas3)/3;
//delay(7);
  //dummy read to avoid tarnishing
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, HIGH);      
delay(3);
  dummy_value = analogRead(probe_pin);
  dummy_value = analogRead(probe_pin);
  dummy_value = analogRead(probe_pin);
  //disable circtuit while waiting
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);
  return value;
}
