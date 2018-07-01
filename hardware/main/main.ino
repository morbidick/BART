#include <SoftwareSerial.h>
#include <bluefruit.h>

#define BAUDRATE 115200
#define MV_PER_LSB 3600.0F/1024.0 // 10-bit ADC with 3.6V input range
#define ABSZERO 273.15
#define MAXANALOGREAD 1023.0
#define T0 25+ABSZERO // Nenntemperatur des NTC-Widerstands in °C
#define R0 1000000 // Nennwiderstand des NTC-Sensors in Ohm
#define B 4608  // Materialkonstante B
#define RV 68000 // Vorwiderstand in Ohm

float tempNTCB(int adcvalue) {
  float VA_VB = adcvalue/MAXANALOGREAD;
  float RN=RV*VA_VB / (1-VA_VB); // aktueller Widerstand des NTC
  return T0 * B / (B + T0 * log(RN / R0))-ABSZERO;
}

void printADC(char *name, int input) {
  int adcvalue = analogRead(input);

  Serial.print(name);
  Serial.print(": ");
  Serial.print(adcvalue);
  Serial.print(" [");
  Serial.print((float)adcvalue * MV_PER_LSB);
  Serial.print(" mV] ");
  Serial.print(tempNTCB(adcvalue));
  Serial.print("°C");

  delay(100);
}

void setup() {
  Serial.begin(BAUDRATE);
}

void loop() {
  printADC("A0", A0);
  Serial.print("    ");
  printADC("A1", A1);
  Serial.println();
}
