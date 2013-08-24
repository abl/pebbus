#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "util.h"
#define INT_DIGITS 3		/* enough for 8 bit integer */

char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

int satoi(char *str, uint16_t size)
{
    int output = 0, i=0;
    int sign = (str[0] == '-' ? -(++i) : 1);
    int8_t digit;
    
    while(i<size && str[i] != '\0') {
      digit = str[i++] - '0';
      if(digit < 0 || digit > 9) continue;
      output = output*10 + digit;
    }

    return output*sign;
}

uint16_t stokenize(char *source, uint16_t index, char delimiter, char *target, uint16_t max_length, uint16_t source_length) {
  char *cursor = source+index;
  uint16_t offset = 0;

  --max_length; //Leave room for a \0

  while(cursor[offset] != delimiter && cursor[offset] != '\0' && cursor[offset] != '\r' && offset < max_length && (cursor+offset) < (source+source_length)) {
    target[offset] = cursor[offset];
    ++offset;
  }
  target[offset] = '\0';

  while(cursor[offset] != delimiter && cursor[offset] != '\0' && cursor[offset] != '\r' && (cursor+offset) < (source+source_length)) ++offset;

  return index + offset + 1;
}