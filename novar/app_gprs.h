#ifndef _APP_GPRS_H
#define _APP_GPRS_H

#ifdef _LOCAL_GPRS
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

/* ***************************  ***************************/
/////////// type & variables

// 定义GPRS返回状态
#define GPRS_POWON_SUCCESS	0x00
#define GPRS_POWON_FAIL		0x01
#define GPRS_RETURN_FAIL	0x02
#define GPRS_ANSWER_SUCCESS	0x00
#define GPRS_ANSWER_FAIL	0x01

/* ***********************************************************
*						功能码定义列表  					*
************************************************************/


/* **************************************************************
*				子站－>主站帧结构类型定义                      *
*				  2015.01.02 zjh							   *
***************************************************************/

/* **************************************************************
*				主站－>子站帧结构类型定义                      *
*				  2015.01.02  zjh							   *
***************************************************************/

_EXTERN uint8_t *port;
_EXTERN uint8_t *hostip; //公司SXJIANSHE.COM
_EXTERN uint8_t or_At[]; //4
_EXTERN uint8_t or_Ate0[]; //6
_EXTERN uint8_t or_Anode[]; //27
_EXTERN uint8_t or_AstatU[];//17
_EXTERN uint8_t or_Acloseu[];//16
_EXTERN uint8_t or_Acgatt[]; //12
_EXTERN uint8_t or_Aopen[]; //13
_EXTERN uint8_t or_Aipset[]; //35
_EXTERN uint8_t re_ok[]; //6
_EXTERN uint8_t re_Atok[]; //10
_EXTERN uint8_t re_Ateok[]; //12
_EXTERN uint8_t re_AstatU[];
_EXTERN uint8_t re_Aclostu[];//15
_EXTERN uint8_t re_Aopen[]; //29
_EXTERN uint8_t re_Aopen2[];
_EXTERN uint8_t re_Aipset[]; //30
_EXTERN uint8_t re_Aipset2[]; //31
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

uint8_t gprs_2639_powon(void);
uint8_t comp_char(uint8_t *ndata0, uint8_t *ndata1, uint8_t num);
BOOL gprs_power_reset(void);
void statemachine_init(void);
BOOL gprs_pow_on(void);
BOOL gprs_pow_off(void);
BOOL gprs_udp_init(void);
BOOL gprs_at_cycle(uint8_t *at_cmd, uint8_t *ack, uint8_t *ack1);
BOOL gprs_sendu(uint8_t *buffer, uint8_t length);
BOOL gprs_main(void);

/////////////////////////////////////////
void GPRS_INIT(void);
void GPRS_TASK(void);
void GPRS_RECV(void);
void GPRS_CHKRPT(void);
void GPRS_RECV_ACK(void);
void GPRS_RECV_ATACK(void);
void GPRS_RECV_DATAFRAME(void);
void GPRS_RECV_PRTCACK(uint8_t appfuncode, uint8_t *buf);
void GPRS_RECV_DOWN(uint8_t appfuncode, uint8_t *buf);
void UDP_AT_INIT(void);
void GPRS_SENDU_ASCII(uint32_t par1, uint32_t par2);
void GPRS_SENDU_HEX(uint32_t par1, uint32_t par2);

void zipsendu(uint8_t mode);
void novar_print(uint8_t *buf, uint16_t len);

#undef	_EXTERN
#endif
/* ******************************  END OF FILE  *******************************/
