#ifndef STOPS_H
#define STOPS_H

#define MAX_STOPS 3
#define STOP_ID_LENGTH 16
#define STOP_NAME_LENGTH 32

typedef struct {
  char id[STOP_ID_LENGTH];
  char direction[3];
  char name[STOP_NAME_LENGTH];
} BusStop;

void stops_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);

void stops_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

int16_t stops_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

int16_t stops_get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

uint16_t stops_get_num_rows(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

uint16_t stops_get_num_sections(struct MenuLayer *menu_layer, void *callback_context);

void stops_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

void stops_select_long_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

void stops_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);

MenuLayer *stops_init_menu();

void stops_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context);

#endif