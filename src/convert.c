#include "convert.h"

#include "wally_core.h"
#include "wally_crypto.h"
#include "wally_address.h"
#include "wally_bip32.h"
#include "wally_bip39.h"

#include "util.h"

static int
mnemoic_read(char *const mnemonic, const size_t mnemonic_length)
{
    size_t mnemonic_at;
    int c;
    int reading_word = TRUE;

    FILE *file;

    mnemonic_at = 0;

    file = stdin;
/*
    file = fopen("stf/wordlist", "r");
    if (file == NULL) {
        fprintf(stderr, "error: opening file");
    }
*/
    while ((c = fgetc(file)) != EOF) {
        if (mnemonic_at + 2 >= mnemonic_length) {
            fprintf(stderr, "error: too many words\n");
            return FALSE;
        }
        if (reading_word) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                mnemonic[mnemonic_at++] = c;
            } else {
                reading_word = FALSE;
            }
        } else {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                mnemonic[mnemonic_at++] = ' ';
                mnemonic[mnemonic_at++] = c;
                reading_word = TRUE;
            }
        }
    }
    mnemonic[mnemonic_at] = '\0';

    /* printf("%s\n", mnemonic); */

/*
    if (fclose(file) == FALSE) {
        return FALSE;
    }
*/
    return TRUE;
}

static int
mnemonic_convert(const char *const mnemonic, const int testnet)
{
    byte seed[BIP39_SEED_LEN_512];
    size_t seed_len;

    struct ext_key key;

    char *xprv;

    if (bip39_mnemonic_validate(NULL, mnemonic) != WALLY_OK) {
        fprintf(stderr, "error wrong checksum\n");
        return FALSE;
    }

    if (bip39_mnemonic_to_seed(mnemonic, NULL, seed, NELEMS(seed), &seed_len) != WALLY_OK) {
        fprintf(stderr, "error converting mnemonic to seed\n");
        return FALSE;
    }
    /* print_hex_main(seed, seed_len); */

    if (bip32_key_from_seed(
        seed, seed_len, testnet ? BIP32_VER_TEST_PRIVATE : BIP32_VER_MAIN_PRIVATE, 0, &key
    ) != WALLY_OK) {
        fprintf(stderr, "error getting xprv\n");
        return FALSE;
    }
    /* print_hex_main(key.priv_key, NELEMS(key.priv_key)); */

    if (bip32_key_to_base58(&key, BIP32_FLAG_KEY_PRIVATE, &xprv) != WALLY_OK) {
        fprintf(stderr, "error getting xprv\n");
        return FALSE;
    }
    printf("%s\n", xprv);

    wally_free_string(xprv);

    return TRUE;
}

int
convert_bip39(const int testnet)
{
    char mnemonic[MNEMONIC_MAX_LENGTH];

    if (!mnemoic_read(mnemonic, NELEMS(mnemonic))) {
        fprintf(stderr, "error: reading mnemonics\n");
        return FALSE;
    }

    if (!mnemonic_convert(mnemonic, testnet)) {
        fprintf(stderr, "error: converting mnemonics\n");
        return FALSE;
    }

    return TRUE;
}

int
xprv_to_xpub(const char *const path)
{
    struct ext_key key_root;
    struct ext_key key_child;

    char xprv_string[EXTENDED_KEY_STRING_LENGTH + 1];

    const char *path_real;

    char *xpub;

    FILE *file;

    size_t at;
    int c;

    file = stdin;

    for (at = 0; at < NELEMS(xprv_string) - 1; at++) {
        c = fgetc(file);
        if (c == EOF) {
            fprintf(
                stderr,
                "error: xprv_string is too short, should be %lu characters\n",
                NELEMS(xprv_string) - 1
            );
            return FALSE;
        }
        xprv_string[at] = c;
    }
    xprv_string[at] = '\0';

    WALLY_RUN_FUNCTION(bip32_key_from_base58(
        xprv_string, &key_root
    ));

    if (path != NULL) {
        path_real = path;
    } else {
        path_real = "84'/0'/0'";
    }

    if (string_equal(path_real, "root")) {
        WALLY_RUN_FUNCTION(bip32_key_to_base58(
            &key_root, BIP32_FLAG_KEY_PUBLIC, &xpub
        ));
    } else {
        WALLY_RUN_FUNCTION(bip32_key_from_parent_path_str(
            &key_root, path_real, 0,
            BIP32_FLAG_KEY_PRIVATE,
            &key_child
        ));

        WALLY_RUN_FUNCTION(bip32_key_to_base58(
            &key_child, BIP32_FLAG_KEY_PUBLIC, &xpub
        ));
    }

    printf("%s\n", xpub);

    WALLY_RUN_FUNCTION(wally_free_string(xpub));

    return TRUE;
}

int
extended_key_check(void)
{
    struct ext_key key_root;

    char extended_string[EXTENDED_KEY_STRING_LENGTH + 1];

    FILE *file;

    size_t at;
    int c;

    file = stdin;

    for (at = 0; at < NELEMS(extended_string) - 1; at++) {
        c = fgetc(file);
        if (c == EOF) {
            fprintf(
                stderr,
                "error: extended_string is too short, should be %lu characters\n",
                NELEMS(extended_string) - 1
            );
            return FALSE;
        }
        extended_string[at] = c;
    }
    extended_string[at] = '\0';

    WALLY_RUN_FUNCTION(bip32_key_from_base58(
        extended_string, &key_root
    ));

    printf("extended key checked successfully\n");

    return TRUE;
}
