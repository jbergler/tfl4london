#include "pebble.h"
#include "menu-bikes.c"

#define MENU_SECTIONS 1
#define MENU_SECTION_MAIN 0

#define MENU_SECTION_MAIN_ITEMS 1
#define MENU_SECTION_MAIN_ITEM_BIKES 0

#define ICONS 1
#define ICON_MENU 0

static Window *window;

static SimpleMenuLayer *simple_menu_layer;
static SimpleMenuSection menu_sections[MENU_SECTIONS];

static SimpleMenuItem menu_main_items[MENU_SECTION_MAIN_ITEMS];

static GBitmap *icons[ICONS];


void menu_select_callback(int index, void *ctx) {
	switch (index) {
		case MENU_SECTION_MAIN_ITEM_BIKES:
			window_bikes_show();
			return;
	}
}

void window_load(Window *window) {
	icons[ICON_MENU] = gbitmap_create_with_resource(RESOURCE_ID_MENU_ICON);

	menu_main_items[MENU_SECTION_MAIN_ITEM_BIKES] = (SimpleMenuItem){
		.title = "Barclays Bikes",
		.subtitle = "Where's the nearest bike?",
		.callback = menu_select_callback,
		.icon = icons[ICON_MENU], //TODO: fix icon
	};

	// Bind the menu items to the corresponding menu sections
	menu_sections[MENU_SECTION_MAIN] = (SimpleMenuSection){
    	.num_items = MENU_SECTION_MAIN_ITEMS,
    	.items = menu_main_items,
  	};

	// Initialize window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Initialize menu layer
	simple_menu_layer = simple_menu_layer_create(bounds, window, menu_sections, MENU_SECTIONS, NULL);

	// Add it to the window for display
	layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));
}

// Deinitialize resources on window unload that were initialized on window load
void window_unload(Window *window) {
	simple_menu_layer_destroy(simple_menu_layer);

	// Cleanup the menu icon
	for (int i = 0; i < ICONS; i++) {
    	gbitmap_destroy(icons[i]);
    }
}

int main(void) {
	window = window_create();

	// Setup the window handlers
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	window_stack_push(window, true /* Animated */);

	window_bikes_init();

	app_event_loop();

	window_destroy(window);
}