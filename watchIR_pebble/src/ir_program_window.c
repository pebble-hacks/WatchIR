#include "ir_program_window.h"

typedef struct {
  Window *window;
  ButtonId pebble_button_to_program;
  TextLayer *status_text_layer;
  char status_text[50];
} IrProgramWindowData;

static const char *prv_get_button_string(ButtonId pebble_button) {
  switch (pebble_button) {
    case BUTTON_ID_SELECT:
      return "select";
    case BUTTON_ID_UP:
      return "up";
    case BUTTON_ID_DOWN:
      return "down";
    default:
      return "";
  }
}

// Window handlers
/////////////////////

static void prv_window_appear(Window *window) {
  IrProgramWindowData *data = window_get_user_data(window);
  snprintf(data->status_text, sizeof(data->status_text), "Programming %s button...",
           prv_get_button_string(data->pebble_button_to_program));
  text_layer_set_text(data->status_text_layer, data->status_text);
}

static void prv_window_load(Window *window) {
  IrProgramWindowData *window_data = window_get_user_data(window);

  Layer *window_layer = window_get_root_layer(window);
  const GRect window_layer_bounds = layer_get_bounds(window_layer);

  window_data->status_text_layer = text_layer_create(window_layer_bounds);
  TextLayer *status_text_layer = window_data->status_text_layer;
  text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(status_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(status_text_layer));
}

static void prv_window_unload(Window *window) {
  IrProgramWindowData *window_data = window_get_user_data(window);
  if (window_data) {
    text_layer_destroy(window_data->status_text_layer);
    window_destroy(window_data->window);
  }
  free(window_data);
}

// Public API
/////////////////////

void ir_program_window_push(ButtonId pebble_button_to_program) {
  IrProgramWindowData *window_data = malloc(sizeof(IrProgramWindowData));
  if (!window_data) {
    return;
  }

  window_data->window = window_create();
  Window *window = window_data->window;
  window_set_user_data(window, window_data);
  window_set_window_handlers(window, (WindowHandlers) {
    .appear = prv_window_appear,
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_data->pebble_button_to_program = pebble_button_to_program;

  window_stack_push(window, true /* animated */);
}
