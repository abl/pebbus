#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "pebbus.h"
#include "http.h"
#include "util.h"
#include "stops.h"
#include "arrivals.h"
#include "http_util.h"

static BusStop STOPS[MAX_STOPS];

static uint8_t stopsCount = 0;

static MenuLayer _stopsMenu;

/*

    Basic Layout...

    H1: "Last Updated $TIME"
        R1: $STOPS[1] -> ARRIVALS[1]
        R2: ...
        R3: ...
    H2: Lat: $LAT
    H3: Lon: $LON
    H4: Options
        R1: Refresh -> refresh()
        R2: Radius $RADIUS -> (NYI) edit_radius()
        R3: Check for Updates -> (NYI) check_for_updates()

*/

#define STOP_SECTION_MAIN 0
#define STOP_SECTION_CREDITS 1
#define STOP_SECTION_LATITUDE 2
#define STOP_SECTION_LONGITUDE 3
#define STOP_SECTION_OPTIONS 4
#define STOP_SECTION_TOTAL 5

#define STOP_SECTION_OPTIONS_ROWS 3

static char stop_time_str[] = "Updated: 00:00 PM";

void stops_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    static char s_longitude[16],s_latitude[16];
    switch(section_index) {
        case STOP_SECTION_MAIN:
            if(!http_located) {
                menu_cell_basic_header_draw(ctx, cell_layer, "Loading...");
            }
            else {
                menu_cell_basic_header_draw(ctx, cell_layer, stop_time_str);
            }
            break;
        case STOP_SECTION_CREDITS:
            menu_cell_basic_header_draw(ctx, cell_layer, "Powered by OneBusAway");
            break;
        case STOP_SECTION_LONGITUDE:
            if(http_located) {
                snprintf(s_longitude, 16, "Lon: %ld.%ld", http_longitude / LOC_FLOAT_SCALAR, labs(http_longitude % LOC_FLOAT_SCALAR));
                menu_cell_basic_header_draw(ctx, cell_layer, s_longitude);
            }
            break;
        case STOP_SECTION_LATITUDE:
            if(http_located) {
                snprintf(s_latitude, 16, "Lat: %ld.%ld", http_latitude / LOC_FLOAT_SCALAR, labs(http_latitude % LOC_FLOAT_SCALAR));
                menu_cell_basic_header_draw(ctx, cell_layer, s_latitude);
            }
            break;
        case STOP_SECTION_OPTIONS:
            menu_cell_basic_header_draw(ctx, cell_layer, "Options");
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "stops_draw_header: unknown section %d", section_index);
    }
}

void stops_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    static char *options[3] = {
        "Refresh",
        "Radius: 150",
        "Check for updates"
    };

    switch(cell_index->section) {
        case STOP_SECTION_MAIN:
            graphics_context_set_text_color(ctx, GColorBlack);
            graphics_text_draw(ctx, STOPS[cell_index->row].direction, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(4, 3, 140, 28), 0, GTextAlignmentLeft, NULL);
            graphics_text_draw(ctx, STOPS[cell_index->row].name, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(32, 0, 108, 36), 0, GTextAlignmentLeft, NULL);
            break;
        case STOP_SECTION_OPTIONS:
            graphics_context_set_text_color(ctx, GColorBlack);
            graphics_text_draw(ctx, options[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(8, 8, 140, 18), 0, GTextAlignmentLeft, NULL);
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "stops_draw_row: unknown section %d", cell_index->section);
    }
}

int16_t stops_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return MENU_CELL_BASIC_HEIGHT;
}

int16_t stops_get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

uint16_t stops_get_num_rows(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    switch(section_index) {
        case STOP_SECTION_MAIN:
            return stopsCount;
        case STOP_SECTION_LONGITUDE:
        case STOP_SECTION_LATITUDE:
        case STOP_SECTION_CREDITS:
            return 0;
        case STOP_SECTION_OPTIONS:
            return STOP_SECTION_OPTIONS_ROWS;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "stops_get_num_rows: unknown section %d", section_index);
    }
    return 0;
}

uint16_t stops_get_num_sections(struct MenuLayer *menu_layer, void *callback_context) {
    return STOP_SECTION_TOTAL;
}

void stops_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    switch(cell_index->section) {
        case STOP_SECTION_MAIN:
            arrivals_init_menu(STOPS[cell_index->row].id);
            break;
        case STOP_SECTION_OPTIONS:
            http_request_stops();
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "stops_get_num_rows: unknown section %d", cell_index->section);
    }
}

void stops_select_long_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return;
}

void stops_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
    return;
}

MenuLayer *stops_init_menu(Window *window, void *ctx) {

  //15px to make room for the title bar.
  menu_layer_init(&_stopsMenu, GRect(0, 0, window->layer.frame.size.w, window->layer.frame.size.h-15));
  menu_layer_set_click_config_onto_window(&_stopsMenu, window);

  static MenuLayerCallbacks stops_callbacks = {
    .draw_header = stops_draw_header,
    .draw_row = stops_draw_row,
    .get_cell_height = stops_get_cell_height,
    .get_header_height = stops_get_header_height,
    .get_num_rows = stops_get_num_rows,
    .get_num_sections = stops_get_num_sections,
    .select_click = stops_select_click,
    .select_long_click = stops_select_long_click,
    .selection_changed = stops_selection_changed
  };

  menu_layer_set_callbacks(&_stopsMenu, ctx, stops_callbacks);

  return &_stopsMenu;

}

void stops_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context)
{
    Tuple *t = dict_find(received, 1);
    char *v = t->value->cstring;
    uint16_t index = 0;
    uint16_t len = t->length;
    stopsCount = 0;
    MenuIndex top = { 0, 0 };

    PblTm now;
    get_time(&now);
    if (clock_is_24h_style()) {
        string_format_time(stop_time_str, sizeof(stop_time_str), "Updated: %H:%M", &now);
    }
    else {
        string_format_time(stop_time_str, sizeof(stop_time_str), "Updated: %l:%M %p", &now);
    }

    for(unsigned char i=0;i<MAX_STOPS && index < len;i++) {
        //stokenize(char *source, uint16_t index, char delimiter, char *target, uint16_t max_length)
        index = stokenize(v, index, ':', STOPS[i].id, STOP_ID_LENGTH, len);
        index = stokenize(v, index, ':', STOPS[i].direction, 3, len);
        index = stokenize(v, index, ':', STOPS[i].name, STOP_NAME_LENGTH, len);

        APP_LOG(APP_LOG_LEVEL_INFO, "Received id:%s direction:%s name:%s (index:%d/%d)", STOPS[i].id, STOPS[i].direction, STOPS[i].name, index, len);
        ++stopsCount;
    }

    menu_layer_reload_data(&_stopsMenu);
    menu_layer_set_selected_index(&_stopsMenu, top, MenuRowAlignBottom, false);

}