#ifndef _APP_NETWORK_H
#define _APP_NETWORK_H

#include "app_fifo.h"
#ifdef _LOCAL_NETWORK
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

#define GET_ARRAY_LEN(array,len) {len = (sizeof(array) / sizeof(array[0]));}

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

_EXTERN uint8_t strbuf[256];    // 该数据缓冲区用来组织AT命令帧
_EXTERN uint8_t data;

_EXTERN uint8_t *port;
_EXTERN uint8_t *hostip;
_EXTERN uint8_t or_At[];
_EXTERN uint8_t or_Ate0[];
                                //
// MG2639D专用                                                                             
_EXTERN uint8_t or_Anode[];
_EXTERN uint8_t or_AstatU[];
_EXTERN uint8_t or_Acloseu[];
_EXTERN uint8_t or_Acgatt[];
_EXTERN uint8_t or_Aopen[];
_EXTERN uint8_t or_Aipset[38];
_EXTERN uint8_t or_Asendu[25];
          
// SIM7600C专用
_EXTERN uint8_t or_CSQ[];
_EXTERN uint8_t or_CIPCLOSE[];
_EXTERN uint8_t or_NETCLOSE[];
_EXTERN uint8_t or_CIPCCFG[];
_EXTERN uint8_t or_CIPSRIP[];
_EXTERN uint8_t or_CIPHEAD[];
_EXTERN uint8_t or_CIPMODE[];
_EXTERN uint8_t or_NETOPEN[];
_EXTERN uint8_t or_CIPRXGET[];
_EXTERN uint8_t or_CIPOPEN[];
_EXTERN uint8_t or_CIPSEND[40];

_EXTERN uint8_t re_NETOPEN[];
_EXTERN uint8_t re_NETCLOSE[];
_EXTERN uint8_t re_CIPOPEN[];
_EXTERN uint8_t re_CIPSEND[];
_EXTERN uint8_t re_CIPCLOSE[];
_EXTERN uint8_t re_Arcv[];
_EXTERN uint8_t re_Enter[];

                    
// AT命令返回字符                                         
_EXTERN uint8_t re_ok[];
_EXTERN uint8_t re_Atok[];
_EXTERN uint8_t re_Ateok[];
_EXTERN uint8_t re_AstatU[];
_EXTERN uint8_t re_Aclostu[];
_EXTERN uint8_t re_Aopen[];
_EXTERN uint8_t re_Aopen2[];
_EXTERN uint8_t re_Aipset[];
_EXTERN uint8_t re_Aipset2[];
_EXTERN uint8_t re_Arecv[];
_EXTERN uint8_t re_Entr[];
_EXTERN uint8_t re_CSQ[];
_EXTERN uint8_t re_Asendu[];
_EXTERN uint8_t or_re1[];                                                                                                


typedef struct 
{
	uint8_t IS_POW_ON;
	uint8_t UART_TX_BUSY;
	uint8_t UDP_INIT_OK;
	uint8_t UDP_LNK_OK;
}STAT_MACHINE_FLAG,*pSTAT_MACHINE_FLAG;

_EXTERN STAT_MACHINE_FLAG smflag;


/////NETWORK////////////////////////////////
void NETWORK_INIT(void);
void NETWORK_TASK(void);
void UDP_AT_INIT(void);
void NETWORK_SENDU_ASCII(uint32_t par1, uint32_t par2);
void NETWORK_SENDU_HEX(uint32_t par1, uint32_t par2);
void NETWORK_AT_SENDDATA(uint8_t priority);
void NETWORK_SENDDATA_PROTOCOL(uint32_t par1, uint32_t par2);
void NETWORK_RECV(void);
void NETWORK_SENDAT_CIPSEND(uint8_t priority);
void NETWORK_CHKRPT(void);
void NETWORK_RECV_DATAFRAME(void);
void NETWORK_RECV_ATACK(void);
void NETWORK_RECV_PRTCACK(uint8_t appfuncode, uint8_t *buf);
void NETWORK_RECV_DOWN(uint8_t appfuncode, uint8_t *buf);



uint8_t comp_char(uint8_t *ndata0, uint8_t *ndata1, uint8_t num);
void statemachine_init(void);
char* my_strstr(const char *s1, const char *s2);
uint8_t* my_arrayarray(uint8_t *a1, uint32_t a1_len, uint8_t *a2, uint32_t a2_len);
uint32_t cmp_char(pRingBuf m, uint8_t *s);
void GET_CSQ(uint32_t par1, uint32_t par2);
void novar_print(uint8_t *buf, uint16_t len);
uint32_t GSM_StringToHex(char *pStr, uint8_t NumDigits);
void GSM_HexToString(uint32_t HexNum, char *pStr, uint8_t NumDigits);
uint32_t GSM_StringToDec(char *pStr, uint8_t NumDigits);


#undef	_EXTERN
#endif
/*******************************  END OF FILE  *******************************/

