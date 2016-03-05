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

void workout_new(Workout* workout, WorkoutType type, int numeric_data, int numeric_type_data, char* custom_msg) {
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
  parent->child = child;
  parent->child_last = child;
  child->parent = parent;
}

void workout_add_sibling(Workout* workout, Workout* next) {
  workout->next = next;
  next->previous = workout;
  next->parent = workout->parent;
  workout->parent->child_last = next;
}
