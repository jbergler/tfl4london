#include <pebble.h>
#include <pebble_fonts.h>

/**
 * Code to init and manage a loading window
 */
TextLayer* loadingLayer;

void showLoadingWindow(Layer* otherLayer) {
    if (loadingLayer == NULL) 
        return;

    layer_set_hidden((Layer *) loadingLayer, false);
    layer_set_hidden((Layer *) otherLayer, true);
}

void hideLoadingWindow(Layer* otherLayer) {
    if (loadingLayer == NULL)
        return;

    layer_set_hidden((Layer *) loadingLayer, true);
    layer_set_hidden((Layer *) otherLayer, false);
}

void setLoadingWindowStatus(char* status) {
    //TODO: write this
}

void initLoadingWindow(Layer* topLayer) {
    if (loadingLayer == NULL) {
        loadingLayer = text_layer_create(GRect(0, 10, 144, 168 - 16));
        text_layer_set_text_alignment(loadingLayer, GTextAlignmentCenter);
        text_layer_set_text(loadingLayer, "Loading...");
        //text_layer_set_font(loadingLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    }
    layer_add_child(topLayer, (Layer*) loadingLayer);
    layer_set_hidden((Layer *) loadingLayer, true);
}