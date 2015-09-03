#include "ir_button.h"

// We need to store IrButton's in persisted storage in a smaller format
typedef struct __attribute__((packed)) {
  uint16_t num_durations;
  uint16_t durations[MAX_SUPPORTED_IR_DURATIONS];
} PersistedIrCode;

typedef struct __attribute__((packed)) {
  ButtonId pebble_button:3;
  uint32_t icon_resource_id;
  PersistedIrCode persisted_ir_code;
} PersistedIrButton;

_Static_assert(sizeof(PersistedIrButton) <= PERSIST_DATA_MAX_LENGTH,
               "sizeof(PersistedIrButton) > PERSIST_DATA_MAX_LENGTH");

static bool prv_convert_ir_button_to_persisted_ir_button(IrButton *ir_button, PersistedIrButton *result) {
  if (!ir_button || !result) {
    return false;
  }
  result->pebble_button = ir_button->pebble_button;
  result->icon_resource_id = ir_button->icon_resource_id;
  result->persisted_ir_code.num_durations = ir_button->ir_code.num_durations;
  for (size_t i = 0; i < ir_button->ir_code.num_durations; i++) {
    result->persisted_ir_code.durations[i] = (uint16_t)ir_button->ir_code.durations[i];
  }
  return true;
}

static bool prv_convert_persisted_ir_button_to_ir_button(PersistedIrButton *persisted_ir_button, IrButton *result) {
  if (!persisted_ir_button || !result) {
    return false;
  }
  result->pebble_button = persisted_ir_button->pebble_button;
  result->icon_resource_id = persisted_ir_button->icon_resource_id;
  result->ir_code.num_durations = persisted_ir_button->persisted_ir_code.num_durations;
  for (size_t i = 0; i < persisted_ir_button->persisted_ir_code.num_durations; i++) {
    result->ir_code.durations[i] = (size_t)persisted_ir_button->persisted_ir_code.durations[i];
  }
  return true;
}

bool ir_button_get_program(ButtonId pebble_button, IrButton *result) {
  if (!result || (pebble_button == BUTTON_ID_BACK) || !persist_exists(pebble_button)) {
    return false;
  }

  PersistedIrButton *persisted_ir_button = malloc(sizeof(PersistedIrButton));
  if (!persisted_ir_button) {
    return false;
  }

  const size_t persisted_ir_button_size = sizeof(PersistedIrButton);
  const size_t read_bytes = persist_read_data(pebble_button,
                                              persisted_ir_button,
                                              persisted_ir_button_size);
  bool success = (read_bytes == persisted_ir_button_size);
  if (success) {
    success = prv_convert_persisted_ir_button_to_ir_button(persisted_ir_button, result);
  }
  free(persisted_ir_button);
  return success;
}

bool ir_button_program(IrButton *ir_button) {
  if (!ir_button) {
    return false;
  }

  PersistedIrButton *persisted_ir_button = malloc(sizeof(PersistedIrButton));
  if (!prv_convert_ir_button_to_persisted_ir_button(ir_button, persisted_ir_button)) {
    free(persisted_ir_button);
    return false;
  }
  const size_t persisted_ir_button_size = sizeof(PersistedIrButton);
  const int bytes_written = persist_write_data(persisted_ir_button->pebble_button,
                                               persisted_ir_button, persisted_ir_button_size);
  free(persisted_ir_button);
  return (bytes_written == (int)persisted_ir_button_size);
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
