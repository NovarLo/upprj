#ifndef _APP_FINGER_H
#define _APP_FINGER_H

#ifdef	_LOCAL_FINGER
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif
///////////  macro ///////////// 
///
#define START 0xef01
#define ADDR 0xffffffff
#define PID_CMD 0x01
#define PID_DAT 0x02
#define PID_ACK 0x07
#define PID_END 0x08

///// default password   /////
#define PWD 0x00000000	// ����

//////////// cmd code //////////////
#define GenImg 0x01	// ¼ָ��ͼ��
#define Img2Tz 0x02	// ͼ��ת����
#define Match  0x03	// �����Ա�
#define Search 0x04	// ����ָ��
#define RegModel 0x05 // �����ϳ�ģ��
#define Store 0x06 // �洢ģ��
#define LoadChar 0x07 // ����ģ��
#define UpChar 0x08 // �ϴ�����
#define DownChar 0x09 // ��������
#define UpImage 0x0a // �ϴ�ͼ��
#define DownImage 0x0b // ����ͼ��
#define DeletChar 0x0c // ɾ��ģ��
#define Empty 0x0d	// ���ָ�ƿ�
#define SetSysPara 0x0e // ����ϵͳ����
#define ReadSysPara 0x0f // ��ϵͳ����
#define SetPwd 0x12 // ���ÿ���
#define VfyPwd 0x13 // Ч�����
#define GetRandomCode 0x14 // ���������
#define SetAddr 0x15 // ���õ�ַ
#define WriteNotepad 0x18 // д���±�
#define ReadNotepad 0x19 // �����±�
#define TemplateNum 0x1d // ��ָ��ģ����
#define ReadConList 0x1f // ��ָ��ģ��������

///////////////// ACK code //////////////////
#define ACK_OK 0x00 			 // ָ��ִ����ϻ�OK��                                    
#define ACK_DatPackErr 0x01 	 // ���ݰ����մ���                                      
#define ACK_NoFngr 0x02 		 // ��������û����ָ��                                    
#define ACK_FngrInputFail 0x03   // ¼��ָ��ͼ��ʧ�ܣ�                                    
#define ACK_FngrChaos 0x06  	 // ָ��ͼ��̫�Ҷ�������������                            
#define ACK_FngrPtLess 0x07 	 // ָ��ͼ����������������̫�٣������̫С����������������
#define ACK_FngrNoMatch 0x08	 // ָ�Ʋ�ƥ�䣻                                          
#define ACK_NoSearch 0x09 		 // û������ָ�ƣ�                                        
#define ACK_FtrMergFail 0x0a	 // �����ϲ�ʧ�ܣ�                                        
#define ACK_OutOfRange 0x0b 	 // ����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ��                  
#define ACK_TpltErr 0x0c		 // ��ָ�ƿ��ģ��������Ч��                            
#define ACK_UpFtrFail 0x0d  	 // �ϴ�����ʧ�ܣ�                                        
#define ACK_DisAcptDatPak 0x0e   // ģ�鲻�ܽ��ܺ������ݰ���                              
#define ACK_UpPicErr 0x0f   	 // �ϴ�ͼ��ʧ�ܣ�                                        
#define ACK_DelTpltFail 0x10	 // ɾ��ģ��ʧ�ܣ�                                        
#define ACK_ClrLibFial 0x11 	 // ���ָ�ƿ�ʧ�ܣ�                                      
#define ACK_PwdErr 0x13 		 // �����ȷ��                                          
#define ACK_NoPic 0x015 		 // ��������û����Чԭʼͼ��������ͼ��                  
#define ACK_RdFlashErr 0x18 	 // ��дFLASH����                                       
#define ACK_InvalidReg 0x1a 	 // ��Ч�Ĵ����ţ�                                        
#define ACK_AddrErr 0x20		 // ��ַ�����                                            
#define ACK_NeedPwd 0x21		 // ������֤���   
#define ACK_SumErr	0xFF		 // У�����

////////
#define FRM_NUM 128
#define CHARBUF1 0x01
#define CHARBUF2 0x02
#define FNGR_RECHK_1SEC 1000
#define FNGR_RECHK_5SEC 5000
#define FNGR_RECHK_10SEC 10000
#define FNGR_RECHK_15SEC 15000
#define FNGR_RECHK_10MIN 600000
#define FNGR_RESET_2HOUR 7200000

// structure
#define MAX_FNGRFRM_SIZE 2048
#pragma pack (1) /* ָ����1�ֽڶ���*/
typedef struct
{
	uint16_t start;
	uint32_t addr;
	uint8_t pid;
	uint16_t length;	// ע�⣺���ֵ���ܳ���(MAX_FNGRFRM_SIZE-11)
	uint8_t buf[MAX_FNGRFRM_SIZE-11];
	uint16_t sum;
}
FrmFngr,*pFrmFngr;
#pragma pack () /* ȡ��ָ�����룬�ָ�ȱʡ����*/

_EXTERN FrmFngr fngr_struct_send;
_EXTERN FrmFngr fngr_struct_recv;
_EXTERN uint8_t fngr_send_frm[MAX_FNGRFRM_SIZE];
_EXTERN uint8_t fngr_recv_frm[MAX_FNGRFRM_SIZE];

typedef struct
{
	uint8_t STAT_FNGRSEND_BUSY;	// ���ʹ���æ��־
	uint8_t STAT_FNGRRECV_BUSY;	// ���մ���æ��־
	uint8_t STAT_10MIN_EN;	// ʮ��������ʹ��
	uint8_t STAT_10MIN_TIMEOUT;	// ʮ�������ӵ���
	uint8_t STAT_2HOUR_EN;	// 2Сʱ����ʹ��
	uint8_t STAT_2HOUR_TIMEOUT;	// 2Сʱ���ӵ���
}
FngrStatus,*pFngStatus;

_EXTERN FngrStatus fngr_stat;

//* ****************** function *********************/
void APP_FNGR_Init(void);
void APP_FNGR_TASK(void);
void APP_FNGR_TEST(void);
uint8_t FNGR_WriteNotepad(uint8_t pagenum, uint8_t *buffer);
uint8_t FNGR_ReadNotepad(uint8_t pagenum, uint8_t *buffer);
uint8_t FNGR_DowChar(pFrmFngr ptr, uint8_t bufferid, uint8_t *fingerbuf, uint16_t length);
uint8_t FNGR_Store(pFrmFngr ptr, uint8_t bufferid, uint16_t pageid);
uint8_t FNGR_DeletChar(uint16_t pageid, uint16_t num);
uint8_t FNGR_Empty(void);

#undef _EXTERN
#endif
