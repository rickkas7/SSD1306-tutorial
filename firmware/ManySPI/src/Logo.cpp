#include "Adafruit_SSD1306_RK.h"

SYSTEM_THREAD(ENABLED);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET  A5
#define OLED_DC     A4

const uint8_t bitmap6[] = {0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x08, 0x01, 0x80, 0x10, 0x06, 0x03, 0xc0, 0x60, 0x07, 0x03, 0xc0, 0xe0, 0x03, 0xc3, 0xc3, 0xc0, 0x01, 0xe3, 0xcf, 0x80, 0x01, 0xf9, 0x9f, 0x80, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x07, 0xbf, 0xfd, 0xe0, 0xff, 0xdf, 0xfb, 0xff, 0xff, 0xdf, 0xfb, 0xff, 0x07, 0xbf, 0xfd, 0xe0, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x01, 0xf9, 0x9f, 0x80, 0x01, 0xe3, 0xcf, 0x80, 0x03, 0xc3, 0xc3, 0xc0, 0x07, 0x03, 0xc0, 0xe0, 0x06, 0x03, 0xc0, 0x60, 0x08, 0x01, 0x80, 0x10, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00};


typedef struct {
	int 	csPin;
	int 	xx;
	int 	yy;
	int		vx;
	int		vy;
} DisplayConfig;

DisplayConfig displayConfig[4] = {
		{ A3, 16, 16, 1, 1 },
		{ A2, 112, 16, 1, -1 },
		{ A1, 112, 48, -1, -1 },
		{ A0, 16, 48, 1, -1 }
};

class Display : public Adafruit_SSD1306 {
public:
	Display(const DisplayConfig *config, int resetPin) :
			Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, resetPin, config->csPin),
			xx(config->xx), yy(config->yy), vx(config->vx), vy(config->vy) {

	}

	virtual ~Display() {
	}

	void updateDisplay() {
	    clearDisplay();
	    drawBitmap(xx - 16, yy - 16, bitmap6, 32, 32, 1);
	    display();


	    // Algorithm from:
	    // https://www.101computing.net/bouncing-algorithm/
	    xx += vx;
	    yy += vy;

	    if (xx < 16 || xx >= (SCREEN_WIDTH - 16)) {
	    	vx = -vx;
	    	xx += vx;
	    }

	    if (yy < 16 || yy >= (SCREEN_HEIGHT - 16)) {
	    	vy = -vy;
	    	yy += vy;
	    }
	}

	int xx = 0;
	int yy = 0;
	int vx = 0;
	int vy = 0;
};

size_t 	numDisplays;
Display **displays;

const unsigned long UPDATE_INTERVAL_MS = 20;
unsigned long lastUpdate = 0;


void setup() {
	numDisplays = sizeof(displayConfig) / sizeof(displayConfig[0]);

	displays = new Display*[numDisplays];
	for(size_t ii = 0; ii < numDisplays; ii++) {
		displays[ii] = new Display(&displayConfig[ii], ((ii == 0) ? OLED_RESET : -1));

		displays[ii]->begin(SSD1306_SWITCHCAPVCC);
	}
}

void loop() {
	if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
		lastUpdate = millis();

		for(size_t ii = 0; ii < numDisplays; ii++) {
			displays[ii]->updateDisplay();
		}
	}
}




