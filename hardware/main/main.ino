#include <SoftwareSerial.h>
#include <bluefruit.h>

#define BAUDRATE 115200
#define MAXANALOGREAD 1023.0 // 10-bit ADC
#define MV_PER_LSB (3300.0F/(MAXANALOGREAD + 1)) // 3.3V input range
#define ABSZERO 273.15
#define T0 (25+ABSZERO) // nominal temperatur NTC-Sensor in Kelvin
#define R0 1000000 // nominal resistance NTC-Sensor in Ohm
#define B 4608  // material constant B
#define RV 68000 // series resistor in Ohm

// https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/nrf52-adc
void setupADC() {
  analogReference(AR_VDD4);
  analogReadResolution(10);

  // Let the ADC settle
  delay(1);
}

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

// calculates the Voltage for a given ADC input
float adcVoltage(int input) {
  return input * MV_PER_LSB;
}
// calculates the series resistance for a given ADC input
float adcSeriesResistance(int input) {
  return (RV*input/MAXANALOGREAD / (1-input/MAXANALOGREAD)) / 1000;
}

void printADC(char *name, int input) {
  int adcvalue = analogRead(input);

  Serial.print(name);
  Serial.print(": ");
  Serial.print(adcvalue);
  Serial.print(".a -> ");
  Serial.print(adcVoltage(adcvalue));
  Serial.print("mV -> ");
  Serial.print(adcSeriesResistance(adcvalue));
  Serial.print("kOhm -> ");
  Serial.print(tempNTCB(adcvalue) - ABSZERO);
  Serial.print(" °C");
}

void setup() {
  Serial.begin(BAUDRATE);

  setupADC();
}

void loop() {
  if (analogRead(A0) < MAXANALOGREAD) {
    printADC("Tip", A0);
    Serial.print("  |  ");
    printADC("Handle", A1);
    Serial.println();
  } else {
    Serial.println("probe not connected");
  }

  delay(100);
}
