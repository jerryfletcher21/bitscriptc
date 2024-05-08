#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#define TRUE                            1
#define FALSE                           0

#define STRING_NOT_EXPAND(x)            #x
#define STRING_EXPAND(x)                STRING_NOT_EXPAND(x)

#define NELEMS(x)                       (sizeof(x) / sizeof((x)[0]))

#define WORD_MAX_LENGTH                 (8 + 1)
#define MNEMONIC_MAX_LENGTH             (WORD_MAX_LENGTH * 24)

#define WALLET_GENERATE_MNEMONIC        1
#define WALLET_EXTENDED_CHECK           2
#define WALLET_XPRV_FROM_MNEMONIC       3
#define WALLET_XPUB_FROM_XPRV           4
#define WALLET_SIGN_PRIVATE_KEY         5
#define WALLET_SIGN_EXTENDED_KEY        6
#define WALLET_DESCRIPTOR               7

#define EXTENDED_KEY_STRING_LENGTH      111
#define PATH_STRING_MAX_LENGTH          256

#define MESSAGE_MAX_LENGTH              65536

/* less than 10 second on my machine */
#define MAX_ADDRESSES_CHECK             65536

#define WALLY_RUN_FUNCTION(function)                    \
    do {                                                \
        int status;                                     \
        if ((status = function) != WALLY_OK) {          \
            fprintf(stderr, #function "\n");            \
            fprint_error_status(stderr, status);        \
            return FALSE;                               \
        }                                               \
    } while (0)

typedef unsigned char byte;

struct FILE;

int
fprint_error_status(FILE *stream, const int status);

void
print_hex_main(const byte *const data, const size_t size);

int
string_equal(const char *const first, const char *const second);

int
fill_random(byte *const data, const size_t size);

int
read_string_from_stdin_terminal(
    char *const string, const size_t string_length
);

#endif /* UTIL_H */
