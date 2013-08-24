
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "util.h"

#include "http_util.h"
#include "stops.h"
#include "arrivals.h"

PBL_APP_INFO_SIMPLE(HTTP_UUID, "OneBus", "Deniable Corp", 1 /* App version */);

Window window;
AppContextRef ctx;

MenuLayer *stopsMenu;
MenuLayer *arrivalsMenu;

void handle_init(AppContextRef _ctx) {
  (void)_ctx;

  ctx = _ctx;

  http_set_app_id(APP_HTTP_COOKIE);

  window_init(&window, "PebBus");
  window_stack_push(&window, true /* Animated */);

  stopsMenu = stops_init_menu(&window, ctx);

  layer_add_child(&window.layer, menu_layer_get_layer(stopsMenu));

  http_register_callbacks((HTTPCallbacks){
    .failure=http_failed,
    .success=http_success,
    .reconnect=http_reconnect,
    .location=http_location
  }, (void*)ctx);

  http_request_stops();
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
