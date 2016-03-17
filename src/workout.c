#include <pebble.h>
#include "workout.h"

const char * const distance_strings[] = {"meters", "yards", "kilometers", "miles", "feet", "units"};

void time_to_str(int seconds, char* output, int size, int rest) {
  char rest_str[3] = "";
  if (rest) {
    snprintf(rest_str, sizeof(rest_str), "r");
  }
  
  int minutes = seconds / 60;
  int seconds_part = seconds % 60;
  if (seconds_part == 0) {
    snprintf(output, size, "%d'%s", minutes, rest_str); 
  } else if (minutes == 0) {
    snprintf(output, size, "%d\"%s", seconds, rest_str); 
  } else {
    snprintf(output, size, "%d'%d\"%s", minutes, seconds_part, rest_str); 
  }
}

void workout_to_str(Workout* workout, char* output, int size, int selected) {
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
    char time[10];
    time_to_str(workout->numeric_data, time, sizeof(time), 1);
    snprintf(output, size, "%s%s", time, selected_str);
  } else if (workout->type == PLACEHOLDER) {
    snprintf(output, size, "+%s", selected_str);
  } else {
    snprintf(output, size, "custom%s", selected_str);
  }
}

Workout* get_next(Workout* current) {
  int starting = 1;
  while (1) {
    // Non-repetition or placeholder
    if (current->type != REPETITIONS && current->type != PLACEHOLDER) {
      if (!starting) {
        return current;
      } else {
        starting = 0;
      }
    }
    
    if (current->child) {
      current = current->child;
    } else if (current->next && !current->next->next && current->next->type == REST && current->parent && current->parent->type == REPETITIONS && current->parent->current_rep == current->parent->numeric_data - 1 && current->parent->next && current->parent->next->type == REST) {
      current->parent->current_rep = 0;
      current = current->parent->next;
    } else if (current->next) {
      current = current->next;
    } else if (current->parent && current->parent->type == REPETITIONS && current->parent->current_rep < current->parent->numeric_data - 1) {
      current = current->parent;
      current->current_rep++;
    } else if (current->parent && current->parent->next) {
      current->parent->current_rep = 0;
      current = current->parent->next;
    } else {
      break;
    }
  }

  return NULL;
}

void reset_reps(Workout* root) {
  root->current_rep = 0;
  if (root->child) {
    reset_reps(root->child);
  }
  if (root->next) {
    reset_reps(root->next);
  }
}

