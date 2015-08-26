#include <pebble.h>

typedef struct {
  Window *window;
} WatchIRAppData;

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  // TODO
//  Layer *window_layer = window_get_root_layer(window);
//  GRect bounds = layer_get_bounds(window_layer);
}

static void prv_window_unload(Window *window) {
  WatchIRAppData *data = window_get_user_data(window);
  if (data) {
    window_destroy(data->window);
  }
  free(data);
}

static void prv_app_init(void) {
  WatchIRAppData *data = malloc(sizeof(WatchIRAppData));
  memset(data, 0, sizeof(WatchIRAppData));

  data->window = window_create();
  Window *window = data->window;
  window_set_user_data(window, data);
  window_set_click_config_provider(window, prv_click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
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
