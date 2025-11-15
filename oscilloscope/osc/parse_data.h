// parse_data.h

#ifndef __PARSE_DATA_H
#define __PARSE_DATA_H

#include "main.h"
#include <stdint.h>

int parse_binary_packet(const uint8_t *packet, uint16_t *values);

#endif /* __PARSE_DATA_H */
