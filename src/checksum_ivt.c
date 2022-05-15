#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <string.h>
#include <unistd.h>

#include "lpcc_log.h"

/**
 * @brief This function returns the first section ID with name.
 *
 * @param handle Elf *
 * @param name section name
 * @return int
 */
static int lpcc_get_section_idx_by_name(Elf *handle, char *name) {
    /* Get total section counts. */
    size_t e_nsections;
    if (elf_getshdrnum(handle, &e_nsections) < 0) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("Failed to retrieve section counts, %s", elf_errmsg(errsv));

        return -1;
    }

    /* Get string table index. */
    size_t e_stridx;
    if (elf_getshdrstrndx(handle, &e_stridx) < 0) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("Failed to get string table index, %s", elf_errmsg(errsv));

        return -2;
    }

    /* Search sections for specific name */
    for (size_t i = 0; i < e_nsections; i++) {
        Elf_Scn *scn = elf_getscn(handle, i);
        if (scn == NULL) {
            int errsv = elf_errno();
            LPCC_LOG_ERROR("Failed to get section ID: %d, %s", i, elf_errmsg(errsv));

            return -3;
        }

        /* Get section header */
        GElf_Shdr  shdr_mem;
        GElf_Shdr *shdr = gelf_getshdr(scn, &shdr_mem);
        if (shdr == NULL) {
            int errsv = elf_errno();
            LPCC_LOG_ERROR("Failed to get section header ID: %d, %s", i, elf_errmsg(errsv));

            return -4;
        }

        /* Get section name string from table */
        const char *s_name = elf_strptr(handle, e_stridx, shdr->sh_name);
        if (s_name == NULL) {
            int errsv = elf_errno();
            LPCC_LOG_ERROR("Failed to get section name for ID: %d, %s", i, elf_errmsg(errsv));

            return -5;
        }

        /* Found a match */
        if (strcmp(s_name, name) == 0) {
            LPCC_LOG_DEBUG("Found a match for section %s at ID: %d", s_name, i);
            return i;
        }
    }

    /* Loop completed with no matching sections found. */
    LPCC_LOG_DEBUG("No section named %s found.", name);
    return -1;
}

static Elf_Scn *lpcc_ivt_get_section(Elf *handle) {
    int idx = lpcc_get_section_idx_by_name(handle, ".interrupts");
    if (idx < 0) {
        LPCC_LOG_ERROR("Failed to find IVT section.");

        return NULL;
    }

    Elf_Scn *scn = elf_getscn(handle, idx);
    if (scn == NULL) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("Failed to get IVT section, %s", elf_errmsg(errsv));

        return NULL;
    }

    return scn;
}

static int lpcc_ivt_get_header(Elf *handle, uint32_t *ivt) {
    Elf_Scn *ivt_scn = lpcc_ivt_get_section(handle);
    if (ivt_scn == NULL) {
        LPCC_LOG_ERROR("Failed to get IVT section.");
        return -1;
    }

    Elf_Data *ivt_data = elf_getdata(ivt_scn, NULL);
    if (ivt_data == NULL) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("Failed to get IVT section content, %s", elf_errmsg(errsv));

        return -2;
    }

    LPCC_LOG_DEBUG("IVT section size: 0x%x, offset: 0x%x", ivt_data->d_size, ivt_data->d_off);

    if (ivt_data->d_size < 8 * sizeof(uint32_t)) {
        LPCC_LOG_ERROR("IVT section size too small, this should not happen.");

        return -2;
    }

    memcpy(ivt, ivt_data->d_buf, 8 * sizeof(uint32_t));

    return 0;
}

static int lpcc_ivt_write_checksum(Elf *handle, uint32_t checksum) {
    Elf_Scn *ivt_scn = lpcc_ivt_get_section(handle);
    if (ivt_scn == NULL) {
        LPCC_LOG_ERROR("Failed to get IVT section.");
        return -1;
    }

    Elf_Data *ivt_data = elf_getdata(ivt_scn, NULL);
    if (ivt_data == NULL) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("Failed to get IVT section content, %s", elf_errmsg(errsv));

        return -2;
    }

    LPCC_LOG_DEBUG("IVT section size: 0x%x, offset: 0x%x", ivt_data->d_size, ivt_data->d_off);

    if (ivt_data->d_size < 8 * sizeof(uint32_t)) {
        LPCC_LOG_ERROR("IVT section size too small, this should not happen.");

        return -2;
    }

    ((uint32_t *)ivt_data->d_buf)[7] = checksum;

    if (elf_flagdata(ivt_data, ELF_C_SET, ELF_F_DIRTY) < 0) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("IVT data flag error, %s", elf_errmsg(errsv));
    }

    if (elf_update(handle, ELF_C_WRITE_MMAP) < 0) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("elf update error, %s", elf_errmsg(errsv));
    }
}

static uint32_t lpcc_calculate_checksum(uint32_t *ivt) {
    /* LPC checksum is calculated so the sum of first 8 words is 0. */
    /* checksum is stored in 0x1C for Cortex-M MCUs */

    uint32_t sum = 0U;

    for (uint32_t i = 0; i < 7; i++) {
        sum += ivt[i];
    }

    return -sum; /* 2's complement */
}

int lpcc_checksum_ivt_append(const char *elf_path) {
    int ret = 0;

    /* Open file for read and write, return -1 if error occurs. */
    int elf_fd = open(elf_path, O_RDWR);
    if (elf_fd < 0) {
        int errsv = errno;
        LPCC_LOG_ERROR("ELF open failed, %s", strerror(errsv));

        return -1;
    }

    elf_version(EV_CURRENT);

    /* Get libelf handle for the file opened, return -2 if error occurs. */
    Elf *e_handle = elf_begin(elf_fd, ELF_C_RDWR, NULL);
    if (e_handle == NULL) {
        int errsv = elf_errno();
        LPCC_LOG_ERROR("ELF parse failed, %s", elf_errmsg(errsv));

        ret = -2;

        goto close_out;
    }

    uint32_t ivt_buf[8] = {0U};
    if (lpcc_ivt_get_header(e_handle, ivt_buf) < 0) {
        LPCC_LOG_ERROR("Failed to get IVT header.");

        ret = -3;

        goto end_out;
    }

    uint32_t ivt_checksum = lpcc_calculate_checksum(ivt_buf);
    LPCC_LOG_INFO("Checksum calculated: 0x%08lx", ivt_checksum);
    if (ivt_buf[7] == ivt_checksum) {
        LPCC_LOG_WARN("Checksum is already present in section: 0x%08lx, skipped writing.", ivt_buf[7]);
    } else {
        lpcc_ivt_write_checksum(e_handle, ivt_checksum);
    }

end_out:
    elf_end(e_handle);

close_out:
    close(elf_fd);

    return ret;
}