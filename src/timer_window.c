#include <pebble.h>
#include "timer_window.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
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

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_stopwatch_icon = gbitmap_create_with_resource(RESOURCE_ID_stopwatch_icon);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_play_pause_icon = gbitmap_create_with_resource(RESOURCE_ID_play_pause_icon);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  // split_time_layer
  split_time_layer = text_layer_create(GRect(8, 53, 93, 33));
  text_layer_set_text(split_time_layer, "00:00.00");
  text_layer_set_font(split_time_layer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split_time_layer);
  
  // stopwatch_icon_layer
  stopwatch_icon_layer = bitmap_layer_create(GRect(116, 122, 24, 24));
  bitmap_layer_set_bitmap(stopwatch_icon_layer, s_res_stopwatch_icon);
  bitmap_layer_set_background_color(stopwatch_icon_layer, GColorWhite);
  layer_add_child(window_get_root_layer(s_window), (Layer *)stopwatch_icon_layer);
  
  // split_name_layer
  split_name_layer = text_layer_create(GRect(8, 40, 93, 15));
  text_layer_set_text(split_name_layer, "Interval 1");
  text_layer_set_font(split_name_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split_name_layer);
  
  // splits_text_layer
  splits_text_layer = text_layer_create(GRect(2, 90, 112, 20));
  text_layer_set_text(splits_text_layer, "No splits recorded");
  layer_add_child(window_get_root_layer(s_window), (Layer *)splits_text_layer);
  
  // play_pause_icon_layer
  play_pause_icon_layer = bitmap_layer_create(GRect(118, 59, 24, 24));
  bitmap_layer_set_bitmap(play_pause_icon_layer, s_res_play_pause_icon);
  bitmap_layer_set_background_color(play_pause_icon_layer, GColorWhite);
  layer_add_child(window_get_root_layer(s_window), (Layer *)play_pause_icon_layer);
  
  // split1_layer
  split1_layer = text_layer_create(GRect(2, 105, 100, 15));
  text_layer_set_text(split1_layer, " ");
  text_layer_set_font(split1_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split1_layer);
  
  // split2_layer
  split2_layer = text_layer_create(GRect(2, 120, 100, 15));
  text_layer_set_text(split2_layer, " ");
  text_layer_set_font(split2_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split2_layer);
  
  // split3_layer
  split3_layer = text_layer_create(GRect(2, 135, 100, 15));
  text_layer_set_text(split3_layer, " ");
  text_layer_set_font(split3_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split3_layer);
  
  // split4_layer
  split4_layer = text_layer_create(GRect(2, 150, 100, 15));
  text_layer_set_text(split4_layer, " ");
  text_layer_set_font(split4_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)split4_layer);
  
  // current_hour
  current_hour = text_layer_create(GRect(115, 3, 27, 14));
  text_layer_set_text(current_hour, " ");
  text_layer_set_font(current_hour, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)current_hour);
  
  // workout_time_layer
  workout_time_layer = text_layer_create(GRect(2, 17, 100, 20));
  text_layer_set_text(workout_time_layer, "00:00.00");
  text_layer_set_font(workout_time_layer, s_res_gothic_18);
  layer_add_child(window_get_root_layer(s_window), (Layer *)workout_time_layer);
  
  // period_layer
  period_layer = text_layer_create(GRect(126, 18, 15, 13));
  text_layer_set_text(period_layer, " ");
  text_layer_set_font(period_layer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)period_layer);
  
  // workout_name_layer
  workout_name_layer = text_layer_create(GRect(2, 2, 109, 18));
  text_layer_set_background_color(workout_name_layer, GColorClear);
  text_layer_set_text(workout_name_layer, "Total Time");
  layer_add_child(window_get_root_layer(s_window), (Layer *)workout_name_layer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(split_time_layer);
  bitmap_layer_destroy(stopwatch_icon_layer);
  text_layer_destroy(split_name_layer);
  text_layer_destroy(splits_text_layer);
  bitmap_layer_destroy(play_pause_icon_layer);
  text_layer_destroy(split1_layer);
  text_layer_destroy(split2_layer);
  text_layer_destroy(split3_layer);
  text_layer_destroy(split4_layer);
  text_layer_destroy(current_hour);
  text_layer_destroy(workout_time_layer);
  text_layer_destroy(period_layer);
  text_layer_destroy(workout_name_layer);
  gbitmap_destroy(s_res_stopwatch_icon);
  gbitmap_destroy(s_res_play_pause_icon);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_timer_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_timer_window(void) {
  window_stack_remove(s_window, true);
}
