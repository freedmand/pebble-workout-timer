#include <pebble.h>
#include "workout_timer.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static void destroy_ui(void) {}
static void initialise_ui(void) {}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_workout_timer(void) {
  initialise_ui();
//   window_set_window_handlers(s_window, (WindowHandlers) {
//     .unload = handle_window_unload,
//   });
//   window_stack_push(s_window, true);
}

void hide_workout_timer(void) {
//   window_stack_remove(s_window, true);
}
