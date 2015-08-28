#include "ir_button.h"

bool ir_button_get_program(ButtonId pebble_button, IrButton *result) {
  if (!result || !persist_exists(pebble_button)) {
    return false;
  }
  const int ir_button_size = sizeof(IrButton);
  return (persist_read_data(pebble_button, result, sizeof(IrButton)) == ir_button_size);
}

bool ir_button_program(IrButton *ir_button) {
  if (!ir_button) {
    return false;
  }
  const int ir_button_size = sizeof(IrButton);
  return (persist_write_data(ir_button->pebble_button,
                             ir_button, ir_button_size) == ir_button_size);
}

bool ir_button_is_programmed(ButtonId pebble_button) {
  return persist_exists(pebble_button);
}

bool ir_button_any_buttons_are_programmed(void) {
  for (ButtonId pebble_button = 0; pebble_button < NUM_BUTTONS; pebble_button++) {
    if (ir_button_is_programmed(pebble_button)) {
      return true;
    }
  }
  return false;
}
