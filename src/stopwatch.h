#pragma once
#include <pebble.h>
#include "workout.h"

typedef enum {START, PLAY, PAUSE, SPLIT, NEXT, MENU, BACK, UP, DOWN} AdvancedEventType;

typedef struct Event {
  uint64_t start_time;
  uint64_t total_time;
  Workout* workout;
  struct Event* next;
  struct Event* previous;
} Event;

typedef struct AdvancedEvent {
  uint64_t start_time;
  AdvancedEventType type;
  struct AdvancedEvent* next;
} AdvancedEvent;

void show_stopwatch_window(Workout* workout);
void hide_stopwatch_window(void);
