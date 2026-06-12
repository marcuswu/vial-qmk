/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// ensure that qmk treats tx/rx as normal uart pins
/* used uart pins on the rp2040 */
#define UART_TX_PIN GP12
#define UART_RX_PIN GP13
#define UART_TX_PAL_MODE PAL_MODE_ALTERNATE_UART
#define UART_RX_PAL_MODE PAL_MODE_ALTERNATE_UART

/* key matrix size */
#define MATRIX_ROWS 4
#define MATRIX_COLS 12
#define MATRIX_COLS_LAST_ROW 4 // 4 keys plus 1 power alert bit in the final row
#define POWER_ALERT_BIT true

//#define ONESHOT_TIMEOUT 500

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT

#define WS2812_TIMING 1250
#define WS2812_T1H 650
#define WS2812_T0H 350
#define WS2812_TRST_US 100

#define WS2812_LED_COUNT 2

#define RGBLIGHT_LIMIT_VAL 200