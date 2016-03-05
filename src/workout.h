#pragma once
// Enums
typedef enum {ROOT, REPETITIONS, ACTIVITY, REST, CUSTOM, PLACEHOLDER} WorkoutType;
typedef enum {METERS, YARDS, KILOMETERS, MILES, FEET, UNKNOWN_DISTANCE} DistanceType;
typedef enum {MINUTES, SECONDS} TimeType;
typedef enum {DELETE_SHIFT, DELETE_ALL, DELETE_ALL_BRANCH} DeleteType;

extern const char * const distance_strings[];

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

// Workout methods
void workout_to_str(Workout* workout, char* output, int size, int selected);
void workout_new(Workout* workout, WorkoutType type, int numeric_data, int numeric_type_data, char* custom_msg);
void workout_destroy(Workout* workout);
void workout_set_child(Workout* parent, Workout* child);
void workout_add_sibling(Workout* workout, Workout* next);

// Other methods

void time_to_str(int seconds, char* output, int size, int rest);