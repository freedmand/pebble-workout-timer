#pragma once
#include <pebble.h>
#include "workout.h"

#define STORE_SIZE 50

typedef enum {NORMAL, PLAY, PAUSE, SPLIT} EventType;
typedef enum {COUNTUP, COUNTDOWN, COUNTTO} CountType;

typedef struct {
  char** strings;
  int num_strings;
} StringStore;

typedef struct Event {
  int8_t store_index;
  int8_t lap;
  int8_t current_rep;
  int8_t total_reps;
  int8_t new_rep;
  uint16_t count;
  EventType type;
  CountType count_type;
  int total_time;
  struct Event* next;
  struct Event* previous;
  uint64_t start_time;
} Event;

void init_string_store(StringStore* store) {
  store->strings = (char**)malloc(sizeof(char*) * STORE_SIZE);
  store->num_strings = 0;
}

int8_t add_workout_to_string_store(StringStore* store, Workout* workout);
void workout_to_event(Workout* workout, Event* event, StringStore* store);
