#include <SoftwareSerial.h>
#include <bluefruit.h>

#define BAUDRATE 115200
#define MAXANALOGREAD 1023.0
#define MV_PER_LSB (3600.0F/(MAXANALOGREAD + 1)) // 10-bit ADC with 3.6V input range
#define ABSZERO 273.15
#define T0 (25+ABSZERO) // nominal temperatur NTC-Sensor in Kelvin
#define R0 1000000 // nominal resistance NTC-Sensor in Ohm
#define B 4608  // material constant B
#define RV 68000 // series resistor in Ohm

// calculates temperature in Kelvin for a given ADC value, 0 if not connected
float tempNTCB(int adcvalue) {
  // probe not connected so return 0
  if (adcvalue == 0) {
    return 0;
  }
  float VA_VB = adcvalue/MAXANALOGREAD;
  float RN=RV*VA_VB / (1-VA_VB); // current ressistance NTC
  return T0 * B / (B + T0 * log(RN / R0));
}

void printADC(char *name, int input) {
  int adcvalue = analogRead(input);

  Serial.print(name);
  Serial.print(": ");
  Serial.print(adcvalue);
  Serial.print(" [");
  Serial.print((float)adcvalue * MV_PER_LSB);
  Serial.print(" mV, ");
  Serial.print(RV*adcvalue/MAXANALOGREAD / (1-adcvalue/MAXANALOGREAD));
  Serial.print(" Ohm] ");
  Serial.print(tempNTCB(adcvalue) - ABSZERO);
  Serial.print("°C");
}

void setup() {
  Serial.begin(BAUDRATE);
}

void loop() {
  printADC("A0", A0);
  Serial.print("    ");
  printADC("A1", A1);
  Serial.println();

  delay(100);
}
