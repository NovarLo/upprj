#define _LOCAL_NETWORK

#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "app_user.h"
#include "app_sensor.h"
#include "app_comser.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "app_mg2639d.h"
#include "app_sim7600ce.h"
#include "app_network.h"

/**************************************************************/

uint8_t *port = "8086";
uint8_t *hostip = "122.114.22.87"; //公司SXJIANSHE.COM
uint8_t or_At[] = "AT\r\n"; //4
uint8_t or_Ate0[] = "ATE1\r\n"; //6
								//
// MG2639D专用
uint8_t or_Anode[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n"; //27
uint8_t or_AstatU[] = "AT+ZIPSTATUSU=1\r\n"; //17
uint8_t or_Acloseu[] = "AT+ZIPCLOSEU=1\r\n"; //16
uint8_t or_Acgatt[] = "AT+CGATT=1\r\n"; //12
uint8_t or_Aopen[] = "AT+ZPPPOPEN\r\n"; //13
uint8_t or_Aipset[38] = "AT+ZIPSETUPU=1,122.114.22.87,8086\r\n"; //最大38：AT+ZIPSETUPU=1,255.255.255.255,65535\r\n
uint8_t or_Asendu[25] = "AT+ZIPSENDU=1,";

// SIM7600C专用
uint8_t or_CSQ[] = "AT+CSQ\r\n";
uint8_t or_CIPCLOSE[] = "AT+CIPCLOSE=1\r\n";
uint8_t or_NETCLOSE[] = "AT+NETCLOSE\r\n";
uint8_t or_CIPCCFG[] = "AT+CIPCCFG\r\n";
uint8_t or_CIPSRIP[] = "AT+CIPSRIP=0\r\n";
uint8_t or_CIPHEAD[] = "AT+CIPHEAD=1\r\n";
uint8_t or_CIPMODE[] = "AT+CIPMODE=0\r\n";
uint8_t or_NETOPEN[] = "AT+NETOPEN\r\n";
uint8_t or_CIPRXGET[] = "AT+CIPRXGET=0\r\n";
uint8_t or_CIPOPEN[] = "AT+CIPOPEN=1,\"UDP\",,,8086\r\n";
uint8_t or_CIPSEND[40] = "AT+CIPSEND=1,";

uint8_t re_NETOPEN[] = "+NETOPEN: 0\r\n";   // 打开成功
uint8_t re_NETCLOSE[] = "+NETCLOSE:";
uint8_t re_CIPOPEN[] = "+CIPOPEN: 1,0\r\n";
uint8_t re_CIPSEND[] = "OK\r\n\r\n+CIPSEND:";
uint8_t re_CIPCLOSE[] = "+CIPCLOSE: 1,";
uint8_t re_Arcv[] = "+IPD"; 
uint8_t re_Enter[] = "\r\n";

// AT命令返回字符
uint8_t re_ok[] = "\r\nOK\r\n"; //6
uint8_t re_Atok[] = "\r\nAT\r\n\r\nOK\r\n"; //10
uint8_t re_Ateok[] = "\r\nATE0\r\n\r\nOK\r\n"; //12
uint8_t re_AstatU[] = "\r\n+ZIPSTATUSU: ESTABLISHED\r\n";
uint8_t re_Aclostu[] = "\r\n+ZIPCLOSEU:OK\r\n"; //15
uint8_t re_Aopen[] = "\r\n+ZPPPOPEN:CONNECTED\r\n\r\nOK\r\n"; //29
uint8_t re_Aopen2[] = "\r\n+ZPPPOPEN:ESTABLISHED\r\n\r\nOK\r\n";
uint8_t re_Aipset[] =  "\r\n+ZIPSETUPU:CONNECTED\r\n\r\nOK\r\n"; //30
uint8_t re_Aipset2[] = "\r\n+ZIPSETUPU:ESTABLISHED\r\n\r\nOK\r\n"; //31
uint8_t re_Arecv[] = "\r\n+ZIPRECVU:1,";
uint8_t re_Entr[] = ",";
uint8_t re_CSQ[] = "\r\n+CSQ: ";
uint8_t re_Asendu[] = "\r\n>";
uint8_t or_re1[30];


/*
*********************************************************************************************************
*   函 数 名: statemachine_init
*
*   功能说明: 状态机标志初始化
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
void statemachine_init(void)
{
	smflag.IS_POW_ON = 0;
	smflag.UART_TX_BUSY = 0;
	smflag.UDP_INIT_OK = 0;
	smflag.UDP_LNK_OK = 0;
	DTU_rssi = 99;
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_INIT
*
*   功能说明: 无线网络变量初始化
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_INIT(void)
{
	statemachine_init();
	TIP_init();
	ComSeverInit();
}

/*
*********************************************************************************************************
*   函 数 名: UDP_AT_INIT
*
*   功能说明: UDP初始化启动AT队列
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void UDP_AT_INIT(void)
{
	COMSERARRAY cr;

	// UDP 初始化完成直接退出
	if (smflag.UDP_INIT_OK) return;

	HAL_UART_Receive_IT(HDL_UART_GPRS, recvbuf_5, QUEUE_REV_SIZE);

	if (device_ver.ver_dtu == DTU_MG2639)
	{
		MG2639D_pow_on();
	}
	else if (device_ver.ver_dtu == DTU_SIM7600CEL)
	{
		SIM7600CE_pow_on();
	}
	else
	{
		return;
	}

	if (!smflag.IS_POW_ON) return;  // 上电不成功直接退出

	cr.SerType = COMTYPE_AT;
	cr.overtime = ATOVERTM;
	cr.par1 = (uint32_t)or_At;  //  需要发送的AT命令首地址
	cr.par2 = (uint32_t)re_ok;  //  等待回复的AT返回命令首地址
	cr.par3 = NETWORK_SENDU_ASCII;

	Add2TxSerArray(&cr, 1);

	smflag.UDP_INIT_OK = 1;
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_TASK
*
*   功能说明: 网络主任务，在main工作循环中调用
*
*   形   参: NONE
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_TASK(void)
{
	UDP_AT_INIT();
	if (!smflag.UDP_INIT_OK) return;

    NETWORK_RECV();  
                     
    ComSer_Main();   
                     
    NETWORK_CHKRPT();

}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_SENDU_ASCII
*
*   功能说明: GPRS发送函数，发送参数指定地址和长度的缓冲区数据
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_SENDU_ASCII(uint32_t par1, uint32_t par2)
{
    if (!StatusFlag.STAT_GPRSSEND_BUSY)
    {
        HAL_UART_Transmit_IT(HDL_UART_GPRS, (uint8_t *)par1, strlen((const char *)par1));
        StatusFlag.STAT_GPRSSEND_BUSY = 1;
        #ifdef PRINT_UART1
            novar_print((uint8_t *)par1, strlen((const char *)par1));
        #endif
    }
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_SENDU_ASCII
*
*   功能说明: GPRS发送函数，发送参数指定地址和长度的缓冲区数据
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_SENDU_HEX(uint32_t par1, uint32_t par2)
{
    if (!StatusFlag.STAT_GPRSSEND_BUSY)
    {
        HAL_UART_Transmit_IT(HDL_UART_GPRS, (uint8_t *)par1, (uint8_t)par2);
        StatusFlag.STAT_GPRSSEND_BUSY = 1;
        #ifdef PRINT_UART1
            novar_print((uint8_t *)par1, (uint8_t)par2);
        #endif
    }
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_AT_SENDDATA
*
*   功能说明: UART通过AT命令发送数据命令
*
*   形   参: uint8_t priority，优先级
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_AT_SENDDATA(uint8_t priority)
{
	if (device_ver.ver_dtu == DTU_MG2639)
	{
		MG2639D_SENDAT_CIPSEND(priority);
	}
	else if (device_ver.ver_dtu == DTU_SIM7600CEL)
	{
		SIM7600CE_SENDAT_CIPSEND(priority);
	}
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_SENDDATA_PROTOCOL
*
*   功能说明: 接收服务接收到"AT+ZIPSENDU"/"AT+CIPSEND"命令的回复">"后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_SENDDATA_PROTOCOL(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;
    //param_ext *ptr;

    //ptr =  (param_ext *)ComSerCtr.TxOpoint->par2;
    // 添加发送数据启动帧，
    rt.SerType = extparam.SerType;  // 登录类型
    rt.overtime = extparam.timeout;         // 超时250*20=5S
    rt.par1 = (uint32_t)(extparam.buf);     // 取出功能码AFN
    rt.par2 = (uint32_t)(extparam.len); // 取出数据域登录识别码
	rt.par3 = NETWORK_SENDU_HEX;

    Add2TxSerArray(&rt,1);
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_RECV
*
*   功能说明: GPRS接收处理函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— AT应答命令
*                           —— 数据帧应答
*                           —— 服务器主动下行帧（设置帧/查询帧）
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_RECV(void)
{
    COMSERARRAY rt;

    // 接收缓冲区无数据退出
    //if (recvfifo.in == recvfifo.out)
    {
        // 在此处插入查询信号质量AT命令，不影响正常AT通信
        if (StatusFlag.STAT_CSQ_CHK && (ComSerCtr.TxWpoint==ComSerCtr.TxRpoint))
        {
                StatusFlag.STAT_CSQ_CHK = 0;
                rt.SerType = COMTYPE_AT;
                rt.overtime = ATOVERTM;
                rt.par1 = (uint32_t)or_CSQ;  //  需要发送的AT命令首地址
                rt.par2 = (uint32_t)re_CSQ;  //  等待回复的AT返回命令首地址
                rt.par3 = NETWORK_SENDU_ASCII;
                Add2TxSerArray(&rt, 0);
                return;
		}

    }
	// 接收AT命令回复
	NETWORK_RECV_ATACK();
	// 接收数据帧
	NETWORK_RECV_DATAFRAME();
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_SENDAT_CIPSEND
*
*   功能说明: 网络发送数据帧
*
*   形   参: priority，优先级。0，正常；1，插队
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_SENDAT_CIPSEND(uint8_t priority)
{
	switch (device_ver.ver_dtu)
	{
		case DTU_MG2639:
			MG2639D_SENDAT_CIPSEND(priority);
				break;
		case DTU_SIM7600CEL:
			SIM7600CE_SENDAT_CIPSEND(priority);
			break;
		default:
			return;
	}
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_CHKRPT
*
*   功能说明: GPRS检测自报信息报文函数，主要用于查询是否有主动上传数据标志，如果有，添加成员至发送队列
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_CHKRPT(void)
{
	uint16_t len;
    uint32_t relink_time;

    relink_time = device_info.link_timeout * 1000 / 20;	//转换成秒

	if (ComSerCtr.TxRpoint == ComSerCtr.TxOpoint) return;//处理中
	// 如果当前发送数据区上锁直接退出
	if (extparam.lock) return;

    // 查询是否有心跳帧需要上传
    if (StatusFlag.STAT_HEART_OV && !StatusFlag.STAT_LINK_OK)
    {
        StatusFlag.STAT_HEART_OV = 0;
        len = FrmHrtDat(extparam.buf);
        extparam.SerType = COMTYPE_TICK;
        extparam.timeout = relink_time; //RXOVERTM;

        NETWORK_SENDAT_CIPSEND(0);
		return;
    }                                      

#ifdef TOWERBOX
    // 查询报警数据标志
	len = FrmTowWrnDat(extparam.buf);
	if (len)	// 表示有
	{	
		extparam.len = len;
		extparam.SerType = COMTYPE_REPORT;
		//extparam.pBuf = extparam.buf;
		extparam.timeout = relink_time;	// 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);

		return;
	}

	// 查询工作循环标志
	len = FrmTowWklpDat(extparam.buf);
	if (len)
	{
		extparam.len = len;
		extparam.SerType = COMTYPE_REPORT;
		//extparam.pBuf = extparam.buf;
		extparam.timeout = relink_time;	// 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
		return;
	}

	// 查询标定数据标志
	len = FrmTowCaliDat(extparam.buf);
	if (len)
	{
		extparam.len = len;
		extparam.SerType = COMTYPE_REPORT;
		//extparam.pBuf = extparam.buf;
		extparam.timeout = relink_time;	// 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
		return;
	}

	// 查询实时数据标志
    len = FrmTowRtDat(extparam.buf);          
    if (len)                                   
    {                                          
        extparam.len = len;                    
        extparam.SerType = COMTYPE_REPORT;     
        //extparam.pBuf = extparam.buf;        
        extparam.timeout = relink_time;    // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
        return;
    }
    #endif

    #ifdef ELIVATOR
    // 查询升降机实时数据标志
    len = FrmElvtRtDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
        return;
    }

    // 查询升降机工作循环标志
    len = FrmElvtWklpDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
        return;
    }

    // 查询报警数据标志
    len = FrmElvtWrnDat(extparam.buf);
    if (len)    // 表示有
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);

        return;
    }

    // 查询升降机标定数据标志
    len = FrmElvtCaliDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
        return;
    }
    #endif

    #ifdef DUSTMON
    // 查询扬尘监测实时数据标志
    len = FrmDustRtDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);
        return;
    }

    // 查询报警数据标志
    len = FrmDustWrnDat(extparam.buf);
    if (len)    // 表示有
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);                           
        return;                                
    }
    #endif

#ifdef UPPLAT

    len = FrmUPPlatRtDat(extparam.buf);                
    if (len)                                           
    {                                                  
        extparam.len = len;                            
        extparam.SerType = COMTYPE_REPORT;             
        //extparam.pBuf = extparam.buf;                
        extparam.timeout = relink_time; // 500X20ms=10S
                                                       
        NETWORK_SENDAT_CIPSEND(0);                     
        return;                                        
    }                                                  
                                                       
    // 查询报警数据标志                                
                                                       
    len = FrmUPPlatWrnDat(extparam.buf);               
    if (len)    // 表示有                              
    {                                                  
        extparam.len = len;                            
        extparam.SerType = COMTYPE_REPORT;             
        //extparam.pBuf = extparam.buf;                
        extparam.timeout = relink_time; // 500X20ms=10S
                                                       
        NETWORK_SENDAT_CIPSEND(0);                     
        return;                                        
    }                                                  

#endif 
    // 查询是否有指纹登录数据上发
	len = FrmFngrDat(extparam.buf);
	if (len)
	{
        extparam.len = len;                    
        extparam.SerType = COMTYPE_REPORT;     
        //extparam.pBuf = extparam.buf;        
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);                           
        return;                                
	}
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_RECV_DATAFRAME
*
*   功能说明: 如果接收到的数据帧是协议帧的处理过程
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_RECV_DATAFRAME(void)
{
    uint8_t afn,*dptr;
    uint32_t a,b,c,ptrbak,start;
    static uint32_t cnt_over=0;

	switch (device_ver.ver_dtu)
	{
		case DTU_MG2639:
			dptr = re_Arecv;
				break;
		case DTU_SIM7600CEL:
			dptr = re_Arcv;
			break;
		default:
			return;
	}
	
    ptrbak = recvfifo_5.out;
    a = cmp_char(&recvfifo_5, dptr);

    if (a)  // 表示收到"+ZIPRECVU:1,"或者"+IPD"
    {
        cnt_over += 1;
        // get length of recvdata
        start = recvfifo_5.out;

		switch (device_ver.ver_dtu)
		{
			case DTU_MG2639:
				dptr = re_Entr;
					break;
			case DTU_SIM7600CEL:
				dptr = re_Enter;
				break;
			default:
				return;
		}

        b = cmp_char(&recvfifo_5, dptr);
        if (b)
        {
			switch (device_ver.ver_dtu)
			{
				case DTU_MG2639:
					c = GSM_StringToDec((char *)(&recvfifo_5.buffer[start & (QUEUE_REV_SIZE - 1)]), 
										(recvfifo_5.out > (start + 1)) ? (recvfifo_5.out - start - 1) : (recvfifo_5.out + QUEUE_REV_SIZE - start - 1));
						break;
				case DTU_SIM7600CEL:
					c = GSM_StringToDec((char *)(&recvfifo_5.buffer[start & (QUEUE_REV_SIZE - 1)]), 
										(recvfifo_5.out > (start + 2)) ? (recvfifo_5.out - start - 2) : (recvfifo_5.out + QUEUE_REV_SIZE - start - 2));
					break;
				default:
					return;
			}

            if (c)
            {
                // receive data is not enough
                if ((recvfifo_5.in == recvfifo_5.out) ||
					((recvfifo_5.in > recvfifo_5.out) && (recvfifo_5.in - recvfifo_5.out) < c) ||
					((recvfifo_5.in < recvfifo_5.out) && (recvfifo_5.in + QUEUE_REV_SIZE - recvfifo_5.out) < c))
                {
                    if (cnt_over > 100)
                    {
                        cnt_over = 0;
                        return;
                    }
                    recvfifo_5.out = ptrbak;
					return;
                }
                
                // 获取有效帧
                if (TIP_frame_get(&recvfifo_5, RFULL_frame))  // 查询接收缓冲区是否有完整帧
                {
                    if (!mrtn.mf_flag) afn = rtn.appzone.functioncode;
                    else afn = mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode;

                    // 判断帧类型
					if (afn >= AFN_SET_ADDRESS && afn < AFN_SELF_REALTIMEDATA)
					{
						NETWORK_RECV_DOWN(afn, RFULL_frame); // 接收协议设置查询帧
					}
					else
					{
						NETWORK_RECV_PRTCACK(afn, RFULL_frame); // 接收协议应答帧
					}
					cnt_over = 0;
                }
                else
                {
                    if (cnt_over > 100)
                    {
                        cnt_over = 0;
                        return;
                    }
                    recvfifo_5.out = ptrbak;
                }
            }
            else
            {
                if (cnt_over > 100)
                {
                    cnt_over = 0;
                    return;
                }
                recvfifo_5.out = ptrbak;
            }
        }
        else 
        {
            if (cnt_over > 100)
            {
                cnt_over = 0;
                return;
            }
            recvfifo_5.out = ptrbak;
        }
    }
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_RECV_ATACK
*
*   功能说明: GPRS接收函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— AT应答命令
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_RECV_ATACK(void)
{
    uint8_t *dptr,*dptr1;
    COMSERARRAY rt;
    uint8_t again = 0;
    uint32_t a,b=0;
    void (*func)(uint32_t par1,uint32_t par2)=NULL; //执行函数

    // 无等待发送应答直接退出
    if (ComSerCtr.TxOpoint != ComSerCtr.TxRpoint) return;

    // 如果不是AT命令，直接退出
    if (ComSerCtr.TxOpoint->SerType != COMTYPE_AT) return;

    // 判断是哪种AT命令
    a = ComSerCtr.TxOpoint->par1;

	switch (device_ver.ver_dtu)
	{
		case DTU_MG2639:
			if (a == (uint32_t)or_At)
			{
				dptr = re_ok;
				func = MG2639DCMD_AT_ACK;
			}
			else if (a == (uint32_t)or_Ate0)
			{
				dptr = re_ok;
				func = MG2639DCMD_ATE0_ACK;
			}
			else if (a == (uint32_t)or_Aopen)
			{
				dptr = re_Aopen;
				dptr1 = re_Aopen2;
				again = 1;
				func = MG2639DCMD_OPENU_ACK;
			}
			else if (a == (uint32_t)or_Aipset)
			{
				dptr = re_Aipset;
				dptr1 = re_Aipset2;
				again = 1;
				func = MG2639DCMD_IPSET_ACK;
			}
			else if (a == (uint32_t)or_Asendu)
			{
				dptr = re_Asendu;
				func = NETWORK_SENDDATA_PROTOCOL;
			}
			else if (a == (uint32_t)or_CSQ)
			{
				dptr = re_CSQ;
				func = GET_CSQ;
			}
			else
			{
				dptr = NULL;
				dptr1  = NULL;
				func = Fnull;
				return;
			}

			break;
		case DTU_SIM7600CEL:
			if (a == (uint32_t)or_At)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_ATE0;
			}
			else if (a == (uint32_t)or_Ate0)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_CIPCCFG;
			}
			else if (a == (uint32_t)or_CIPCCFG)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_CIPSRIP;
			}
			else if (a == (uint32_t)or_CIPSRIP)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_CIPHEAD;
			}
			else if (a == (uint32_t)or_CIPHEAD)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_CIPMODE;
			}
			else if (a == (uint32_t)or_CIPMODE)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_CIPRXGET;
			}
			else if (a == (uint32_t)or_CIPRXGET)
			{
				dptr = re_ok;
				func = SIM7600CE_SENDAT_NETOPEN;
			}
			else if (a == (uint32_t)or_NETOPEN)
			{
				dptr = re_NETOPEN;
				func = SIM7600CE_SENDAT_CIPOPEN;
			}
			else if (a == (uint32_t)or_CIPOPEN)
			{
				dptr = re_CIPOPEN;
				func = LOADFRM_LOGIN;
			}
			else if (a == (uint32_t)or_CIPSEND)
			{
				dptr = re_Asendu;
				func = NETWORK_SENDDATA_PROTOCOL;
			}
			else if (a == (uint32_t)or_CSQ)
			{
				dptr = re_CSQ;
				func = GET_CSQ;
			}
			else
			{
				dptr = NULL;
				dptr1  = NULL;
				func = Fnull;
				return;
			}
				break;
		default:
			return;
	}
	


    // recvfifo.in未掉头
    b = cmp_char(&recvfifo_5, dptr);
    if (b)
    {
        // 添加类型到接收队列中
        rt.SerType = COMTYPE_AT;
        rt.overtime = ATOVERTM;
        rt.par1 = ComSerCtr.TxOpoint->par1;
        rt.par2 = (uint32_t)dptr;
        rt.par3 = func;
        Add2RxSerArray(&rt);
        #ifdef PRINT_UART1
            novar_print(dptr, strlen((const char *)dptr));
        #endif
    }
    else if (again)
    {
        b = cmp_char(&recvfifo_5,dptr1);
        if (b)
        {
            // 添加类型到接收队列中
            rt.SerType = COMTYPE_AT;
            rt.overtime = ATOVERTM;
            rt.par1 = ComSerCtr.TxOpoint->par1;
            rt.par2 = (uint32_t)dptr1;
            rt.par3 = func;
            Add2RxSerArray(&rt);
        #ifdef PRINT_UART1
            novar_print(dptr1, strlen((const char *)dptr1));
        #endif
        }
    }
}

/*
*********************************************************************************************************
*   函 数 名: NETWORK_RECV_PRTCACK
*
*   功能说明: 网络接收函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— 数据帧应答
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_RECV_PRTCACK(uint8_t appfuncode,uint8_t *buf)
{
    COMSERARRAY rt;

    rt.par1 = (uint32_t)appfuncode;

 switch (appfuncode)
    {
        case AFN_LINKCHK:
            rt.SerType = COMTYPE_TICK;
            rt.par2 = (uint32_t)(*(uint8_t *)rtn.appzone.userdata);
            StatusFlag.STAT_LINK_OK = 1;
            break;
        #ifdef TOWERBOX
        case AFN_SELF_REALTIMEDATA:
		case AFN_SELF_WORKCYCLE:
		case AFN_SELF_WARNING:
		case AFN_SELF_CALIBRATION:
        #endif
        #ifdef ELIVATOR
        case AFN_SELF_ELVTRT:
        case AFN_SELF_ELVTWKLP:
        case AFN_SELF_ELVTWARN:
        case AFN_SELF_ELVTCALI:
        #endif
        #ifdef DUSTMON
        case AFN_SELF_DUSTRT:
        case AFN_SELF_DUSTWARN:
        #endif
		#ifdef UPPLAT
		case AFN_SELF_UPPLATRT:
		case AFN_SELF_UPPLATWARN:
		#endif 

        case AFN_SELF_FINGER:
			rt.SerType = COMTYPE_REPORT;
			rt.par2 = 0;	// 
			break;
		default:return;
    }
    gprs_rssi = 3;
    _LED_COM_ON();
    // 应答报文的回调函数只是起到应答发送的作用，无需回调函数进行处理
    rt.par3 = FProtocolAck;
    Add2RxSerArray(&rt);
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}
/*
*********************************************************************************************************
*   函 数 名: NETWORK_RECV_DOWN
*
*   功能说明: GPRS接收函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— 服务器主动下行帧（设置帧/查询帧）
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void NETWORK_RECV_DOWN(uint8_t appfuncode,uint8_t *buf)
{
	// 下行帧分单帧和多帧
	switch (appfuncode)
	{
		case AFN_SET_ADDRESS:// 设置遥测终端地址  
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevAddrDatSet(buf,extparam.buf);
		break;                                                   
		case AFN_QRY_ADDRESS:// 查询遥测终端地址   
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevAddrDatQry(buf,extparam.buf);
		break;                                                   
		case AFN_SET_CLOCK:// 设置遥测终端时钟                                      
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevRtcDatSet(buf,extparam.buf);
		break;                                                 
		case AFN_QRY_CLOCK:// 查询遥测终端时钟                                
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevRtcQry(buf,extparam.buf);
		break;                                         
		case AFN_SET_WORKMODE:// 设置遥测终端工作模式                               
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevWkModSet(buf,extparam.buf);
		break;                                        
		case AFN_QRY_WORKMODE:// 查询遥测终端工作模式                            
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevWkModQry(buf,extparam.buf);
		break;                                                
		case AFN_SET_SENSORTYPE:// 设置遥测终端的传感器种类
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSnsrTypSet(buf,extparam.buf);
		break;
		case AFN_QRY_SENSORTYPE:// 查询遥测终端的传感器种类  
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSnsrTypQry(buf,extparam.buf);
		break;
		case AFN_SET_SENSORPARAM:// 设置遥测终端的传感器参数
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSnsrCfgSet(buf,extparam.buf);
		break;
		case AFN_QRY_SENSORPARAM:// 查询遥测终端的传感器参数
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSnsrCfgQry(buf,extparam.buf);
		break;
		case AFN_SET_DEVIPPORT:// 设置遥测终端存储的中心站IP地址和端口号
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevIpPortSet(buf,extparam.buf);
		break;
		case AFN_QRY_DEVIPPORT:// 查询遥测终端存储的中心站IP地址和端口号
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevIpPortQry(buf,extparam.buf);
		break;
		case AFN_SET_HEARTINTERVAL:// 设置遥测终端链路心跳间隔
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevHrtIntvlSet(buf,extparam.buf);
		break;          
		case AFN_QRY_HEARTINTERVAL:// 查询遥测终端链路心跳间隔
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevHrtIntvlQry(buf,extparam.buf);
		break;			
		case AFN_SET_RECONNECTINTERVAL:// 设置遥测终端链路重连间隔
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevLnkReconIntvlSet(buf,extparam.buf);
		break;  
		case AFN_QRY_RECONNECTINTERVAL:// 查询遥测终端链路重连间隔
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevLnkReconIntvlQry(buf,extparam.buf);
		break; 
		case AFN_SET_DATRECINTERVAL:// 设置遥测终端历史数据存盘间隔
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSavIntvlSet(buf,extparam.buf);
		break;    
		case AFN_QRY_DATRECINTERVAL:// 查询遥测终端历史数据存盘间隔
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSavIntvlQry(buf,extparam.buf);
		break;    
		case AFN_SET_DATUPLOADINTERVAL:// 设置遥测终端的实时数据上报间隔
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevRtdRptIntvlSet(buf,extparam.buf);
		break;    
		case AFN_QRY_DATUPLOADINTERVAL:// 查询遥测终端的实时数据上报间隔
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevRtdRptIntvlQry(buf,extparam.buf);
		break;    
		case AFN_SET_UPDATE:// 设置遥测终端的升级数据
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevUpdSet(buf,extparam.buf);
		break;    
		case AFN_QRY_VERINFO:// 查询遥测终端的版本信息
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevVerInfoQry(buf,extparam.buf);
		break;    
		case AFN_SET_PASSWORD:// 设置遥测终端的密码
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevPwdSet(buf,extparam.buf);
		break;    
		case AFN_QRY_PASSWORD:// 查询遥测终端的密码
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevPwdQry(buf,extparam.buf);
		break;    
		case AFN_CHG_FINGER:// 指纹数据变更
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmFngrDatSet(buf,extparam.buf);
		break;    
		case AFN_DEL_FINGER:// 删除指纹数据
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmFngrDatDel(buf,extparam.buf);
        break;
        case AFN_RESTART:       // 遥测终端重启
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmRestart(buf,extparam.buf);
            while (1);
        #ifdef TOWERBOX
        case AFN_SET_LOCATION:// 设置遥测终端地理位置/经纬度
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevLctSet(buf,extparam.buf);
        break;
        case AFN_QRY_LOCATION:// 查询遥测终端GPS位置
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevLctQry(buf,extparam.buf);
		break;    
		case AFN_SET_TOWERPARAM:// 设置塔机静态结构参数
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrInfoSet(buf,extparam.buf);
		break;    
		case AFN_QRY_TOWERPARAM:// 查询塔机静态结构参数
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrInfoQry(buf,extparam.buf);
		break;    
		case AFN_SET_PROTECTIONZONE:// 设置塔机保护区
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrPrtcZoneSet(buf,extparam.buf);
		break;    
		case AFN_QRY_PROECTIONZONE:// 查询塔机保护区
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrPrtcZoneQry(buf,extparam.buf);
		break;    
		case AFN_SET_LIMIT:// 设置塔机限位信息
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrLmtSet(buf,extparam.buf);
		break;    
		case AFN_QRY_LIMIT:// 查询塔机限位信息
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrLmtQry(buf,extparam.buf);
		break;    
		case AFN_SET_MOMENTCURVE:// 设置塔机力矩曲线
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrTorqSet(buf,extparam.buf);
		break;    
		case AFN_QRY_MOMENTCURVE:// 查询塔机力矩曲线
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrTorqQry(buf,extparam.buf);
		break;    
		case AFN_SET_CALIBRATPARAM:// 设置塔机标定参数
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrCaliSet(buf,extparam.buf);
		break;    
		case AFN_QRY_CALIBRATPARAM:// 查询塔机标定参数
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrCaliQry(buf,extparam.buf);
		break;    
		case AFN_SET_TOWERLIFT:// 设置塔机顶升数据
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrLiftSet(buf,extparam.buf);
		break;    
		case AFN_QRY_TOWERLIFT:// 查询塔机顶升数据
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrLiftQry(buf,extparam.buf);
		break;
        #endif
        #ifdef ELIVATOR
        case AFN_SET_ELVTINFO:// 设置升降机基本结构参数
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmElvtInfoSet(buf,extparam.buf);
		break;    
		case AFN_QRY_ELVTINFO:// 查询升降机基本结构参数
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmElvtInfoQry(buf,extparam.buf);
		break;	  
		case AFN_SET_ELVTFLOOR:	// 设置升降机楼层参数
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmElvtFloorSet(buf,extparam.buf);
		break;    
		case AFN_QRY_ELVTFLOOR:// 查询升降机楼层参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmElvtFloorQry(buf,extparam.buf);
        break;
        #endif
        #ifdef DUSTMON
        case AFN_SET_VALVELMT:
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmValveLmtSet(buf,extparam.buf);
		break;    
		case AFN_QRY_VALVELMT:
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmValveLmtQry(buf, extparam.buf);
            break;
        case AFN_SET_MANVALVE:
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmValveManual(buf,extparam.buf);
        break;
        case AFN_SET_VALVELMT_EXT:
            extparam.SerType = COMTYPE_SET;
            FrmValveLmtSet_Ext(buf, extparam.buf);
        break;
        case AFN_QRY_VALVELMT_EXT:
            extparam.SerType = COMTYPE_QUERY;
            FrmValveLmtQry_Ext(buf, extparam.buf);
        break;
        case AFN_SET_NOTICE:
            extparam.SerType = COMTYPE_QUERY;
            FrmNotice(buf, extparam.buf);
        break;
        #endif
		#ifdef UPPLAT
        case AFN_SET_UPLMT:
            extparam.SerType = COMTYPE_SET;
            FrmUPLmtSet(buf, extparam.buf);
        break;
        case AFN_QRY_UPLMT:
            extparam.SerType = COMTYPE_QUERY;
            FrmUPLmtQry(buf, extparam.buf);
        break;
		#endif 

        default:
			return;
	}    
	// 主动下行报文的回调函数为上行应答帧启动做准备

    // 组织应答帧
    extparam.timeout = ATOVERTM; // ATOVERTMX20ms=5S
    extparam.lock = 1;  // 锁定数据，下行报文优先级高于主动上报报文
    NETWORK_SENDAT_CIPSEND(1);    // 如果上次应答未成功收到，避免和应答冲突，使用加急命令，丢弃之前发送队列中未处理帧
    //Add2RxSerArray(&rt);

    gprs_rssi = 3;
    _LED_COM_ON();
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}

/*
*********************************************************************************************************
*   函 数 名: my_strstr
*
*   功能说明: strstr函数的原型，在S1字符串中找到s2字符串首次出现的地址
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
char* my_strstr(const char *s1, const char *s2)
{
	if (*s1 == 0) //如果 数组s1中没有字符串出现，s2中有字符串返回空，如果两个字符串均为空，返回s1首地址，也算找到匹配
	{
		if (*s2) return (char *)NULL;
		return (char *)s1;
	}
	while (*s1) // 直到s1中结束符出现
	{
		size_t i;
		i = 0;
		while (1)
		{
			if (s2[i] == 0)
			{
				return (char *)s1;
			}
			if (s2[i] != s1[i])
			{
				break;
			}
			i++;
		}
		s1++;
	}
	return (char *)NULL;
}


/*
*********************************************************************************************************
*   函 数 名: my_arraystr
*
*   功能说明: strstr函数的改进型，在S1数组中找到s2数组首次出现的地址
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint8_t* my_arrayarray(uint8_t *a1, uint32_t a1_len, uint8_t *a2, uint32_t a2_len)
{

	uint32_t i, j;
	uint8_t *k = NULL, ret = 0;

	if (a1_len < a2_len) return NULL;   // 被查找数组小于查找数组退出

	for (j = 0; j < a1_len - a2_len + 1; j++)
	{
		if (a1[j] == a2[0])
		{
			ret = 0;
			k = a1 + j;
			for (i = 0; i < a2_len; i++)
			{
				if (a1[j + i] != a2[i])
				{
					ret++;
					break;
				}
			}
			if (!ret) return k;
		}
	}

	return NULL;
}


/*
*********************************************************************************************************
*   函 数 名: cmp_char
*
*   功能说明: 字符串比较函数，查询被比较字符串中是否存在比较字符串
*
*   形   参: uint8_t *m —— 被比较字符串
*            uint8_t *s —— 比较字符串
*            uint32_t num —— 被比较字符串长度
*
*   返 回 值: 0,不匹配；匹配串启始地址
*
*********************************************************************************************************
*/
uint32_t cmp_char(pRingBuf m, uint8_t *s)
{
	char *res;
	uint32_t size;
	uint32_t ret;
	uint8_t ss[QUEUE_REV_SIZE];

	if (m->in == m->out) return 0;

	if (m->in > m->out)
	{
		size = m->in - m->out;
		memcpy(ss, &m->buffer[m->out & (m->size - 1)], size);
	}
	else
	{
		size = (m->size - m->out) + m->in;
		memcpy(ss, &m->buffer[m->out & (m->size - 1)], m->size - m->out);
		memcpy(ss + (m->size - m->out), m->buffer, m->in);
	}

	res = (char *)my_arrayarray(ss, size, s, (uint32_t)strlen((const char *)s));;
	if (res)
	{
		ret = ((uint32_t)res - (uint32_t)ss + (uint32_t)strlen((const char *)s));
		m->out += ret;
		m->out &= (m->size - 1);
	}
	else ret = 0;

	//free(ss);

	return ret;
}

/*
*********************************************************************************************************
*   函 数 名: GET_CSQ
*
*   功能说明: 接收服务接收到"+CSQ: "后调用的回调函数，
* 
*             获取信号强度值并保存
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GET_CSQ(uint32_t par1, uint32_t par2)
{
    uint8_t ss[2];

    while ((recvfifo_5.in - recvfifo_5.out) < 2){;}
    __ring_buffer_get(&recvfifo_5, ss, 2);

    DTU_rssi = GSM_StringToDec((char *)ss, 2);
}

// for print
void novar_print(uint8_t *buf, uint16_t len)
{
/*
	if (!StatusFlag.STAT_PRINT_BUSY)
	{
		HAL_UART_Transmit_IT(HDL_UART_232, buf, len);
		StatusFlag.STAT_PRINT_BUSY = 1;
	}
*/
	//HAL_UART_Transmit(HDL_UART_232, buf, len, 500);
}
/************************************************************************************************************************* 
*函数         :   uint32_t GSM_StringToHex(char *pStr, uint8_t NumDigits) 
*功能         :   将16进制样式字符串转换为16进制整型数(必须保证字符串字母都是大写) 
*参数         :   pStr:字符串起始指针 
*                   NumDigits:数字位数,16进制数字位数 
*返回         :   转换后的数字 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-10-17 
*说明         :   比如字符串"A865"转换后为0xA865,位数为4位 
					必须保证字符串字母都是大写 
*************************************************************************************************************************/
uint32_t GSM_StringToHex(char *pStr, uint8_t NumDigits)
{
	uint8_t temp;
	uint32_t HEX = 0;
	uint8_t i;

	NumDigits = (NumDigits > 8) ? 8 : NumDigits; //最大支持8位16进制数

	for (i = 0; i < NumDigits; i++)
	{
		HEX <<= 4;
		temp = pStr[i];
		temp = (temp > '9') ? temp - 'A' + 10 : temp - '0';
		HEX |= temp;
	}
	return HEX;
}


