void setup() {
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  digitalWrite(3,HIGH);
  delay(1000);
  digitalWrite(3,LOW);
}
