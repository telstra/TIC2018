/*
  # Example code for the moisture sensor
  # Connect the sensor to the A0(Analog 0) pin on the Arduino board
  
  # the sensor value description
  # 0  ~300     dry soil
  # 300~700     humid soil
  # 700~950     in water
*/

#define MoisturePin A0

void setup(){
  Serial.begin(115200);
}

void loop(){
  Serial.print("MoistureValue:");
  Serial.println(analogRead(MoisturePin));  
  delay(100);
  
}
