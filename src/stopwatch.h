#pragma once
#include <pebble.h>
#include "workout.h"

typedef enum {NORMAL, PLAY, PAUSE, SPLIT} EventType;

typedef struct Event {
  uint64_t start_time;
  uint64_t total_time;
  int lap;
  EventType type;
  Workout* workout;
  struct Event* next;
  struct Event* previous;
} Event;

void show_stopwatch_window(Workout* workout);
void hide_stopwatch_window(void);
