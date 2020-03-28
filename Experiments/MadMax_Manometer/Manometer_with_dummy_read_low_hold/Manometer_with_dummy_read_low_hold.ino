int ANEODE_OUTPUT_PIN=24;
int CATHODE_OUTPUT_PIN=22;
int MANOMETER_INPUT_PIN=A0;

int value=0;
int dummy_value=0;

void setup() {
  // put your setup code here, to run once:
pinMode(ANEODE_OUTPUT_PIN, OUTPUT);
pinMode(CATHODE_OUTPUT_PIN, OUTPUT);
Serial.begin(9600);
//no need for the analog setup
}

void loop() {
  // real read
  digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);      
  value = analogRead(MANOMETER_INPUT_PIN);

  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, HIGH);  
  Serial.println(value);
  
  //dummy read to avoid tarnishing
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, HIGH);      
  dummy_value = analogRead(MANOMETER_INPUT_PIN);

  digitalWrite(ANEODE_OUTPUT_PIN, HIGH);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);  
  //end dummy read

  //disable circtuit while waiting
  digitalWrite(ANEODE_OUTPUT_PIN, LOW);
  digitalWrite(CATHODE_OUTPUT_PIN, LOW);
  
  delay(10);
}
