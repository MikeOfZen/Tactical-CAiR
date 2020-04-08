int ANEODE_OUTPUT_PIN=24;
int CATHODE_OUTPUT_PIN=22;

int MANOMETER_INPUT_PINS[4]={A0,A1,A2,A3};

int values[4]={0};
int meas1=0,meas2=0,meas3=0;
int probe_pin=0;
int dummy_value=0;

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
  sense_probes();

  for(int i=0;i<4;i++){
    Serial.print("P");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(values[i]);
    if(i<3)Serial.print(" ");
  }
  Serial.println();
  delay(10);
}

void sense_probes(){

      digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
      digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
      delay(3);
      for(int i=0;i<4;i++){
      // real read
        probe_pin=MANOMETER_INPUT_PINS[i];
        meas1 = analogRead(probe_pin);
        meas2 = analogRead(probe_pin);
        meas3 = analogRead(probe_pin);
        values[i]=(meas1+meas2+meas3)/3;
      }
      //dummy read to avoid tarnishing
      digitalWrite(ANEODE_OUTPUT_PIN, LOW);
      digitalWrite(CATHODE_OUTPUT_PIN, HIGH);      
      delay(3);
      for(int i=0;i<4;i++){
      // dummy read, same operations, no assignment
        probe_pin=MANOMETER_INPUT_PINS[i];
        meas1 = analogRead(probe_pin);
        meas2 = analogRead(probe_pin);
        meas3 = analogRead(probe_pin);
        (meas1+meas2+meas3)/3;
      }
      //disable circuit while waiting
      digitalWrite(ANEODE_OUTPUT_PIN, LOW);
      digitalWrite(CATHODE_OUTPUT_PIN, LOW);   
}
