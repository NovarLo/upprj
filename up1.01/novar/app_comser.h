/************************************************************************
		ComSer.h
************************************************************************/
#ifndef _APP_COMSER_H
#define _APP_COMSER_H

#ifdef	_LOCAL_COMSER
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

//队列常数
#define TXSERARRAYNUM   100     // 发送队列长度
#define RXSERARRAYNUM   100     // 接收队列长度
//超时常数
#define ATOVERTM        250     // AT命令返回超时 250 * 20ms = 5S
#define RXOVERTM        3000    // 协议帧返回接收应答超时时间 3000 * 20ms = 60s                            
#define REPTXTIMS       3       // 重发次数

// 收发服务大类类型(高字节为大类型,分5类; 低字节为子类型,最多255类)
#define COMTYPE_AT      0x0000  //配置模块类
#define COMTYPE_TICK    0x0100  //登录/心跳包类
#define COMTYPE_REPORT  0x0200  //自报类
#define COMTYPE_SET     0x0300  //设置类
#define COMTYPE_QUERY   0x0400  //查询类
#define COMTYPE_ERR     0x0500  //错误类

//收发命令队列数据结构 
typedef struct
{
    uint16_t  SerType;    // 收发服务类型
                          // 1次交互中，收发的值应相同，即应答类型=发起类型，由此配对。
    uint16_t  overtime;   // 等待应答超时计数器初值,仅用于主动的上行命令,否则必须赋0 
    uint32_t  par1;       // 参数1
    uint32_t  par2;       // 参数2
    void      (*par3)(uint32_t par1,uint32_t par2); //执行函数
}COMSERARRAY;

//-----主机到GPRS通讯控制数据结构----------------------
typedef struct
{
    COMSERARRAY  TxArray[TXSERARRAYNUM];
    COMSERARRAY *TxRpoint;      // 读
    COMSERARRAY *TxWpoint;      // 写
    COMSERARRAY *TxOpoint;      // 处理中
                                
    COMSERARRAY  RxArray[RXSERARRAYNUM];
    COMSERARRAY *RxRpoint;      // 读
    COMSERARRAY *RxWpoint;      // 写
    COMSERARRAY *RxOpoint;      // 处理中
                                 
	uint8_t     TxRepTimsEn;    // 重发计数标志
    uint8_t     TxRepTims;      // 重发次数计数器
    uint8_t     TxOverDlyFlag;  // 发送超时启动标志
    uint16_t    TxOverDlyCnt;   // 发送超时计数器
                                 
}COMSERCTR;
_EXTERN COMSERCTR  ComSerCtr;	

typedef struct
{
	uint16_t SerType;
	uint8_t len;
	uint32_t timeout;
	uint8_t buf[260];
	uint8_t lock;   // =1，表示锁定，=0，可以写入
}param_ext;

_EXTERN param_ext extparam;		// 当队列中的参数par1和par2不够用来传递信息时作为参数缓冲区，指针赋给par2

// extern variable

//-------函数--------------------------------------
void ComSer_Main(void);
void ComSeverInit(void);  // 收发服务队列初始化
int16_t Add2TxSerArray(COMSERARRAY *point,uint8_t priority); // 向发送队列加入1条待发送的命令
void ComTxSeverExc(void); // 由主程序调用的发送服务

int16_t Add2RxSerArray(COMSERARRAY *point); // 由接收中断程序调用的向接收队列加入1条待发送的命令
void ComRxSeverExc(void); // 由主程序调用的接收服务
void RxOverDly(void);     // 由定时中断程序调用的接收应答超时
void Fnull(uint32_t par1, uint32_t par2);
void FProtocolAck(uint32_t par1, uint32_t par2);
void StartTimeoutCnt(void);// 启动超时定时器

#undef _EXTERN
#endif

/***************** end line ********************/

