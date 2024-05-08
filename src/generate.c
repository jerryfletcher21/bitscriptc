#include "generate.h"

#include "wally_core.h"
#include "wally_bip39.h"

#include "util.h"

int
generate_bip39(const int words_length)
{
    byte bytes[BIP39_ENTROPY_LEN_256];
    size_t bytes_len;
    char *mnemonic;
    int status;
    /* int i; */

    if (words_length == BIP39_24_WORDS) {
        bytes_len = BIP39_ENTROPY_LEN_256;
    } else if (words_length == BIP39_12_WORDS) {
        bytes_len = BIP39_ENTROPY_LEN_128;
    } else {
        fprintf(stderr, "words length not supported\n");
        return FALSE;
    }

    if (!fill_random(bytes, bytes_len)) {
        fprintf(stderr, "error: generating random bytes\n");
        return FALSE;
    }
/*
    for (i = 0; i < 8; i++) {
        bytes[256 + i] = bytes[i];
    }
*/

    if ((status = bip39_mnemonic_from_bytes(NULL, bytes, bytes_len, &mnemonic)) != WALLY_OK) {
        fprintf(stderr, "error generating mnemonic\n");
        fprint_error_status(stderr, status);

        return FALSE;
    }

    printf("%s\n", mnemonic);

    wally_free_string(mnemonic);

    return TRUE;
}
