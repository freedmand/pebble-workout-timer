#include <pebble.h>
// #include "timer_window.h"
#include "workout_creator.h"
#include "number_picker.h"

static Window *timer_window;

static Layer *timer_window_layer;
static Layer *main_timer_layer;

static char main_count[9] = "00:00.00";

static uint64_t start_time;
static uint64_t pause_time;
static int paused = 0;


static GFont s_res_gothic_28_bold;
static GBitmap *s_res_stopwatch_icon;
static GFont s_res_gothic_14;
static GBitmap *s_res_play_pause_icon;
static GFont s_res_gothic_18;
static TextLayer *split_time_layer;
static BitmapLayer *stopwatch_icon_layer;
static TextLayer *split_name_layer;
static TextLayer *splits_text_layer;
static BitmapLayer *play_pause_icon_layer;
static TextLayer *split1_layer;
static TextLayer *split2_layer;
static TextLayer *split3_layer;
static TextLayer *split4_layer;
static TextLayer *current_hour;
static TextLayer *workout_time_layer;
static TextLayer *period_layer;
static TextLayer *workout_name_layer;

static GRect workout_time_bounds;

static AppTimer *timer;

const int TIMER_UPDATE_INTERVAL = 67;

// Return time in milliseconds since epoch
uint64_t get_time() {
  time_t t = 0;
  uint16_t t_ms = 0;
  time_ms(&t, &t_ms);
  
  uint64_t total_time = (uint64_t)t * 1000 + t_ms;
  return total_time;
}

// Supports up to 99 minutes, 59 seconds, and 99 centiseconds
int millis_to_str(uint64_t millis, char* output) {
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
  
  snprintf(output, 9, "%02u:%02u.%02u", (unsigned int)minutes, seconds, cents);
  return 1;
}

static void update_layer_callback(Layer *layer, GContext *ctx) {
  uint64_t now = get_time();
  uint64_t current_time;
  if (paused) {
    current_time = pause_time - start_time;
  } else {
    current_time = now - start_time;
  }
  millis_to_str(current_time, main_count);
  graphics_context_set_text_color(ctx, GColorBlack);
  
  // Draw workout time
  graphics_draw_text(ctx, main_count, s_res_gothic_18, workout_time_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void timer_callback(void *data) {
  layer_mark_dirty(timer_window_layer);
  timer = app_timer_register(TIMER_UPDATE_INTERVAL, timer_callback, NULL);
}

static void timer_window_load(Window *window) {
  timer_window_layer = window_get_root_layer(window);
  
  // Create a layer for the main timer.
  GRect bounds = layer_get_bounds(timer_window_layer);
  main_timer_layer = layer_create(bounds);
  
  // BEGIN AUTO_GEN
  // Grab font resources.
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_stopwatch_icon = gbitmap_create_with_resource(RESOURCE_ID_stopwatch_icon);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_play_pause_icon = gbitmap_create_with_resource(RESOURCE_ID_play_pause_icon);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  // split_time_layer
  split_time_layer = text_layer_create(GRect(8, 53, 93, 33));
  text_layer_set_text(split_time_layer, "00:00.00");
  text_layer_set_font(split_time_layer, s_res_gothic_28_bold);
  layer_add_child(main_timer_layer, (Layer *)split_time_layer);
  
  // stopwatch_icon_layer
  stopwatch_icon_layer = bitmap_layer_create(GRect(116, 122, 24, 24));
  bitmap_layer_set_bitmap(stopwatch_icon_layer, s_res_stopwatch_icon);
  bitmap_layer_set_background_color(stopwatch_icon_layer, GColorWhite);
  layer_add_child(main_timer_layer, (Layer *)stopwatch_icon_layer);
  
  // split_name_layer
  split_name_layer = text_layer_create(GRect(8, 40, 93, 15));
  text_layer_set_text(split_name_layer, "Interval 1");
  text_layer_set_font(split_name_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)split_name_layer);
  
  // splits_text_layer
  splits_text_layer = text_layer_create(GRect(2, 90, 112, 20));
  text_layer_set_text(splits_text_layer, "No splits recorded");
  layer_add_child(main_timer_layer, (Layer *)splits_text_layer);
  
  // play_pause_icon_layer
  play_pause_icon_layer = bitmap_layer_create(GRect(118, 59, 24, 24));
  bitmap_layer_set_bitmap(play_pause_icon_layer, s_res_play_pause_icon);
  bitmap_layer_set_background_color(play_pause_icon_layer, GColorWhite);
  layer_add_child(main_timer_layer, (Layer *)play_pause_icon_layer);
  
  // split1_layer
  split1_layer = text_layer_create(GRect(2, 105, 100, 15));
  text_layer_set_text(split1_layer, " ");
  text_layer_set_font(split1_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)split1_layer);
  
  // split2_layer
  split2_layer = text_layer_create(GRect(2, 120, 100, 15));
  text_layer_set_text(split2_layer, " ");
  text_layer_set_font(split2_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)split2_layer);
  
  // split3_layer
  split3_layer = text_layer_create(GRect(2, 135, 100, 15));
  text_layer_set_text(split3_layer, " ");
  text_layer_set_font(split3_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)split3_layer);
  
  // split4_layer
  split4_layer = text_layer_create(GRect(2, 150, 100, 15));
  text_layer_set_text(split4_layer, " ");
  text_layer_set_font(split4_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)split4_layer);
  
  // current_hour
  current_hour = text_layer_create(GRect(115, 3, 27, 14));
  text_layer_set_text(current_hour, " ");
  text_layer_set_font(current_hour, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)current_hour);
  
  // workout_time_layer
  workout_time_bounds = GRect(2, 17, 100, 20);