// callback(workout, depth, count, current rep, total reps, next_rep)
uint8_t workout_iterate(Workout* current, void (*callback)(Workout*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)) {
  uint8_t depth = 0;
  uint8_t count = 0;

  if (current->type == ROOT) {
    current = current->child;
  }
  if (!current) {
    return 0;
  }
  // Whether or not it is the start of the next rep
  int next_ping = 0;
  while (1) {
    if (current->type != REPETITIONS && current->type != PLACEHOLDER) {
      if (callback) {
        if (current->parent->type == REPETITIONS) {
          callback(current, depth, count, current->parent->current_rep + 1, current->parent->numeric_data, next_ping);
          next_ping = 0;
        } else {
          callback(current, depth, count, 0, 0, 0);
        }
      }
      count++;
    }
    
    if (current->child) {
      current = current->child;
      depth++;
    } else if (current->next && !current->next->next && current->next->type == REST && current->parent && current->parent->type == REPETITIONS && current->parent->current_rep == current->parent->numeric_data - 1 && current->parent->next && current->parent->next->type == REST) {
      current->parent->current_rep = 0;
      current = current->parent->next;
      depth--;
    } else if (current->next) {
      current = current->next;
    } else if (current->parent && current->parent->type == REPETITIONS && current->parent->current_rep < current->parent->numeric_data - 1) {
      current = current->parent;
      current->current_rep++;
      next_ping = 1;
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

void workout_new(Workout* workout, WorkoutType type, int numeric_data, int numeric_type_data, char* custom_msg) {
  workout->type = type;
  workout->numeric_data = numeric_data;
  workout->numeric_type_data = numeric_type_data;
  workout->custom_msg = custom_msg;
  workout->next = NULL;
  workout->previous = NULL;
  workout->parent = NULL;
  workout->child = NULL;
  workout->child_last = NULL; 
  if (workout->type == REPETITIONS) {
    Workout* placeholder = (Workout*)malloc(sizeof(Workout));
    workout_new(placeholder, PLACEHOLDER, 0, REPETITION_PLACEHOLDER, NULL);
    workout_set_child(workout, placeholder);
  } else if (workout->type == ROOT) {
    Workout* placeholder = (Workout*)malloc(sizeof(Workout));
    workout_new(placeholder, PLACEHOLDER, 0, END_PLACEHOLDER, NULL);
    workout_set_child(workout, placeholder);
  }
}

void workout_destroy(Workout* workout) {
  if (workout->child) {
    workout_destroy(workout->child);
  }
  if (workout->next) {
    workout_destroy(workout->next);
  }
  free(workout);
}

void workout_set_child(Workout* parent, Workout* child) {
  int set = 0;
  if (parent->child) {
    if (parent->child == parent->child_last && parent->child->type == PLACEHOLDER) {
      if (parent->child->numeric_type_data == REPETITION_PLACEHOLDER) {
        free(parent->child);
      } else if (parent->child->numeric_type_data == END_PLACEHOLDER) {
        child->next = parent->child;
        parent->child->previous = child;
        parent->child_last = parent->child;
        set = 1;
      }
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "(workout set child) Parent should not have child unless placeholder.");
      return;
    }
  }
  parent->child = child;
  if (!set) {
    parent->child_last = child;
  }
  child->parent = parent;

  if (child->type == REPETITIONS && !child->next) {
    Workout* end_placeholder = (Workout*)malloc(sizeof(Workout));
    workout_new(end_placeholder, PLACEHOLDER, 0, END_PLACEHOLDER, NULL);
    workout_add_sibling(child, end_placeholder);
  }
}

void workout_add_sibling(Workout* workout, Workout* next) {
  if (workout->next && workout->next->type == PLACEHOLDER && workout->next->numeric_type_data == END_PLACEHOLDER) {
    if (next->type != REPETITIONS) {
      free(workout->next);
    } else {
      printf("Nexting\n");
      next->next = workout->next;
      workout->next->previous = next;
    }
  }
  workout->next = next;
  next->previous = workout;
  next->parent = workout->parent;
  workout->parent->child_last = next;
}

void move_cursor_down(Cursor* cursor) {
  if (cursor->type == BEFORE) {
    cursor->type = MODIFY;
  } else if (cursor->type == MODIFY) {
    // Find child or next element
    if (cursor->workout->child) {
      cursor->workout = cursor->workout->child;
      cursor->type = cursor->workout->type == PLACEHOLDER && cursor->workout->numeric_type_data == REPETITION_PLACEHOLDER ? MODIFY : BEFORE;
    } else if (cursor->workout->next) {
      cursor->workout = cursor->workout->next;
      cursor->type = cursor->workout->type == PLACEHOLDER && cursor->workout->numeric_type_data == END_PLACEHOLDER ? MODIFY : BEFORE;
    } else {
      if (cursor->workout->type == PLACEHOLDER) {
        if (!cursor->workout->parent || cursor->workout->parent->type != ROOT) {
          Workout* parent = cursor->workout->parent;
          while (parent && !parent->next) {
            parent = parent->parent;
          }
          if (!parent) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "(move cursor down) Placeholder element needs parent.");
            return;
          }
          cursor->workout = parent->next;
          cursor->type = parent->next->type == PLACEHOLDER ? MODIFY : BEFORE;
        }
      } else {
        cursor->type = AFTER;
      }
    }
  } else if (cursor->type == AFTER) {
    if (cursor->workout->next && !cursor->workout->child) {
      cursor->workout = cursor->workout->next;
      cursor->type = MODIFY;
    } else if (cursor->workout->parent && cursor->workout->parent->next) {
      cursor->type = cursor->workout->parent->next->type == PLACEHOLDER ? MODIFY : BEFORE;
      cursor->workout = cursor->workout->parent->next;
    }
  }
}

