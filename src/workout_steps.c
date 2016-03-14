#include <pebble.h>
#include "workout_steps.h"

static Window* s_window;
static Workout* root;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_events_level;

static int count;
static char** workouts;

void reset_reps(Workout* root) {
  root->current_rep = 0;
  if (root->child) {
    reset_reps(root->child);
  }
  if (root->next) {
    reset_reps(root->next);
  }
}

int workout_iterate(Workout* root, char** workouts) {
  reset_reps(root);
  char workout_str[15];
  int depth = 0;

  int count = 0;

  Workout* current = root->child;
  while (1) {
    if (current->type != REPETITIONS && current->type != PLACEHOLDER) {
      // print_spaces(depth);
      if (workouts) {
        workout_to_str(current, workout_str, sizeof(workout_str), 0);
        strcpy(workouts[count], workout_str);
      }
      // printf("%s (%d)\n", workout_str, ++current->current_rep);
      count++;
    } else if (current->type == REPETITIONS && current->child && current->child->type != PLACEHOLDER) {
      // print_spaces(depth);
      // printf("REP %d/%d\n", current->current_rep + 1, current->numeric_data);
      if (workouts) {
        workout_to_str(current, workout_str, sizeof(workout_str), 0);
        strcpy(workouts[count], workout_str);
      }
      count++;
    }
    if (current->child) {
      current = current->child;
      depth++;
    } else if (current->next && !current->next->next && current->next->type == REST && current->parent && current->parent->type == REPETITIONS && current->parent->current_rep == current->parent->numeric_data - 1 && current->parent->next && current->parent->next->type == REST) {
      // print_spaces(depth);
      workout_to_str(current->next, workout_str, sizeof(workout_str), 0);
      // printf("(skip) %s\n", workout_str);
      current->parent->current_rep = 0;
      current = current->parent->next;
      depth--;
    } else if (current->next) {
      current = current->next;
    } else if (current->parent && current->parent->type == REPETITIONS && current->parent->current_rep < current->parent->numeric_data - 1) {
      current = current->parent;
      current->current_rep++;
      depth--;
    } else if (current->parent && current->parent->next) {
      current->parent->current_rep = 0;
      current = current->parent->next;
      depth--;
    } else {
      break;
    }
  }

  return count;
}
static void action_performed_callback(ActionMenu *action_menu,
                                      const ActionMenuItem *action, void *context) {
  
}
static void init_action_menus() {
  count = workout_iterate(root, NULL);
//   int count = 7;
  char** workouts = (char**)malloc(sizeof(char*) * count);
  int i;
  for (i = 0; i < count; i++) {
    workouts[i] = (char*)malloc(sizeof(char) * 15);
  }
  workout_iterate(root, workouts);
  
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
