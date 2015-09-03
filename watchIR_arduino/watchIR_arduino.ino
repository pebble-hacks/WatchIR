// Available from:
// https://github.com/pebble/ArduinoPebbleSerial
#include <ArduinoPebbleSerial.h>
// Requires the special Teensy version of IRremote available from:
// https://www.pjrc.com/teensy/td_libs_IRremote.html
#include <IRremote.h>

#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// WatchIrSmartStrapData
//////////////////////

#define MAX_SUPPORTED_IR_DURATIONS RAWBUF // RAWBUF defined in IRremote.h
typedef struct {
  size_t num_durations;
  size_t durations[MAX_SUPPORTED_IR_DURATIONS];
} WatchIrSmartstrapData;

#define WATCH_IR_SMARTSTRAP_BUFFER_SIZE GET_PAYLOAD_BUFFER_SIZE(sizeof(WatchIrSmartstrapData))
static uint8_t s_watch_ir_smartstrap_data_buffer[WATCH_IR_SMARTSTRAP_BUFFER_SIZE];

static WatchIrSmartstrapData s_watch_ir_smartstrap_data;
#define WatchIrSmartstrapDataRef ((uint8_t *)&s_watch_ir_smartstrap_data)

static const uint16_t WATCH_IR_SERVICE_ID = 0x1001;
static const uint16_t WATCH_IR_SERVICES[] = {WATCH_IR_SERVICE_ID};

// WatchIrAttributes
//////////////////////

static void prv_handle_reset_request(void);
static void prv_handle_record_request(void);
static void prv_handle_record_result_request(void);
static void prv_handle_transmit_request(void);

typedef enum {
  WatchIrAttributeIdReset = 0x0000,
  WatchIrAttributeIdRecord,
  WatchIrAttributeIdRecordResult,
  WatchIrAttributeIdTransmit,
  NumWatchIrAttributes
} WatchIrAttributeId;

typedef void (*WatchIrAttributeRequestHandler)(void);

typedef struct {
  WatchIrAttributeId attribute_id;
  size_t attribute_length;
  RequestType request_type;
  WatchIrAttributeRequestHandler request_handler;
} WatchIrAttribute;

static const WatchIrAttribute s_watch_ir_attributes[] = {
  [WatchIrAttributeIdReset] = {WatchIrAttributeIdReset, 
                               1, 
                               RequestTypeWrite, 
                               prv_handle_reset_request},
  [WatchIrAttributeIdRecord] = {WatchIrAttributeIdRecord, 
                                1, 
                                RequestTypeWrite, 
                                prv_handle_record_request},
  [WatchIrAttributeIdRecordResult] = {WatchIrAttributeIdRecordResult, 
                                      0,
                                      RequestTypeRead, 
                                      prv_handle_record_result_request},
  [WatchIrAttributeIdTransmit] = {WatchIrAttributeIdTransmit, 
                                  sizeof(WatchIrSmartstrapData), 
                                  RequestTypeWrite, 
                                  prv_handle_transmit_request},
};

// WatchIrState
//////////////////////

typedef struct {
  bool recording;
  bool recorded_result;
} WatchIrState;

static WatchIrState s_watch_ir_state;

// WatchIrRemoteData 
//////////////////////

static int IR_RECV_PIN = 11;
static int IR_STATUS_PIN = LED_BUILTIN;

struct WatchIrRemoteData {
  IRrecv *ir_recv;
  IRsend ir_send;
  decode_results ir_decode_results;
  WatchIrRemoteData(int ir_recv_pin) {
    ir_recv = new IRrecv(ir_recv_pin);
  }
  ~WatchIrRemoteData() {
    delete ir_recv;
  }
};
typedef struct WatchIrRemoteData WatchIrRemoteData;

static WatchIrRemoteData s_watch_ir_remote_data(IR_RECV_PIN);

// Arduino Setup
//////////////////////

void setup()
{
  Serial.begin(115200);
  s_watch_ir_remote_data.ir_recv->enableIRIn(); // Start the receiver
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Teensy 3.0/3.1 uses hardware serial mode (pins 0/1) with RX/TX shorted together
  ArduinoPebbleSerial::begin_hardware(s_watch_ir_smartstrap_data_buffer, sizeof(s_watch_ir_smartstrap_data_buffer), 
                                      Baud9600, WATCH_IR_SERVICES, ARRAY_LENGTH(WATCH_IR_SERVICES));
}

// IR Functions
//////////////////////

// Stores the code for later playback
// Most of this code is just logging
static void prv_store_record_result(decode_results *results) {
  Serial.println("Saving raw code:");
  // We have to limit the number of durations so it will fit in the Pebble persisted storage
  s_watch_ir_smartstrap_data.num_durations = MIN(results->rawlen - 1, MAX_SUPPORTED_IR_DURATIONS);
  Serial.print("Length: ");
  Serial.println(s_watch_ir_smartstrap_data.num_durations);
  size_t *rawCodes = s_watch_ir_smartstrap_data.durations;
  // To store raw codes:
  // Drop first value (gap)
  // Convert from ticks to microseconds
  // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
  for (size_t i = 1; i <= s_watch_ir_smartstrap_data.num_durations; i++) {
    if (i % 2) {
      // Mark
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
      Serial.print(" m");
    } 
    else {
      // Space
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
      Serial.print(" s");
    }
    Serial.print(rawCodes[i - 1], DEC);
  }
  Serial.println("");
}

