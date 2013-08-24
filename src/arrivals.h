#ifndef ARRIVALS_H
#define ARRIVALS_H

#define MAX_ARRIVALS 6
#define ARRIVAL_ROUTE_LENGTH 8
#define ARRIVAL_TIME_LENGTH 6

extern char current_stop_id[];

typedef struct {
  char route[ARRIVAL_ROUTE_LENGTH];
  char actual[ARRIVAL_TIME_LENGTH];
  char scheduled[ARRIVAL_TIME_LENGTH];
} BusArrival;

void arrivals_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);

void arrivals_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

int16_t arrivals_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

int16_t arrivals_get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

uint16_t arrivals_get_num_rows(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

uint16_t arrivals_get_num_sections(struct MenuLayer *menu_layer, void *callback_context);

void arrivals_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

void arrivals_select_long_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

void arrivals_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);

void arrivals_init_menu(char *stop);

void arrivals_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context);

#endif