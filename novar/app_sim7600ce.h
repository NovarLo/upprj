#ifndef _APP_SIM7600CE_H
#define _APP_SIM7600CE_H

#include "app_fifo.h"
#ifdef _LOCAL_SIM7600CE
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

/****************************  ***************************/
/////////// type & variables

/************************************************************
*						功能码定义列表  					*
************************************************************/


/***************************************************************
*				子站－>主站帧结构类型定义                      *
*				  2015.01.02 zjh							   *
***************************************************************/

/***************************************************************
*				主站－>子站帧结构类型定义                      *
*				  2015.01.02  zjh							   *
***************************************************************/

BOOL SIM7600CE_pow_on(void);

///////////////////////////////////////// 
/// 

void SIM7600CE_SENDAT_ATE0(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPCCFG(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPSRIP(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPHEAD(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPMODE(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_NETOPEN(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPRXGET(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPOPEN(uint32_t par1, uint32_t par2);
void SIM7600CE_SENDAT_CIPSEND(uint8_t priority);
void LOADFRM_LOGIN(uint32_t par1, uint32_t par2);
#undef	_EXTERN
#endif
/*******************************  END OF FILE  *******************************/
