#include <stdio.h>

#include "checksum_ivt.h"
#include "lpcc_log.h"

int main(int argc, const char *argv[]) {
    LPCC_LOG_INFO("lpchecksum - Patch ELF for IVT checksum used by LPC series MCUs.");

    if (argc < 2) {
        LPCC_LOG_ERROR("Usage: lpchecksum ELF_FILE");
        return -1;
    }

    if (lpcc_checksum_ivt_append(argv[1]) < 0) {
        LPCC_LOG_ERROR("Append failed.");
        return -1;
    }

    LPCC_LOG_INFO("Patch done.");

    return 0;
}