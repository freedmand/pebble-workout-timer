#include <pebble.h>
#include "stopwatch_mock.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_14;
static GFont s_res_gothic_18;
static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_18_bold;
static TextLayer *s_textlayer_1;
static TextLayer *s_textlayer_2;
static TextLayer *s_textlayer_3;
static TextLayer *s_textlayer_4;
static TextLayer *s_textlayer_5;
static TextLayer *s_textlayer_6;
static TextLayer *s_textlayer_7;
static BitmapLayer *s_bitmaplayer_2;
static BitmapLayer *s_bitmaplayer_3;
static TextLayer *s_textlayer_8;
static TextLayer *s_textlayer_9;
static TextLayer *s_textlayer_10;
static TextLayer *s_textlayer_11;
static TextLayer *s_textlayer_12;
static TextLayer *s_textlayer_13;
static TextLayer *s_textlayer_14;
static TextLayer *s_textlayer_15;
static TextLayer *s_textlayer_16;
static TextLayer *s_textlayer_17;
static TextLayer *s_textlayer_18;
static TextLayer *s_textlayer_19;
static TextLayer *s_textlayer_20;
static TextLayer *s_textlayer_21;
static TextLayer *s_textlayer_22;
static TextLayer *s_textlayer_23;
static TextLayer *s_textlayer_24;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(0, 1, 22, 16));
  text_layer_set_background_color(s_textlayer_1, GColorClear);
  text_layer_set_text(s_textlayer_1, "All");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
  
  // s_textlayer_2
  s_textlayer_2 = text_layer_create(GRect(25, 1, 45, 15));
  text_layer_set_background_color(s_textlayer_2, GColorClear);
  text_layer_set_text(s_textlayer_2, "00:00.00");
  text_layer_set_font(s_textlayer_2, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_2);
  
  // s_textlayer_3
  s_textlayer_3 = text_layer_create(GRect(0, 16, 22, 16));
  text_layer_set_text(s_textlayer_3, "Rep");
  text_layer_set_text_alignment(s_textlayer_3, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_3);
  
  // s_textlayer_4
  s_textlayer_4 = text_layer_create(GRect(25, 16, 45, 15));
  text_layer_set_text(s_textlayer_4, "00:00.00");
  text_layer_set_font(s_textlayer_4, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_4);
  
  // s_textlayer_5
  s_textlayer_5 = text_layer_create(GRect(71, 1, 42, 20));
  text_layer_set_text(s_textlayer_5, "33/57");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_5);
  
  // s_textlayer_6
  s_textlayer_6 = text_layer_create(GRect(71, 16, 42, 20));
  text_layer_set_text(s_textlayer_6, "3/8");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_6);
  
  // s_textlayer_7
  s_textlayer_7 = text_layer_create(GRect(2, 35, 111, 19));
  text_layer_set_text(s_textlayer_7, "Next: (34) 200m");
  text_layer_set_font(s_textlayer_7, s_res_gothic_18);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_7);
  
  // s_bitmaplayer_2
  s_bitmaplayer_2 = bitmap_layer_create(GRect(4, 34, 99, 1));
  bitmap_layer_set_background_color(s_bitmaplayer_2, GColorBlack);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_2);
  
  // s_bitmaplayer_3
  s_bitmaplayer_3 = bitmap_layer_create(GRect(0, 56, 103, 48));
  bitmap_layer_set_background_color(s_bitmaplayer_3, GColorBlack);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_3);
  
  // s_textlayer_8
  s_textlayer_8 = text_layer_create(GRect(2, 71, 79, 29));
  text_layer_set_background_color(s_textlayer_8, GColorClear);
  text_layer_set_text_color(s_textlayer_8, GColorWhite);
  text_layer_set_text(s_textlayer_8, "00:00.00");
  text_layer_set_font(s_textlayer_8, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_8);
  
  // s_textlayer_9
  s_textlayer_9 = text_layer_create(GRect(2, 57, 100, 19));
  text_layer_set_background_color(s_textlayer_9, GColorClear);
  text_layer_set_text_color(s_textlayer_9, GColorWhite);
  text_layer_set_text(s_textlayer_9, "200m");
  text_layer_set_font(s_textlayer_9, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_9);
  
  // s_textlayer_10
  s_textlayer_10 = text_layer_create(GRect(23, 106, 44, 15));
  text_layer_set_text(s_textlayer_10, "00:26.27");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_10);
  
  // s_textlayer_11
  s_textlayer_11 = text_layer_create(GRect(69, 106, 42, 15));
  text_layer_set_text(s_textlayer_11, "200m");
  text_layer_set_font(s_textlayer_11, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_11);
  
  // s_textlayer_12
  s_textlayer_12 = text_layer_create(GRect(23, 121, 44, 15));
  text_layer_set_text(s_textlayer_12, "00.25.29");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_12);
  
  // s_textlayer_13
  s_textlayer_13 = text_layer_create(GRect(23, 136, 44, 15));
  text_layer_set_text(s_textlayer_13, "00.23.28");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_13);
  
  // s_textlayer_14
  s_textlayer_14 = text_layer_create(GRect(23, 151, 44, 15));
  text_layer_set_text(s_textlayer_14, "00:21.29");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_14);
  
  // s_textlayer_15
  s_textlayer_15 = text_layer_create(GRect(81, 85, 21, 15));
  text_layer_set_background_color(s_textlayer_15, GColorClear);
  text_layer_set_text_color(s_textlayer_15, GColorWhite);
  text_layer_set_text(s_textlayer_15, "033");
  text_layer_set_font(s_textlayer_15, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_15);
  
  // s_textlayer_16
  s_textlayer_16 = text_layer_create(GRect(81, 74, 21, 15));
  text_layer_set_background_color(s_textlayer_16, GColorClear);
  text_layer_set_text_color(s_textlayer_16, GColorWhite);
  text_layer_set_text(s_textlayer_16, "LAP");
  text_layer_set_font(s_textlayer_16, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_16);
  
  // s_textlayer_17
  s_textlayer_17 = text_layer_create(GRect(2, 106, 20, 15));
  text_layer_set_text(s_textlayer_17, "032");
  text_layer_set_font(s_textlayer_17, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_17);
  
  // s_textlayer_18
  s_textlayer_18 = text_layer_create(GRect(2, 121, 20, 15));
  text_layer_set_text(s_textlayer_18, "030");
  text_layer_set_font(s_textlayer_18, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_18);
  
  // s_textlayer_19
  s_textlayer_19 = text_layer_create(GRect(2, 136, 20, 15));
  text_layer_set_text(s_textlayer_19, "028");
  text_layer_set_font(s_textlayer_19, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_19);
  
  // s_textlayer_20
  s_textlayer_20 = text_layer_create(GRect(2, 151, 20, 15));
  text_layer_set_text(s_textlayer_20, "026");
  text_layer_set_font(s_textlayer_20, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_20);
  
  // s_textlayer_21
  s_textlayer_21 = text_layer_create(GRect(114, -1, 30, 171));
  text_layer_set_background_color(s_textlayer_21, GColorBlack);
  text_layer_set_text(s_textlayer_21, "Text layer");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_21);
  
  // s_textlayer_22
  s_textlayer_22 = text_layer_create(GRect(69, 136, 42, 15));
  text_layer_set_text(s_textlayer_22, "188m");
  text_layer_set_font(s_textlayer_22, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_22);
  
  // s_textlayer_23
  s_textlayer_23 = text_layer_create(GRect(69, 121, 42, 15));
  text_layer_set_text(s_textlayer_23, "200m");
  text_layer_set_font(s_textlayer_23, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_23);
  
  // s_textlayer_24
  s_textlayer_24 = text_layer_create(GRect(69, 151, 42, 15));
  text_layer_set_text(s_textlayer_24, "160m");
  text_layer_set_font(s_textlayer_24, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_24);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);
  text_layer_destroy(s_textlayer_3);
  text_layer_destroy(s_textlayer_4);
  text_layer_destroy(s_textlayer_5);
  text_layer_destroy(s_textlayer_6);
  text_layer_destroy(s_textlayer_7);
  bitmap_layer_destroy(s_bitmaplayer_2);
  bitmap_layer_destroy(s_bitmaplayer_3);
  text_layer_destroy(s_textlayer_8);
  text_layer_destroy(s_textlayer_9);
  text_layer_destroy(s_textlayer_10);
  text_layer_destroy(s_textlayer_11);
  text_layer_destroy(s_textlayer_12);
  text_layer_destroy(s_textlayer_13);
  text_layer_destroy(s_textlayer_14);
  text_layer_destroy(s_textlayer_15);
  text_layer_destroy(s_textlayer_16);
  text_layer_destroy(s_textlayer_17);
  text_layer_destroy(s_textlayer_18);
  text_layer_destroy(s_textlayer_19);
  text_layer_destroy(s_textlayer_20);
  text_layer_destroy(s_textlayer_21);
  text_layer_destroy(s_textlayer_22);
  text_layer_destroy(s_textlayer_23);
  text_layer_destroy(s_textlayer_24);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_stopwatch_mock(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_stopwatch_mock(void) {
  window_stack_remove(s_window, true);
}
