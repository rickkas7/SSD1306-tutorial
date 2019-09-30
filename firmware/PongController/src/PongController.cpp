#include "Adafruit_BNO055.h" // Library: Adafruit_BNO055_RK

//#include <Adafruit_Sensor.h>
//#include <utility/imumaths.h>

SYSTEM_MODE(MANUAL);

Adafruit_BNO055 bno = Adafruit_BNO055(55);

SerialLogHandler logHandler;

void updateAdvertisingData(bool updateOnly, int16_t tilt);

const unsigned long CHECK_INTERVAL_MS = 100;
unsigned long lastCheck = 0;
bool sensorValid = false;
String localIP;

const BleUuid serviceUuid("b05c776b-6f55-4e99-8aa9-33cbc36a4fc0");


void setup(void) {
	// Initialize the sensor
	sensorValid = bno.begin();
	if (sensorValid) {
		// Use external crystal
		bno.setExtCrystalUse(true);

		updateAdvertisingData(false, 0);
	}
	else {
		// Wait for a USB serial connection for up to 15 seconds
		waitFor(Serial.isConnected, 15000);

		Log.info("BNO055 not found on I2C bus");
	}
}


void loop(void) {
	if (sensorValid && millis() - lastCheck >= CHECK_INTERVAL_MS) {
		lastCheck = millis();

		sensors_event_t event;
		bno.getEvent(&event);

		// y is the roll setting, front to back with the USB connector on the left
		// event.orientation.y
		// Ranges from -180 to +180, 0 is laying flat with the top of the Xenon facing up

		updateAdvertisingData(true, (int16_t) event.orientation.y );
	}
}


void updateAdvertisingData(bool updateOnly, int16_t tilt) {
	uint8_t buf[BLE_MAX_ADV_DATA_LEN];

	size_t offset = 0;

	// Company ID (0xffff internal use/testing)
	buf[offset++] = 0xff;
	buf[offset++] = 0xff;

	// Our specific data is the ADC value, as uint16_t in big endian byte order
	buf[offset++] = (uint8_t)(tilt >> 8);
	buf[offset++] = (uint8_t)tilt;

	BleAdvertisingData advData;
	advData.appendCustomData(buf, offset);
	advData.appendServiceUUID(serviceUuid);

	if (updateOnly) {
		// Only update data
		BLE.setAdvertisingData(&advData);
	}
	else {
		// Advertise every 100 milliseconds. Unit is 0.625 millisecond intervals.
		// TODO: Possibly reduce to 50 ms
		BLE.setAdvertisingInterval(130);

		// Continuously advertise
		BLE.advertise(&advData);
	}
}
