#include "Adafruit_SSD1306_RK.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

const int16_t SCREEN_WIDTH = 128; // OLED display width, in pixels
const int16_t SCREEN_HEIGHT = 64; // OLED display height, in pixels
const size_t NUM_SCREENS = 2;

#define OLED_RESET  A5
#define OLED_DC     A4

const int csPins[4] = { A3, A2 };


class Display : public Adafruit_SSD1306 {
public:
	Display(int resetPin, int csPin) :
		Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, resetPin, csPin) {

	}

	virtual ~Display() {
	}

};

Display *displays[NUM_SCREENS];


const unsigned long UPDATE_INTERVAL_MS = 20;
unsigned long lastUpdate = 0;
int xx = 16;
int yy = 16;
int vx = 2;
int vy = 2;

class HorizontalSpanDisplay : public Adafruit_GFX {
public:
	HorizontalSpanDisplay() : Adafruit_GFX(SCREEN_WIDTH * NUM_SCREENS, SCREEN_HEIGHT) {
	}

	virtual ~HorizontalSpanDisplay() {

	}

	void drawPixel(int16_t x, int16_t y, uint16_t color) {
		int screenNum = x / SCREEN_WIDTH;
		if (screenNum >= 0 && screenNum < (int)NUM_SCREENS) {
			int displayx = x - screenNum * SCREEN_WIDTH;
			displays[screenNum]->drawPixel(displayx, y, color);
		}
	}

	void clearDisplay() {
		for(size_t ii = 0; ii < NUM_SCREENS; ii++) {
			displays[ii]->clearDisplay();
		}
	}

	void display() {
		for(size_t ii = 0; ii < NUM_SCREENS; ii++) {
			displays[ii]->display();
		}
	}

};

HorizontalSpanDisplay spanDisplay;

class Player {
public:
	String remoteIP;
	int pos = 32;
	unsigned long lastUpdated = 0;
};

const size_t NUM_PLAYERS = 2;
Player players[NUM_PLAYERS];

const BleUuid serviceUuid("b05c776b-6f55-4e99-8aa9-33cbc36a4fc0");

void setup() {

	for(size_t ii = 0; ii < NUM_SCREENS; ii++) {
		displays[ii] = new Display(((ii == 0) ? OLED_RESET : -1), csPins[ii]);

		displays[ii]->begin(SSD1306_SWITCHCAPVCC);
	}

}

void loop() {
	if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
		lastUpdate = millis();

		spanDisplay.clearDisplay();

		// Update display here

		spanDisplay.display();
	}
}

size_t findPlayer(const char *remoteIP) {

	for(size_t ii = 0; ii < NUM_PLAYERS; ii++) {
		if (players[ii].remoteIP == remoteIP) {
			// Found it
			players[ii].lastUpdated = millis();
			return ii;
		}
	}

	// Not found; find an empty slot if possible
	for(size_t ii = 0; ii < NUM_PLAYERS; ii++) {
		if (players[ii].remoteIP.length() == 0) {
			// Found an empty slot
			players[ii].remoteIP = remoteIP;
			players[ii].lastUpdated = millis();
			return ii;
		}
	}

	// Find oldest last seen and replace it
	unsigned long oldestAge = 0;
	size_t oldestIndex = 0;
	for(size_t ii = 0; ii < NUM_PLAYERS; ii++) {
		unsigned long age = millis() - players[ii].lastUpdated;
		if (age > oldestAge) {
			oldestAge = age;
			oldestIndex = ii;
		}
	}

	players[oldestIndex].remoteIP = remoteIP;
	players[oldestIndex].lastUpdated = millis();
	return oldestIndex;
}




