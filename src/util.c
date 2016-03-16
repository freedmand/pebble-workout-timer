#include <pebble.h>
#include "util.h"

// Return time in milliseconds since epoch
uint64_t get_time() {
  time_t t = 0;
  uint16_t t_ms = 0;
  time_ms(&t, &t_ms);
  
  uint64_t total_time = (uint64_t)t * 1000 + t_ms;
  return total_time;
}

// Supports up to 99 minutes, 59 seconds, and 99 centiseconds
int cents_to_str(uint64_t millis, char* output, int output_size) {
  int round = millis % 10;
  if (round >= 5) {
    millis += 5;
  }
  
  unsigned int cents = (millis / 10) % 100;
  unsigned int seconds = (millis / 1000) % 60;
  uint64_t minutes = (millis / 60000);
  if (minutes > 99) {
    return 0;
  }
  
  snprintf(output, output_size, "%02u:%02u.%02u", (unsigned int)minutes, seconds, cents);
  return 1;
}
