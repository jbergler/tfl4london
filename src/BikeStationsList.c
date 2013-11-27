#include <pebble.h>
#include <pebble_fonts.h>
#include "Watchapp.h"
#include "WatchappTypes.h"

Window* listWindow;
MenuLayer* listMenuLayer;

bool ending = false;

#define MAX_STATIONS (5)
#define MAX_LENGTH (64)

typedef struct {
    int id;
    char name[MAX_LENGTH];
    char info[MAX_LENGTH];
} BikeStation;

BikeStation bike_station_list[MAX_STATIONS];
int bike_station_list_size = 0;

BikeStation* bike_station_list_get(int index)
{
    if (index < 0 || index >= MAX_STATIONS) {
        APP_LOG(APP_LOG_LEVEL_INFO, "bike_station_list_get(%i) return NULL (OutOfBounds)", index);
        return NULL;
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "bike_station_list_get(%i) return *BikeStation{id = %i, name = %s, info = %s}", index, bike_station_list[index].id, bike_station_list[index].name, bike_station_list[index].info);
    return &bike_station_list[index];
}

void bike_station_list_append(uint8_t id, char* name, char* info)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bike_station_list_append(%i, %s, %s)", id, name, info);

    if (bike_station_list_size >= MAX_STATIONS) { 
        return;
    }

    // Copy data in
    bike_station_list[bike_station_list_size].id = id;
    strcpy(bike_station_list[bike_station_list_size].name, name);
    strcpy(bike_station_list[bike_station_list_size].info, info);

    // Increment list lize
    bike_station_list_size++;
}


void requestNotification(int id)
{
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "requestNotification(%i)", id);

	DictionaryIterator *iterator;
	int begin = app_message_outbox_begin(&iterator);
	int key1 = dict_write_uint8(iterator, KEY_CMD, CMD_FETCH_BIKES);
	int key2 = dict_write_uint8(iterator, KEY_BIKES_ID, id);
	
	int sent = app_message_outbox_send();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "send begin=%i key1=%i key2=%i sent=%i", begin, key1, key2, sent);

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

	ending = true;
}

void requestAdditionalEntries()
{
	if (ending) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "requestAdditionalEntries() ending => true");
		return;
	}

	requestNotification(bike_station_list_size);
}

uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
	return 1;
}

uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	return bike_station_list_size;
}

int16_t menu_get_row_height_callback(MenuLayer *me,  MenuIndex *cell_index, void *data) {
	return 40;
}

void menu_pos_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context)
{
	// Nothing
}


void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	
    BikeStation* item;
    const int index = cell_index->row;

    if ((item = bike_station_list_get(index)) == NULL) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "draw_row_callback() item == NULL");
        return;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "draw_row_callback() item => BikeStation{id = %i, name = %s, info = %s}", bike_station_list[index].id, bike_station_list[index].name, bike_station_list[index].info);
    menu_cell_basic_draw(ctx, cell_layer, item->name, item->info, NULL);

	// graphics_context_set_text_color(ctx, GColorBlack);

	// graphics_draw_text(ctx, item->name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(34, 0, 144 - 35, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	// graphics_draw_text(ctx, item->info, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(34, 20, 144 - 35, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

}


void menu_select_callback(MenuLayer *me, MenuIndex *cell_index, void *data) {
	//sendpickedEntry(cell_index->row, 0);
}

void receivedEntries(DictionaryIterator* data)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler()");
    Tuple *name = dict_find(data, KEY_BIKES_NAME);
    Tuple *info = dict_find(data, KEY_BIKES_INFO);
    Tuple *id = dict_find(data, KEY_BIKES_ID);

    if (name && info && id) {
        bike_station_list_append(id->value->uint8, name->value->cstring, info->value->cstring);
    }

	menu_layer_reload_data(listMenuLayer);
	ending = false;
	requestAdditionalEntries();
}

void list_data_received(int cmd, DictionaryIterator* data)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "list_data_received(%i)", cmd);

	switch (cmd)
	{
		case CMD_FETCH_BIKES:
			receivedEntries(data);
			break;
	}
}
////from here.

void list_window_load(Window *me) {
	setCurWindow(WINDOW_BIKE_LIST);
}

void list_window_unload(Window *me) {

}

void init_notification_list_window()
{
	listWindow = window_create();
	Layer* topLayer = window_get_root_layer(listWindow);

	listMenuLayer = menu_layer_create(GRect(0, 0, 144, 168 - 16));

	// Set all the callbacks for the menu layer
	menu_layer_set_callbacks(listMenuLayer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_cell_height = menu_get_row_height_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.selection_changed = menu_pos_changed
	});

	menu_layer_set_click_config_onto_window(listMenuLayer, listWindow);

	layer_add_child(topLayer, (Layer*) listMenuLayer);

	window_set_window_handlers(listWindow, (WindowHandlers){
		.appear = list_window_load,
		.unload = list_window_unload

	});

	window_stack_push(listWindow, true /* Animated */);
	requestAdditionalEntries();
}
