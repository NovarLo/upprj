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
#define PWD 0x00000000	// 密码

//////////// cmd code //////////////
#define GenImg 0x01	// 录指纹图像
#define Img2Tz 0x02	// 图像转特征
#define Match  0x03	// 特征对比
#define Search 0x04	// 搜索指纹
#define RegModel 0x05 // 特征合成模版
#define Store 0x06 // 存储模版
#define LoadChar 0x07 // 读出模版
#define UpChar 0x08 // 上传特征
#define DownChar 0x09 // 下载特征
#define UpImage 0x0a // 上传图像
#define DownImage 0x0b // 下载图像
#define DeletChar 0x0c // 删除模板
#define Empty 0x0d	// 清空指纹库
#define SetSysPara 0x0e // 设置系统参数
#define ReadSysPara 0x0f // 读系统参数
#define SetPwd 0x12 // 设置口令
#define VfyPwd 0x13 // 效验口令
#define GetRandomCode 0x14 // 采样随机数
#define SetAddr 0x15 // 设置地址
#define WriteNotepad 0x18 // 写记事本
#define ReadNotepad 0x19 // 读记事本
#define TemplateNum 0x1d // 读指纹模板数
#define ReadConList 0x1f // 读指纹模板索引表

///////////////// ACK code //////////////////
#define ACK_OK 0x00 			 // 指令执行完毕或OK；                                    
#define ACK_DatPackErr 0x01 	 // 数据包接收错误；                                      
#define ACK_NoFngr 0x02 		 // 传感器上没有手指；                                    
#define ACK_FngrInputFail 0x03   // 录入指纹图像失败；                                    
#define ACK_FngrChaos 0x06  	 // 指纹图像太乱而生不成特征；                            
#define ACK_FngrPtLess 0x07 	 // 指纹图像正常，但特征点太少（或面积太小）而生不成特征；
#define ACK_FngrNoMatch 0x08	 // 指纹不匹配；                                          
#define ACK_NoSearch 0x09 		 // 没搜索到指纹；                                        
#define ACK_FtrMergFail 0x0a	 // 特征合并失败；                                        
#define ACK_OutOfRange 0x0b 	 // 访问指纹库时地址序号超出指纹库范围；                  
#define ACK_TpltErr 0x0c		 // 从指纹库读模板出错或无效；                            
#define ACK_UpFtrFail 0x0d  	 // 上传特征失败；                                        
#define ACK_DisAcptDatPak 0x0e   // 模块不能接受后续数据包；                              
#define ACK_UpPicErr 0x0f   	 // 上传图像失败；                                        
#define ACK_DelTpltFail 0x10	 // 删除模板失败；                                        
#define ACK_ClrLibFial 0x11 	 // 清空指纹库失败；                                      
#define ACK_PwdErr 0x13 		 // 口令不正确；                                          
#define ACK_NoPic 0x015 		 // 缓冲区内没有有效原始图而生不成图像；                  
#define ACK_RdFlashErr 0x18 	 // 读写FLASH出错；                                       
#define ACK_InvalidReg 0x1a 	 // 无效寄存器号；                                        
#define ACK_AddrErr 0x20		 // 地址码错误                                            
#define ACK_NeedPwd 0x21		 // 必须验证口令；   
#define ACK_SumErr	0xFF		 // 校验错误

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
#pragma pack (1) /* 指定按1字节对齐*/
typedef struct
{
	uint16_t start;
	uint32_t addr;
	uint8_t pid;
	uint16_t length;	// 注意：最大值不能超过(MAX_FNGRFRM_SIZE-11)
	uint8_t buf[MAX_FNGRFRM_SIZE-11];
	uint16_t sum;
}
FrmFngr,*pFrmFngr;
#pragma pack () /* 取消指定对齐，恢复缺省对齐*/

_EXTERN FrmFngr fngr_struct_send;
_EXTERN FrmFngr fngr_struct_recv;
_EXTERN uint8_t fngr_send_frm[MAX_FNGRFRM_SIZE];
_EXTERN uint8_t fngr_recv_frm[MAX_FNGRFRM_SIZE];

typedef struct
{
	uint8_t STAT_FNGRSEND_BUSY;	// 发送串口忙标志
	uint8_t STAT_FNGRRECV_BUSY;	// 接收串口忙标志
	uint8_t STAT_10MIN_EN;	// 十分钟闹钟使能
	uint8_t STAT_10MIN_TIMEOUT;	// 十分钟闹钟到达
	uint8_t STAT_2HOUR_EN;	// 2小时闹钟使能
	uint8_t STAT_2HOUR_TIMEOUT;	// 2小时闹钟到达
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
