#include <pebble.h>

#include "ir_button.h"
#include "ir_program_window.h"

typedef struct {
  Window *window;
  TextLayer *instruction_text_layer;
} WatchIrAppData;

// Click handlers
/////////////////////

static void prv_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void prv_select_long_click_down_handler(ClickRecognizerRef recognizer, void *context) {
  ir_program_window_push(BUTTON_ID_SELECT);
}

static void prv_up_single_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void prv_up_long_click_down_handler(ClickRecognizerRef recognizer, void *context) {
  ir_program_window_push(BUTTON_ID_UP);
}

static void prv_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {

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
  if (!ir_button_any_buttons_are_programmed()) {
    text_layer_set_text(data->instruction_text_layer, "Hold down a button to program");
  }
}

static void prv_window_load(Window *window) {
  WatchIrAppData *data = window_get_user_data(window);

  Layer *window_layer = window_get_root_layer(window);
  const GRect window_layer_bounds = layer_get_bounds(window_layer);

  data->instruction_text_layer = text_layer_create(window_layer_bounds);
  TextLayer *instruction_text_layer = data->instruction_text_layer;
  text_layer_set_font(instruction_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(instruction_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(instruction_text_layer));
}

static void prv_window_unload(Window *window) {
  WatchIrAppData *data = window_get_user_data(window);
  if (data) {
    text_layer_destroy(data->instruction_text_layer);
    window_destroy(data->window);
  }
  free(data);
}

// App boiler plate
/////////////////////

static void prv_app_init(void) {
  WatchIrAppData *data = malloc(sizeof(WatchIrAppData));
  memset(data, 0, sizeof(WatchIrAppData));

  data->window = window_create();
  Window *window = data->window;
  window_set_user_data(window, data);
  window_set_click_config_provider(window, prv_click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .appear = prv_window_appear,
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(window, true /* animated */);
}

static void prv_app_deinit(void) {

}

int main(void) {
  prv_app_init();
  app_event_loop();
  prv_app_deinit();
}
