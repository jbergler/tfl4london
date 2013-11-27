#include <pebble.h>
#include <pebble_fonts.h>
#include "Watchapp.h"
#include "WatchappTypes.h"
#include "BikeStationsList.h"

Window* menuWindow;

SimpleMenuItem mainMenuItems[1] = {};
SimpleMenuSection mainMenuSection[1] = {};

GBitmap* bikesIcon;

TextLayer* menuLoadingLayer;

SimpleMenuLayer* menuLayer;

void show_loading() {
	layer_set_hidden((Layer *) menuLoadingLayer, false);
	if (menuLayer != NULL) layer_set_hidden((Layer *) menuLayer, true);
}

void menu_picked(int index, void* context) {
	show_loading();

	switch(index) {
		case 0:
			window_stack_pop(true);
			init_notification_list_window();
			break;
	}
}

void show_menu()
{
	mainMenuSection[0].items = mainMenuItems;
	mainMenuSection[0].num_items = 1;

	mainMenuItems[0].title = "Barclays Bikes";
	mainMenuItems[0].icon = bikesIcon;
	mainMenuItems[0].callback = menu_picked;

	Layer* topLayer = window_get_root_layer(menuWindow);

	if (menuLayer != NULL) layer_remove_from_parent((Layer *) menuLayer);
	menuLayer = simple_menu_layer_create(GRect(0, 0, 144, 156), menuWindow, mainMenuSection, 1, NULL);
	layer_add_child(topLayer, (Layer *) menuLayer);

	layer_set_hidden((Layer *) menuLoadingLayer, true);
	layer_set_hidden((Layer *) menuLayer, false);
}

void menu_data_received(int cmd, DictionaryIterator* data)
{
	// not sure what i want here?
	return;
}

void window_unload(Window* me)
{
	gbitmap_destroy(bikesIcon);
	window_destroy(me);
}

void window_load(Window *me) {
	setCurWindow(WINDOW_MAIN_MENU);
}

void init_menu_window()
{
	bikesIcon = gbitmap_create_with_resource(RESOURCE_ID_MENU_ICON);

	menuWindow = window_create();

	Layer* topLayer = window_get_root_layer(menuWindow);

	menuLoadingLayer = text_layer_create(GRect(0, 10, 144, 168 - 16));
	text_layer_set_text_alignment(menuLoadingLayer, GTextAlignmentCenter);
	text_layer_set_text(menuLoadingLayer, "Loading...");
	//text_layer_set_font(menuLoadingLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(topLayer, (Layer*) menuLoadingLayer);

	window_set_window_handlers(menuWindow, (WindowHandlers){
		.appear = window_load,
		.unload = window_unload
	});

	window_stack_push(menuWindow, true /* Animated */);

	show_loading();
	show_menu();
}
