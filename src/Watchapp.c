#include <pebble.h>
#include <pebble_fonts.h>
#include "WatchappTypes.h"
#include "MainMenu.h"
#include "BikeStationsList.h"


uint8_t curWindow = 0;
bool gotNotification = false;

uint8_t getCurWindow() {
	return curWindow;
}

void setCurWindow(uint8_t newWindow) {
	curWindow = newWindow;
}

void switchWindow(uint8_t newWindow)
{
	switch(newWindow) {
		case WINDOW_MAIN_MENU:
			curWindow = WINDOW_MAIN_MENU;
			init_menu_window();
			break;

		case WINDOW_BIKE_LIST:
			curWindow = WINDOW_BIKE_LIST;
			init_notification_list_window();
		 	break;
	}
}

void received_data(DictionaryIterator *received, void *context) {
	gotNotification = true;

	uint8_t cmd = dict_find(received, KEY_CMD)->value->uint8;

	APP_LOG(APP_LOG_LEVEL_INFO, "AppMsgReceived CMD: %d WINDOW: %d", cmd, curWindow);

	switch (curWindow) {
		case WINDOW_MAIN_MENU:
			menu_data_received(cmd, received);
			break;
		case WINDOW_BIKE_LIST:
			list_data_received(cmd, received);
			break;
	}

	// app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	// app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
}

void data_sent(DictionaryIterator *received, void *context)
{
	switch (curWindow) {
		// case 1:
		// 	notification_data_sent(received, context);
		// 	break;
	}
}

void timerTriggered(void* context)
{
	if (!gotNotification) {
		DictionaryIterator *iterator;
		app_message_outbox_begin(&iterator);
		dict_write_uint8(iterator, KEY_CMD, CMD_READY);
		app_message_outbox_send();

		app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
		app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

		app_timer_register(500, timerTriggered, NULL);
	}
}

int main(void) {
	app_message_register_inbox_received(received_data);
	app_message_register_outbox_sent(data_sent);
	app_message_open(768, 512);

	switchWindow(WINDOW_MAIN_MENU);

	app_timer_register(300, timerTriggered, NULL);

	app_event_loop();
	return 0;
}