void move_cursor_up(Cursor* cursor) {
  if (cursor->type == AFTER) {
    cursor->type = MODIFY;
  } else if (cursor->type == MODIFY) {
    // Find previous or parent element
    if (cursor->workout->previous && !cursor->workout->previous->child) {
      if (cursor->workout->type == PLACEHOLDER && cursor->workout->numeric_type_data == END_PLACEHOLDER) {
        cursor->workout = cursor->workout->previous;
        cursor->type = MODIFY;
      } else {
        cursor->type = BEFORE;
      }
    } else {
      if (cursor->workout->type == PLACEHOLDER && cursor->workout->numeric_type_data == REPETITION_PLACEHOLDER && cursor->workout->parent) {
        cursor->workout = cursor->workout->parent;
        cursor->type = MODIFY;
      } else if (cursor->workout->type == PLACEHOLDER && cursor->workout->numeric_type_data == END_PLACEHOLDER && cursor->workout->previous->child_last) {
        Workout* child_last = cursor->workout->previous->child_last;
        while (child_last && child_last->child_last) {
          child_last = child_last->child_last;
        }
        cursor->workout = child_last;
        cursor->type = cursor->workout->type == PLACEHOLDER ? MODIFY : AFTER;
      } else {
        cursor->type = BEFORE;
      }
    }
  } else if (cursor->type == BEFORE) {
    if (cursor->workout->previous && !cursor->workout->previous->child) {
      cursor->workout = cursor->workout->previous;
      cursor->type = MODIFY;
    } else if (cursor->workout->previous && cursor->workout->previous->child_last) {
      Workout* child_last = cursor->workout->previous->child_last;
      while (child_last && child_last->child_last) {
        child_last = child_last->child_last;
      }
      cursor->workout = child_last;
      cursor->type = cursor->workout->type == PLACEHOLDER ? MODIFY : AFTER;
    } else if (cursor->workout->parent && cursor->workout->parent->type != ROOT) {
      cursor->workout = cursor->workout->parent;
      cursor->type = MODIFY;
    } 
  }
}

void delete_workout(Cursor* cursor, Workout *past, DeleteType delete_type) {
  // If deleting everything in existence, replace with a single END_PLACEHOLDER.
  if (!past->next || (past->next && past->next->type == PLACEHOLDER && past->next->numeric_type_data == END_PLACEHOLDER)) {
    if (!past->previous && past->parent && past->parent->type == ROOT) {
      if ((past->type == REPETITIONS && delete_type != DELETE_SHIFT) || (past->type != REPETITIONS)) {
        if (past->child) {
          workout_destroy(past->child);
          workout_destroy(past->next);
          past->child = NULL;
          past->next = NULL;
          past->previous = NULL;
          past->type = PLACEHOLDER;
          past->numeric_type_data = END_PLACEHOLDER;
          if (cursor) {
            cursor->workout = past;
            cursor->type = MODIFY;
          }
          return;
        }
      }
    }
  }

  if (delete_type == DELETE_ALL) {
    if (past->child) {
      delete_workout(cursor, past->child, DELETE_ALL_BRANCH);
    }
  } else if (delete_type == DELETE_ALL_BRANCH) {
    if (past->child) {
      delete_workout(cursor, past->child, DELETE_ALL_BRANCH);
    }
    if (past->next) {
      delete_workout(cursor, past->next, DELETE_ALL_BRANCH);
    }
  }
  
  // Create special cases:
  // Delete placeholder with parent of placeholder.
  if (past->child && past->child->type == PLACEHOLDER) {
    delete_workout(NULL, past->child, DELETE_SHIFT);
  }
  // If deleting last child of repetition event, create repetition placeholder in place.
  if (past->type != PLACEHOLDER && past->parent && !past->child && past->parent->type == REPETITIONS && past->parent->child == past && past->parent->child_last == past) {
    past->type = PLACEHOLDER;
    past->numeric_type_data = REPETITION_PLACEHOLDER;
    return;
  }

  // If deleting last event after repetition event, create end placeholder in place.
  if (!past->next && past->previous && past->previous->type == REPETITIONS) {
    past->type = PLACEHOLDER;
    past->numeric_type_data = END_PLACEHOLDER;
    return;
  }
  
  if (cursor) {
    // Set the cursor to the next workout
    if (past->child && past->child->type != PLACEHOLDER) {
      cursor->workout = past->child;
      cursor->type = MODIFY;
    } else if (past->next && past->next->type != PLACEHOLDER) {
      cursor->workout = past->next;
      cursor->type = MODIFY;
    } else if (past->previous && !past->previous->child) {
      cursor->workout = past->previous;
      cursor->type = MODIFY;
    } else if (past->next && past->next->type == PLACEHOLDER) {
      cursor->workout = past->next;
      cursor->type = MODIFY;
    } else if (past->previous && past->previous->child_last) {
      Workout* child_last = past->previous->child_last;
      while (child_last && child_last->child_last) {
        child_last = child_last->child_last;
      }
      cursor->workout = child_last;
      cursor->type = MODIFY;
    } else if (past->parent && past->parent->next) {
      cursor->workout = past->parent->next;
      cursor->type = MODIFY;
    } else if (past->parent) {
      cursor->workout = past->parent;
      cursor->type = MODIFY;
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "(Delete Workout) No place to put cursor.");
      return;
    }
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

  // Delete END_PLACEHOLDER following self if past is in existence and not a repetition.
  if (past->next && past->next->type == PLACEHOLDER && past->next->numeric_type_data == END_PLACEHOLDER) {
    if (!past->previous || past->previous->type != REPETITIONS) {
      if (!past->parent || past->parent->type != REPETITIONS || past->previous) {
        delete_workout(NULL, past->next, DELETE_SHIFT);
      }
    }
  }

  // Delete the workout.
  free(past);
}

