#ifndef _APP_SDS011_H
#define _APP_SDS011_H 

#ifdef _LOCAL_SDS011
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

_EXTERN uint16_t PM2_5VAL;
_EXTERN uint16_t PM10_VAL;

void SDS011_Init(void);

#undef _EXTERN
#endif
