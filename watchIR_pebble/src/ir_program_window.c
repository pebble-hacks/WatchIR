#include "ir_program_window.h"

#include "defs.h"
#include "icon_selection_window.h"
#include "ir_button.h"
#include "ir_smartstrap.h"

#include <inttypes.h>

typedef struct {
  Window *window;
  ButtonId pebble_button_to_program;
  TextLayer *status_text_layer;
  char status_text[50];
#if DEBUG == 1
  AppTimer *debug_timer;
#else
  IrCode recorded_ir_code;
#endif
} IrProgramWindowData;

// Helpers
/////////////////////

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

// Icon selection window
/////////////////////////

static void prv_icon_selection_window_callback(uint32_t selected_icon_resource_id, void *context) {
  IrProgramWindowData *data = context;
  if (selected_icon_resource_id != RESOURCE_ID_INVALID) {
    IrButton *new_ir_button = malloc(sizeof(IrButton));
    if (new_ir_button) {
      *new_ir_button = (IrButton) {
        .pebble_button = data->pebble_button_to_program,
        .icon_resource_id = selected_icon_resource_id,
      };
#if DEBUG == 0
      memcpy(&new_ir_button->ir_code, &data->recorded_ir_code, sizeof(IrCode));
#endif
    }
    if (ir_button_program(new_ir_button)) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully programmed %s button, using icon resource ID: %"PRIu32" ",
              prv_get_button_string(data->pebble_button_to_program), selected_icon_resource_id);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to program %s button",
              prv_get_button_string(data->pebble_button_to_program));
    }
    free(new_ir_button);
  }

  // Pop the icon selection window and the IR program window
  const bool animated = false;
  window_stack_pop(animated);
  window_stack_pop(animated);
}

// Debug
/////////////////////

#if DEBUG == 1
static void prv_debug_timer_callback(void *context) {
  IrProgramWindowData *window_data = context;
  window_data->debug_timer = NULL;

  // Fake that we received a programming response from the smartstrap
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got fake programming");
  icon_selection_window_push(prv_icon_selection_window_callback, window_data);
}
#endif

// IR Smartstrap record result callback
////////////////////////////////////////

#if DEBUG == 0
static void prv_record_result_callback(const IrCode *ir_code, void *context) {
  if (!ir_code) {
    return;
  }
  IrProgramWindowData *window_data = context;
  window_data->recorded_ir_code = *ir_code;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received record result");
  icon_selection_window_push(prv_icon_selection_window_callback, window_data);
}
#endif

// Window handlers
/////////////////////

static void prv_window_appear(Window *window) {
  IrProgramWindowData *data = window_get_user_data(window);
  const char *button_string = prv_get_button_string(data->pebble_button_to_program);
  snprintf(data->status_text, sizeof(data->status_text), "Programming %s button...",
           button_string);
  text_layer_set_text(data->status_text_layer, data->status_text);

#if DEBUG == 1
  const uint32_t timeout_ms = 3000;
  data->debug_timer = app_timer_register(timeout_ms, prv_debug_timer_callback, data);
#endif
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

#if DEBUG == 0
  ir_smartstrap_record(prv_record_result_callback, window_data);
#endif
}

static void prv_window_unload(Window *window) {
  IrProgramWindowData *window_data = window_get_user_data(window);
  if (window_data) {
#if DEBUG == 1
    if (window_data->debug_timer) {
      app_timer_cancel(window_data->debug_timer);
    }
#endif
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
  memset(window_data, 0, sizeof(IrProgramWindowData));

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
