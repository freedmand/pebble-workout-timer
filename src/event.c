#include <pebble.h>
#include "event.h"
#include "workout.h"

int8_t add_workout_to_string_store(StringStore* store, Workout* workout) {
  char workout_str[15];
  workout_to_str(workout, workout_str, sizeof(workout_str), 0);
  int8_t i;
  for (i = 0; i < store->num_strings; i++) {
    if (strcmp(store->strings[i], workout_str) == 0) {
      return i;
    }
  }
  
  store->strings[store->num_strings] = (char*)malloc(sizeof(char) * (strlen(workout_str) + 1));
  strcpy(store->strings[store->num_strings], workout_str);
  store->num_strings++;
  return store->num_strings - 1;
}

void workout_to_event(Workout* workout, Event* event, StringStore* store) {
  event->store_index = add_workout_to_string_store(store, workout);
  if (workout->type == REST) {
    if (workout->numeric_type_data == TIMED_REST) {
      event->count_type = COUNTDOWN;
      event->count = workout->numeric_data;
    } else if (workout->numeric_type_data == UNTIMED_REST) {
      event->count_type = COUNTUP;
      event->count = 0;
    } else if (workout->numeric_type_data == REP_TIMED_REST) {
      event->count_type = COUNTTO;
      event->count = workout->numeric_data;
    }
  } else if (workout->type == ACTIVITY) {
    if (workout->numeric_type_data == TIMED_ACTIVITY) {
      event->count_type = COUNTDOWN;
      event->count = workout->numeric_data;
    } else if (workout->numeric_type_data == REP_TIMED_ACTIVITY) {
      event->count_type = COUNTTO;
      event->count = workout->numeric_data;
    } else {
      event->count_type = COUNTUP;
      event->count = 0;
    }
  }
}