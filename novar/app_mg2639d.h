#ifndef _APP_MG2639D_H
#define _APP_MG2639D_H

#ifdef _LOCAL_MG2639D
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

/****************************  ***************************/
/////////// type & variables

// 定义GPRS返回状态
#define GPRS_POWON_SUCCESS	0x00
#define GPRS_POWON_FAIL		0x01
#define GPRS_RETURN_FAIL	0x02
#define GPRS_ANSWER_SUCCESS	0x00
#define GPRS_ANSWER_FAIL	0x01

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

BOOL MG2639D_pow_on(void);

/////////////////////////////////////////
void MG2639DCMD_AT_ACK(uint32_t par1, uint32_t par2);
void MG2639DCMD_ATE0_ACK(uint32_t par1, uint32_t par2);
void MG2639DCMD_OPENU_ACK(uint32_t par1, uint32_t par2);
void MG2639D_SENDAT_CIPSEND(uint8_t priority);
void MG2639DCMD_IPSET_ACK(uint32_t par1, uint32_t par2);


#undef	_EXTERN
#endif
/*******************************  END OF FILE  *******************************/
