#pragma once

#include <pebble.h>

#include "ir_button.h"

typedef void (*IrSmartstrapRecordResultCallback)(const IrCode *ir_code, void *context);

void ir_smartstrap_init(void);
void ir_smartstrap_deinit(void);
void ir_smartstrap_reset(void); // TODO use this to reset the smartstrap out of recording mode
void ir_smartstrap_transmit(const IrCode *ir_code);
void ir_smartstrap_record(IrSmartstrapRecordResultCallback callback, void *context);