static void prv_transmit_code(void) {
  // Transmit code multiple times to help with communication
  const uint8_t num_transmissions = 3;
  Serial.print("Transmitting code ");
  Serial.print(num_transmissions);
  Serial.println(" times");

  for (int i = 0; i < num_transmissions; i++) {
    digitalWrite(IR_STATUS_PIN, HIGH);

    s_watch_ir_remote_data.ir_send.sendRaw(s_watch_ir_smartstrap_data.durations, 
                                           s_watch_ir_smartstrap_data.num_durations, 
                                           38); // Transmit at 38KHz
    
    digitalWrite(IR_STATUS_PIN, LOW);
    
    // Wait a bit before successive transmissions
    delay(50);
  }
}

// Smartstrap functions
////////////////////////

static void prv_handle_reset_request() {
  Serial.println("Received reset request");
  
  // Reset the state
  s_watch_ir_state = {0};
  
  // ACK that the reset request was received
  const bool success = true;
  ArduinoPebbleSerial::write(success, NULL, 0);
}

static void prv_handle_record_request() {
  Serial.println("Received record request");
  
  // Go into recording mode
  s_watch_ir_state.recording = true;
  s_watch_ir_remote_data.ir_recv->enableIRIn();
  
  // ACK that the record request was received
  const bool success = true;
  ArduinoPebbleSerial::write(success, NULL, 0);
}

static void prv_handle_record_result_request() {
  Serial.println("Received record result request");
  
  // Send the result of the recording to the watch
  const bool success = s_watch_ir_state.recorded_result;
  ArduinoPebbleSerial::write(success, 
                             WatchIrSmartstrapDataRef, 
                             sizeof(WatchIrSmartstrapData));
}

static void prv_handle_transmit_request() {
  Serial.println("Received transmit request");

  memcpy(WatchIrSmartstrapDataRef, s_watch_ir_smartstrap_data_buffer, sizeof(WatchIrSmartstrapData));
  
  // Transmit the IR code received from the watch
  prv_transmit_code();
  
  // ACK that the transmit request was received
  const bool success = !s_watch_ir_state.recording;
  ArduinoPebbleSerial::write(success, NULL, 0);
}

static bool prv_validate_watch_request(uint16_t attribute_id, RequestType type, size_t length) {
  if (!(attribute_id < NumWatchIrAttributes)) {
    Serial.println("Invalid attribute_id");
    return false;
  }
  
  WatchIrAttribute attribute_record = s_watch_ir_attributes[attribute_id];
  if (attribute_record.request_type != type) {
    Serial.print("Type mismatch; expected ");
    Serial.print(attribute_record.request_type);
    Serial.print(" but received ");
    Serial.println(type);
    return false;
  }

  if (attribute_record.attribute_length != length) {
    Serial.print("Length mismatch; expected ");
    Serial.print(attribute_record.attribute_length);
    Serial.print(" but received ");
    Serial.println(length);
    return false;
  }
  
  return true; 
}

static void prv_process_watch_request(uint16_t attribute_id, RequestType type, size_t length) {
  if (!prv_validate_watch_request(attribute_id, type, length)) {
    ArduinoPebbleSerial::write(false, NULL, 0);
    return;
  }

  WatchIrAttribute attribute_record = s_watch_ir_attributes[attribute_id];
  attribute_record.request_handler();
}

// Arduino Loop
////////////////

void loop() {
  uint16_t service_id;
  uint16_t attribute_id;
  size_t length;
  RequestType type;
  if (ArduinoPebbleSerial::feed(&service_id, &attribute_id, &length, &type)) {
    // Process the request
    // TODO move or remove these debug print statements
    Serial.print("Received raw request: service_id=");
    Serial.print(service_id, HEX);
    Serial.print(" attribute_id=");
    Serial.print(attribute_id, HEX);
    Serial.print(" length=");
    Serial.print(length);
    Serial.print(" type=");
    Serial.println(type);
    if (service_id == WATCH_IR_SERVICE_ID) {
      prv_process_watch_request(attribute_id, type, length);
    }
  }

  static bool s_last_connected = false;
  const bool is_connected = ArduinoPebbleSerial::is_connected();

  if (is_connected != s_last_connected) {
    if (is_connected) {
      Serial.println("Connected to watch");
    } else {
      Serial.println("Disconnected from watch");
    }
  }
  s_last_connected = is_connected;

  // Don't do anything if a watch is not connected
  if (!is_connected) {
    return;
  }
  
  // If we're in recording mode, check if we've received a code
  if (s_watch_ir_state.recording) {
    decode_results *results = &s_watch_ir_remote_data.ir_decode_results;
    if (s_watch_ir_remote_data.ir_recv->decode(results)) {
      digitalWrite(IR_STATUS_PIN, HIGH);

      // Note that we don't call s_watch_ir_remote_data.ir_recv->resume();
      // because we only want to record a single IR code
      prv_store_record_result(results);

      // Update our state
      s_watch_ir_state.recorded_result = true;
      s_watch_ir_state.recording = false;
      
      digitalWrite(IR_STATUS_PIN, LOW);

      // Notify the watch that we've recorded a code
      ArduinoPebbleSerial::notify(WATCH_IR_SERVICE_ID, 
                                  WatchIrAttributeIdRecordResult);
    }
  }
}

