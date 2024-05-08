#include "descriptor.h"

/* PRIu32 */
#include <inttypes.h>

#include "wally_core.h"
#include "wally_address.h"
#include "wally_script.h"
#include "wally_descriptor.h"

#include "util.h"

int
read_descriptor(
    const char *const child_string, const char *const descriptor_string
)
{
    struct wally_descriptor *descriptor;

    char descriptor_stdin[BITCOIN_MESSAGE_MAX_LEN + 1];
    const char *descriptor_pointer;

    size_t at;
    int c;

    char *descriptor_canon;
    char *script_string;
    char *address;

    uint32_t paths_num;
    uint32_t depth;

    uint32_t child;

    byte script[1024];
    size_t script_length;

    size_t script_type;

    child = (uint32_t) strtoul(child_string, NULL, 10);

    if (descriptor_string) {
        descriptor_pointer = descriptor_string;
    } else {
        at = 0;
        while ((c = fgetc(stdin)) != EOF && c != '\n') {
            descriptor_stdin[at++] = c;
            if (at == NELEMS(descriptor_stdin) - 1) {
                fprintf(
                    stderr, "\
error: descriptor is too long, probably an error, if you are sure descriptor \n\
is the right lenght, you should increase it in the code\n\
current maximum descriptor allowed: %lu\n\
    ",
                    NELEMS(descriptor_stdin)
                );
            }
        }
        descriptor_stdin[at++] = '\0';

        descriptor_pointer = descriptor_stdin;
    }

    WALLY_RUN_FUNCTION(wally_descriptor_parse(
        descriptor_pointer,
        NULL, WALLY_NETWORK_BITCOIN_MAINNET, 0,
        &descriptor
    ));

    WALLY_RUN_FUNCTION(wally_descriptor_canonicalize(
        descriptor, 0, &descriptor_canon
    ));
    /* printf("%s\n", descriptor_canon); */
    WALLY_RUN_FUNCTION(wally_free_string(descriptor_canon));

    WALLY_RUN_FUNCTION(wally_descriptor_get_num_paths(
        descriptor, &paths_num
    ));
    /* printf("paths number: %" PRIu32 "\n", paths_num); */

    WALLY_RUN_FUNCTION(wally_descriptor_get_depth(
        descriptor, &depth
    ));
    /* printf("depth: %" PRIu32 "\n", depth); */

    WALLY_RUN_FUNCTION(wally_descriptor_to_script(
        descriptor,
        0, 0,
        0, 0, child, 0,
        script, NELEMS(script),
        &script_length
    ));

    WALLY_RUN_FUNCTION(wally_hex_from_bytes(
        script, script_length,
        &script_string
    ));
    printf("script hex: %s\n", script_string);
    WALLY_RUN_FUNCTION(wally_free_string(script_string));

    WALLY_RUN_FUNCTION(wally_scriptpubkey_get_type(
        script, script_length, &script_type
    ));
    printf("script type: %lu\n", script_type);

    WALLY_RUN_FUNCTION(wally_descriptor_to_address(
        descriptor,
        0, 0, child, 0,
        &address
    ));
    printf("%s\n", address);
    WALLY_RUN_FUNCTION(wally_free_string(address));

    WALLY_RUN_FUNCTION(wally_descriptor_free(descriptor));

    return TRUE;
}
