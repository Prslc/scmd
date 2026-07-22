#include <stdio.h>
#include <stdlib.h>

#include "context.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: scmd <config_path>\n");
        return EXIT_FAILURE;
    }

    Context ctx;
    if (!init_context(&ctx, argv[1]))
        return EXIT_FAILURE;

    event_loop(&ctx, argv[1]);

    cleanup_context(&ctx);
    return EXIT_SUCCESS;
}
