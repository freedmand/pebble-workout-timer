#include <pebble.h>
#include "workout_steps.h"
#include "workout.h"

static Window* s_window;
static Workout* root;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_events_level;

static int count;
static int pos;
static char** workouts;

static void action_performed_callback(ActionMenu *action_menu,
                                      const ActionMenuItem *action, void *context) {
  
}

static void add_to_workouts(Workout* workout) {
  char workout_str[15];
  workout_to_str(workout, workout_str, sizeof(workout_str), 0);
  workouts[pos++] = workout_str;
}

static void init_action_menus() {
  count = workout_iterate(root, NULL);
//   int count = 7;
  char** workouts = (char**)malloc(sizeof(char*) * count);
  int i;
  for (i = 0; i < count; i++) {
    workouts[i] = (char*)malloc(sizeof(char) * 15);
  }
  
  pos = 0;
  reset_reps(root);
  workout_iterate(root, add_to_workouts);
  
  s_events_level = action_menu_level_create(count);
  for (i = 0; i < count; i++) {
    action_menu_level_add_action(s_events_level, workouts[i], action_performed_callback, NULL);
  }
}

void close_menu(ActionMenu *menu, const ActionMenuItem *performed_action, void *context) {
  int i;
  for (i = 0; i < count; i++) {
    free(workouts[i]);
  }
  free(workouts);
}

static void initialise_ui(void) {
  s_window = window_create();
  init_action_menus();
}

static void destroy_ui(void) {
  window_destroy(s_window);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_workout_steps_window(Workout* workout) {
  root = workout;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  ActionMenuConfig config = (ActionMenuConfig) {
    .root_level = s_events_level,
    .colors = {
      .background = GColorWhite,
      .foreground = GColorBlack,
    },
    .did_close = close_menu,
    .align = ActionMenuAlignCenter,
  };
  s_action_menu = action_menu_open(&config);
}

void hide_workout_steps_window(void) {
  window_stack_remove(s_window, true);
}
