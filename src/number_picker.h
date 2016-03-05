#pragma once

typedef enum {PICK_NUMBER, PICK_TIME, PICK_REPETITIONS} NumberPickerType;

void show_number_picker(int digits, int initial, char* message, char* units, NumberPickerType type, void (*callback)(int));
void hide_number_picker(void);
