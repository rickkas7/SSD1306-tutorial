#include "Adafruit_SSD1306_RK.h"

SYSTEM_THREAD(ENABLED);

const int16_t SCREEN_WIDTH = 128; // OLED display width, in pixels
const int16_t SCREEN_HEIGHT = 64; // OLED display height, in pixels
const size_t NUM_SCREENS = 4;

#define OLED_RESET  A5
#define OLED_DC     A4

const int csPins[4] = { A3, A2, A1, A0 };


const uint8_t bitmap6[] = {0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x08, 0x01, 0x80, 0x10, 0x06, 0x03, 0xc0, 0x60, 0x07, 0x03, 0xc0, 0xe0, 0x03, 0xc3, 0xc3, 0xc0, 0x01, 0xe3, 0xcf, 0x80, 0x01, 0xf9, 0x9f, 0x80, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x07, 0xbf, 0xfd, 0xe0, 0xff, 0xdf, 0xfb, 0xff, 0xff, 0xdf, 0xfb, 0xff, 0x07, 0xbf, 0xfd, 0xe0, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x01, 0xf9, 0x9f, 0x80, 0x01, 0xe3, 0xcf, 0x80, 0x03, 0xc3, 0xc3, 0xc0, 0x07, 0x03, 0xc0, 0xe0, 0x06, 0x03, 0xc0, 0x60, 0x08, 0x01, 0x80, 0x10, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00};


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

class Logo {
public:
	Logo(int xx, int yy, int vx, int vy) : xx(xx), yy(yy), vx(vx), vy(vy) {

	}
	virtual ~Logo() {
	}
	void updateLogo() {
		spanDisplay.drawBitmap(xx - 16, yy - 16, bitmap6, 32, 32, 1);

	    // Algorithm from:
	    // https://www.101computing.net/bouncing-algorithm/
	    xx += vx;
	    yy += vy;

	    if (xx < 16 || xx >= (int)(SCREEN_WIDTH * NUM_SCREENS - 16)) {
	    	vx = -vx;
	    	xx += vx;
	    }

	    if (yy < 16 || yy >= (SCREEN_HEIGHT - 16)) {
	    	vy = -vy;
	    	yy += vy;
	    }
	}

	int xx;
	int yy;
	int vx;
	int vy;
};

typedef struct LogoConfig {
	int xx;
	int yy;
	int vx;
	int vy;
};

LogoConfig logoConfig[2] = {
		{ 16, 16, 2, 2 },
		{ SCREEN_WIDTH * NUM_SCREENS - 16, SCREEN_HEIGHT - 16, -3, -2 }
};

size_t numLogos = sizeof(logoConfig) / sizeof(logoConfig[0]);
Logo **logos;

void setup() {

	for(size_t ii = 0; ii < NUM_SCREENS; ii++) {
		displays[ii] = new Display(((ii == 0) ? OLED_RESET : -1), csPins[ii]);

		displays[ii]->begin(SSD1306_SWITCHCAPVCC);
	}

	logos = new Logo*[numLogos];
	for(size_t ii = 0; ii < numLogos; ii++) {
		logos[ii] = new Logo(logoConfig[ii].xx, logoConfig[ii].yy, logoConfig[ii].vx, logoConfig[ii].vy);
	}

}

void loop() {
	if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
		lastUpdate = millis();

		spanDisplay.clearDisplay();

		for(size_t ii = 0; ii < numLogos; ii++) {
			logos[ii]->updateLogo();
		}

		spanDisplay.display();
	}
}