void delete_cursor(Cursor* cursor, DeleteType delete_type) {
  if (cursor->type != MODIFY) {
    return;
  }
  
  Workout* past = cursor->workout;

  // Get the delete type.
  delete_workout(cursor, past, delete_type);
}

void reset_cursor(Cursor* cursor, Workout* root) {
  cursor->workout = root->child;
  cursor->type = BEFORE;
}

Workout* create_init(Cursor* cursor) {
  Workout* root = (Workout*)malloc(sizeof(Workout));
  // Workout* placeholder = (Workout*)malloc(sizeof(Workout));
  workout_new(root, ROOT, 0, 0, NULL);
  // workout_new(placeholder, PLACEHOLDER, 0, END_PLACEHOLDER, NULL);

  // root->child = placeholder;
  // root->child_last = placeholder;
  // placeholder->parent = root;
  cursor->workout = root->child;
  cursor->type = MODIFY;
  return root;
}

Workout* create_workout(Cursor* cursor, WorkoutType type, int amount, int numeric_type) {  
  int placeholder = cursor->workout->type == PLACEHOLDER;
  if (cursor->type == MODIFY) {
    if (placeholder) {
      cursor->workout->type = type;
      cursor->workout->numeric_data = amount;
      cursor->workout->numeric_type_data = numeric_type;
      if (type == REPETITIONS) {
        Workout* placeholder = (Workout*)malloc(sizeof(Workout));
        Workout* end_placeholder = (Workout*)malloc(sizeof(Workout));
        workout_new(placeholder, PLACEHOLDER, 0, REPETITION_PLACEHOLDER, NULL);
        workout_new(end_placeholder, PLACEHOLDER, 0, END_PLACEHOLDER, NULL);
        workout_add_sibling(cursor->workout, end_placeholder);
        workout_set_child(cursor->workout, placeholder);
      }
      return cursor->workout;
    } else {
      return NULL;
    }
  }
  
  // Create a new event.
  Workout* new_workout = (Workout *)malloc(sizeof(Workout));
  workout_new(new_workout, type, amount, numeric_type, NULL);
  
  Workout* workout = cursor->workout;
  
  new_workout->parent = workout->parent;
  if (cursor->type == BEFORE) {
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
  } else if (cursor->type == AFTER) {
    int end_p_added = 0;
    if (workout->next) {
      workout->next->previous = new_workout;
    } else {
      if (workout->parent && workout->parent->child_last == workout) {
        workout->parent->child_last = new_workout;
      }
      if (type == REPETITIONS) {
        end_p_added = 1;
        Workout* end_placeholder = (Workout *)malloc(sizeof(Workout));
        workout_new(end_placeholder, PLACEHOLDER, 0, END_PLACEHOLDER, NULL);
        workout_add_sibling(new_workout, end_placeholder);
      }
    }
    if (!end_p_added) {
      new_workout->next = workout->next;
    }
    new_workout->previous = workout;
    workout->next = new_workout;
  }
  
  cursor->workout = new_workout;
  cursor->type = MODIFY;
  return new_workout;
}

