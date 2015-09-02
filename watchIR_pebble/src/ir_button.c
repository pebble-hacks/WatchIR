#include "ir_button.h"

bool ir_button_get_program(ButtonId pebble_button, IrButton *result) {
  if (!result || (pebble_button == BUTTON_ID_BACK) || !persist_exists(pebble_button)) {
    return false;
  }
  const size_t ir_button_size = sizeof(IrButton);
  return (persist_read_data(pebble_button, result, sizeof(IrButton)) == (int)ir_button_size);
}

bool ir_button_program(IrButton *ir_button) {
  if (!ir_button) {
    return false;
  }
  const size_t ir_button_size = sizeof(IrButton);
  const int bytes_written = persist_write_data(ir_button->pebble_button,
                                               ir_button, ir_button_size);
  return (bytes_written == (int)ir_button_size);
}

bool ir_button_is_programmed(ButtonId pebble_button) {
  return persist_exists(pebble_button);
}

bool ir_button_any_buttons_are_programmed(void) {
  for (ButtonId pebble_button = 0; pebble_button < NUM_BUTTONS; pebble_button++) {
    if ((pebble_button != BUTTON_ID_BACK) &&
        ir_button_is_programmed(pebble_button)) {
      return true;
    }
  }
  return false;
}
