#include <pebble.h>
#include "workout_creator.h"


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

// Enums
typedef enum {ROOT, REPETITIONS, ACTIVITY, REST, CUSTOM, PLACEHOLDER} WorkoutType;
typedef enum {METERS, YARDS, KILOMETERS, MILES, FEET, UNKNOWN_DISTANCE} DistanceType;
typedef enum {MINUTES, SECONDS} TimeType;
typedef enum {BEFORE, MODIFY, AFTER} CursorType;
typedef enum {DELETE_SHIFT, DELETE_ALL} DeleteType;

#define LINE_HEIGHT 16
#define LINE_TAB 28
#define REPETITIONS_BOX_WIDTH 24
#define REPETITIONS_LINE_WIDTH 2

typedef struct Workout {
  WorkoutType type;
  int numeric_data;
  int numeric_type_data;
  char* custom_msg;
  // Assigned automatically with helper methods.
  struct Workout* next;
  struct Workout* previous;
  struct Workout* child;
  struct Workout* child_last;
  struct Workout* parent;
} Workout;

typedef struct {
  Workout* workout;
  CursorType type;
} Cursor;

static Workout *root;
static Cursor cursor;

static void workout_to_str(Workout* workout, char* output, int size, int selected) {
  char selected_str[3] = "";
  if (selected) {
    snprintf(selected_str, sizeof(selected_str), " <");
  }
  
  if (workout->type == ROOT) {
    snprintf(output, size, "ROOT%s", selected_str);
  } else if (workout->type == REPETITIONS) {
    snprintf(output, size, "%dx", workout->numeric_data);
  } else if (workout->type == ACTIVITY) {
    if (workout->numeric_type_data == METERS) {
      snprintf(output, size, "%dm%s", workout->numeric_data, selected_str);
    } else {
      // TODO: add other distances
      snprintf(output, size, "%d units%s", workout->numeric_data, selected_str);
    }
  } else if (workout->type == REST) {
    if (workout->numeric_type_data == MINUTES) {
      snprintf(output, size, "%d'r%s", workout->numeric_data, selected_str);
    } else if (workout->numeric_type_data == SECONDS) {
      snprintf(output, size, "%d\"r%s", workout->numeric_data, selected_str);
    } else {
      snprintf(output, size, "%d units%s", workout->numeric_data, selected_str);
    }
  } else if (workout->type == PLACEHOLDER) {
    snprintf(output, size, "+%s", selected_str);
  } else {
    snprintf(output, size, "custom%s", selected_str);
  }
}

static void move_cursor_down() {
  if (cursor.type == BEFORE) {
    cursor.type = MODIFY;
  } else if (cursor.type == MODIFY) {
    // Find child or next element
    if (cursor.workout->child) {
      cursor.workout = cursor.workout->child;
      cursor.type = cursor.workout->type == PLACEHOLDER ? MODIFY : BEFORE;
    } else if (cursor.workout->next) {
      cursor.workout = cursor.workout->next;
      cursor.type = BEFORE;
    } else {
      if (cursor.workout->type == PLACEHOLDER) {
        if (cursor.workout->parent && cursor.workout->parent->next) {
          cursor.workout = cursor.workout->parent->next;
          cursor.type = BEFORE;
        }
      } else {
        cursor.type = AFTER;
      }
    }
  } else if (cursor.type == AFTER) {
    if (cursor.workout->next && !cursor.workout->child) {
      cursor.workout = cursor.workout->next;
      cursor.type = MODIFY;
    } else if (cursor.workout->parent && cursor.workout->parent->next) {
      cursor.workout = cursor.workout->parent->next;
      cursor.type = BEFORE;
    }
  }
}

static void move_cursor_up() {
  if (cursor.type == AFTER) {
    cursor.type = MODIFY;
  } else if (cursor.type == MODIFY) {
    // Find previous or parent element
    if (cursor.workout->previous && !cursor.workout->previous->child) {
      cursor.workout = cursor.workout->previous;
      cursor.type = AFTER;
    } else {
      if (cursor.workout->type == PLACEHOLDER && cursor.workout->parent) {
        cursor.workout = cursor.workout->parent;
        cursor.type = MODIFY;
      } else {
        cursor.type = BEFORE;
      }
    }
  } else if (cursor.type == BEFORE) {
    if (cursor.workout->previous && !cursor.workout->previous->child) {
      cursor.workout = cursor.workout->previous;
      cursor.type = MODIFY;
    } else if (cursor.workout->previous && cursor.workout->previous->child_last) {
      cursor.workout = cursor.workout->previous->child_last;
      cursor.type = cursor.workout->type == PLACEHOLDER ? MODIFY : AFTER;
    } else if (cursor.workout->parent) {
      cursor.workout = cursor.workout->parent;
      cursor.type = MODIFY;
    } 
  }
}

static void workout_new(Workout* workout, WorkoutType type, int numeric_data, int numeric_type_data, char* custom_msg) {
  workout->type = type;
  workout->numeric_data = numeric_data;
  workout->numeric_type_data = numeric_type_data;
  workout->custom_msg = custom_msg;
  workout->next = NULL;
  workout->previous = NULL;
  workout->child = NULL;
  workout->child_last = NULL;
  workout->parent = NULL;
}

