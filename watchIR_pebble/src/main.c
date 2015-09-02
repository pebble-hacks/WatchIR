#include <pebble.h>

#include "ir_button.h"
#include "ir_program_window.h"
#include "ir_smartstrap.h"

typedef struct {
  Window *window;
  TextLayer *instruction_text_layer;
  ActionBarLayer *action_bar_layer;
  GBitmap *action_bar_layer_icons[NUM_BUTTONS]; // Allocates an extra for the back button even though we won't use it
} WatchIrAppData;

// Click handlers
/////////////////////

static void prv_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  IrButton *programmed_button = malloc(sizeof(IrButton));
  if (ir_button_get_program(BUTTON_ID_SELECT, programmed_button)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored button: %d", programmed_button->pebble_button);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored icon: %d", (int)programmed_button->icon_resource_id);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored num durations: %d", programmed_button->ir_code.num_durations);
    for (size_t i = 0; i < programmed_button->ir_code.num_durations; i++) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored duration[%d]: %d", i, programmed_button->ir_code.durations[i]);
    }
    ir_smartstrap_transmit(&programmed_button->ir_code);
  }
  free(programmed_button);
}

static void prv_select_long_click_down_handler(ClickRecognizerRef recognizer, void *context) {
  ir_program_window_push(BUTTON_ID_SELECT);
}

static void prv_up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  IrButton *programmed_button = malloc(sizeof(IrButton));
  if (ir_button_get_program(BUTTON_ID_UP, programmed_button)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored button: %d", programmed_button->pebble_button);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored icon: %d", (int)programmed_button->icon_resource_id);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored num durations: %d", programmed_button->ir_code.num_durations);
    for (size_t i = 0; i < programmed_button->ir_code.num_durations; i++) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored duration[%d]: %d", i, programmed_button->ir_code.durations[i]);
    }
    ir_smartstrap_transmit(&programmed_button->ir_code);
  }
  free(programmed_button);
}

static void prv_up_long_click_down_handler(ClickRecognizerRef recognizer, void *context) {
  ir_program_window_push(BUTTON_ID_UP);
}

static void prv_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  IrButton *programmed_button = malloc(sizeof(IrButton));
  if (ir_button_get_program(BUTTON_ID_DOWN, programmed_button)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored button: %d", programmed_button->pebble_button);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored icon: %d", (int)programmed_button->icon_resource_id);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored num durations: %d", programmed_button->ir_code.num_durations);
    for (size_t i = 0; i < programmed_button->ir_code.num_durations; i++) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored duration[%d]: %d", i, programmed_button->ir_code.durations[i]);
    }
    ir_smartstrap_transmit(&programmed_button->ir_code);
  }
  free(programmed_button);
}

static void prv_down_long_click_down_handler(ClickRecognizerRef recognizer, void *context) {
  ir_program_window_push(BUTTON_ID_DOWN);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_single_click_handler);

  const uint16_t delay_ms = 500;
  window_long_click_subscribe(BUTTON_ID_SELECT, delay_ms, prv_select_long_click_down_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, delay_ms, prv_up_long_click_down_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, delay_ms, prv_down_long_click_down_handler, NULL);
}

// Window handlers
/////////////////////

static void prv_window_appear(Window *window) {
  WatchIrAppData *data = window_get_user_data(window);
  const GRect window_layer_bounds = layer_get_bounds(window_get_root_layer(window));
  TextLayer *instruction_text_layer = data->instruction_text_layer;
  GRect adjusted_text_layer_frame = window_layer_bounds;
  adjusted_text_layer_frame.size.w -= ACTION_BAR_WIDTH;
  layer_set_frame(text_layer_get_layer(instruction_text_layer), adjusted_text_layer_frame);
  text_layer_set_font(instruction_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(instruction_text_layer, "Hold down a button to program it");

  for (ButtonId pebble_button = 0; pebble_button < NUM_BUTTONS; pebble_button++) {
    IrButton ir_button;
    if (!ir_button_get_program(pebble_button, &ir_button)) {
      continue;
    }
    GBitmap **action_bar_layer_icon = &data->action_bar_layer_icons[pebble_button];
    if (*action_bar_layer_icon) {
      gbitmap_destroy(*action_bar_layer_icon);
      *action_bar_layer_icon = NULL;
    }
    *action_bar_layer_icon = gbitmap_create_with_resource(ir_button.icon_resource_id);
    action_bar_layer_set_icon(data->action_bar_layer, pebble_button, *action_bar_layer_icon);
  }
}

static void prv_window_load(Window *window) {
  WatchIrAppData *data = window_get_user_data(window);

  Layer *window_layer = window_get_root_layer(window);
  const GRect window_layer_bounds = layer_get_bounds(window_layer);

  data->action_bar_layer = action_bar_layer_create();
  ActionBarLayer *action_bar_layer = data->action_bar_layer;
  action_bar_layer_add_to_window(data->action_bar_layer, window);
  action_bar_layer_set_click_config_provider(action_bar_layer, prv_click_config_provider);

  data->instruction_text_layer = text_layer_create(window_layer_bounds);
  TextLayer *instruction_text_layer = data->instruction_text_layer;
  text_layer_set_text_alignment(instruction_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(instruction_text_layer));
}

static void prv_window_unload(Window *window) {
  WatchIrAppData *data = window_get_user_data(window);
  if (data) {
    text_layer_destroy(data->instruction_text_layer);

    for (size_t i = 0; i < ARRAY_LENGTH(data->action_bar_layer_icons); i++) {
      GBitmap *action_bar_layer_icon = data->action_bar_layer_icons[i];
      if (action_bar_layer_icon) {
        gbitmap_destroy(action_bar_layer_icon);
        data->action_bar_layer_icons[i] = NULL;
      }
    }

    action_bar_layer_destroy(data->action_bar_layer);
    window_destroy(data->window);
  }
  free(data);
}

// App boiler plate
/////////////////////

static void prv_app_init(void) {
  WatchIrAppData *data = malloc(sizeof(WatchIrAppData));
  memset(data, 0, sizeof(WatchIrAppData));

  ir_smartstrap_init();

  data->window = window_create();
  window_set_user_data(data->window, data);
  window_set_click_config_provider_with_context(data->window, prv_click_config_provider, data);
  window_set_window_handlers(data->window, (WindowHandlers) {
    .appear = prv_window_appear,
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(data->window, true /* animated */);
}

static void prv_app_deinit(void) {
  ir_smartstrap_deinit();
}

int main(void) {
  prv_app_init();
  app_event_loop();
  prv_app_deinit();
}
