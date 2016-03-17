#include <pebble.h>
#include "stopwatch.h"
#include "workout.h"
#include "util.h"

static Window* s_window;
static Workout* root_workout;
static Workout* current_workout;
static Event* root_event;
static Event* current_event;
static Workout* next_workout;

static int num_events;
static int event_index;
static int total_event_index;

static int idx_shift;

// Graphics
static GBitmap* split_bmp;
static GBitmap* play_bmp;
static GBitmap* pause_bmp;
static GBitmap* stop_bmp;
static GBitmap* up_bmp;
static GBitmap* down_bmp;
static GBitmap* stopwatch_bmp;

// UI
static GFont s_res_gothic_14;
static GFont s_res_gothic_14_bold;
static GFont s_res_gothic_18;
static GFont s_res_gothic_18_bold;
static GFont s_res_gothic_28_bold;
static Layer* root_layer;
static ActionBarLayer* stopwatch_action_bar;

// Time management
static AppTimer *timer;
static int started;
static int paused;
static int stopped;
static uint64_t start_time;
static uint64_t pause_time;
static uint64_t pause_deficit;
static uint64_t total_pause_deficit;

#define TIMER_RUNNING_INTERVAL 67
int TIMER_UPDATE_INTERVAL = TIMER_RUNNING_INTERVAL;

static void timer_callback(void *data) {
  layer_mark_dirty(root_layer);
  if (started && !paused && !stopped) {
    timer = app_timer_register(TIMER_UPDATE_INTERVAL, timer_callback, NULL);
  }
}

void insert_next_event(Event* current, Event* next) {
  if (current->next) {
    next->next = current->next;
    current->next->previous = next;
  }
  current->next = next;
  next->previous = current;
}

void insert_previous_event(Event* current, Event* previous) {
  if (current->previous) {
    previous->previous = current->previous;
    current->previous->next = previous;
  }
  current->previous = previous;
  previous->next = current;
  
  // Change root event if need be.
  if (current == root_event) {
    root_event = previous;
  }
}

void select_handler() {
  if (!started || paused) {
    if (paused) {
      if (!stopped) {
        // Resume from pause again
        vibes_short_pulse();
        idx_shift = 0;
        action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_UP, split_bmp);
        action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_DOWN, stopwatch_bmp);
        paused = 0;
        int deficit = get_time() - pause_time;
        pause_deficit += deficit;
        total_pause_deficit += deficit;

        Event* pause_event = (Event*)malloc(sizeof(Event));
        pause_event->start_time = pause_time;
        pause_event->total_time = deficit;
        pause_event->lap = -1;
        pause_event->type = PAUSE;
        pause_event->workout = NULL;
        insert_previous_event(current_event, pause_event);
        total_event_index++;
      }
    } else {
      // Start from beginning
      vibes_short_pulse();
      started = 1;
      start_time = get_time();
      current_event->start_time = start_time;
    }
    if (!stopped) {
      action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_SELECT, pause_bmp);
    }
  } else {
    // Pause from play
    vibes_short_pulse();
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_UP, up_bmp);
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_SELECT, play_bmp);
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_DOWN, down_bmp);
    paused = 1;
    pause_time = get_time();
    
    Event* play_event = (Event*)malloc(sizeof(Event));
    play_event->start_time = current_event->start_time;
    play_event->total_time = pause_time - play_event->start_time;
    play_event->lap = -1;
    play_event->type = PLAY;
    play_event->workout = NULL;
    insert_previous_event(current_event, play_event);
    total_event_index++;
  }
  timer_callback(NULL);
}

void up_handler() {
  if (paused) {
    idx_shift = idx_shift == 0 ? 0 : idx_shift - 1;
  } else {
    Event* split_event = (Event*)malloc(sizeof(Event));
    split_event->start_time = current_event->start_time;
    split_event->total_time = get_time() - split_event->start_time - pause_deficit;
    split_event->lap = -1;
    split_event->type = SPLIT;
    split_event->workout = NULL;
    insert_previous_event(current_event, split_event);
    total_event_index++;
    vibes_short_pulse();
  }
  timer_callback(NULL);
}

