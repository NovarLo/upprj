#ifndef _CRC_H
#define _CRC_H


#include "stm32f4xx_hal.h"

uint8_t GetCRC7ByLeftByTable(uint8_t *data,uint8_t len);

#endif
