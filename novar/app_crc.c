#include "app_crc.h"

const uint8_t CRCTable[] =
{
	0x00,0xE5,0x2F,0xCA,0x5E,0xBB,0x71,0x94,0xBC,0x59,0x93,0x76,0xE2,0x07,0xCD,0x28,
	0x9D,0x78,0xB2,0x57,0xC3,0x26,0xEC,0x09,0x21,0xC4,0x0E,0xEB,0x7F,0x9A,0x50,0xB5,
	0xDF,0x3A,0xF0,0x15,0x81,0x64,0xAE,0x4B,0x63,0x86,0x4C,0xA9,0x3D,0xD8,0x12,0xF7,
	0x42,0xA7,0x6D,0x88,0x1C,0xF9,0x33,0xD6,0xFE,0x1B,0xD1,0x34,0xA0,0x45,0x8F,0x6A,
	0x5B,0xBE,0x74,0x91,0x05,0xE0,0x2A,0xCF,0xE7,0x02,0xC8,0x2D,0xB9,0x5C,0x96,0x73,
	0xC6,0x23,0xE9,0x0C,0x98,0x7D,0xB7,0x52,0x7A,0x9F,0x55,0xB0,0x24,0xC1,0x0B,0xEE,
	0x84,0x61,0xAB,0x4E,0xDA,0x3F,0xF5,0x10,0x38,0xDD,0x17,0xF2,0x66,0x83,0x49,0xAC,
	0x19,0xFC,0x36,0xD3,0x47,0xA2,0x68,0x8D,0xA5,0x40,0x8A,0x6F,0xFB,0x1E,0xD4,0x31,
	0xB6,0x53,0x99,0x7C,0xE8,0x0D,0xC7,0x22,0x0A,0xEF,0x25,0xC0,0x54,0xB1,0x7B,0x9E,
	0x2B,0xCE,0x04,0xE1,0x75,0x90,0x5A,0xBF,0x97,0x72,0xB8,0x5D,0xC9,0x2C,0xE6,0x03,
	0x69,0x8C,0x46,0xA3,0x37,0xD2,0x18,0xFD,0xD5,0x30,0xFA,0x1F,0x8B,0x6E,0xA4,0x41,
	0xF4,0x11,0xDB,0x3E,0xAA,0x4F,0x85,0x60,0x48,0xAD,0x67,0x82,0x16,0xF3,0x39,0xDC,
	0xED,0x08,0xC2,0x27,0xB3,0x56,0x9C,0x79,0x51,0xB4,0x7E,0x9B,0x0F,0xEA,0x20,0xC5,
	0x70,0x95,0x5F,0xBA,0x2E,0xCB,0x01,0xE4,0xCC,0x29,0xE3,0x06,0x92,0x77,0xBD,0x58,
	0x32,0xD7,0x1D,0xF8,0x6C,0x89,0x43,0xA6,0x8E,0x6B,0xA1,0x44,0xD0,0x35,0xFF,0x1A,
	0xAF,0x4A,0x80,0x65,0xF1,0x14,0xDE,0x3B,0x13,0xF6,0x3C,0xD9,0x4D,0xA8,0x62,0x87
};
uint8_t GetCRC7ByLeftByTable(uint8_t *data,uint8_t len)
{
	///目前返回0xAA，不进行CRC校验
	//return 0xAA;

	uint32_t inx;
	uint8_t crc = 0;

	for (inx = 0; inx < len; inx++)
	{
		crc = CRCTable[crc ^ data[inx]];
	}

	return crc;
}
/* 
uint8_t GetCRC7ByLeft(uint8_t *data)
{
	int num = 0;
	foreach (byte num2 in data)
	{
		num ^= num2;
		for (int i = 1; i <= 8; i++)
		{
			if ((num & 0x80) == 0x80)
			{
				num = (num << 1) ^ 0xe5;
			}
			else
			{
				num = num << 1;
			}
		}
	}
	return (byte)num;
}
*/

/* ******************************** endline ************************************/


