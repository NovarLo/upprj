#define _LOCAL_BCD

#include "stm32f4xx_hal.h"
#include "math.h"
#include "string.h"
#include "app_bcd.h"
/* ----------------------------------------------------------------------
* BCD编码与解码不带符号位*
* char* buff : 目标缓冲区 字节数为（width/2+1）
* |  高四位 | 低四位 |
* |  最高位 |  次高位| 
* |  XXXX位| XXXX位| 
* |  次低位 | 最低位 | 
* |  小数位 |  | 
* char* buff : 目标缓冲区
* float value : 值
* int width : 目标宽度
* int decimal: 目标小数位数
*--------------------------------------------------------------------*/
void PackBCD(uint8_t *buff, float value, int width, int decimal) 
{
	uint16_t i;
	uint8_t xbuf[10];

	if ((float)value >= pow(10.0, width - decimal)) 
	{
		memset(xbuf, 0, width);
		return;
	}

	for (i = 0; i < (width - decimal) / 2; i++) 
	{
		xbuf[i] = (int)(value / pow(10.0, width - decimal - i * 2 - 1)) % 10 * 16
				+ (int)(value / pow(10.0, width - decimal - i * 2 - 2)) % 10;
	}

	for (i = (width - decimal) / 2; i < width / 2; i++) 
	{
		xbuf[i] = (int)(value * pow(10, i * 2 + decimal - width + 1)) % 10 * 16
				+ (int)(value * pow(10, i * 2 + decimal - width + 2)) % 10;
	}

	for (i = 0; i < width / 2; i++)
	{
		buff[i] = xbuf[i];
	}
}

float UnPackBCD(uint8_t *buff, int width, int decimal) 
{
	float value = 0;
	int i;

	for (i = 0; i < (width - decimal) / 2; i++) 
	{
		value += (buff[i] / 16 * pow(10, width - decimal - i * 2 - 1)
			    + buff[i] % 16 * pow(10, width - decimal - i * 2 - 2));
	}

	for (i = (width - decimal) / 2; i < width / 2; i++) 
	{
		value += (buff[i] / 16 / pow(10, i * 2 + decimal - width + 1)
				+ buff[i] % 16 / pow(10, i * 2 + decimal - width + 2));
	}

	return value;
}

/* ----------------------------------------------------------------------
* BCD编码与解码带符号位
* char* buff : 目标缓冲区 字节数为（width/2+1）
* |  高四位 | 低四位 |
* |  符号位 | 最高位 | 
* |  次高位 | XXXX位 | 
* |  XXXX位 | 次低位 | 
* |  最低位 | 小数位 | 
*  
* float value : 值
* int width : 目标宽度
* int decimal: 目标小数位数
*--------------------------------------------------------------------*/

void PacksBCD(uint8_t *buff, float value, int width, int decimal) 
{
	uint16_t i;
	float abs_v;
	uint8_t xbuf[10];
	
	abs_v = fabs(value);

	if ((float)abs_v >= pow(10.0, width - decimal)) 
	{
		memset(xbuf, 0, width);
		return;
	}

	if (value < 0)xbuf[0] = 1*16;
	else xbuf[0] = 0;
	

	for (i = 0; i < (width - decimal) / 2; i++)
	{
		xbuf[i] += (int)(abs_v / pow(10.0, width - decimal - i * 2 - 1)) % 10;
		xbuf[i + 1] = (int)(abs_v / pow(10.0, width - decimal - i * 2 - 2)) % 10 * 16;
	}

	for (i = (width - decimal) / 2; i < width / 2+1; i++) 
	{
		xbuf[i] += (int)(abs_v * pow(10, i * 2 + decimal - width + 1)) % 10;
		xbuf[i + 1] = (int)(abs_v * pow(10, i * 2 + decimal - width + 2)) % 10 * 16;
	}
	for (i = 0; i < (width + 1) / 2; i++)
	{
		buff[i] = xbuf[i];
	}
}

float UnPacksBCD(uint8_t *buff, int width, int decimal) 
{
	float value = 0;
	int i;

	for (i = 0; i < (width - decimal) / 2; i++) 
	{
		value += (buff[i] % 16 * pow(10, width - decimal - i * 2 - 1)
			    + buff[i+1] / 16 * pow(10, width - decimal - i * 2 - 2));
	}

	for (i = (width - decimal) / 2; i < width / 2+1; i++) 
	{
		value += (buff[i] % 16 / pow(10, i * 2 + decimal - width + 1)
				+ buff[i+1] / 16 / pow(10, i * 2 + decimal - width + 2));
	}

	if (buff[0] >> 4) value = -value;

	return value;
}

#undef _LOCAL_BCD
/* ************ end line *******************/
