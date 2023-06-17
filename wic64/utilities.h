#ifndef WIC64_HEXDUMP_H
#define WIC64_HEXDUMP_H

#include <cstdint>

void hexdump(const char *msg, uint8_t *data, uint32_t size);
void log_free_mem(void);

#endif // WIC64_HEXDUMP_H
