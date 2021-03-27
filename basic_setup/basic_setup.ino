// Basic use for PZEM-004T with Arduino/ESP chip

#include<PZEM004Tv30.h>

/* Use software serial for the PZEM
    * Pin D6 (Connects to the RX pin on the PZEM)
    * Pin D5 (Connects to the TX pin on the PZEM)
*/
PZEM004Tv30 pzem(D6, D5);

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Here are basic info
    float voltage = pzem.voltage();
    float current = pzem.current();
    float energy = pzem.energy();
    // We can print these numbers out
    Serial.printf("V: %.0f\tA: %.1f\tkWh: %.1f\n", voltage, current, energy);
    delay(5000);
}
