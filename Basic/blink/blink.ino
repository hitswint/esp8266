void setup() {
        // initialize digital pin 13 as an output.
        pinMode(5, OUTPUT);
        Serial.begin(115200);
}
// the loop function runs over and over again forever
void loop() {
        digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(1000);              // wait for a second


        digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW
        delay(1000);              // wait for a second
        
        Serial.println("haha");
}
