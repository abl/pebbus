#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "util.h"
#include "arrivals.h"
#include "pebbus.h"
#include "http_util.h"

static BusArrival ARRIVALS[MAX_ARRIVALS];

static uint8_t arrivalsCount = 0;

static MenuLayer _arrivalsMenu;

/*

    Basic Layout...

    H1: "Last Updated $TIME"
        R1: $ARRIVALS[1] -> ARRIVALS[1]
        R2: ...
        R3: ...
    H1: Options
        R1: Refresh -> refresh()

*/

#define ARRIVAL_SECTION_MAIN 0
#define ARRIVAL_SECTION_OPTIONS 1

#define ARRIVAL_SECTION_OPTIONS_ROWS 1

static char arrival_time_str[] = "Updated: 00:00 PM";

void arrivals_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    switch(section_index) {
        case ARRIVAL_SECTION_MAIN:
            if(!http_located) {
                menu_cell_basic_header_draw(ctx, cell_layer, "Loading...");
            }
            else {
                menu_cell_basic_header_draw(ctx, cell_layer, arrival_time_str);
            }
            break;
        case ARRIVAL_SECTION_OPTIONS:
            menu_cell_basic_header_draw(ctx, cell_layer, "Options");
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "arrivals_draw_header: unknown section %d", section_index);
    }
}

void arrivals_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    static char *options[1] = {
        "Refresh"
    };

    static char buffer[16]; //Long enough for "99:99 scheduled\0"

    switch(cell_index->section) {
        case ARRIVAL_SECTION_MAIN:
            graphics_context_set_text_color(ctx, GColorBlack);
            graphics_text_draw(ctx, ARRIVALS[cell_index->row].route, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(4, 3, 140, 28), 0, GTextAlignmentLeft, NULL);
            snprintf(buffer, sizeof(buffer), "%s expected", ARRIVALS[cell_index->row].actual);
            graphics_text_draw(ctx, buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(32, 0, 108, 36), 0, GTextAlignmentLeft, NULL);
            snprintf(buffer, sizeof(buffer), "%s scheduled", ARRIVALS[cell_index->row].scheduled);
            graphics_text_draw(ctx, buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(32, 18, 108, 36), 0, GTextAlignmentLeft, NULL);
            break;
        case ARRIVAL_SECTION_OPTIONS:
            graphics_context_set_text_color(ctx, GColorBlack);
            graphics_text_draw(ctx, options[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(8, 8, 140, 18), 0, GTextAlignmentLeft, NULL);
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "arrivals_draw_row: unknown section %d", cell_index->section);
    }
}

int16_t arrivals_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return MENU_CELL_BASIC_HEIGHT;
}

int16_t arrivals_get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

uint16_t arrivals_get_num_rows(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    switch(section_index) {
        case ARRIVAL_SECTION_MAIN:
            return arrivalsCount;
        case ARRIVAL_SECTION_OPTIONS:
            return ARRIVAL_SECTION_OPTIONS_ROWS;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "arrivals_get_num_rows: unknown section %d", section_index);
    }
    return 0;
}

uint16_t arrivals_get_num_sections(struct MenuLayer *menu_layer, void *callback_context) {
    return 2;
}

void arrivals_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    http_request_arrivals(current_stop_id);
}

void arrivals_select_long_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return;
}

void arrivals_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
    return;
}

Window arrival_window;

void arrivals_init_menu(char *stop) {
	static char title[16];
	snprintf(title, sizeof(title), "Arrivals: %s", current_stop_id);

	window_init(&arrival_window, title);

	//15px to make room for the title bar.
	menu_layer_init(&_arrivalsMenu, GRect(0, 0, arrival_window.layer.frame.size.w, arrival_window.layer.frame.size.h-15));
	menu_layer_set_click_config_onto_window(&_arrivalsMenu, &arrival_window);

	static MenuLayerCallbacks arrivals_callbacks = {
		.draw_header = arrivals_draw_header,
		.draw_row = arrivals_draw_row,
		.get_cell_height = arrivals_get_cell_height,
		.get_header_height = arrivals_get_header_height,
		.get_num_rows = arrivals_get_num_rows,
		.get_num_sections = arrivals_get_num_sections,
		.select_click = arrivals_select_click,
		.select_long_click = arrivals_select_long_click,
		.selection_changed = arrivals_selection_changed
	};

	menu_layer_set_callbacks(&_arrivalsMenu, ctx, arrivals_callbacks);

	layer_add_child(&arrival_window.layer, menu_layer_get_layer(&_arrivalsMenu));

	window_stack_push(&arrival_window, true);

	arrivalsCount = 0;
	http_request_arrivals(stop);

}

void arrivals_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context)
{
    Tuple *t = dict_find(received, 1);
    char *v = t->value->cstring;
    uint16_t index = 0;
    uint16_t len = t->length;
    arrivalsCount = 0;
    int16_t time = 0;
    MenuIndex top = { 0, 0 };
    

    PblTm now;
    get_time(&now);
    if (clock_is_24h_style()) {
        string_format_time(arrival_time_str, sizeof(arrival_time_str), "Updated: %H:%M", &now);
    }
    else {
        string_format_time(arrival_time_str, sizeof(arrival_time_str), "Updated: %l:%M %p", &now);
    }

    for(unsigned char i=0;i<MAX_ARRIVALS && index < len;i++) {
        //stokenize(char *source, uint16_t index, char delimiter, char *target, uint16_t max_length)
        index = stokenize(v, index, ':', ARRIVALS[i].route, ARRIVAL_ROUTE_LENGTH, len);
        index = stokenize(v, index, ':', ARRIVALS[i].actual, sizeof(ARRIVALS[i].actual), len);
        index = stokenize(v, index, ':', ARRIVALS[i].scheduled, sizeof(ARRIVALS[i].scheduled), len);

        //TODO: DRY

        time = satoi(ARRIVALS[i].actual, strlen(ARRIVALS[i].actual));

        if(abs(time) > 99*60 + 59) //Maximum displayable value.
        	time = (99*60 + 59) * (time < 0 ? -1 : 1);

        if(time > 60 || time < -60) {
	        snprintf(ARRIVALS[i].actual, ARRIVAL_TIME_LENGTH, "%d:%02d", time / 60, abs(time % 60));
	    } else {
	    	snprintf(ARRIVALS[i].actual, ARRIVAL_TIME_LENGTH, "%d", time % 60);
	    }

	    time = satoi(ARRIVALS[i].scheduled, strlen(ARRIVALS[i].scheduled));

        if(abs(time) > 99*60 + 59) //Maximum displayable value.
        	time = (99*60 + 59) * (time < 0 ? -1 : 1);

        if(time > 60 || time < -60) {
	        snprintf(ARRIVALS[i].scheduled, ARRIVAL_TIME_LENGTH, "%d:%02d", time / 60, abs(time % 60));
	    } else {
	    	snprintf(ARRIVALS[i].scheduled, ARRIVAL_TIME_LENGTH, "%d", time % 60);
	    }

        APP_LOG(APP_LOG_LEVEL_INFO, "Received route:%s actual:%s scheduled:%s (index:%d/%d)", ARRIVALS[i].route, ARRIVALS[i].actual, ARRIVALS[i].scheduled, index, len);
        ++arrivalsCount;
    }

    menu_layer_reload_data(&_arrivalsMenu);
    menu_layer_set_selected_index(&_arrivalsMenu, top, MenuRowAlignBottom, false);

}