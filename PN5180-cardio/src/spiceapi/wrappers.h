#ifndef SPICEAPI_WRAPPERS_H
#define SPICEAPI_WRAPPERS_H

#include <Arduino.h>
#include "connection.h"

// default buffer sizes
#ifndef SPICEAPI_WRAPPER_BUFFER_SIZE_STR
#define SPICEAPI_WRAPPER_BUFFER_SIZE_STR 256
#endif

namespace spiceapi {
    
    // static storage
    char JSON_BUFFER_STR[SPICEAPI_WRAPPER_BUFFER_SIZE_STR];
    
     /*
      * Helpers
      */
    
    uint64_t msg_gen_id() {
        static uint64_t id_global = 0;
        return ++id_global;
    }

    char *request_gen_simple(const char *module, const char *function, const char *param1, const char *param2) {
        sprintf(JSON_BUFFER_STR, "{\"id\":%ld,\"module\":\"%s\",\"function\":\"%s\",\"params\":[%s,\"%s\"]}", (long) msg_gen_id(), module, function, param1, param2);
        return JSON_BUFFER_STR;
    }
    
    /*
     * Wrappers
     */

    bool card_insert(Connection &con, size_t index, const char *card_id) {
        char indexstr[2];
        sprintf(indexstr, "%d", index);

        auto req = request_gen_simple("card", "insert", indexstr, card_id);
        con.request(req);
        return true;
    }

    bool keypads_write(Connection &con, unsigned int keypad, const char *input) {
        char indexstr[2];
        sprintf(indexstr, "%d", keypad);

        auto req = request_gen_simple("keypads", "write", indexstr, input);
        con.request(req);
        return true;
    }
}

#endif //SPICEAPI_WRAPPERS_H
