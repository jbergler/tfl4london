#include "pebble.h"

static Window *window_bikes;
static MenuLayer *menu_layer;

enum {
    CMD_FETCH      = 1000,
    BIKES_KEY_NAME = 1001,
    BIKES_KEY_INFO = 1002,
    BIKES_KEY_ID   = 1003
};

#define MAX_STATIONS (5)
#define MAX_TEXT_LENGTH (64)

typedef struct {
    int id;
    char name[MAX_TEXT_LENGTH];
    char info[MAX_TEXT_LENGTH];
} BikeStation;

static BikeStation bike_station_list[MAX_STATIONS];
static int bike_station_list_size = 0;

static BikeStation* bike_station_list_get(int index) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    if (index < 0 || index >= MAX_STATIONS) {
        return NULL;
    }
    return &bike_station_list[index];
}

static void bike_station_list_append(uint8_t id, char* name, char* info) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    if (bike_station_list_size == MAX_STATIONS) { 
        return;
    }

    // Copy data in
    bike_station_list[bike_station_list_size].id = id;
    strcpy(bike_station_list[bike_station_list_size].name, name);
    strcpy(bike_station_list[bike_station_list_size].info, info);

    // Increment list lize
    bike_station_list_size++;
}

static void bike_station_list_init(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    DictionaryIterator *iter;

    if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
        return;
    }
    if (dict_write_uint8(iter, CMD_FETCH, 0) != DICT_OK) {
        return;
    }
    app_message_outbox_send();
}


// UI Code

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    return 44;
}

static void draw_row_callback(GContext* ctx, Layer *cell_layer, MenuIndex *cell_index, void *data) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    BikeStation* item;
    const int index = cell_index->row;

    if ((item = bike_station_list_get(index)) == NULL) {
        return;
    }

    menu_cell_basic_draw(ctx, cell_layer, item->name, item->info, NULL);
}

static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *data) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    return bike_station_list_size;
}

static void window_bikes_load(Window* window) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    Layer *window_layer = window_get_root_layer(window);
    GRect window_frame = layer_get_frame(window_layer);
    menu_layer = menu_layer_create(window_frame);
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
        .get_cell_height = (MenuLayerGetCellHeightCallback) get_cell_height_callback,
        .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) get_num_rows_callback,
        .select_click = NULL,
        .select_long_click = NULL
    });
    menu_layer_set_click_config_onto_window(menu_layer, window);
    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_bikes_unload(Window *window) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    // delete objects
    window_destroy(window_bikes);
    menu_layer_destroy(menu_layer);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    Tuple *name = dict_find(iter, BIKES_KEY_NAME);
    Tuple *info = dict_find(iter, BIKES_KEY_INFO);
    Tuple *id = dict_find(iter, BIKES_KEY_ID);

    if (name && info && id) {

        bike_station_list_append(id->value->uint8, name->value->cstring, info->value->cstring);
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "got to before menu reload");
    menu_layer_reload_data(menu_layer);
}

static void app_message_init(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    app_message_open(256, 32);
    app_message_register_inbox_received(in_received_handler);
}


static void window_bikes_init() {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    window_bikes = window_create();

    app_message_init();
    bike_station_list_init();
    
    // Setup the window handlers
    window_set_window_handlers(window_bikes, (WindowHandlers) {
        .load = window_bikes_load,
        .unload = window_bikes_unload,
    });
}

static void window_bikes_show() {
    APP_LOG(APP_LOG_LEVEL_INFO, "got here");

    window_stack_push(window_bikes, true);
}