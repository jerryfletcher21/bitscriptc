#include "sign.h"

#include <string.h>

#include "wally_core.h"
#include "wally_crypto.h"
#include "wally_address.h"
#include "wally_bip32.h"

#define MESSAGE_STDIN_STRING \
    "insert message, press Enter to go to newline and C-d to finish it"

int
sign_message_from_bytes(
    const byte *const private_key, const size_t private_key_length,
    const char *const message_string
)
{
    /* byte message[BITCOIN_MESSAGE_MAX_LEN]; */
    /* size_t message_length; */

    byte message_hash[EC_MESSAGE_HASH_LEN];
    /* byte message_hash[SHA256_LEN]; */
    size_t message_hash_length;

    /* byte signature[EC_SIGNATURE_LEN]; */
    byte signature[EC_SIGNATURE_RECOVERABLE_LEN];

    char *signature_string;

    /* this produces the same one as electrum */
    uint32_t signature_flags = EC_FLAG_ECDSA | EC_FLAG_GRIND_R | EC_FLAG_RECOVERABLE;
    /* uint32_t signature_flags = EC_FLAG_ECDSA | EC_FLAG_RECOVERABLE; */

    size_t message_string_length = strlen(message_string);

    if (private_key_length != EC_PRIVATE_KEY_LEN) {
        fprintf(
            stderr,
            "error: private_key_length should be %d\n",
            EC_PRIVATE_KEY_LEN
        );
        return FALSE;
    }

    if (message_string_length > BITCOIN_MESSAGE_MAX_LEN) {
        fprintf(
            stderr,
            "error: message is too long, maximum message length: %d, current message length: %lu\n",
            BITCOIN_MESSAGE_MAX_LEN, message_string_length
        );
        return FALSE;
    }
    WALLY_RUN_FUNCTION(wally_format_bitcoin_message(
        (byte *)message_string, message_string_length,
        BITCOIN_MESSAGE_FLAG_HASH,
        message_hash, NELEMS(message_hash),
        &message_hash_length
    ));
    if (message_hash_length != NELEMS(message_hash)) {
        fprintf(stderr, "error: message hash is not the correct size\n");
        return FALSE;
    }

    WALLY_RUN_FUNCTION(wally_ec_sig_from_bytes(
        private_key, private_key_length,
        message_hash, message_hash_length,
        signature_flags,
        signature, NELEMS(signature)
    ));

    WALLY_RUN_FUNCTION(wally_base64_from_bytes(
        signature, NELEMS(signature),
        0,
        &signature_string
    ));

    printf("%s\n", signature_string);

    wally_free_string(signature_string);

    return TRUE;
}

int
sign_message(
    const char *const private_key_string,
    const char *const message_string
)
{
    byte private_key[EC_PRIVATE_KEY_LEN];

    char message_stdin[BITCOIN_MESSAGE_MAX_LEN];
    const char *message_pointer;

    size_t private_key_string_length = strlen(private_key_string);

    if (private_key_string_length == 64) {
        size_t private_key_length = NELEMS(private_key);
        WALLY_RUN_FUNCTION(wally_hex_to_bytes(
            private_key_string,
            private_key, NELEMS(private_key), &private_key_length
        ));
        if (private_key_length != NELEMS(private_key)) {
            fprintf(stderr, "error: private key is not the correct size\n");
            return FALSE;
        }
    } else {
        /* not used */
        WALLY_RUN_FUNCTION(wally_wif_to_bytes(
            private_key_string,
            WALLY_ADDRESS_VERSION_WIF_MAINNET,
            WALLY_WIF_FLAG_COMPRESSED,
            private_key, NELEMS(private_key)
        ));
    }

    if (message_string) {
        message_pointer = message_string;
    } else {
        printf(MESSAGE_STDIN_STRING "\n");
        if (!read_string_from_stdin_terminal(
            message_stdin, NELEMS(message_stdin)
        )) {
            fprintf(stderr, "error: reading message from stdin\n");
            return FALSE;
        }
        message_pointer = message_stdin;
    }

    if (!sign_message_from_bytes(
        private_key, NELEMS(private_key),
        message_pointer
    )) {
        fprintf(stderr, "error: signing message\n");
        return FALSE;
    }

    return TRUE;
}

int
extended_key_find_from_xprv_path_address(
    struct ext_key *const key_child,
    const char *const xprv_string,
    const char *const path_string,
    const char *const address_string
)
{
    struct ext_key key_root;

    char *address_current;

    uint32_t child;

    uint32_t version = BIP32_FLAG_KEY_PRIVATE;

    WALLY_RUN_FUNCTION(bip32_key_from_base58(
        xprv_string, &key_root
    ));

    child = 0;
    while (TRUE) {
        WALLY_RUN_FUNCTION(bip32_key_from_parent_path_str(
            &key_root, path_string, child++,
            version | BIP32_FLAG_STR_WILDCARD,
            key_child
        ));
        WALLY_RUN_FUNCTION(wally_bip32_key_to_addr_segwit(
            key_child,
            "bc", 0,
            &address_current
        ));
        if (string_equal(address_current, address_string)) {
            break;
        }
        WALLY_RUN_FUNCTION(wally_free_string(address_current));

        if (child >= MAX_ADDRESSES_CHECK) {
            fprintf(stderr, "error: maximum number of addresses check, not found\n");
            return FALSE;
        }
    }

    return TRUE;
}

int
sign_message_xprv_address(
    const char *const address_string,
    const char *const message
)
{
    struct ext_key ext_key;

    char xprv[EXTENDED_KEY_STRING_LENGTH + 1];
    char path[PATH_STRING_MAX_LENGTH + 1];

    char message_stdin[BITCOIN_MESSAGE_MAX_LEN + 1];
    const char *message_pointer;

    FILE *file;

    size_t at;
    int c;

    file = stdin;

    for (at = 0; at < NELEMS(xprv) - 1; at++) {
        c = fgetc(file);
        if (c == EOF) {
            fprintf(
                stderr,
                "error: xprv is too short, should be %lu characters\n",
                NELEMS(xprv) - 1
            );
            return FALSE;
        }
        xprv[at] = c;
    }
    xprv[at] = '\0';

    if (fgetc(file) != '/') {
        fprintf(stderr, "error: after xprv there should be a slash (/)\n");
        return FALSE;
    }

    at = 0;
    while ((c = fgetc(file)) != EOF && c != ' ' && c != '\t' && c != '\n') {
        path[at++] = c;
        if (at == NELEMS(path) - 1) {
            fprintf(
                stderr, "\
error: path is too long, probably an error, if you are sure path \n\
is the right lenght, you should increase it in the code\n\
current maximum path allowed: %lu\n\
",
                NELEMS(path)
            );
        }
    }
    path[at] = '\0';

    if (c != EOF) {
        while ((c = fgetc(file)) != EOF);
    }

    if (message) {
        message_pointer = message;
    } else {
        printf(MESSAGE_STDIN_STRING "\n");
        if (!read_string_from_stdin_terminal(
            message_stdin, NELEMS(message_stdin)
        )) {
            fprintf(stderr, "error: reading message from stdin\n");
            return FALSE;
        }
        message_pointer = message_stdin;
    }

    if (!extended_key_find_from_xprv_path_address(
        &ext_key,
        xprv,
        path,
        address_string
    )) {
        fprintf(stderr, "error: getting extended key from xprv path address\n");
        return FALSE;
    }

    return sign_message_from_bytes(
        ext_key.priv_key + 1, NELEMS(ext_key.priv_key) - 1,
        message_pointer
    );
}
