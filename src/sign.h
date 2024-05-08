#ifndef SIGN_H
#define SIGN_H

#include "util.h"

struct ext_key;

int
sign_message_from_bytes(
    const byte *const private_key, const size_t private_key_length,
    const char *const message_string
);

int
sign_message(
    const char *const private_key_string,
    const char *const message_string
);

int
extended_key_find_from_xprv_path_address(
    struct ext_key *const key_child,
    const char *const xprv_string,
    const char *const path_string,
    const char *const address_string
);

int
sign_message_xprv_address(
    const char *const address_string,
    const char *const message
);

#endif /* SIGN_H */
