/*
 * mbedTLS calls this whenever MBEDTLS_ENTROPY_HARDWARE_ALT is defined
 * (see mbedtls_config_additions.h). Requires RNG peripheral enabled in
 * CubeMX (Analog > RNG > Activated) - regenerate main.c so `hrng` exists.
 */
#include "main.h"     /* for extern RNG_HandleTypeDef hrng; */
#include <string.h>

extern RNG_HandleTypeDef hrng;

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    (void)data;
    uint32_t rnd;
    size_t produced = 0;

    while (produced < len) {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {
            return -1; /* MBEDTLS_ERR_ENTROPY_SOURCE_FAILED, avoids pulling the whole error header */
        }
        size_t chunk = (len - produced) < sizeof(rnd) ? (len - produced) : sizeof(rnd);
        memcpy(output + produced, &rnd, chunk);
        produced += chunk;
    }
    *olen = produced;
    return 0;
}
