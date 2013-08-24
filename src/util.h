#ifndef UTIL_H
#define UTIL_H

#define MENU_CELL_BASIC_HEIGHT 44

#define LOC_FLOAT_SCALAR 10000000L

char* itoa(int i);

uint16_t stokenize(char *source, uint16_t index, char delimiter, char *target, uint16_t max_length, uint16_t source_length);

int satoi(char *str, uint16_t size);

#endif // UTIL_H