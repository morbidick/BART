#include <SoftwareSerial.h>
#include <bluefruit.h>

#define BAUDRATE 115200
#define MAXANALOGREAD 4095.0 // 12-bit ADC
#define MV_PER_LSB (3300.0F/(MAXANALOGREAD + 1)) // 3.3V input range
#define ABSZERO 273.15
#define T0 (25+ABSZERO) // nominal temperatur NTC-Sensor in Kelvin
#define R0 1000000 // nominal resistance NTC-Sensor in Ohm
#define B 4608  // material constant B
#define RV 68000 // series resistor in Ohm

#define VBAT_PIN (A7)
#define VBAT_DIVIDER_COMP (1.403F) // Compensation factor for the VBAT divider, 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))

#define UUID16_SVC_ENVIRONMENTAL_SENSING   0x181A
#define UUID16_CHR_TEMPERATURE_MEASUREMENT 0x2A1F

#define CONNECTED    0x01
#define DISCONNECTED 0x00

BLEService        envService = BLEService(UUID16_SVC_ENVIRONMENTAL_SENSING);
BLECharacteristic tempChar = BLECharacteristic(UUID16_CHR_TEMPERATURE_MEASUREMENT);

BLEDis bledis; // DIS (Device Information Service) helper class instance
BLEBas blebas; // BAS (Battery Service) helper class instance


// https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/nrf52-adc
void setupADC() {
  analogReference(AR_VDD4);
  analogReadResolution(12);

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

// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Hardware/adc_vbat/adc_vbat.ino
uint8_t batteryCharge() {
  uint8_t battery_level;
  int raw = analogRead(VBAT_PIN);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // VBAT voltage divider is 2M + 0.806M, which needs to be added back
  float mvolts = (float)raw * MV_PER_LSB * VBAT_DIVIDER_COMP;

  if (mvolts >= 4000) {
    battery_level = 100;
  } else if (mvolts > 3900) {
    battery_level = 100 - ((4000 - mvolts) * 58) / 100;
  } else if (mvolts > 3740) {
    battery_level = 42 - ((3900 - mvolts) * 24) / 160;
  } else if (mvolts > 3440) {
    battery_level = 18 - ((3740 - mvolts) * 12) / 300;
  } else if (mvolts > 2100) {
    battery_level = 6 - ((3440 - mvolts) * 6) / 340;
  } else {
    battery_level = 0;
  }

  return battery_level;
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


  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.notify(100);

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


// switch to timers https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Hardware/software_timer/software_timer.ino
void loop() {
  digitalToggle(LED_RED);

  uint8_t battery = batteryCharge();
  blebas.notify(battery);
  Serial.print("Battery charge: ");
  Serial.print(battery);
  Serial.println("%");

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
