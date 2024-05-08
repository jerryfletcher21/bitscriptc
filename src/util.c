#include "util.h"

#include <termios.h>
#include <unistd.h>
#include <string.h>

#if defined(__linux__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif

#include "wally_core.h"

int
fprint_error_status(FILE *stream, const int status)
{
    if (status == WALLY_OK) {
        fprintf(stream, "WALLY_OK\n");
    } else if (status == WALLY_ERROR) {
        fprintf(
            stream,
            "WALLY_ERROR\n"
            "An internal or unexpected error happened in the library.\n"
            "In some cases this code may indicate a specific error\n"
            "condition which will be documented with the function\n"
        );
    } else if (status == WALLY_EINVAL) {
        fprintf(
            stream,
            "WALLY_EINVAL\n"
            "One or more parameters passed to the function is not valid.\n"
            "For example, a required buffer value is NULL or of the wrong length\n"
        );
    } else if (status == WALLY_ENOMEM) {
        fprintf(
            stream,
            "WALLY_ENOMEM\n"
            "The function required memory allocation but\n"
            "no memory could be allocated from the O/S\n"
        );
    }
    return TRUE;
}

void
print_hex_main(const byte *const data, const size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int
string_equal(const char *const first, const char *const second)
{
    if (first == NULL && second == NULL) return TRUE;
    if (first == NULL || second == NULL) return FALSE;

    return strcmp(first, second) == 0 ? TRUE : FALSE;
}

int
fill_random(byte *const data, const size_t size)
{
#if defined(__linux__) || defined(__FreeBSD__)
    ssize_t res = getrandom(data, size, 0);

    if (res < 0 || (size_t) res != size) {
        return FALSE;
    } else {
        return TRUE;
    }
#elif defined(__OpenBSD__)
    /* If `getentropy(2)` is not available you should fallback to either
     * `SecRandomCopyBytes` or /dev/urandom */
    int res = getentropy(data, size);
    if (res == 0) {
        return 1;
    } else {
        return 0;
    }
#endif
}

int
read_string_from_stdin_terminal(
    char *const string, const size_t string_length
)
{
    size_t at;
    int c;

    struct termios term;

    if (!isatty(STDIN_FILENO)) {
        if (freopen("/dev/tty", "r", stdin) == NULL) {
            fprintf(stderr, "error: reopening stdin\n");
            return FALSE;
        }
    }

    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    at = 0;
    while ((c = fgetc(stdin)) != EOF && at < string_length - 1) {
        if (c == 4) {
            break;
        } else if (c == 127) {
            if (at > 0) {
                fputs("\b \b", stdout);
                string[--at] = '\0';
            }
        } else {
            fputc(c, stdout);
            string[at++] = c;
        }
    }
    string[at++] = '\0';

    term.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    printf("\n");

    return TRUE;
}
