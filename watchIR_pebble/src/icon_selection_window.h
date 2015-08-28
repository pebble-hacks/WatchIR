#pragma once

#include <pebble.h>

typedef void (*IconSelectionWindowCallback)(uint32_t selected_icon, void *context);

void icon_selection_window_push(IconSelectionWindowCallback selection_callback, void *selection_callback_context);
