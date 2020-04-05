int ANEODE_OUTPUT_PIN=24;
int CATHODE_OUTPUT_PIN=23;

int MANOMETER_INPUT_PINS[6]={A0,A1,A2,A3};

int value=0;
int dummy_value=0;

int THERSHOLD=80;

void setup() {
  // put your setup code here, to run once:
pinMode(ANEODE_OUTPUT_PIN, OUTPUT);
pinMode(CATHODE_OUTPUT_PIN, OUTPUT);
Serial.begin(9600);
//no need for the analog setup
}

void loop() {
  for(int i=0;i<4;i++){
  // real read

  value=sense_probe(MANOMETER_INPUT_PINS[i]);
  Serial.print("Probe");
  Serial.print(i+1,DEC);
  Serial.print(":");
  Serial.print(value);
  Serial.print(" ");
  delay(1);
  }
  Serial.println();
  delay(1);
}

int sense_probe(int probe_pin){
    digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
  value = analogRead(probe_pin);

  //dummy read to avoid tarnishing
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, HIGH);      
  dummy_value = analogRead(probe_pin);

  //disable circtuit while waiting
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);
  return value;
}
