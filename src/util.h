#pragma once
#include <pebble.h>

// Return time in milliseconds since epoch
uint64_t get_time();

// Supports up to 99 minutes, 59 seconds, and 99 centiseconds
int cents_to_str(uint64_t millis, char* output, int output_size);