static void workout_destroy(Workout* workout) {
  if (workout->child) {
    workout_destroy(workout->child);
  }
  if (workout->next) {
    workout_destroy(workout->next);
  }
  free(workout);
}

static void workout_set_child(Workout* parent, Workout* child) {
  parent->child = child;
  parent->child_last = child;
  child->parent = parent;
}

static void workout_add_sibling(Workout* workout, Workout* next) {
  workout->next = next;
  next->previous = workout;
  next->parent = workout->parent;
  workout->parent->child_last = next;
}

int recursive_draw(GContext* ctx, Workout* workout, int left, int top, int width) {
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
  
  // Create a placeholder if none exists under repetitions event.
  if (workout->type == REPETITIONS && !workout->child) {
    Workout* placeholder = (Workout*)malloc(sizeof(Workout));
    workout_new(placeholder, PLACEHOLDER, 0, 0, NULL);
    workout_set_child(workout, placeholder);
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
}

static void change_event(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  if (cursor.type == MODIFY) {
    cursor.workout->numeric_data = 12;
    cursor.workout->numeric_type_data = METERS;
  }
}

static void change_amount(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  if (cursor.type == MODIFY) {
    cursor.workout->numeric_data = 13;
  }
}

static void delete_workout(Workout *past) {
//   if (!past->child && past->parent && past->parent->child == past && past->parent->child_last == past) {
//     delete_workout(past->parent);
//   }
  
  // Create special cases:
  // Delete placeholder with parent of placeholder.
  if (past->child && past->child->type == PLACEHOLDER) {
    delete_workout(past->child);
  }
  // If deleting last child of repetition event, create placeholder in place.
  if (past->type != PLACEHOLDER && past->parent && !past->child && past->parent->type == REPETITIONS && past->parent->child == past && past->parent->child_last == past) {
    past->type = PLACEHOLDER;
    return;
  }
  
  // Set the cursor to the next workout
  if (past->child && past->child->type != PLACEHOLDER) {
    cursor.workout = past->child;
    cursor.type = MODIFY;
  } else if (past->next) {
    cursor.workout = past->next;
    cursor.type = MODIFY;
  } else if (past->previous && past->previous->child) {
    cursor.workout = past->previous->child_last;
    cursor.type = MODIFY;
  } else if (past->previous) {
    cursor.workout = past->previous;
    cursor.type = MODIFY;
  } else if (past->parent && past->parent->next) {
    cursor.workout = past->parent->next;
    cursor.type = MODIFY;
  } else if (past->parent) {
    cursor.workout = past->parent;
    cursor.type = MODIFY;
  } else {
    // TODO: Reset cursor.
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nowhere to place cursor");
  }
  
  // Make sure the workout's last child (if it has one) points to the next workout now.
  if (past->child_last) {
    past->child_last->next = past->next;
    // And the next workout points to the workout's last child.
    if (past->next) {
      past->next->previous = past->child_last;
    }
  }
  if (past->child) {
    // If the workout is a parent and is the first child of its parent, set the first child of the parent to the workout's child.
    if (past->parent && past->parent->child == past) {
      past->parent->child = past->child;
    }
    // If the workout is a parent, ensure its first child points to the previous of the workout.
    past->child->previous = past->previous;
    // And the workout's previous (if it has one) points to the workout's first child.
    if (past->previous) {
      past->previous->next = past->child;
    }
  } else {
    // If the workout is not a parent is the first child of its parent, set the parent's child to the workout's next.
    if (past->parent && past->parent->child == past) {
      past->parent->child = past->next;
    }
    // And if the workout is the last child of its parent, set the last child of the parent to the workout's previous.
    if (past->parent && past->parent->child_last == past) {
      past->parent->child_last = past->previous;
    }
    // If the workout is not a parent, set adjacent members' previous and nexts to accommodate for the workout's deletion.
    if (past->previous) {
      past->previous->next = past->next;
    } else {
      // If there is no previous
    }
    if (past->next) {
      past->next->previous = past->previous;
    }
  }
  // If the workout is the last child of its parent, set the last child of the parent to the last child of the workout.
  if (past->parent && past->parent->child_last == past) {
    past->parent->child_last = past->child_last;
  }
  // Cycle through each of the workout's children, assigning their parent to the workout's parent.
  Workout* child = past->child;
  while (child) {
    child->parent = past->parent;
    child = child->next;
  }

  // Delete the workout.
  free(past);
}

static void delete_event(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  if (cursor.type != MODIFY) {
    return;
  }
  
  Workout* past = cursor.workout;

  // Get the delete type.
  DeleteType delete_type = (DeleteType)action_menu_item_get_action_data(action);

  if (delete_type == DELETE_SHIFT) {
    delete_workout(past);
  }
}

static void create_event(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  int placeholder = cursor.workout->type == PLACEHOLDER;
  if (cursor.type == MODIFY && !placeholder) {
    return;
  }
  
  // Create a new event.
  Workout* new_workout = (Workout *)malloc(sizeof(Workout));
  workout_new(new_workout, ACTIVITY, 500, METERS, NULL);
  
  if (placeholder) {
    cursor.workout->type = new_workout->type;
    cursor.workout->numeric_data = new_workout->numeric_data;
    cursor.workout->numeric_type_data = new_workout->numeric_type_data;
    cursor.workout->custom_msg = new_workout->custom_msg;
    free(new_workout);
    return;
  }
  
  Workout* workout = cursor.workout;
  
  new_workout->parent = workout->parent;
  if (cursor.type == BEFORE) {
    if (workout->previous) {
      workout->previous->next = new_workout;
    } else {
      if (workout->parent && workout->parent->child == workout) {
        workout->parent->child = new_workout;
      }
    }
    new_workout->previous = workout->previous;
    new_workout->next = workout;
    workout->previous = new_workout;
  } else if (cursor.type == AFTER) {
    if (workout->next) {
      workout->next->previous = new_workout;
    } else {
      if (workout->parent && workout->parent->child_last == workout) {
        workout->parent->child_last = new_workout;
      }
    }
    new_workout->next = workout->next;
    new_workout->previous = workout;
    workout->next = new_workout;
  }
  
  delete_workout(cursor.workout);
  // Set the cursor to the new workout.
  cursor.workout = new_workout;
  cursor.type = MODIFY;
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
  action_menu_level_add_action(s_modify_event_level, "Change Event", change_event, NULL);
  
  s_modify_repetitions_level = action_menu_level_create(2);
  action_menu_level_add_action(s_modify_repetitions_level, "Change Repetitions", change_amount, NULL);
  action_menu_level_add_action(s_modify_repetitions_level, "Delete", delete_event, (void *)DELETE_SHIFT);
  
  s_create_event_level = action_menu_level_create(5);
  action_menu_level_add_action(s_create_event_level, "Create repetitions", create_event, NULL);
  action_menu_level_add_action(s_create_event_level, "Create distance event", create_event, NULL);
  action_menu_level_add_action(s_create_event_level, "Create rest event", create_event, NULL);
  action_menu_level_add_action(s_create_event_level, "Create gym event", create_event, NULL);
  action_menu_level_add_action(s_create_event_level, "Create custom event", create_event, NULL);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {  
  move_cursor_up();
  layer_mark_dirty(main_layer);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {  
  move_cursor_down();
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
  root = (Workout*)malloc(sizeof(Workout));
  workout_new(root, REPETITIONS, 1, 0, NULL);
  
  Workout* w_first_repeat = (Workout*)malloc(sizeof(Workout));
  Workout* w_second_repeat = (Workout*)malloc(sizeof(Workout));
  Workout* w_200m = (Workout*)malloc(sizeof(Workout));
  Workout* w_30r = (Workout*)malloc(sizeof(Workout));
  Workout* w_4r = (Workout*)malloc(sizeof(Workout));
  Workout* w_3re = (Workout*)malloc(sizeof(Workout));
  Workout* w_100m = (Workout*)malloc(sizeof(Workout));
  Workout* w_20r = (Workout*)malloc(sizeof(Workout));
  Workout* w_6r = (Workout*)malloc(sizeof(Workout));
  Workout* w_600m = (Workout*)malloc(sizeof(Workout));
  
  workout_new(w_first_repeat, REPETITIONS, 22, 0, NULL);
  workout_new(w_second_repeat, REPETITIONS, 3, 0, NULL);
  workout_new(w_200m, ACTIVITY, 200, METERS, NULL);
  workout_new(w_30r, REST, 30, SECONDS, NULL);
  workout_new(w_4r, REST, 4, MINUTES, NULL);
  workout_new(w_3re, REPETITIONS, 4, 0, NULL);
  workout_new(w_100m, ACTIVITY, 100, METERS, NULL);
  workout_new(w_20r, REST, 20, SECONDS, NULL);
  workout_new(w_6r, REST, 6, MINUTES, NULL);
  workout_new(w_600m, ACTIVITY, 600, METERS, NULL);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Assigning workout data hierarchy");
  // Assign workout data hierarchy.
  workout_set_child(root, w_first_repeat);
  workout_set_child(w_first_repeat, w_second_repeat);
  workout_set_child(w_second_repeat, w_200m);
  workout_add_sibling(w_200m, w_30r);
  workout_add_sibling(w_second_repeat, w_4r);
  workout_add_sibling(w_4r, w_3re);
  workout_add_sibling(w_3re, w_100m);
  workout_add_sibling(w_100m, w_20r);
  workout_add_sibling(w_first_repeat, w_6r);
  workout_add_sibling(w_6r, w_600m);
  
  cursor.workout = w_first_repeat;
  cursor.type = BEFORE;
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying resources");
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting update procedure");
  layer_set_update_proc(main_layer, update_main_layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding child layer to window root layer");
  layer_add_child(root_layer, main_layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Marking child layer as dirty");
  
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
