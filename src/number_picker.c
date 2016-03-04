#include <pebble.h>
#include "number_picker.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_18;
static GFont s_res_gothic_18_bold;
static GFont s_res_bitham_28;
static GFont s_res_bitham_28_bold;

#define NUMBER_PICKER_BOX_WIDTH 23
#define NUMBER_PICKER_BOX_HEIGHT 40
#define NUMBER_PICKER_PADDING 2
#define CONTAINER_WIDTH 114
#define COLON_WIDTH 10
#define COLON_PADDING 4
#define COLON_VERTICAL_OFFSET 11
#define LABEL_OFFSET 2

static int cursor;
static int num_digits;
static NumberPickerType picker_type;
static int* numbers;
static char* title_message;
static char* sub_message;

// Graphics
static GBitmap* up_bmp;
static GBitmap* down_bmp;
static GBitmap* right_bmp;

// UI
static Layer* root_layer;
static ActionBarLayer* picker_action_bar;

static void update_picker(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int v_offset = bounds.size.h / 2 - NUMBER_PICKER_BOX_HEIGHT / 2;
  int width = NUMBER_PICKER_BOX_WIDTH * num_digits + (picker_type == PICK_TIME ? COLON_WIDTH : 0);
  int offset = (CONTAINER_WIDTH - width) / 2;
  
  graphics_context_set_text_color(ctx, GColorBlack);
  if (picker_type == PICK_TIME) {
    // Draw "min" text.
    graphics_draw_text(ctx, "min", s_res_gothic_18_bold, GRect(offset, v_offset + NUMBER_PICKER_BOX_HEIGHT + LABEL_OFFSET, NUMBER_PICKER_BOX_WIDTH * 2, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Draw "sec" text.
    graphics_draw_text(ctx, "sec", s_res_gothic_18_bold, GRect(offset + NUMBER_PICKER_BOX_WIDTH * 2 + COLON_WIDTH, v_offset + NUMBER_PICKER_BOX_HEIGHT + LABEL_OFFSET, NUMBER_PICKER_BOX_WIDTH * 2, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  } else if (sub_message && strlen(sub_message) > 0) {
    graphics_draw_text(ctx, sub_message, s_res_gothic_18_bold, GRect(offset, v_offset + NUMBER_PICKER_BOX_HEIGHT + LABEL_OFFSET, NUMBER_PICKER_BOX_WIDTH * 4, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  
  graphics_draw_text(ctx, title_message, s_res_gothic_18_bold, GRect(2, 10, CONTAINER_WIDTH - 4, v_offset - 10), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  char num_str[4];
  for (int i = 0; i < num_digits; i++) {
    if (picker_type == PICK_TIME && i == 2) {
      // Draw colon.
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_circle(ctx, GPoint(
        offset + COLON_WIDTH / 2,
        v_offset + COLON_VERTICAL_OFFSET), COLON_WIDTH / 2 - COLON_PADDING);
      graphics_fill_circle(ctx, GPoint(
        offset + COLON_WIDTH / 2,
        v_offset + NUMBER_PICKER_BOX_HEIGHT - COLON_VERTICAL_OFFSET), COLON_WIDTH / 2 - COLON_PADDING);
      offset += COLON_WIDTH;
    }
    GFont font;
    if (i == cursor) {
      font = s_res_bitham_28_bold;
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_context_set_text_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_context_set_stroke_color(ctx, GColorBlack);
      font = i < cursor ? s_res_bitham_28_bold : s_res_bitham_28;
    }
    snprintf(num_str, sizeof(num_str), "%d", numbers[i]);
    GRect box = GRect(offset + NUMBER_PICKER_PADDING / 2, v_offset + NUMBER_PICKER_PADDING / 2, NUMBER_PICKER_BOX_WIDTH - NUMBER_PICKER_PADDING / 2, NUMBER_PICKER_BOX_HEIGHT - NUMBER_PICKER_PADDING / 2);
    graphics_fill_rect(ctx, box, 0, 0);
    graphics_draw_text(ctx, num_str, font, box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    graphics_draw_rect(ctx, GRect(offset, v_offset, NUMBER_PICKER_BOX_WIDTH, NUMBER_PICKER_BOX_HEIGHT));
    offset += NUMBER_PICKER_BOX_WIDTH;
  }
}


void change_current_value(int amount) {
  int base = cursor == 2 && picker_type == PICK_TIME ? 6 : 10;
  numbers[cursor] = (numbers[cursor] + amount + base) % base;
  layer_mark_dirty(root_layer);
}

void toggle(ClickRecognizerRef recognizer, void *context) {
  if (cursor < num_digits - 1) {
    cursor++;
  }
  layer_mark_dirty(root_layer);
}

void increment(ClickRecognizerRef recognizer, void *context) {
  change_current_value(1);
}

void decrement(ClickRecognizerRef recognizer, void *context) {
  change_current_value(-1);
}


void picker_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, increment);
  window_single_click_subscribe(BUTTON_ID_SELECT, toggle);
  window_single_click_subscribe(BUTTON_ID_DOWN, decrement);
}

static void initialise_ui(void) {
  s_window = window_create();
  
  // Get font resources.
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_bitham_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_res_bitham_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  
  // Grabbing icons
  up_bmp = gbitmap_create_with_resource(RESOURCE_ID_up_icon);
  down_bmp = gbitmap_create_with_resource(RESOURCE_ID_down_icon);
  right_bmp = gbitmap_create_with_resource(RESOURCE_ID_right_icon);
  
  // Get root layer.
  root_layer = window_get_root_layer(s_window);
  layer_set_update_proc(root_layer, update_picker);
  
  picker_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(picker_action_bar, s_window);
  // Set the click config provider.
  action_bar_layer_set_click_config_provider(picker_action_bar,
                                             picker_config_provider);

  // Set the icons.
  action_bar_layer_set_icon_animated(picker_action_bar, BUTTON_ID_UP, up_bmp, true);
  action_bar_layer_set_icon(picker_action_bar, BUTTON_ID_SELECT, right_bmp);
  action_bar_layer_set_icon_animated(picker_action_bar, BUTTON_ID_DOWN, down_bmp, true);
  
  layer_mark_dirty(root_layer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
}

static void handle_window_unload(Window* window) {
  free(numbers);
  destroy_ui();
}

void show_number_picker(int digits, int initial, char* message, char* units, NumberPickerType type) {
  num_digits = digits;
  numbers = (int*)malloc(sizeof(int) * digits);
  picker_type = type;
  cursor = 0;
  int i;
  int div = 10;
  for (i = 0; i < digits; i++) {
    numbers[digits - i - 1] = (initial % div) / (div / 10);
    div *= 10;
  }
  title_message = message;
  sub_message = units;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_number_picker(void) {
  window_stack_remove(s_window, true);
}
