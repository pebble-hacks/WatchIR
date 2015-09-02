#pragma once

#include <pebble.h>
#include <sys/cdefs.h>

// This struct must match `WatchIrSmartstrapData` on Arduino
#define MAX_SUPPORTED_IR_DURATIONS 50
typedef struct {
  size_t num_durations;
  size_t durations[MAX_SUPPORTED_IR_DURATIONS];
} IrCode;

typedef struct {
  ButtonId pebble_button;
  uint32_t icon_resource_id;
  IrCode ir_code;
} IrButton;

_Static_assert(sizeof(IrButton) <= PERSIST_DATA_MAX_LENGTH, "sizeof(IrCode) > PERSIST_DATA_MAX_LENGTH");

bool ir_button_get_program(ButtonId pebble_button, IrButton *result);
bool ir_button_program(IrButton *ir_button);
bool ir_button_is_programmed(ButtonId pebble_button);
bool ir_button_any_buttons_are_programmed(void);
