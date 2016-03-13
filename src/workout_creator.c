#include <pebble.h>
#include "workout_creator.h"
#include "workout.h"
#include "number_picker.h"

// Resources
static GFont s_res_gothic_14;
static GFont s_res_gothic_14_bold;
static GBitmap* up_bmp;
static GBitmap* down_bmp;
static GBitmap* ellipses_bmp;

// UI
static Window* s_window;
static Layer* main_layer;
static ActionBarLayer* editor_action_bar;

// Menus
static SimpleMenuLayer *s_modify_event_menu;
static SimpleMenuLayer *s_modify_repetitions_menu;
static SimpleMenuLayer *s_create_event_menu;
static SimpleMenuLayer *s_distance_menu;

static SimpleMenuSection s_modify_event_sections[1];
static SimpleMenuSection s_modify_repetitions_sections[1];
static SimpleMenuSection s_create_event_sections[1];
static SimpleMenuSection s_distance_sections[1];

static SimpleMenuItem s_modify_event_items[2];
static SimpleMenuItem s_modify_repetitions_items[3];
static SimpleMenuItem s_create_event_items[3];
static SimpleMenuItem s_distance_items[6];

// Create pending data
static WorkoutType saved_workout_type;
static int saved_amount_type;

// Workout data
static Workout *root;
static Cursor cursor;

#define LINE_HEIGHT 16
#define LINE_TAB 28
#define REPETITIONS_BOX_WIDTH 24
#define REPETITIONS_LINE_WIDTH 2

int recursive_draw(GContext* ctx, Workout* workout, int left, int top, int width) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Type: %d", workout->type);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Amount: %d", workout->numeric_data);
  
  char str[15];
  int selected = cursor.workout == workout && cursor.type == MODIFY;
  workout_to_str(workout, str, sizeof(str), selected);
  GRect bounds = GRect(left, top, width, LINE_HEIGHT);
  
  // Determine layout based on workout type.
  GFont font;
  int inverted = 0;
  if (workout->type == ROOT) {
    font = s_res_gothic_14;
  } else if (workout->type == REPETITIONS) {
    font = s_res_gothic_14;
    inverted = 1;
  } else if (workout->type == ACTIVITY || workout->type == PLACEHOLDER) {
    font = s_res_gothic_14_bold;
  } else if (workout->type == REST) {
    font = s_res_gothic_14;
  } else {
    font = s_res_gothic_14;
  }
  
  if (inverted) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorWhite);
  } else {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_text_color(ctx, GColorBlack);
  }
  
  // Draw the line of text.
  if (inverted) {
    GRect box = GRect(left, top, REPETITIONS_BOX_WIDTH, LINE_HEIGHT);
    graphics_fill_rect(ctx, box, 0, 0);
    graphics_draw_text(ctx, str, font, box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    if (selected) {
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, " <", font, GRect(left + REPETITIONS_BOX_WIDTH, top, width, LINE_HEIGHT), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    }
  } else {
    graphics_draw_text(ctx, str, font, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }
  // Draw the over/under-line if selected above/after.
  if (cursor.workout == workout) {
    if (cursor.type == BEFORE) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx, GPoint(left, top), GPoint(left + 50, top));
    } else if (cursor.type == AFTER) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx, GPoint(left, top + LINE_HEIGHT), GPoint(left + 50, top + LINE_HEIGHT));
    }
  }
  
  int lines = 1;
  if (workout->child) {
    int lines_drawn = recursive_draw(ctx, workout->child, left + LINE_TAB, top + LINE_HEIGHT, width);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(left + REPETITIONS_BOX_WIDTH / 2 - REPETITIONS_LINE_WIDTH / 2, top + LINE_HEIGHT, REPETITIONS_LINE_WIDTH / 2, lines_drawn * LINE_HEIGHT), 0, 0);
    lines += lines_drawn;
  }
  if (workout->next) {
    lines += recursive_draw(ctx, workout->next, left, top + LINE_HEIGHT * lines, width);
  }
  return lines;
}