//   workout_time_layer = text_layer_create();
//   text_layer_set_text(workout_time_layer, "00:00.00");
//   text_layer_set_font(workout_time_layer, s_res_gothic_18);
//   layer_add_child(main_timer_layer, (Layer *)workout_time_layer);
  
  // period_layer
  period_layer = text_layer_create(GRect(126, 18, 15, 13));
  text_layer_set_text(period_layer, " ");
  text_layer_set_font(period_layer, s_res_gothic_14);
  layer_add_child(main_timer_layer, (Layer *)period_layer);
  
  // workout_name_layer
  workout_name_layer = text_layer_create(GRect(2, 2, 109, 18));
  text_layer_set_background_color(workout_name_layer, GColorClear);
  text_layer_set_text(workout_name_layer, "Total Time");
  layer_add_child(main_timer_layer, (Layer *)workout_name_layer);
  
  // END AUTO_GEN
  

  start_time = get_time();
  layer_set_update_proc(main_timer_layer, update_layer_callback);
  timer = app_timer_register(TIMER_UPDATE_INTERVAL, timer_callback, NULL);
  layer_add_child(timer_window_layer, main_timer_layer);
}

static void timer_window_unload(Window *window) {
  layer_destroy(main_timer_layer);
}

void pause() {
  pause_time = get_time();
  paused = 1;
}

void resume() {
  start_time += (get_time() - pause_time);
  paused = 0;
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (paused) {
    resume();
  } else {
    pause();
  }
}

void config_provider(Window *window) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

static void callback(int amount) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received amount %d", amount);
}

static void init() {
  show_workout_creator();
//   show_number_picker(4, 800, "Select a distance", "meters", PICK_NUMBER, callback);
//   show_timer_window();
//   timer_window = window_create();
  
//   window_set_window_handlers(timer_window, (WindowHandlers) {
//     .load = timer_window_load,
//     .unload = timer_window_unload
//   });
  
//   window_stack_push(timer_window, true);
//   window_set_click_config_provider(timer_window, (ClickConfigProvider) config_provider);
}

static void deinit() {
  window_destroy(timer_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}