void next_event() {
  uint64_t current_time = get_time();
  current_event->total_time = current_time - current_event->start_time - pause_deficit;
  pause_deficit = 0;
  if (next_workout) {
    vibes_short_pulse();
    
    Event* event = (Event*)malloc(sizeof(Event));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Getting next workout");
    event->workout = get_next(current_event->workout, 1);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got next workout 1");
    next_workout = get_next(event->workout, 0);
    event->type = NORMAL;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got next workout 2");
    insert_next_event(current_event, event);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Inserted next event");
    event->lap = current_event->lap + 1;
    current_event = event;
    
    current_event->start_time = current_time;
    event_index++;
    total_event_index++;
  } else {
    // Stop if at the end of the event chain.
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_UP, up_bmp);
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_SELECT, stop_bmp);
    action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_DOWN, down_bmp);
    paused = 1;
    stopped = 1;
    pause_time = get_time();
    vibes_short_pulse();
  }
}

void down_handler() {
  if (paused) {
    idx_shift = idx_shift < total_event_index - 4 ? idx_shift + 1 : idx_shift;
  } else {
    next_event();
  }
  timer_callback(NULL);
}

Event* get_previous(Event* current, int reps) {
  current = current->previous;
//   while (current && current->wo current->workout->type != ACTIVITY && current->workout->type != REST) {
//     current = current->previous;
//   }
  if (reps <= 0) {
    return current;
  } else {
    return get_previous(current, reps - 1);
  }
}

