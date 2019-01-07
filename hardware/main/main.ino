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

#define UUID16_SVC_ENVIRONMENTAL_SENSING    0x181A
#define UUID16_CHR_TEMPERATURE_MEASUREMENT0 0x2A1E
#define UUID16_CHR_TEMPERATURE_MEASUREMENT1 0x2A1F

BLEService        envservice = BLEService(UUID16_SVC_ENVIRONMENTAL_SENSING);
BLECharacteristic characteristic_temp0 = BLECharacteristic(UUID16_CHR_TEMPERATURE_MEASUREMENT0);
BLECharacteristic characteristic_temp1 = BLECharacteristic(UUID16_CHR_TEMPERATURE_MEASUREMENT1);

BLEDis bledis; // DIS (Device Information Service) helper class instance

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

char tempADC(int input) {
	char buffer[6];
  float temp = tempNTCB(analogRead(input)) - ABSZERO;
	return snprintf(buffer, sizeof buffer, "%f", temp*100);
}

void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  Serial.println("Advertising!");
}

void setupBLE() {
	// Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();
  Bluefruit.setName("Bluefruit52");

  // Set the connect/disconnect callback handlers
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the Service");
  envservice.begin();
	characteristic_temp0.setProperties(CHR_PROPS_READ ^ CHR_PROPS_NOTIFY);
	characteristic_temp0.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
	characteristic_temp0.begin();
	characteristic_temp0.notify(0, 1);

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
    // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include HTM Service UUID
  Bluefruit.Advertising.addService(envservice);

  // Include Name
  Bluefruit.Advertising.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   *
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
	Bluefruit.Advertising.start(0); // 0 = Don't stop advertising after n seconds

  Serial.println("Ready Player One!!!");
	Serial.println("\nAdvertising");
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
	setupBLE();
}

void loop() {
	digitalToggle(LED_RED);

  if ( Bluefruit.connected() ) {
		if (analogRead(A1) != 0) {
			int temp = analogRead(A1);
			characteristic_temp0.notify32(temp);
			Serial.print("BLE value updated to ");
			Serial.print(temp);
			Serial.println("°C");
		} else {
			characteristic_temp0.notify(0,1);
			Serial.println("probe not connected");
		}
	}

  // Only send update once per second
	delay(1000);
}
