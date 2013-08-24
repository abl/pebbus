#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "util.h"

#include "http_util.h"
#include "stops.h"
#include "arrivals.h"

long http_latitude=0;
long http_longitude=0;
bool http_located = false;
bool http_connected = true;

char current_stop_id[16];

inline bool http_isFailure(HTTPResult result) {
  if(result == HTTP_SEND_TIMEOUT ||
     result == HTTP_NOT_CONNECTED ||
     result == HTTP_BRIDGE_NOT_RUNNING ||
     result == HTTP_INVALID_BRIDGE_RESPONSE) {
    http_connected = false;
    return true;
  }
  if(result == HTTP_INTERNAL_INCONSISTENCY) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "HTTP_INTERNAL_INCONSISTENCY");
    return true;
  }
  if(result == HTTP_BUFFER_OVERFLOW) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "HTTP_BUFFER_OVERFLOW");
    return true;
  }
  if(result == HTTP_NOT_ENOUGH_STORAGE) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "HTTP_NOT_ENOUGH_STORAGE");
    return true;
  }
  if(result == HTTP_INVALID_ARGS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "HTTP_INVALID_ARGS");
    return true;
  }
  http_connected = true;
  if(result == HTTP_OK) {
    return false;
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown HTTP response code: %d", result);
  return false;
}

void http_request_stops() {

  APP_LOG(APP_LOG_LEVEL_INFO, "http_request_stops()");
  if(!http_located) {
    HTTPResult result = http_location_request();
    if(http_isFailure(result)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "http_location_request: %d", result);
    }
    return;
  }
  DictionaryIterator *body;
  HTTPResult result = http_out_get("http://bottlebus.herokuapp.com/stops", STOP_HTTP_COOKIE, &body);
  if(http_isFailure(result)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "http_out_get /stops: %d", result);
    return;
  }

  dict_write_int(body, 1, &http_latitude, 4, true);
  dict_write_int(body, 2, &http_longitude, 4, true);
  http_out_send();
}

void http_request_arrivals(char *stop) {
  uint8_t filter = 0;

  APP_LOG(APP_LOG_LEVEL_INFO, "http_request_arrivals()");

  strncpy(current_stop_id, stop, sizeof(current_stop_id));

  DictionaryIterator *body;
  HTTPResult result = http_out_get("http://bottlebus.herokuapp.com/arrivals", ARRIVAL_HTTP_COOKIE, &body);
  if(http_isFailure(result)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "http_out_get /arrivals: %d", result);
    return;
  }

  dict_write_cstring(body, 1, stop);
  dict_write_int(body, 2, &filter, 1, true); //Filter function; not yet used.
  http_out_send();
}

void http_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "http_success(%d, %d, ...)", cookie, http_status);
  switch(cookie) {
    case STOP_HTTP_COOKIE:
      stops_success(cookie, http_status, received, context);
      break;
    case ARRIVAL_HTTP_COOKIE:
      arrivals_success(cookie, http_status, received, context);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "SUCCESS,UNKNOWN_COOKIE_%d: %d", cookie, http_status);
      break;
  }
}

void http_failed(int32_t cookie, int http_status, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "http_failed(%d, %d, ...)", cookie, http_status);
  switch(cookie) {
    case STOP_HTTP_COOKIE:
      APP_LOG(APP_LOG_LEVEL_ERROR, "STOP_HTTP_COOKIE: %d", http_status);
      break;
    case ARRIVAL_HTTP_COOKIE:
      APP_LOG(APP_LOG_LEVEL_ERROR, "ARRIVAL_HTTP_COOKIE: %d", http_status);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "FAILURE,UNKNOWN_COOKIE_%d: %d", cookie, http_status);
      break;
  }
}

void http_location(float latitude, float longitude, float altitude, float accuracy, void* context) {
  http_connected = true;
  http_latitude = latitude * LOC_FLOAT_SCALAR;
  http_longitude = longitude * LOC_FLOAT_SCALAR;
  APP_LOG(APP_LOG_LEVEL_INFO, "Location: lat %d lon %d", http_latitude, http_longitude);
  http_located = true;
  http_request_stops();
}

void http_reconnect(void* context) {
  http_connected = true;
}