static void update_ui(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, 0);
  
  uint64_t current_time = get_time();
  
  char placeholder[25];
  char workout_str[15];
  
  graphics_context_set_text_color(ctx, GColorBlack);
  // All line
  graphics_draw_text(ctx, "All", s_res_gothic_14_bold, GRect(0, 1, 22, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  cents_to_str(started ? (paused ? pause_time : current_time) - start_time - total_pause_deficit : 0, placeholder, sizeof(placeholder));
  graphics_draw_text(ctx, placeholder, s_res_gothic_14, GRect(25, 1, 44, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  snprintf(placeholder, sizeof(placeholder), "%d/%d", event_index + 1, num_events);
  graphics_draw_text(ctx, placeholder, s_res_gothic_14_bold, GRect(71, 1, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  // Rep line
  graphics_draw_text(ctx, "Rep", s_res_gothic_14_bold, GRect(0, 16, 22, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  graphics_draw_text(ctx, "00:00.00", s_res_gothic_14, GRect(25, 16, 44, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  if (0) {
    snprintf(placeholder, sizeof(placeholder), "%d/%d", current_event->workout->current_rep + 1, current_event->workout->parent->numeric_data);
    graphics_draw_text(ctx, placeholder, s_res_gothic_14_bold, GRect(71, 16, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  } else {
    graphics_draw_text(ctx, "1/1", s_res_gothic_14_bold, GRect(71, 16, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
  
  // Horizontal rule
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(4, 34), GPoint(103, 34));
  
  // Next line
  if (next_workout) {
    workout_to_str(next_workout, workout_str, sizeof(workout_str), 0);
    snprintf(placeholder, sizeof(placeholder), "Next: (%d) %s", event_index + 2, workout_str);
    graphics_draw_text(ctx, placeholder, s_res_gothic_18, GRect(2, 34, 111, 19), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  } else {
    graphics_draw_text(ctx, "Last workout", s_res_gothic_18, GRect(2, 34, 111, 19), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
  
  // Draw stopwatch display
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 56, 103, 48), 0, 0);
  // Draw current workout
  if (current_event) {
    workout_to_str(current_event->workout, workout_str, sizeof(workout_str), 0);
    graphics_draw_text(ctx, workout_str, s_res_gothic_18_bold, GRect(2, 57, 100, 19), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
  
  uint64_t workout_time;
  int next = 0;
  if (current_event->workout->type == REST) {
    if ((uint64_t)current_event->workout->numeric_data * 1000 < (paused ? pause_time : current_time) - current_event->start_time - pause_deficit) {
      next = 1;
      workout_time = 0;
    } else {
      workout_time = current_event->workout->numeric_data * 1000 - ((paused ? pause_time : current_time) - current_event->start_time - pause_deficit);
    }
  } else {
    workout_time = (paused ? pause_time : current_time) - current_event->start_time - pause_deficit;
  }
  cents_to_str(started ? workout_time : 0, placeholder, sizeof(placeholder));
  graphics_draw_text(ctx, placeholder, s_res_gothic_28_bold, GRect(2, 71, 79, 29), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, "LAP", s_res_gothic_14, GRect(81, 74, 21, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  snprintf(placeholder, sizeof(placeholder), "%03d", event_index + 1);
  graphics_draw_text(ctx, placeholder, s_res_gothic_14, GRect(81, 85, 85, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  int drawing_v = 106;
  int i = 0;
  Event* previous = get_previous(current_event, idx_shift);
  int drew = 0;
  while (i < 4 && previous) {
    drew = 1;
    // Draw history line
    if (previous->type == NORMAL) {
      snprintf(placeholder, sizeof(placeholder), "%03d", previous->lap);
      graphics_draw_text(ctx, placeholder, s_res_gothic_14, GRect(2, drawing_v, 20, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
    cents_to_str(previous->total_time, placeholder, sizeof(placeholder));
    graphics_draw_text(ctx, placeholder, s_res_gothic_14_bold, GRect(23, drawing_v, 44, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    if (previous->workout) {
      workout_to_str(previous->workout, workout_str, sizeof(workout_str), 0);
      graphics_draw_text(ctx, workout_str, s_res_gothic_14, GRect(69, drawing_v, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);      
    } else if (previous->type == PLAY) {
      graphics_draw_text(ctx, "Play", s_res_gothic_14, GRect(69, drawing_v, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);      
    } else if (previous->type == PAUSE) {
      graphics_draw_text(ctx, "Pause", s_res_gothic_14, GRect(69, drawing_v, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);      
    } else if (previous->type == SPLIT) {
      graphics_draw_text(ctx, "Split", s_res_gothic_14, GRect(69, drawing_v, 42, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);      
    }
    // Increment counts
    drawing_v += 15;
    i++;
    previous = previous->previous;
  }
  
  if (!drew) {
    graphics_draw_text(ctx, "Press play to begin recording workout", s_res_gothic_14_bold, GRect(2, 112, 100, 39), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  
  if (next) {
    next_event();
  }
}

void stopwatch_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);

  // Get font resources.
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_gothic_14_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  
  // Grabbing icons
  split_bmp = gbitmap_create_with_resource(RESOURCE_ID_split_icon);
  play_bmp = gbitmap_create_with_resource(RESOURCE_ID_play_icon);
  pause_bmp = gbitmap_create_with_resource(RESOURCE_ID_pause_icon);
  stop_bmp = gbitmap_create_with_resource(RESOURCE_ID_stop_icon);
  stopwatch_bmp = gbitmap_create_with_resource(RESOURCE_ID_stopwatch_icon);
  up_bmp = gbitmap_create_with_resource(RESOURCE_ID_up_icon);
  down_bmp = gbitmap_create_with_resource(RESOURCE_ID_down_icon);
  
  
  // Get root layer.
  root_layer = window_get_root_layer(s_window);
  layer_set_update_proc(root_layer, update_ui);
  
  stopwatch_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(stopwatch_action_bar, s_window);
  // Set the click config provider.
  action_bar_layer_set_click_config_provider(stopwatch_action_bar,
                                             stopwatch_config_provider);

  // Set the icons.
  action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_UP, split_bmp);
  action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_SELECT, play_bmp);
  action_bar_layer_set_icon(stopwatch_action_bar, BUTTON_ID_DOWN, stopwatch_bmp);
  
  layer_mark_dirty(root_layer);
  timer = app_timer_register(TIMER_UPDATE_INTERVAL, timer_callback, NULL);
}

static void destroy_ui(void) {
  window_destroy(s_window);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void populate_event_chain(Workout* workout) {
  if (!root_event) {
    root_event = (Event*)malloc(sizeof(Event));
    // Set start time?
    root_event->type = NORMAL;
    root_event->start_time = 0;
    root_event->lap = 1;
    root_event->workout = workout;
    root_event->previous = NULL;
    root_event->next = NULL;
    current_event = root_event;
  } else {
    Event* event = (Event*)malloc(sizeof(Event));
    event->workout = workout;
    event->type = NORMAL;
    insert_next_event(current_event, event);
    event->lap = current_event->lap + 1;
    current_event = event;
  }
}

void show_stopwatch_window(Workout* workout) {
  started = 0;
  paused = 0;
  stopped = 0;
  idx_shift = 0;
  pause_deficit = 0;
  total_pause_deficit = 0;
  root_workout = workout;
  
  reset_reps(root_workout);
  num_events = workout_iterate(workout, NULL);
  reset_reps(root_workout);
  
  root_event = (Event*)malloc(sizeof(Event));
  root_event->type = NORMAL;
  root_event->start_time = 0;
  root_event->lap = 1;
  root_event->workout = get_next(workout->child, 1);
  root_event->previous = NULL;
  root_event->next = NULL;
  
  current_event = root_event;
  next_workout = get_next(current_event->workout, 0);
  event_index = 0;
  total_event_index = 0;
  
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_stopwatch_window(void) {
  window_stack_remove(s_window, true);
}