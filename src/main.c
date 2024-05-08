#include <stdlib.h>

#include "util.h"
#include "convert.h"
#include "generate.h"
#include "sign.h"
#include "descriptor.h"

int
main(int argc, char **argv)
{
    int mode;

    if (argc < 2) {
        fprintf(stderr, "error: insert action\n");
        return EXIT_FAILURE;
    }

    if (string_equal(argv[1], "-h") || string_equal(argv[1], "--help")) {
        fprintf(stdout, "\
bitscriptc action [options]\n\
\n\
actions:\n\
    generate-bip39\n\
    extended-key-check\n\
    xprv-from-bip39 [testnet]\n\
    xpub-from-xprv [path]\n\
    sign-private-key private-key [message]\n\
    sign-extended-key address [message]\n\
    descriptor-info child-number [descriptor]\n\
\n\
"
        );
        fprintf(stdout, "\
xprv-from-bip39 require the bip39 mnemonics in stdin\n\
extended-key-check requires an xprv or an xpub in stdin\n\
xpub-from-xprv requires the xprv in stdin\n\
sign-extended-key requires the xprv with the path in stdin\n\
descriptor-info requires the descriptor in stdin if not present as an argument\n\
\n\
"
        );
        fprintf(stdout, "\
if 'testnet' is passed as parameter in xprv-from-bip39 a tprv will be returned\n\
path in xpub-from-xprv can be 'root' to get the root xpub of the xprv\n\
if message is not given as a parameter will be asked interactively\n\
in xpub-from-xprv can be passed a tprv and a tpub will be returned\n\
"
        );
        return EXIT_SUCCESS;
    } else if (string_equal(argv[1], "generate-bip39")) {
        mode = WALLET_GENERATE_MNEMONIC;
    } else if (string_equal(argv[1], "extended-key-check")) {
        mode = WALLET_EXTENDED_CHECK;
    } else if (string_equal(argv[1], "xprv-from-bip39")) {
        mode = WALLET_XPRV_FROM_MNEMONIC;
    } else if (string_equal(argv[1], "xpub-from-xprv")) {
        mode = WALLET_XPUB_FROM_XPRV;
    } else if (string_equal(argv[1], "sign-private-key")) {
        mode = WALLET_SIGN_PRIVATE_KEY;
    } else if (string_equal(argv[1], "sign-extended-key")) {
        mode = WALLET_SIGN_EXTENDED_KEY;
    } else if (string_equal(argv[1], "descriptor-info")) {
        mode = WALLET_DESCRIPTOR;
    } else {
        fprintf(stderr, "error: %s not recognized\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (mode == WALLET_GENERATE_MNEMONIC) {
        int words_length = BIP39_24_WORDS;
        if (argc >= 3) {
            if (string_equal(argv[2], "24")) {
                words_length = BIP39_24_WORDS;
            } else if (string_equal(argv[2], "12")) {
                words_length = BIP39_12_WORDS;
            } else {
                fprintf(stderr, "error: insert 24 or 12\n");
                return EXIT_FAILURE;
            }
        }
        if (!generate_bip39(words_length)) {
            fprintf(stderr, "error: generating mnemonics\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_EXTENDED_CHECK) {
        if (!extended_key_check()) {
            fprintf(stderr, "error: checking extended key\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_XPRV_FROM_MNEMONIC) {
        int testnet = FALSE;
        if (argc >= 3 && string_equal(argv[2], "testnet")) {
            testnet = TRUE;
        }
        if (!convert_bip39(testnet)) {
            fprintf(stderr, "error: converting mnemonics to xprv\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_XPUB_FROM_XPRV) {
        const char *path = NULL;
        if (argc >= 3) {
            path = argv[2];
        }
        if (!xprv_to_xpub(path)) {
            fprintf(stderr, "error: converting xprv to xpub\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_SIGN_PRIVATE_KEY) {
        const char *message = NULL;
        if (argc < 3) {
            fprintf(stderr, "error: insert private-key\n");
            return EXIT_FAILURE;
        }
        if (argc >= 4) {
            message = argv[3];
        }
        if (!sign_message(argv[2], message)) {
            fprintf(stderr, "error: converting mnemonics\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_SIGN_EXTENDED_KEY) {
        const char *message = NULL;
        if (argc < 3) {
            fprintf(stderr, "error: insert address\n");
            return EXIT_FAILURE;
        }
        if (argc >= 4) {
            message = argv[3];
        }
        if (!sign_message_xprv_address(argv[2], message)) {
            fprintf(stderr, "error: signing with key\n");
            return EXIT_FAILURE;
        }
    } else if (mode == WALLET_DESCRIPTOR) {
        const char *descriptor = NULL;
        if (argc < 3) {
            fprintf(stderr, "error: insert child number\n");
            return EXIT_FAILURE;
        }
        if (argc < 4) {
            descriptor = argv[3];
        }
        if (!read_descriptor(argv[2], descriptor)) {
            fprintf(stderr, "error: reading descriptor\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