#ifdef DEBUG
int check_for_workout(Workout* root, Workout* check) {
  if (root == check) {
    return 1;
  }

  int has = 0;
  if (root->child) {
    has |= check_for_workout(root->child, check);
  }
  if (root->next) {
    has |= check_for_workout(root->next, check);
  }
  return has;
}

void print_spaces(int spaces) {
  int i;
  for (i = 0; i < spaces; i++) {
    printf(" ");
  }
}

void debug_print_helper(Cursor* cursor, Workout* workout, int indent) {
  print_spaces(indent);
  if (cursor->workout == workout && cursor->type == BEFORE) {
    printf("---(BEFORE)---\n");
    print_spaces(indent);
  }
  int selected = cursor->workout == workout && cursor->type == MODIFY;
  char workout_str[15];
  workout_to_str(workout, workout_str, sizeof(workout_str), selected);
  printf("%s\n", workout_str);
  if (cursor->workout == workout && cursor->type == AFTER) {
    print_spaces(indent);
    printf("---(AFTER)---\n");
  }
  if (workout->child) {
    debug_print_helper(cursor, workout->child, indent + 2);
  }
  if (workout->next) {
    debug_print_helper(cursor, workout->next, indent);
  }
}

void debug_print(Cursor* cursor, Workout* root) {
  debug_print_helper(cursor, root->child, 0);
  printf("\n");
}

