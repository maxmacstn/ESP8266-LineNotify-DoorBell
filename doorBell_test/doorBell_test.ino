/* 
 * Copyright (c) 2019 Sitinut Waisara
 * magiapp.me | github.com/maxmacstn
 * 
 * For test your connection with doorbell.
 * 
 * Hardware : Generic doorbell with active high or you can connect with button + generic Chinese P2P IP Cam
 * 
 */

int signalPin = D1;

void setup() {
  Serial.begin(115200);
  pinMode(signalPin,INPUT);

}

void loop() {
  if(digitalRead(signalPin) == HIGH){
    Serial.println("Doorbell Pressed");
    delay(2000);
  }

}
