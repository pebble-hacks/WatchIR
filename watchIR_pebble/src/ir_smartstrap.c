#include "ir_smartstrap.h"

// Defines and Types
/////////////////////

// This define must match on Arduino
static const uint16_t WATCH_IR_SERVICE_ID = 0x1001;

// This enum must match on Arduino
typedef enum {
  WatchIrAttributeIdReset = 0x0000,
  WatchIrAttributeIdRecord,
  WatchIrAttributeIdRecordResult,
  WatchIrAttributeIdTransmit,
  NumWatchIrAttributes
} WatchIrAttributeId;

typedef struct {
  SmartstrapAttribute *smartstrap_attributes[NumWatchIrAttributes];
  IrSmartstrapRecordResultCallback record_result_callback;
  void *record_result_context;
} IrSmartstrapData;

static IrSmartstrapData s_ir_smartstrap_data;

// Helpers
///////////

static SmartstrapAttribute *prv_get_smartstrap_attribute_by_id(WatchIrAttributeId attribute_id) {
  return s_ir_smartstrap_data.smartstrap_attributes[attribute_id];
}

void prv_ir_smartstrap_record_result(void) {
  SmartstrapAttribute *record_result_attribute = prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdRecordResult);

  SmartstrapResult result = smartstrap_attribute_read(record_result_attribute);

  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Read record result failed with error %d", result);
    return;
  }
}

// Smartstrap Handlers
///////////////////////

static void prv_availablility_status_changed(SmartstrapServiceId service_id, bool is_available) {
  if (service_id == WATCH_IR_SERVICE_ID) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "IR smartstrap availability: %d", is_available);
  }
}

static void prv_did_write(SmartstrapAttribute *attr, SmartstrapResult result) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "did_write(<%p>, %d)", attr, result);
}

static void prv_did_read(SmartstrapAttribute *attr, SmartstrapResult result,
                         const uint8_t *data, size_t length) {
  if (attr == prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdRecordResult)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "did_read(WatchIrAttributeIdRecordResult, %d, %d)", result, length);
    if (result == SmartstrapResultOk && length == sizeof(IrCode)) {
      IrCode *recorded_ir_code = (IrCode *)data;
      if (s_ir_smartstrap_data.record_result_callback) {
        s_ir_smartstrap_data.record_result_callback(recorded_ir_code, s_ir_smartstrap_data.record_result_context);
      }
    } else if (length != sizeof(IrCode)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Expected length %d, got %d", sizeof(IrCode), length);
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "did_read(<%p>, %d)", attr, result);
  }
}

static void prv_notified(SmartstrapAttribute *attr) {
  if (attr == prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdRecordResult)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "notified(WatchIrAttributeIdRecordResult)");
    prv_ir_smartstrap_record_result();
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "notified(<%p>)", attr);
  }
}

// Public API
/////////////////////

void ir_smartstrap_init(void) {
#if DEBUG == 0
  memset(&s_ir_smartstrap_data, 0, sizeof(IrSmartstrapData));
  smartstrap_subscribe((SmartstrapHandlers) {
    .availability_did_change = prv_availablility_status_changed,
    .did_write = prv_did_write,
    .did_read = prv_did_read,
    .notified = prv_notified,
  });
  const uint16_t timeout_ms = 1000;
  smartstrap_set_timeout(timeout_ms);

  for (WatchIrAttributeId attribute_id = 0; attribute_id < NumWatchIrAttributes; attribute_id++) {
    // All the attributes have size 1 except for transmit and record result, which are the size of an IrCode
    size_t attribute_length = 1;
    if ((attribute_id == WatchIrAttributeIdTransmit) || (attribute_id == WatchIrAttributeIdRecordResult)) {
      attribute_length = sizeof(IrCode);
    }
    s_ir_smartstrap_data.smartstrap_attributes[attribute_id] = smartstrap_attribute_create(WATCH_IR_SERVICE_ID,
                                                                                           attribute_id,
                                                                                           attribute_length);
  }
#endif
}

void ir_smartstrap_deinit(void) {
#if DEBUG == 0
  smartstrap_unsubscribe();
  for (WatchIrAttributeId attribute_id = 0; attribute_id < NumWatchIrAttributes; attribute_id++) {
    smartstrap_attribute_destroy(prv_get_smartstrap_attribute_by_id(attribute_id));
  }
#endif
}

void ir_smartstrap_reset(void) {
#if DEBUG == 0
  SmartstrapAttribute *reset_attribute = prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdReset);

  SmartstrapResult result;
  uint8_t *buffer;
  size_t length;
  result = smartstrap_attribute_begin_write(reset_attribute, &buffer, &length);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Begin write reset failed with error %d", result);
    return;
  }

  result = smartstrap_attribute_end_write(reset_attribute, 1, false);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "End write reset failed with error %d", result);
    return;
  }
#endif
}

void ir_smartstrap_transmit(const IrCode *ir_code) {
#if DEBUG == 0
  if (!ir_code) {
    return;
  }

  SmartstrapAttribute *transmit_attribute = prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdTransmit);

  SmartstrapResult result;
  uint8_t *buffer;
  size_t length;
  result = smartstrap_attribute_begin_write(transmit_attribute, &buffer, &length);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Begin write transmit failed with error %d", result);
    return;
  }

  memcpy(buffer, ir_code, sizeof(IrCode));

  result = smartstrap_attribute_end_write(transmit_attribute, sizeof(IrCode), false);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "End write transmit failed with error %d", result);
    return;
  }
#endif
}

void ir_smartstrap_record(IrSmartstrapRecordResultCallback callback, void *context) {
#if DEBUG == 0
  SmartstrapAttribute *record_attribute = prv_get_smartstrap_attribute_by_id(WatchIrAttributeIdRecord);

  s_ir_smartstrap_data.record_result_callback = callback;
  s_ir_smartstrap_data.record_result_context = context;

  SmartstrapResult result;
  uint8_t *buffer;
  size_t length;
  result = smartstrap_attribute_begin_write(record_attribute, &buffer, &length);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Begin write record failed with error %d", result);
    return;
  }

  result = smartstrap_attribute_end_write(record_attribute, 1, false);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "End write record failed with error %d", result);
    return;
  }
#endif
}
