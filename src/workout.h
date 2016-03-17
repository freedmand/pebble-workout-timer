#pragma once

// Enums
typedef enum {ROOT, REPETITIONS, ACTIVITY, REST, CUSTOM, PLACEHOLDER} WorkoutType;
typedef enum {REPETITION_PLACEHOLDER, END_PLACEHOLDER} PlaceholderType;
typedef enum {TIMED_ACTIVITY, REP_TIMED_ACTIVITY, METERS, YARDS, KILOMETERS, MILES, FEET, UNKNOWN_DISTANCE} DistanceType;
typedef enum {TIMED_REST, REP_TIMED_REST, UNTIMED_REST} RestType;
typedef enum {MINUTES, SECONDS} TimeType;
typedef enum {DELETE_SHIFT, DELETE_ALL, DELETE_ALL_BRANCH} DeleteType;

typedef enum {BEFORE, MODIFY, AFTER} CursorType;

extern const char * const distance_strings[];

typedef struct Workout {
  WorkoutType type;
  int numeric_data;
  int numeric_type_data;
  char* custom_msg;
  int current_rep;
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


// Workout methods
void workout_to_str(Workout* workout, char* output, int size, int selected);
void workout_new(Workout* workout, WorkoutType type, int numeric_data, int numeric_type_data, char* custom_msg);
void workout_destroy(Workout* workout);
void workout_set_child(Workout* parent, Workout* child);
void workout_add_sibling(Workout* workout, Workout* next);
void time_to_str(int seconds, char* output, int size, int rest);

// Workout creation methods
void move_cursor_down(Cursor* cursor);
void move_cursor_up(Cursor* cursor);
void delete_workout(Cursor* cursor, Workout *past, DeleteType delete_type);
void delete_cursor(Cursor* cursor, DeleteType delete_type);
void reset_cursor(Cursor* cursor, Workout* root);
Workout* create_init(Cursor* cursor);
Workout* create_workout(Cursor* cursor, WorkoutType type, int amount, int numeric_type);

// Workout iteration methods
Workout* get_next(Workout* current);
void reset_reps(Workout* root);
int workout_iterate(Workout* root, void (*callback)(Workout*));

#ifdef DEBUG
void print_spaces(int spaces);
void debug_print(Cursor* cursor, Workout* root);
void run_tests();
#endif
