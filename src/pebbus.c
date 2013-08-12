
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "util.h"

PBL_APP_INFO_SIMPLE(HTTP_UUID, "OneBus", "Deniable Corp", 1 /* App version */);

#define STOP_HTTP_COOKIE 1949327672
#define ARRIVAL_HTTP_COOKIE 1949327673

#define LOC_FLOAT_SCALAR 10000000L
  
Window window;
TextLayer textLayer;

static long our_latitude;
static long our_longitude;
bool located = false;

void request_stops() {
  if(!located) {
    text_layer_set_text(&textLayer, "LOCATING...");
    APP_LOG(APP_LOG_LEVEL_INFO, "LOCATION_REQUEST: %d", http_location_request());
    return;
  }
  text_layer_set_text(&textLayer, "GETTING STOPS!");
  DictionaryIterator *body;
  HTTPResult result = http_out_get("http://bottlebus.herokuapp.com/stops", STOP_HTTP_COOKIE, &body);
  if(result != HTTP_OK) {
    text_layer_set_text(&textLayer, itoa(result));
    return;
  }
  dict_write_int(body, 1, &our_latitude, 4, true);
  dict_write_int(body, 2, &our_longitude, 4, true);

  text_layer_set_text(&textLayer, itoa(http_out_send()));
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
  uint8_t len;
  static char v[32];
  switch(cookie) {
    case STOP_HTTP_COOKIE:
      len = dict_find(received, 1)->value->uint8;
      for(unsigned char i=0;i<len;i++) {
        strcpy(v,dict_find(received, i+2)->value->cstring);
        break;
      }
      text_layer_set_text(&textLayer, v);
      break;
  }
}

void failed(int32_t cookie, int http_status, void* context) {
  return;
}

void location(float latitude, float longitude, float altitude, float accuracy, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "LOCATED");
  text_layer_set_text(&textLayer, "LOCATED!");
  our_latitude = latitude * LOC_FLOAT_SCALAR;
  our_longitude = longitude * LOC_FLOAT_SCALAR;
  located = true;
  request_stops();
}

void reconnect(void* context) {
  request_stops();
}

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

  request_stops();
}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

}


// This usually won't need to be modified

void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}


// Standard app initialisation

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Button App");
  window_stack_push(&window, true /* Animated */);

  text_layer_init(&textLayer, window.layer.frame);
  text_layer_set_text(&textLayer, "Hello World");
  text_layer_set_font(&textLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(&window.layer, &textLayer.layer);

  // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

  http_set_app_id(STOP_HTTP_COOKIE);

  http_register_callbacks((HTTPCallbacks){
    .failure=failed,
    .success=success,
    .reconnect=reconnect,
    .location=location
  }, (void*)ctx);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 256,
      }
    }
  };
  app_event_loop(params, &handlers);
}
