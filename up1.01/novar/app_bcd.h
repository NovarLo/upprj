#ifndef _APP_BCD_H
#define _APP_BCD_H


#ifdef	_LOCAL_BCD
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

void PackBCD(uint8_t *buff, float value, int width, int decimal);
float UnPackBCD(uint8_t *buff, int width, int decimal);
void PacksBCD(uint8_t *buff, float value, int width, int decimal);
float UnPacksBCD(uint8_t *buff, int width, int decimal);


#undef	_EXTERN
#endif
