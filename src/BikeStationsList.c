#include <pebble.h>
#include <pebble_fonts.h>
#include "Watchapp.h"
#include "WatchappTypes.h"
#include "LoadingWindow.h"
#include "ImageWindow.h"

Window* listWindow;
MenuLayer* listMenuLayer;

bool ending = false;

#define MAX_STATIONS (5)
#define MAX_LENGTH (64)

typedef struct {
    int id;
    char name[MAX_LENGTH];
    char info[MAX_LENGTH];
    char url[MAX_LENGTH];
} BikeStation;

BikeStation bike_station_list[MAX_STATIONS];
int bike_station_list_size = 0;

BikeStation* bike_station_list_get(int index)
{
    if (index < 0 || index >= MAX_STATIONS) {
        APP_LOG(APP_LOG_LEVEL_INFO, "bike_station_list_get(%i) return NULL (OutOfBounds)", index);
        return NULL;
    }

    return &bike_station_list[index];
}

void bike_station_list_append(uint8_t id, char* name, char* info, char* url)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bike_station_list_append(%i, %s, %s)", id, name, info);

    if (bike_station_list_size >= MAX_STATIONS) { 
        return;
    }

    // Copy data in
    bike_station_list[bike_station_list_size].id = id;
    strcpy(bike_station_list[bike_station_list_size].name, name);
    strcpy(bike_station_list[bike_station_list_size].info, info);
    strcpy(bike_station_list[bike_station_list_size].url, url);

    // Increment list lize
    bike_station_list_size++;
}


void fetchBikeInfo()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "fetchBikeInfo(CMD => FETCH_BIKES)");

	DictionaryIterator *iterator;
    app_message_outbox_begin(&iterator);
    dict_write_uint8(iterator, KEY_CMD, CMD_FETCH_BIKES);
    //dict_write_uint8(iterator, KEY_CMD, CMD_FETCH_IMAGE);
    app_message_outbox_send();

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

	ending = false;
}

uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
	return 1;
}

uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
	return bike_station_list_size;
}

int16_t menu_get_row_height_callback(MenuLayer *me,  MenuIndex *cell_index, void *data) {
	return 44;
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

    menu_cell_basic_draw(ctx, cell_layer, item->name, item->info, NULL);

	// graphics_context_set_text_color(ctx, GColorBlack);

	// graphics_draw_text(ctx, item->name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(34, 0, 144 - 35, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	// graphics_draw_text(ctx, item->info, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(34, 20, 144 - 35, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

}

void menu_select_callback(MenuLayer *me, MenuIndex *cell_index, void *data) {
    BikeStation *station = bike_station_list_get(cell_index->row); 
    imagewindow_init(station->url);
}

void receivedEntries(DictionaryIterator* data)
{
	Tuple *idP = dict_find(data, KEY_BIKES_ID);
	int id = -1;

	if (idP && (id = idP->value->uint8) == 255) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "receivedEntries() id=>255 ... we must be done");
        ending = true;
        menu_layer_reload_data(listMenuLayer);
        hideLoadingWindow((Layer *) listMenuLayer);
	}
	else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "receivedEntries() id=>%i ... processing entry", id);
		Tuple *name = dict_find(data, KEY_BIKES_NAME);
		Tuple *info = dict_find(data, KEY_BIKES_INFO);
        Tuple *url = dict_find(data, KEY_BIKES_URL);

	    if (name && info && id >= 0) {
	        bike_station_list_append(id, name->value->cstring, info->value->cstring, url->value->cstring);
	    }
	}
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



void list_window_load(Window *me) {
	setCurWindow(WINDOW_BIKE_LIST);
}

void list_window_unload(Window *me) {

}

void init_notification_list_window()
{
	listWindow = window_create();
	Layer* topLayer = window_get_root_layer(listWindow);

	// Build menu
	listMenuLayer = menu_layer_create(GRect(0, 0, 144, 168 - 16));

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

    initLoadingWindow(topLayer);
	showLoadingWindow((Layer*) listMenuLayer);

	window_set_window_handlers(listWindow, (WindowHandlers){
		.appear = list_window_load,
		.unload = list_window_unload

	});

	window_stack_push(listWindow, true /* Animated */);
	fetchBikeInfo();
}