/************************************************************************************************************************* 
*函数         :   void GSM_HexToString(uint32_t HexNum,c har *pStr, uint8_t NumDigits) 
*功能         :   将整型数字转换为16进制样式字符串(字母为大写,不带结束符) 
*参数         :   HexNum:16进制数字 
					pStr:字符缓冲区指针 
*                   NumDigits:数字位数,16进制数字位数 
*返回         :   无 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-04-30 
*说明         :   比如字符串0xA865转换后为"A865",位数为4位 
*************************************************************************************************************************/
void GSM_HexToString(uint32_t HexNum, char *pStr, uint8_t NumDigits)
{
	uint8_t temp;
	uint8_t i;

	NumDigits = (NumDigits > 8) ? 8 : NumDigits; //最大支持8位16进制数

	for (i = 0; i < NumDigits; i++)
	{
		temp = 0x0f & (HexNum >> (4 * (NumDigits - 1 - i)));
		temp = (temp > 0x09) ? (temp - 0x0A + 'A') : (temp + '0');
		pStr[i] = temp;
	}
}




/************************************************************************************************************************* 
*函数         :   uint32_t GSM_StringToDec(char *pStr, uint8_t NumDigits) 
*功能         :   将10进制样式字符串转换为整型数(必须保证完全为数字字符) 
*参数         :   pStr:字符串起始指针 
*                   NumDigits:数字位数,10进制数字位数 
*返回         :   转换后的数字 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-04-30 
*说明         :   比如字符串"1865"转换后为1865,位数为4位 
					必须保证完全为数字字符 
*************************************************************************************************************************/
uint32_t GSM_StringToDec(char *pStr, uint8_t NumDigits)
{
	uint32_t temp;
	uint32_t DEC = 0;
	uint8_t i;
	uint8_t j;

	NumDigits = (NumDigits > 10) ? 10 : NumDigits;   //最大支持10位10进制数

	for (i = 0; i < NumDigits; i++)
	{
		temp = pStr[i] - '0';
		if (temp > 9)         //只能是数字范围
			return 0;
		for (j = 1; j < (NumDigits - i); j++)
		{
			temp *= 10;
		}
		DEC += temp;
	}
	return DEC;
}

#undef  _LOCAL_NETWORK

/*****************************  END OF FILE  *********************************/

