#include "leaf_fall.h"
#include "ws2812.h"

void matrix_init_kb(void) {
	// put your keyboard start-up code here
	// runs once when the firmware starts up
	matrix_init_user();
	ws2812_init();
	ws2812_set_color(0, 0, 0, 255); // Dim blue on power up to show that the keyboard is on and the firmware is running
	ws2812_set_color(1, 0, 0, 255); // Dim blue on power up to show that the keyboard is on and the firmware is running
	ws2812_flush();
//	led_init();
}