static void update_main_layer(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In update routine");
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_text_color(ctx, GColorBlack);
  recursive_draw(ctx, root->child, 5, 5, bounds.size.w);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done Drawing");
}

static void change_event(int index, void *context) {
  if (cursor.type == MODIFY) {
    cursor.workout->numeric_data = 12;
    cursor.workout->numeric_type_data = METERS;
  }
}

void change_amount_callback(int amount) {
  cursor.workout->numeric_data = amount;
}

static void change_amount(int index, void *context) {
  if (cursor.type == MODIFY) {
    if (cursor.workout->type == REPETITIONS) {
      show_number_picker(2, cursor.workout->numeric_data, "Select an amount", "repetitions", PICK_NUMBER, change_amount_callback);
    } else if (cursor.workout->type == ACTIVITY) {
      show_number_picker(4, cursor.workout->numeric_data, "Select a distance", (char*)distance_strings[cursor.workout->numeric_type_data], PICK_NUMBER, change_amount_callback);
    } else if (cursor.workout->type == REST) {
      show_number_picker(4, cursor.workout->numeric_data, "Select a rest time", NULL, PICK_TIME, change_amount_callback);
    }
  }
}

static void delete_event(int index, void *context) {
  if (cursor.type != MODIFY) {
    return;
  }
  
  Workout* past = cursor.workout;

  // Get the delete type.
  DeleteType delete_type = index == 2 ? DELETE_ALL : DELETE_SHIFT;
  delete_workout(&cursor, past, delete_type);
}

static void create_event(int amount) {
  create_workout(&cursor, saved_workout_type, amount, saved_amount_type);
}

static void create_event_handler(int index, void* context) {
  WorkoutType type = index + 1;
  saved_workout_type = type;
  saved_amount_type = 0;
  if (type == REPETITIONS) {
    show_number_picker(2, 2, "Select an amount", "repetitions", PICK_NUMBER, create_event);
  } else if (type == REST) {
    show_number_picker(4, 60, "Select a rest time", NULL, PICK_TIME, create_event);
  }
}

