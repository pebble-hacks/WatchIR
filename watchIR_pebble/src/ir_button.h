#pragma once

#include <pebble.h>

typedef struct {
  ButtonId pebble_button;
  uint32_t ir_code;
} IrButton;

bool ir_button_get_program(ButtonId pebble_button, IrButton *result);
bool ir_button_program(IrButton *ir_button);
bool ir_button_is_programmed(ButtonId pebble_button);
bool ir_button_any_buttons_are_programmed(void);
