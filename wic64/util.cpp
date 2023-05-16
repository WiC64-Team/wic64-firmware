#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "esp32-hal-log.h"

void hexdump(const char* msg, uint8_t *data, uint16_t size) {
  char *str = (char*) calloc(strlen(msg)+1 + size * 3 + (size/16), sizeof(uint8_t) + 1);
  char *pos = str;

  snprintf(pos, strlen(msg)+2, "%s\n", msg);
  pos = pos + strlen(msg) + 1;

  for (uint8_t i=0; i<size; i++) {
    snprintf(pos, 5, "%02X ", data[i]);
    pos += 3;

    if ((i+1) % 16 == 0) {
      pos[0] = '\n';
      pos++;
    }
  }

  log_d("%s", str);
  free(str);
}