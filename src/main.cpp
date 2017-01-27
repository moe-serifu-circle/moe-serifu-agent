/* Main source code file. */

#include "msa_core.hpp"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

int main(int argc, char *argv[]) {
    msa_core::HANDLE hdl;
    if (msa_core::init(&hdl) != 0)
    {
        perror("could not init msa handle");
        return EXIT_FAILURE;
    }
    printf("Waiting to kill\n");
    sleep(10);
    printf("Killing.\n");
    if (msa_core::quit(hdl) != 0)
    {
        perror("could not quit msa");
        return EXIT_FAILURE;
    }
    printf("Dead\n");
    return EXIT_SUCCESS;
}
