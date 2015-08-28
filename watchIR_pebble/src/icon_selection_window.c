#include "icon_selection_window.h"

typedef struct {
  Window *window;
  IconSelectionWindowCallback selection_callback;
  void *selection_callback_context;
  MenuLayer *menu_layer;
} IconSelectionWindowData;


// Helpers
/////////////////////

static uint32_t prv_get_resource_id_from_menu_layer_row(MenuIndex *cell_index) {
  if (!cell_index) {
    return RESOURCE_ID_INVALID;
  }

  return cell_index->row + 1;
}

// Window handlers
/////////////////////

static uint16_t prv_menu_layer_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return RESOURCE_ID_NUM_RESOURCES - 1;
}

static int16_t prv_menu_layer_get_header_height(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 34;
}

static void prv_menu_layer_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  GBitmap *icon_bitmap = gbitmap_create_with_resource(prv_get_resource_id_from_menu_layer_row(cell_index));

  const GRect cell_layer_bounds = layer_get_bounds(cell_layer);
  GRect icon_frame = gbitmap_get_bounds(icon_bitmap);
  grect_align(&icon_frame, &cell_layer_bounds, GAlignCenter, true /* clip */);

  graphics_draw_bitmap_in_rect(ctx, icon_bitmap, icon_frame);

  gbitmap_destroy(icon_bitmap);
}

static void prv_menu_layer_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *context) {
  const GRect cell_layer_bounds = layer_get_bounds(cell_layer);
  graphics_draw_text(ctx, "Choose an icon:", fonts_get_system_font(FONT_KEY_GOTHIC_28), cell_layer_bounds,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void prv_menu_layer_select_click(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  IconSelectionWindowData *data = context;
  if (data->selection_callback) {
    data->selection_callback(prv_get_resource_id_from_menu_layer_row(cell_index), data->selection_callback_context);
  }
}

// Window handlers
/////////////////////

static void prv_window_load(Window *window) {
  IconSelectionWindowData *window_data = window_get_user_data(window);

  Layer *window_layer = window_get_root_layer(window);
  const GRect window_layer_bounds = layer_get_bounds(window_layer);

  window_data->menu_layer = menu_layer_create(window_layer_bounds);
  MenuLayer *menu_layer = window_data->menu_layer;
  menu_layer_set_callbacks(menu_layer, window_data, (MenuLayerCallbacks) {
    .get_num_rows = prv_menu_layer_get_num_rows,
    .get_header_height = prv_menu_layer_get_header_height,
    .draw_row = prv_menu_layer_draw_row,
    .draw_header = prv_menu_layer_draw_header,
    .select_click = prv_menu_layer_select_click,
  });
  menu_layer_set_click_config_onto_window(menu_layer, window);
  menu_layer_set_normal_colors(menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(menu_layer, GColorCobaltBlue, GColorWhite);
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void prv_window_unload(Window *window) {
  IconSelectionWindowData *window_data = window_get_user_data(window);
  if (window_data) {
    menu_layer_destroy(window_data->menu_layer);
    window_destroy(window_data->window);
  }
  free(window_data);
}

void icon_selection_window_push(IconSelectionWindowCallback selection_callback, void *selection_callback_context) {
  IconSelectionWindowData *window_data = malloc(sizeof(IconSelectionWindowData));
  if (!window_data) {
    return;
  }
  memset(window_data, 0, sizeof(IconSelectionWindowData));

  window_data->window = window_create();
  Window *window = window_data->window;
  window_set_user_data(window, window_data);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_data->selection_callback = selection_callback;
  window_data->selection_callback_context = selection_callback_context;

  window_stack_push(window, true /* animated */);
}
