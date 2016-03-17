#include <pebble.h>
#include "workout_creator.h"
#include "workout.h"
#include "number_picker.h"
#include "workout_steps.h"

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
static ActionMenu* s_action_menu;
static ActionMenuLevel* s_modify_event_level;
static ActionMenuLevel* s_modify_repetitions_level;
static ActionMenuLevel* s_create_event_level;
static ActionMenuLevel* s_distance_level;

// Create pending data
static WorkoutType saved_workout_type;
static int saved_amount_type;

// Workout data
static Workout *root;
static Cursor cursor;

static void (*saved_callback)(Workout*);

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

static void change_event(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  if (cursor.type == MODIFY) {
    cursor.workout->numeric_data = 12;
    cursor.workout->numeric_type_data = METERS;
  }
}

void change_amount_callback(int amount) {
  cursor.workout->numeric_data = amount;
}

static void change_amount(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
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

static void delete_event(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  if (cursor.type != MODIFY) {
    return;
  }
  
  Workout* past = cursor.workout;

  // Get the delete type.
  DeleteType delete_type = (DeleteType)action_menu_item_get_action_data(action);
  delete_workout(&cursor, past, delete_type);
}

static void create_event(int amount) {
  create_workout(&cursor, saved_workout_type, amount, saved_amount_type);
}

static void create_event_handler(ActionMenu *action_menu, const ActionMenuItem *action, void* context) {
  WorkoutType type = (WorkoutType)action_menu_item_get_action_data(action);
  saved_workout_type = type;
  saved_amount_type = 0;
  if (type == REPETITIONS) {
    show_number_picker(2, 2, "Select an amount", "repetitions", PICK_NUMBER, create_event);
  } else if (type == REST) {
    show_number_picker(4, 60, "Select a rest time", NULL, PICK_TIME, create_event);
  }
}

static void create_distance_event_handler(ActionMenu *action_menu, const ActionMenuItem *action, void* context) {
  DistanceType type = (DistanceType)action_menu_item_get_action_data(action);
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

static void hide_workout_creator_handler(ActionMenu *action_menu, const ActionMenuItem *action, void* context) {
  show_workout_steps_window(root);
}

void creator_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (cursor.type == MODIFY && cursor.workout->type != PLACEHOLDER) {
    if (cursor.workout->type == REPETITIONS) {
      // Show repetitions-specific modify menu.
      ActionMenuConfig config = (ActionMenuConfig) {
        .root_level = s_modify_repetitions_level,
        .colors = {
          .background = GColorWhite,
          .foreground = GColorBlack,
        },
        .align = ActionMenuAlignCenter
      };
      s_action_menu = action_menu_open(&config);
    } else {
      // Show general event modify menu.
      ActionMenuConfig config = (ActionMenuConfig) {
        .root_level = s_modify_event_level,
        .colors = {
          .background = GColorWhite,
          .foreground = GColorBlack,
        },
        .align = ActionMenuAlignCenter
      };
      s_action_menu = action_menu_open(&config);
    }
  } else {
    // Show create event menu.
    ActionMenuConfig config = (ActionMenuConfig) {
      .root_level = s_create_event_level,
      .colors = {
        .background = GColorWhite,
        .foreground = GColorBlack,
      },
        .align = ActionMenuAlignCenter
    };
    s_action_menu = action_menu_open(&config);
  }
}



static void init_action_menus() {
  s_modify_event_level = action_menu_level_create(3);
  action_menu_level_add_action(s_modify_event_level, "Change Amount", change_amount, NULL);
  action_menu_level_add_action(s_modify_event_level, "Delete", delete_event, (void *)DELETE_SHIFT);
  action_menu_level_add_action(s_modify_event_level, "Done", hide_workout_creator_handler, NULL);
  
  s_modify_repetitions_level = action_menu_level_create(3);
  action_menu_level_add_action(s_modify_repetitions_level, "Change repetitions", change_amount, NULL);
  action_menu_level_add_action(s_modify_repetitions_level, "Delete", delete_event, (void *)DELETE_SHIFT);
  action_menu_level_add_action(s_modify_repetitions_level, "Delete all in group", delete_event, (void *)DELETE_ALL);
  
  s_create_event_level = action_menu_level_create(6);
  action_menu_level_add_action(s_create_event_level, "Create repetitions", create_event_handler, (void *)REPETITIONS);
  // Create and set up the secondary level, adding it as a child to the root one
  s_distance_level = action_menu_level_create(6);
  action_menu_level_add_child(s_create_event_level, s_distance_level, "Create distance event");
  int i;
  for (i = 0; i < 6; i++) {
    // Set up the secondary actions
    action_menu_level_add_action(s_distance_level, distance_strings[i], create_distance_event_handler, (void *)i);
  }
  action_menu_level_add_action(s_create_event_level, "Create rest event", create_event_handler, (void *)REST);
  action_menu_level_add_action(s_create_event_level, "Create gym event", create_event_handler, NULL);
  action_menu_level_add_action(s_create_event_level, "Create custom event", create_event_handler, NULL);
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
  init_action_menus();
  
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
  action_bar_layer_set_icon_animated(editor_action_bar, BUTTON_ID_UP, up_bmp, true);
  action_bar_layer_set_icon(editor_action_bar, BUTTON_ID_SELECT, ellipses_bmp);
  action_bar_layer_set_icon_animated(editor_action_bar, BUTTON_ID_DOWN, down_bmp, true);
  
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

void show_workout_creator(Workout* init, void (*callback)(Workout*)) {
  saved_callback = callback;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_workout_creator(void) {
  window_stack_remove(s_window, true);
  saved_callback(root);
}
