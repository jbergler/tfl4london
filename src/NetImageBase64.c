#include "NetImageBase64.h"

NetImageContext* netimage_create_context(NetImageCallback callback) {
    NetImageContext *ctx = malloc(sizeof(NetImageContext));

    ctx->length = 0;
    ctx->index = 0;
    ctx->data = NULL;
    ctx->base64 = NULL;
    ctx->callback = callback;

    return ctx;
}

char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                        'w', 'x', 'y', 'z', '0', '1', '2', '3',
                        '4', '5', '6', '7', '8', '9', '+', '/'};
char decoding_table[256];
int mod_table[] = {0, 2, 1};

void netimage_destroy_context(NetImageContext *ctx) {
    if (ctx->data) free(ctx->data);
    if (ctx->base64) free(ctx->base64);
    free(ctx);
}

void netimage_initialize(NetImageCallback callback) {
    NetImageContext *ctx = netimage_create_context(callback);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NetImageContext = %p", ctx);
    app_message_set_context(ctx);

    app_message_register_inbox_received(netimage_receive);
    app_message_register_inbox_dropped(netimage_dropped);
    app_message_register_outbox_sent(netimage_out_success);
    app_message_register_outbox_failed(netimage_out_failed);

    // The formula to calculate the size of a Dictionary in bytes is:
    // 1 + (n * 7) + D1 + ... + Dn
    uint32_t overhead = 1 + (2*7); // Support to 2 tuples in msg. CMD & DATA

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Max buffer sizes are %li / %li", app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Our buffer sizes are %li / %li", NETIMAGE_BUFFER_SIZE_IN - overhead, NETIMAGE_BUFFER_SIZE_OUT - overhead);
    //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum()); // don't need this, it will be open elsewhere in the app.
}

void netimage_deinitialize() {
    netimage_destroy_context(app_message_get_context());
    app_message_set_context(NULL);
}

void netimage_request(char *url) {
    DictionaryIterator *outbox;
    app_message_outbox_begin(&outbox);

    // Tell the javascript how big we want each chunk of data - 8 is the dictionary overhead
    //NETIMAGE_BUFFER_SIZE_IN - (1 + (2*7))
    dict_write_uint8(outbox, KEY_NETIMAGE_CMD, CMD_NETIMAGE_BEGIN);
    uint16_t size = 512;
    dict_write_uint16(outbox, KEY_NETIMAGE_CHUNK_SIZE, size);
    
    // Send the URL
    dict_write_cstring(outbox, KEY_NETIMAGE_URL, url);

    int error = app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_ERROR, "SENT NETIMAGE_BEGIN = %i", error);
}

void netimage_receive(DictionaryIterator *iter, void *context) {
    NetImageContext *ctx = (NetImageContext*) context;

    Tuple *cmd = dict_find(iter, KEY_NETIMAGE_CMD);

    if (!cmd) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Got a message with no cmd key!");
        return;
    }

    Tuple *tupleData;
    Tuple *tupleSize;

    switch (cmd->value->uint32) {
        case CMD_NETIMAGE_DATA:
            tupleData = dict_find(iter, KEY_NETIMAGE_DATA);
            if (ctx->index + tupleData->length - 1 <= ctx->length) {
                memcpy(ctx->base64 + ctx->index, tupleData->value->cstring, tupleData->length - 1);
                ctx->index += tupleData->length - 1 ;
            }
            else {
                APP_LOG(APP_LOG_LEVEL_WARNING, "Not overriding rx buffer. Bufsize=%li BufIndex=%li DataLen=%i", ctx->length, ctx->index, tupleData->length);
            }
            break;

        case CMD_NETIMAGE_BEGIN:
            tupleSize = dict_find(iter, KEY_NETIMAGE_SIZE);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Start transmission. Size=%lu", tupleSize->value->uint32);

            if (ctx->data != NULL) free(ctx->data);
            if (ctx->base64 != NULL) free(ctx->base64);

            ctx->data = malloc(tupleSize->value->uint32);
            ctx->base64 = malloc(tupleSize->value->uint32);

            if (ctx->data != NULL && ctx->base64 != NULL) {
                ctx->length = tupleSize->value->uint32;
                ctx->index = 0;
                app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
            }
            else {
                APP_LOG(APP_LOG_LEVEL_WARNING, "Unable to allocate memory to receive image.");
                ctx->length = 0;
                ctx->index = 0;
            }
            break;

        case CMD_NETIMAGE_END:
            APP_LOG(APP_LOG_LEVEL_DEBUG, "End transmission. Size=%lu Received=%lu", ctx->length, ctx->index);

            if (ctx->base64 && ctx->data && ctx->length > 0 && ctx->index > 0) {
                app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
                GBitmap *bitmap = NULL;

                for (int i = 0; i < 64; i++) {
                    decoding_table[(unsigned char) encoding_table[i]] = i;
                }

                if (ctx->index % 4 == 0) {

                    uint32_t output_length = ctx->index / 4 * 3;
                    if (ctx->base64[ctx->index - 1] == '=') (output_length)--;
                    if (ctx->base64[ctx->index - 2] == '=') (output_length)--;


                    for (uint i = 0, j = 0; i < ctx->index;) {

                        uint32_t sextet_a = ctx->base64[i] == '=' ? 0 & i++ : decoding_table[(uint8_t) ctx->base64[i++]];
                        uint32_t sextet_b = ctx->base64[i] == '=' ? 0 & i++ : decoding_table[(uint8_t) ctx->base64[i++]];
                        uint32_t sextet_c = ctx->base64[i] == '=' ? 0 & i++ : decoding_table[(uint8_t) ctx->base64[i++]];
                        uint32_t sextet_d = ctx->base64[i] == '=' ? 0 & i++ : decoding_table[(uint8_t) ctx->base64[i++]];

                        uint32_t triple = (sextet_a << 3 * 6)
                                        + (sextet_b << 2 * 6)
                                        + (sextet_c << 1 * 6)
                                        + (sextet_d << 0 * 6);

                        if (j < output_length) ctx->data[j++] = (triple >> 2 * 8) & 0xFF;
                        if (j < output_length) ctx->data[j++] = (triple >> 1 * 8) & 0xFF;
                        if (j < output_length) ctx->data[j++] = (triple >> 0 * 8) & 0xFF;
                    }

                    bitmap = gbitmap_create_with_data(ctx->data);
                }


                if (bitmap) {
                    ctx->callback(bitmap);
                    // We have transfered ownership of this memory to the app. Make sure we dont free it.
                    ctx->data = NULL;
                    ctx->index = ctx->length = 0;
                }
                else {
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "Unable to create GBitmap. Is this a valid PBI?");
                    // free memory
                    free(ctx->data);
                    free(ctx->base64);
                    ctx->data = NULL;
                    ctx->base64 = NULL;
                    ctx->index = ctx->length = 0;
                }
            }
            else {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "Got End message but we have no image...");
            }
            break;

        default:
            APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown key in dict: %lu", cmd->value->uint32);
            break;
    }
}

void netimage_dropped(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Dropped message! Reason given: %i", reason);
}

void netimage_out_success(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sent.");
}

void netimage_out_failed(DictionaryIterator *iter, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send message. Reason = %i", reason);
}
