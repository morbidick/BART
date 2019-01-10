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

#define UUID16_SVC_ENVIRONMENTAL_SENSING   0x181A
#define UUID16_CHR_TEMPERATURE_MEASUREMENT 0x2A1F

#define CONNECTED    0x01
#define DISCONNECTED 0x00

BLEService        envService = BLEService(UUID16_SVC_ENVIRONMENTAL_SENSING);
BLECharacteristic tempChar = BLECharacteristic(UUID16_CHR_TEMPERATURE_MEASUREMENT);

BLEDis bledis; // DIS (Device Information Service) helper class instance

// https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/nrf52-adc
void setupADC() {
  analogReference(AR_VDD4);
  analogReadResolution(10);

  // Let the ADC settle
  delay(1);
}

// calculates temperature in 1/10 Celsius for a given ADC value
float tempNTCB(int adcvalue) {
  float VA_VB = adcvalue/MAXANALOGREAD;
  float RN = RV*VA_VB / (1-VA_VB); // current ressistance NTC
  float kelvin = T0 * B / (B + T0 * log(RN / R0));
  return (kelvin - ABSZERO)*10.00;
}

void connect_callback(uint16_t conn_handle) {
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  Serial.println("Advertising!");
}

void setupBLE() {
  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();
  Bluefruit.setName("BarT 9000");

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
  envService.begin();
  tempChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  tempChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  tempChar.begin();
  tempChar.notify8(DISCONNECTED);

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
    // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include HTM Service UUID
  Bluefruit.Advertising.addService(envService);

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

void setup() {
  Serial.begin(BAUDRATE);

  setupADC();
  setupBLE();
}

void loop() {
  digitalToggle(LED_RED);

  if ( Bluefruit.connected() ) {
    int probeValue = analogRead(A1);

    if (probeValue != 0) {
      int temp0 = tempNTCB(probeValue);
      int temp1 = tempNTCB(analogRead(A0));
      uint8_t payload[5] = {CONNECTED, highByte(temp0), lowByte(temp0), highByte(temp1), lowByte(temp1)};
      tempChar.notify(payload, 5);
      Serial.print("BLE value updated to ");
      Serial.print(temp0);
      Serial.println("°C");
    } else {
      tempChar.notify8(DISCONNECTED);
      Serial.println("probe not connected");
    }
  }

  // Only send update once per second
  delay(1000);
}
