#include <pebble.h>

#ifndef NETIMAGEB64_H_
#define NETIMAGEB64_H_

#define NETIMAGE_BUFFER_SIZE_IN 768
#define NETIMAGE_BUFFER_SIZE_OUT 128

enum NETIMAGE_KEY {
    KEY_NETIMAGE_CMD        = 51,    // Send value as int
    KEY_NETIMAGE_URL        = 52,    // Send value as string requesting data
    KEY_NETIMAGE_DATA       = 53,    // Data
    KEY_NETIMAGE_SIZE       = 54,    // Data
    KEY_NETIMAGE_CHUNK_SIZE = 55     // Send value as bytes to send at a time
};

enum NETIMAGE_CMD {
    CMD_NETIMAGE_BEGIN  = 1,
    CMD_NETIMAGE_DATA   = 2,
    CMD_NETIMAGE_END    = 3
};

typedef void (*NetImageCallback)(GBitmap *image);

typedef struct {
  uint32_t length;  // Size of buffer
  uint32_t index;   // Current position of transfer
  char *base64;     // Store Base64 incoming data
  uint8_t *data;    // Location for binary data

  /* Callback to call when we are done loading the image */
  NetImageCallback callback;
} NetImageContext;

NetImageContext* netimage_create_context(NetImageCallback callback);

void netimage_initialize();
void netimage_deinitialize();

void netimage_request(char *url);

void netimage_receive(DictionaryIterator *iter, void *context);
void netimage_dropped(AppMessageResult reason, void *context);
void netimage_out_success(DictionaryIterator *iter, void *context);
void netimage_out_failed(DictionaryIterator *iter, AppMessageResult reason, void *context);

#endif /* NETIMAGEB64_H_ */