void run_tests() {
  // Create workout data.
  Workout *root;
  Cursor cursor;

  root = create_init(&cursor);

  // Create workout data hierarchy.
  Workout* w_first_repeat = create_workout(&cursor, REPETITIONS, 22, 0);
  assert(w_first_repeat);
  assert(cursor.workout == w_first_repeat);
  assert(cursor.type == MODIFY);

  move_cursor_down(&cursor);
  Workout* w_second_repeat = create_workout(&cursor, REPETITIONS, 3, 0);

  assert(w_second_repeat);
  move_cursor_down(&cursor);
  Workout* w_200m = create_workout(&cursor, ACTIVITY, 200, METERS);
  assert(w_200m);
  move_cursor_down(&cursor);
  Workout* w_30r = create_workout(&cursor, REST, 30, SECONDS);
  assert(w_30r);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  Workout* w_4r = create_workout(&cursor, REST, 4, MINUTES);
  assert(w_4r);
  move_cursor_down(&cursor);
  Workout* w_3re = create_workout(&cursor, REPETITIONS, 4, 0);
  assert(w_3re);

  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  Workout* w_100m = create_workout(&cursor, ACTIVITY, 100, METERS);
  assert(w_100m);
  move_cursor_down(&cursor);
  Workout* w_20r = create_workout(&cursor, REST, 20, SECONDS);
  assert(w_20r);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  Workout* w_6r = create_workout(&cursor, REST, 6, MINUTES);
  assert(w_6r);
  move_cursor_down(&cursor);
  Workout* w_600m = create_workout(&cursor, ACTIVITY, 600, METERS);
  assert(w_600m);

  // Reset cursor.
  reset_cursor(&cursor, root);

  // BEGIN TEST: RELATIONS
  assert(w_first_repeat->child->type != PLACEHOLDER);
  assert(w_first_repeat->child == w_second_repeat);
  assert(w_first_repeat->child_last == w_20r);
  assert(w_first_repeat->previous == NULL);
  assert(w_first_repeat->next == w_6r);
  assert(w_first_repeat->parent == root);

  assert(w_second_repeat->child == w_200m);
  assert(w_second_repeat->child_last == w_30r);
  assert(w_second_repeat->previous == NULL);
  assert(w_second_repeat->next == w_4r);
  assert(w_second_repeat->parent == w_first_repeat);

  assert(w_200m->child == NULL);
  assert(w_200m->child_last == NULL);
  assert(w_200m->previous == NULL);
  assert(w_200m->next == w_30r);
  assert(w_200m->parent == w_second_repeat);

  assert(w_30r->child == NULL);
  assert(w_30r->child_last == NULL);
  assert(w_30r->previous == w_200m);
  assert(w_30r->next == NULL);
  assert(w_30r->parent == w_second_repeat);

  assert(w_4r->child == NULL);
  assert(w_4r->child_last == NULL);
  assert(w_4r->previous == w_second_repeat);
  assert(w_4r->next == w_3re);
  assert(w_4r->parent == w_first_repeat);

  assert(w_3re->child && w_3re->child->type == PLACEHOLDER);
  assert(w_3re->child_last && w_3re->child_last->type == PLACEHOLDER);
  assert(w_3re->child->numeric_type_data == REPETITION_PLACEHOLDER);
  assert(w_3re->child_last->numeric_type_data == REPETITION_PLACEHOLDER);
  assert(w_3re->child == w_3re->child_last);
  assert(w_3re->previous == w_4r);
  assert(w_3re->next == w_100m);
  assert(w_3re->parent == w_first_repeat);

  assert(w_3re->child->child == NULL);
  assert(w_3re->child->child_last == NULL);
  assert(w_3re->child->previous == NULL);
  assert(w_3re->child->next == NULL);
  assert(w_3re->child->parent == w_3re);

  assert(w_100m->child == NULL);
  assert(w_100m->child_last == NULL);
  assert(w_100m->previous == w_3re);
  assert(w_100m->next == w_20r);
  assert(w_100m->parent == w_first_repeat);

  assert(w_20r->child == NULL);
  assert(w_20r->child_last == NULL);
  assert(w_20r->previous == w_100m);
  assert(w_20r->next == NULL);
  assert(w_20r->parent == w_first_repeat);

  assert(w_20r->child == NULL);
  assert(w_20r->child_last == NULL);
  assert(w_20r->previous == w_100m);
  assert(w_20r->next == NULL);
  assert(w_20r->parent == w_first_repeat);

  assert(w_6r->child == NULL);
  assert(w_6r->child_last == NULL);
  assert(w_6r->previous == w_first_repeat);
  assert(w_6r->next == w_600m);
  assert(w_6r->parent == root);

  assert(w_600m->child == NULL);
  assert(w_600m->child_last == NULL);
  assert(w_600m->previous == w_6r);
  assert(w_600m->next == NULL);
  assert(w_600m->parent == root);

  // BEGIN TEST: CURSOR
  assert(cursor.workout == w_first_repeat && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_second_repeat && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_second_repeat && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_200m && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_200m && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_30r && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_30r && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_30r && cursor.type == AFTER);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_4r && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_4r && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_3re && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_3re && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == REPETITION_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_100m && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_100m && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_20r && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_20r && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_20r && cursor.type == AFTER);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_6r && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_6r && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == AFTER);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == AFTER);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == AFTER);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == AFTER);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_600m && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_600m && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_6r && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_6r && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_20r && cursor.type == AFTER);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_20r && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_20r && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_100m && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_100m && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == REPETITION_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_3re && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_3re && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_4r && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_4r && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_30r && cursor.type == AFTER);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_30r && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_30r && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_200m && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_200m && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_second_repeat && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_second_repeat && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == BEFORE);

  // TEST OUT DELETE/CREATE CASES
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_100m && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout == w_20r && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == END_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_6r && cursor.type == BEFORE);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == END_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_600m && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout == w_6r && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == END_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == END_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == REPETITION_PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_3re && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_4r && cursor.type == AFTER);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_down(&cursor);
  assert(cursor.workout == w_4r && cursor.type == BEFORE);
  move_cursor_down(&cursor);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  create_workout(&cursor, REPETITIONS, 5, 0);
  assert(cursor.workout->type == REPETITIONS && cursor.type == MODIFY);
  assert(cursor.workout->child && cursor.workout->child->type == PLACEHOLDER);
  assert(cursor.workout->next && cursor.workout->next->type == PLACEHOLDER);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  assert(cursor.workout->type == PLACEHOLDER);
  assert(cursor.workout->next == NULL);
  assert(cursor.workout->previous && cursor.workout->previous->type == REPETITIONS);
  create_workout(&cursor, REPETITIONS, 5, 0);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  assert(cursor.workout->type == REPETITIONS && cursor.workout->numeric_data == 5);
  assert(cursor.workout->previous == NULL);
  assert(cursor.workout->next && cursor.workout->next->type == PLACEHOLDER);
  assert(cursor.workout->next->next == NULL);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  assert(cursor.workout->type == PLACEHOLDER);
  assert(cursor.workout->previous == NULL && cursor.workout->next == NULL && cursor.workout->child == NULL);
  assert(cursor.workout->parent == w_first_repeat);
  create_workout(&cursor, REPETITIONS, 7, 0);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == REPETITIONS && cursor.workout->numeric_data == 7 && cursor.type == BEFORE);
  create_workout(&cursor, REPETITIONS, 8, 0);
  assert(cursor.workout->type == REPETITIONS && cursor.workout->numeric_data == 8 && cursor.type == MODIFY);
  assert(cursor.workout->previous == NULL);
  assert(cursor.workout->next && cursor.workout->next->type == REPETITIONS);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  assert(cursor.workout == w_first_repeat && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  assert(cursor.workout->type == PLACEHOLDER && cursor.workout->numeric_type_data == END_PLACEHOLDER && cursor.type == MODIFY);
  assert(cursor.workout->previous == NULL && cursor.workout->next == NULL && cursor.workout->child == NULL);
  assert(cursor.workout->parent && cursor.workout->parent->type == ROOT);
  create_workout(&cursor, REPETITIONS, 9, 0);
  move_cursor_down(&cursor);
  create_workout(&cursor, REPETITIONS, 10, 0);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);
  move_cursor_up(&cursor);
  assert(cursor.workout->type == REPETITIONS && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);
  assert(cursor.workout->next == NULL && cursor.workout->previous == NULL && cursor.workout->child == NULL);
  assert(cursor.workout->parent && cursor.workout->parent->type == REPETITIONS);
  create_workout(&cursor, REPETITIONS, 11, 0);
  move_cursor_down(&cursor);
  create_workout(&cursor, REPETITIONS, 12, 0);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  create_workout(&cursor, REPETITIONS, 13, 0);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);
  assert(cursor.workout->next == NULL && cursor.workout->previous && cursor.workout->previous->numeric_data == 12);
  assert(cursor.workout->child == NULL && cursor.workout->parent && cursor.workout->parent->numeric_data == 11);
  create_workout(&cursor, REPETITIONS, 14, 0);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  create_workout(&cursor, REPETITIONS, 15, 0);
  move_cursor_up(&cursor);
  move_cursor_up(&cursor);
  assert(cursor.workout->numeric_data == 9 && cursor.type == MODIFY);
  delete_workout(&cursor, cursor.workout, DELETE_SHIFT);
  assert(cursor.workout->numeric_data == 15 && cursor.type == MODIFY);
  assert(cursor.workout->next && cursor.workout->next->numeric_data == 11);
  assert(cursor.workout->next->next && cursor.workout->next->next->type == PLACEHOLDER);
  assert(cursor.workout->next->next->next == NULL);


  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  delete_workout(&cursor, cursor.workout, DELETE_ALL);
  create_workout(&cursor, REPETITIONS, 14, 0);
  move_cursor_down(&cursor);
  create_workout(&cursor, ACTIVITY, 100, METERS);
  move_cursor_down(&cursor);
  move_cursor_down(&cursor);
  assert(cursor.workout->type == PLACEHOLDER && cursor.type == MODIFY);

  printf("All tests pass\n");
  workout_destroy(root);
}
#endif