static void create_distance_event_handler(int index, void* context) {
  DistanceType type = index;
  saved_workout_type = ACTIVITY;
  saved_amount_type = type;
  if (type == METERS) {
    show_number_picker(4, 100, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  } else if (type == YARDS) {
    show_number_picker(4, 100, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  } else if (type == KILOMETERS) {
    show_number_picker(4, 1, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  } else if (type == MILES) {
    show_number_picker(4, 1, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  } else if (type == FEET) {
    show_number_picker(4, 100, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  } else if (type == UNKNOWN_DISTANCE) {
    show_number_picker(4, 1, "Select a distance", (char*)distance_strings[type], PICK_NUMBER, create_event);
  }
}

void creator_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (cursor.type == MODIFY && cursor.workout->type != PLACEHOLDER) {
    if (cursor.workout->type == REPETITIONS) {
      // Show repetitions-specific modify menu.
      layer_add_child(main_layer, simple_menu_layer_get_layer(s_modify_repetitions_menu));
    } else {
      // Show general event modify menu.
      layer_add_child(main_layer, simple_menu_layer_get_layer(s_modify_event_menu));
    }
  } else {
    // Show create event menu.
    layer_add_child(main_layer, simple_menu_layer_get_layer(s_create_event_menu));
  }
}

static void init_action_menus() {
  int num_items = 0;
  // 2
  s_modify_event_items[num_items++] = (SimpleMenuItem) {
    .title = "Change amount",
    .callback = change_amount,
  };
  s_modify_event_items[num_items++] = (SimpleMenuItem) {
    .title = "Delete",
    .callback = delete_event,
  };
  
  num_items = 0;
  // 3
  s_modify_repetitions_items[num_items++] = (SimpleMenuItem) {
    .title = "Change repetitions",
    .callback = change_amount,
  };
  s_modify_repetitions_items[num_items++] = (SimpleMenuItem) {
    .title = "Delete",
    .callback = delete_event,
  };
  s_modify_repetitions_items[num_items++] = (SimpleMenuItem) {
    .title = "Delete whole rep",
    .callback = delete_event,
  };
  
  num_items = 0;
  // 3
  s_create_event_items[num_items++] = (SimpleMenuItem) {
    .title = "Create repetitions",
    .callback = create_event_handler,
  };
  s_create_event_items[num_items++] = (SimpleMenuItem) {
    .title = "Create activity",
    .subtitle = "Running workout",
    .callback = create_event_handler,
  };
  s_create_event_items[num_items++] = (SimpleMenuItem) {
    .title = "Create rest",
    .subtitle = "Timed or untimed",
    .callback = create_event_handler,
  };
  
  // 6
  for (num_items = 0; num_items < 6; num_items++) {
    s_distance_items[num_items] = (SimpleMenuItem) {
      .title = distance_strings[num_items],
      .callback = create_distance_event_handler,
    };
  }
  
  s_modify_event_sections[0] = (SimpleMenuSection) {
    .title = "Modify event",
    .num_items = 2,
    .items = s_modify_event_items,
  };
  s_modify_repetitions_sections[0] = (SimpleMenuSection) {
    .title = "Modify event",
    .num_items = 3,
    .items = s_modify_repetitions_items,
  };
  s_create_event_sections[0] = (SimpleMenuSection) {
    .title = "Modify event",
    .num_items = 3,
    .items = s_create_event_items,
  };
  s_distance_sections[0] = (SimpleMenuSection) {
    .title = "Choose a distance unit",
    .num_items = 6,
    .items = s_distance_items,
  };
  
  GRect bounds = layer_get_frame(main_layer);
  s_modify_event_menu = simple_menu_layer_create(bounds, s_window, s_modify_event_sections, 1, NULL);
  s_modify_repetitions_menu = simple_menu_layer_create(bounds, s_window, s_modify_repetitions_sections, 1, NULL);
  s_create_event_menu = simple_menu_layer_create(bounds, s_window, s_create_event_sections, 1, NULL);
  s_distance_menu = simple_menu_layer_create(bounds, s_window, s_distance_sections, 1, NULL);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {  
  move_cursor_up(&cursor);
  layer_mark_dirty(main_layer);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {  
  move_cursor_down(&cursor);
  layer_mark_dirty(main_layer);
}

void creator_config_provider(void *context) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, creator_select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}

static void initialise_ui(void) {
  // Grabbing fonts
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_gothic_14_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  
  // Grabbing icons
  up_bmp = gbitmap_create_with_resource(RESOURCE_ID_up_icon);
  down_bmp = gbitmap_create_with_resource(RESOURCE_ID_down_icon);
  ellipses_bmp = gbitmap_create_with_resource(RESOURCE_ID_ellipses_icon);
  
  // Initializing menus
//   init_action_menus();
  
  s_window = window_create();
  Layer *root_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(root_layer);
  main_layer = layer_create(bounds);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating workout data");
  // Create workout data.
  root = create_init(&cursor);
  
  layer_set_update_proc(main_layer, update_main_layer);
  layer_add_child(root_layer, main_layer);
  
  editor_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(editor_action_bar, s_window);
  // Set the click config provider.
  action_bar_layer_set_click_config_provider(editor_action_bar,
                                             creator_config_provider);

  // Set the icons.
  action_bar_layer_set_icon(editor_action_bar, BUTTON_ID_UP, up_bmp);
  action_bar_layer_set_icon(editor_action_bar, BUTTON_ID_SELECT, ellipses_bmp);
  action_bar_layer_set_icon(editor_action_bar, BUTTON_ID_DOWN, down_bmp);
  
  layer_mark_dirty(main_layer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  layer_destroy(main_layer);
  workout_destroy(root);
  gbitmap_destroy(up_bmp);
  gbitmap_destroy(down_bmp);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_workout_creator(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_workout_creator(void) {
  window_stack_remove(s_window, true);
}
