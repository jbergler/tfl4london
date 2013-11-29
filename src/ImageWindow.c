#include <pebble.h>
#include "NetImageBase64.h"
#include "LoadingWindow.h"
#include "ImageWindow.h"

static Window *window;
static BitmapLayer *bitmap_layer;
static GBitmap *current_image;

static char *image;

static void window_load(Window *window) {

}

static void window_unload(Window *window) {
    bitmap_layer_destroy(bitmap_layer);

    if (current_image) {
        free(current_image);  // Cleanup image
        current_image = NULL; // Don't use it again
    }
    imagewindow_deinit();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unloading window");
}

void display_image(GBitmap *image) {

    bitmap_layer_set_bitmap(bitmap_layer, image);
    if (current_image) free(current_image); // Cleanup previous image
    current_image = image;

    hideLoadingWindow((Layer *) bitmap_layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "display_image callback with *image => %p", image);
}

void imagewindow_init(char *url) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "imagewindow_init called with *url => %s", url);

    image = url;

    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);


    bitmap_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_background_color(bitmap_layer, GColorBlack);
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
    current_image = NULL;

    initLoadingWindow(window_layer);
    showLoadingWindow((Layer*) bitmap_layer);

    netimage_initialize(display_image);
    netimage_request(image);
}

void imagewindow_deinit() {
    netimage_deinitialize(); // call this to avoid 20B memory leak
    window_destroy(window);
}
