#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cctype>
#include "esp32-hal-log.h"

void hexdump(const char* title, uint8_t *data, uint32_t size) {
  const uint8_t newline = 1;
  const uint8_t nullbyte = 1;
  const uint8_t bytes_per_line = 16;
  const uint8_t chars_per_line = (bytes_per_line * 4) + 10 + 1 + 2;

  uint64_t num_lines = (size / bytes_per_line) + ((size % bytes_per_line) ? 1 : 0);
  uint64_t required = strlen(title) + (2 * newline) + (num_lines * chars_per_line) + nullbyte;

  char *str = (char*) calloc(required, sizeof(char));
  char *line = str;

  line += snprintf(line, strlen(title)+3, "%s\n\n", title);

  uint64_t addr;
  uint64_t data_index;
  char* cursor;

  for(uint64_t line_index = 0; line_index < num_lines; line_index++) {
    cursor = line;

    addr = line_index * bytes_per_line;
    cursor += snprintf(cursor, 9, "%08llx", addr);

    cursor += snprintf(cursor, 4, "| ");

    for(uint8_t i=0; i<bytes_per_line; i++) {
      data_index = line_index * bytes_per_line + i;

      if(data_index < size) {
        cursor += snprintf(cursor, 3, "%02x", data[data_index]);
        cursor += snprintf(cursor, 2, " ");
      }
      else {
        cursor += snprintf(cursor, 4, "   ");
      }
    }
    cursor += snprintf(cursor, 2, "|");

    for(uint8_t i=0; i<bytes_per_line; i++) {
      data_index = line_index * bytes_per_line + i;
      if(data_index < size) {
        uint8_t c = data[data_index];
        cursor[i] = isprint((char)c) ? c : '.';
      }
      else {
        cursor[i] = ' ';
      }
    }
    cursor += bytes_per_line;
    cursor += snprintf(cursor, 3, "|\n");

    line += chars_per_line;
  }

  log_d("%s", str);
  free(str);
}
