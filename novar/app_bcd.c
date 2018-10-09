#define _LOCAL_BCD

#include "stm32f4xx_hal.h"
#include "math.h"
#include "string.h"
#include "app_bcd.h"
/* ----------------------------------------------------------------------
* BCD��������벻������λ*
* char* buff : Ŀ�껺���� �ֽ���Ϊ��width/2+1��
* |  ����λ | ����λ |
* |  ���λ |  �θ�λ| 
* |  XXXXλ| XXXXλ| 
* |  �ε�λ | ���λ | 
* |  С��λ |  | 
* char* buff : Ŀ�껺����
* float value : ֵ
* int width : Ŀ����
* int decimal: Ŀ��С��λ��
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
* BCD��������������λ
* char* buff : Ŀ�껺���� �ֽ���Ϊ��width/2+1��
* |  ����λ | ����λ |
* |  ����λ | ���λ | 
* |  �θ�λ | XXXXλ | 
* |  XXXXλ | �ε�λ | 
* |  ���λ | С��λ | 
*  
* float value : ֵ
* int width : Ŀ����
* int decimal: Ŀ��С��λ��
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
