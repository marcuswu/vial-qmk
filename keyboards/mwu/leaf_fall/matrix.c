/*
Copyright 2012 Jun Wako
Copyright 2014 Jack Humbert
... (copyright header) ...
*/

#include "config.h"
#include "matrix.h"
#include "uart.h"
#include "print.h" // For debug printing
#include "ws2812.h" // For debug printing

#define UART_MATRIX_RESPONSE_TIMEOUT 10000

// --- PHASE 1 MODIFICATION: Define new constants ---
#define NUM_HALVES 2
#define KEYS_PER_ROW_HALF MATRIX_COLS / NUM_HALVES // Number of keys in each half of a row (e.g. 6 for a 12-column layout)
#define KEYS_PER_ROW_FULL MATRIX_COLS
#define FULL_ROWS MATRIX_ROWS - 1 // Number of full rows (rows that have keys in both halves)
#define KEYS_FINAL_ROW_HALF 4 + 1 // 4 keys in the final row (thumb cluster) plus 1 power alert bit
#define KEYS_PER_HALF (KEYS_PER_ROW_HALF * FULL_ROWS + KEYS_FINAL_ROW_HALF)
#define TOTAL_KEYS (KEYS_PER_HALF * 2)

#define BITS_PER_ROW_HALF KEYS_PER_ROW_HALF // Number of bits needed to represent the state of each half row of keys
// A bit pattern that will never appear in the actual key state data, used to indicate end of frame to host
#define END_OF_FRAME_BYTE (0xFF>>BITS_PER_ROW_HALF) << BITS_PER_ROW_HALF // 0xC0 for 6 columns per half, 0xE0 for 5 columns per half.
#define ROW_MASK (0xFF>>(8-BITS_PER_ROW_HALF))
#define TOTAL_DATA_BYTES ((FULL_ROWS + 1) * 2) // 2 bytes per row for full keyboard

#define TOTAL_PACKET_BYTES (TOTAL_DATA_BYTES + 1)      // +1 for the end-of-frame byte

void matrix_init_custom(void) {
    uart_init(1000000);
    print("matrix_init_custom: UART Initialized at 1M baud (34-key layout)\n");
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    uint32_t timeout = 0;
    bool     changed = false;
    bool    left_power_alert_bit = false;
    bool    right_power_alert_bit = false;
    matrix_row_t row_state = 0;

    uart_write('s');
    uprintf("Scanning...\n");
    
    uint8_t uart_data[TOTAL_PACKET_BYTES] = {0};

    // Read keystate bits
    for (uint8_t i = 0; i < TOTAL_PACKET_BYTES; i++) {
        while (!uart_available()) {
            timeout++;
            if (timeout > UART_MATRIX_RESPONSE_TIMEOUT) {
                break;
            }
        }

        if (timeout < UART_MATRIX_RESPONSE_TIMEOUT) {
            uart_data[i] = uart_read();
        } else {
            uart_data[i] = 0x00;
        }
    }

    // --- Check end byte; exit if invalid ---
    if (uart_data[TOTAL_DATA_BYTES] != END_OF_FRAME_BYTE) {
        uprintf("Error: Invalid end of frame byte! Expected 0x%02X, got 0x%02X\n", END_OF_FRAME_BYTE, uart_data[TOTAL_DATA_BYTES]);
        return false; // Don't update matrix if we didn't get a valid response
    }

    // Print key state data for debugging
    uprintf("Data: ");
    for(int i = 0; i < TOTAL_DATA_BYTES; i++) {
        // each byte in uart_data represents the state of one row of one half of the keyboard,
        // with each bit representing one key
        // key bits are packed to the lower bits of each byte, so we need to shift and mask to extract them
        // even bytes represent the left half of the keyboard, odd bytes represent the right half
        // print each byte as binary, with a separator between left and right halves
        if (i % 2 == 0) {
            uprintf("|"); // Separator for left half
        }
        // Print LSB to MSB to match the physical layout of the keys
        for (int bit = 0; bit < BITS_PER_ROW_HALF; bit++) {
            uprintf("%d", (uart_data[i] >> bit) & 0x01);
        }
    }
    uprintf("\n");

    for (int row = 0; row < FULL_ROWS; row++) {
        int left_idx = row * 2; // Even indices for left half
        int right_idx = left_idx + 1; // Odd indices for right half
        row_state = 0;

        row_state |= (uart_data[left_idx] & ROW_MASK); // Left half cols 0-5
        row_state |= (uart_data[right_idx] & ROW_MASK) << 6; // Right half cols 6-11

        if (current_matrix[row] != row_state) {
            changed = true;
            current_matrix[row] = row_state;
        }
    }
    // handle final row separately since it has a different number of keys and also includes the power alert bit
    int final_row_idx = FULL_ROWS * 2; // Start of final row data in uart_data
    // matrix_row_t row_state = 0;
    // First 4 bits are keys in the final row
    row_state |= (uart_data[final_row_idx] & 0x0F); // Final row cols 0-3
    row_state |= (uart_data[final_row_idx + 1] & 0x0F) << 4; // Map power alert bit to bit 11 (col 11) of the final row
    // 5th bit is the power alert bit, which we will map to the highest bit of the final row's state for now
    left_power_alert_bit = (uart_data[final_row_idx] >> 4) & 0x01; // Extract the power alert bit from bit 4 of the final byte
    right_power_alert_bit = (uart_data[final_row_idx + 1] >> 4) & 0x01; // Extract the power alert bit from bit 4 of the final byte
    if (current_matrix[FULL_ROWS] != row_state) {
        changed = true;
        current_matrix[FULL_ROWS] = row_state;
    }
    if (left_power_alert_bit) {
        uprintf("Left half power alert!\n");
        ws2812_set_color(1, 255, 255, 0); // Yellow if left half power alert is active
    } else {
        uprintf("Left half power OK.\n");
        ws2812_set_color(1, 0, 255, 0); // Green if left half power alert is not active
    }
    if (right_power_alert_bit) {
        uprintf("Right half power alert!\n");
        ws2812_set_color(0, 255, 255, 0); // Yellow if left half power alert is active
    } else {
        uprintf("Right half power OK.\n");
        ws2812_set_color(0, 0, 255, 0); // Green if left half power alert is not active
    }

    return changed;
}