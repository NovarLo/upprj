#define _LOCAL_PRTC
/*TIP —— tower information protocol */

#include "stm32f4xx_hal.h"
#include "app_fifo.h"
#include "app_user.h"
#include "app_crc.h"
#include "app_bcd.h"
#include "string.h"
#include "stdio.h"
#include "app_network.h"
#include "math.h"
#include "app_sensor.h"
#include "app_comser.h"
#include "app_finger.h"
#include "app_prtc.h"


//extern RTC_HandleTypeDef hrtc;
//extern RNG_HandleTypeDef hrng;
/*
*********************************************************************************************************
*   函 数 名: TIP_init
*
*   功能说明: 初始化
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void TIP_init(void)
{

    // 初始化发送和接收环形缓冲区
    recvfifo_5.buffer = recvbuf_5;
    memset(recvfifo_5.buffer, 0, QUEUE_REV_SIZE);
    recvfifo_5.size = QUEUE_REV_SIZE;
    recvfifo_5.in = 0;
    recvfifo_5.out = 0;

    sendfifo_5.buffer = sendbuf_5;
    memset(sendfifo_5.buffer, 0, BUFFERSIZE);
    sendfifo_5.size = BUFFERSIZE;
    sendfifo_5.in = 0;
    sendfifo_5.out = 0;

    // 初始化多帧应用数据FIFO
    mfappfifo_5.buffer = mfappbuf_5;
    memset(mfappfifo_5.buffer, 0, BUFFERSIZE);
    mfappfifo_5.size = BUFFERSIZE;
    mfappfifo_5.in = 0;
    mfappfifo_5.out = 0;

    // 初始化标志位
    StatusFlag.STAT_PRINT_BUSY = 0;
    StatusFlag.STAT_GPRSSEND_BUSY = 0;  // 允许发送
    StatusFlag.STAT_GPRS_ZPPP = 1;  // 初始化GPRS 未连接
    StatusFlag.STAT_HEART_OV = 0;   // 允许发送心跳包
    StatusFlag.STAT_CSQ_CHK =1;     // 
    StatusFlag.STAT_LINK_OK = 0;    // 链接成功标志，初始化为未连接
    StatusFlag.resend_start = HAL_GetTick();    // 重发机制的启始节拍在发送完帧结构开始赋值
    StatusFlag.resend_interval = RESEND_INTERVAL;   // 重发机制的时间间隔定义为常数
    StatusFlag.resend_times = RESEND_TIMES; // 最大重发次数为3
    StatusFlag.mframecnt = 0;   // 默认单帧
    StatusFlag.STAT_LINK_LOG = 1;   // =1表示链路需要初始化
    StatusFlag.STAT_WAIT_ACK = 0;   // =1表示等待应答
    StatusFlag.STAT_NO_INIT = 0;
    mrtn.mf_flag = 0;

    // 临时初始化调试帧用，正式版本删除
    {
        system_parameter.Taddr.a1_low = 01;
        system_parameter.Taddr.a1_high = 01;
        system_parameter.Taddr.a2_low = 02;
        system_parameter.Taddr.a2_middle = 02;
        system_parameter.Taddr.a2_high = 02;
        system_parameter.versioninfo.protocol.firstver = 0;
        system_parameter.versioninfo.protocol.secondver = 1;
    }
}

/*
*********************************************************************************************************
*   函 数 名: TIP_frame_get
*
*   功能说明: 从接收FIFO中取出一帧完整的帧
*
*   形   参: pRingBuf ringbuf —— 源接收缓冲区结构指针
*           void *buffer —— 目标接收帧数组缓冲区首地址
*           pTX101 pframe —— 目标结构指针
*
*   返 回 值: 0，该帧不符合链路规则要求，丢弃；
*            1，该帧为符合链路规则要求的完整帧，解码
*
*********************************************************************************************************
*/
uint8_t TIP_frame_get(pRingBuf ringbuf,uint8_t *buffer)
{
	uint8_t bytebuf, bytebuf1, *sptr, *dptr;
	uint32_t i, len = 0, in, out;

	sptr = buffer;
	// 首先取出关键字节看是否符合帧格式要求
	// 起始字符

	if (ringbuf->out == ringbuf->in) return TIP_FAIL;

	if (ringbuf->out < ringbuf->in)
	{
		in = ringbuf->in;
		out = ringbuf->out;
	}
	else
	{
		in = ringbuf->in + ringbuf->size;
		out = ringbuf->out;
	}

	// 如果有可读数据，直到找到启动字符
	for (i = out; i < in; i++)
	{
		bytebuf = ringbuf->buffer[i & (ringbuf->size - 1)];
		if (bytebuf == STARTCHAR) break;
	}
	if ((i + 2) > in) return TIP_FAIL;

	bytebuf1 = ringbuf->buffer[((i + 2) & (ringbuf->size - 1))];

	if (bytebuf1 != STARTCHAR) return TIP_FAIL;

	// 用户数据长度
	len = ringbuf->buffer[((i + 1) & (ringbuf->size - 1))];

	if ((in - i) < len + 5) return TIP_FAIL;

	if (ringbuf->buffer[((i + len + 4) & (ringbuf->size - 1))] != ENDCHAR) return TIP_FAIL; 

	// 读入完整帧信息
	ringbuf->out = i;
	__ring_buffer_get(ringbuf, sptr, len + 5);

	// 判断CRC校验位
	if (sptr[len + 3] != GetCRC7ByLeftByTable(&buffer[3], len)) return TIP_FAIL;

	// 有效帧数据放入帧结构
	// 链路部分
	sptr = buffer;
	if ((buffer[3] >> 6) & 0x01)    // 多帧处理
	{
		// 判断是第几帧
		mrtn.mframe_num = *((uint16_t *)(&buffer[4]));
		mrtn.mframe_cnt = *((uint16_t *)(&buffer[6]));
		if (mrtn.mframe_num == mrtn.mframe_cnt) // 总帧数和帧计数相等表示是第一帧数据
		{
			mrtn.mf_flag = 1;
			//StatusFlag.mframecnt = 1;	// 多帧标志
			mrtn.mframe_st = 1 << (mrtn.mframe_cnt - 1);   // 帧对应状态位置位

		}
		else
		{
			mrtn.mframe_st |= 1 << (mrtn.mframe_cnt - 1);   // 帧对应状态位置位
		}

		if (mrtn.mframe_cnt == 1 && mrtn.mframe_st != (uint16_t)(pow(2, mrtn.mframe_num) - 1)) // 多帧最后一帧没有收满，直接退出不予回复
		{
			mrtn.mf_flag = 0;   // 表示多帧接收失败
			return NULL;
		}
		mrtn.frame[mrtn.mframe_cnt - 1].startb1 = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].length = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].startb2 = *sptr++;
		dptr = (uint8_t *)&mrtn.frame[mrtn.mframe_cnt - 1].ctrlzone;
		*dptr++ = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].framenum = mrtn.mframe_num;
		mrtn.frame[mrtn.mframe_cnt - 1].framecnt = mrtn.mframe_cnt;
		sptr += 4;
		mrtn.frame[mrtn.mframe_cnt - 1].addrzone.a1_low = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].addrzone.a1_high = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].addrzone.a2_low = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].addrzone.a2_middle = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].addrzone.a2_high = *sptr++;
		dptr = (uint8_t *)&mrtn.frame[mrtn.mframe_cnt - 1].version;
		*dptr = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].appzone.functioncode = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].appzone.userdata = mrtnbuf[mrtn.mframe_cnt - 1];

		// 应用层数据部分(含数据区和附加域)
		dptr = mrtn.frame[mrtn.mframe_cnt - 1].appzone.userdata;
		for (i = 0; i < mrtn.frame[mrtn.mframe_cnt - 1].length - 12; i++)
		{
			*dptr++ = *sptr++;
		}
		mrtn.mlen[mrtn.mframe_cnt - 1] = mrtn.frame[mrtn.mframe_cnt - 1].length - 12; //该帧应用层缓冲区占用长度

		mrtn.frame[mrtn.mframe_cnt - 1].cs = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].endbyte = *sptr;
	}
	else    // 单帧处理
	{
		rtn.startb1 = *sptr++;
		rtn.length = *sptr++;
		rtn.startb2 = *sptr++;
		dptr = (uint8_t *)&rtn.ctrlzone;
		*dptr = *sptr++;
		rtn.addrzone.a1_low = *sptr++;
		rtn.addrzone.a1_high = *sptr++;
		rtn.addrzone.a2_low = *sptr++;
		rtn.addrzone.a2_middle = *sptr++;
		rtn.addrzone.a2_high = *sptr++;
		dptr = (uint8_t *)&rtn.version;
		*dptr = *sptr++;
		rtn.appzone.functioncode = *sptr++;

		// 应用层数据部分(含数据区和附加域)
		dptr = rtn.appzone.userdata = rtnbuf;
		for (i = 11; i < rtn.length + 3; i++)    // 8:控制域1+地址域5+协议版本1+应用层功能码
		{
			*dptr++ = *sptr++;
		}
		// CS+结束字符
		rtn.cs = *sptr++;
		rtn.endbyte = *sptr;
		//StatusFlag.mframecnt = 0;
		//mrtn.mf_flag=0;	// 接收到任一单帧，多帧标志清零
	}
	return len + 5;
}

/*
*********************************************************************************************************
*   函 数 名: TIP_login
*
*   功能说明: 协议登录，登录并等待确认，完成登录
*
*   形   参: none
*
*   返 回 值: 返回登录成功或者失败
*
*********************************************************************************************************
*/
void TIP_login(void)
{
    frame_link_chk(DATAFIELD_LOGIN);    // 发送登录帧

    // 收到确认帧后才能修改登录状态为登录成功
}

/*
*********************************************************************************************************
*   函 数 名: frame_link_chk
*
*   功能说明: 上行帧,回应链路检测报文。AFN=03H
*           数据域：1个字节，F0登录，F1退出登录，F2在线保持
*
*   形   参: pTX101 frame —— 接收到的下行帧
*           uint8 answer —— 1/0 确认/否认
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 frame_link_chk(uint8_t datazone)
{
    //uint8_t i;

    rtn.length = 9;
    link_layer_pack(&rtn,1,0,0,LFN_DIR0_LNKRESPON);

    // 因为数据域与下行报文的数据域相同，所以长度不变
    rtn.appzone.functioncode = AFN_LINKCHK;
    rtn.appzone.userdata = rtnbuf;
    rtnbuf[0] = datazone;
    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: frame_invalid
*
*   功能说明: 上行帧,回应无效请求报文。AFN=02H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 frame_invalid(pTX101 frame)
{
    //uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_DENYRESPON);

    // 未处理链路功能码

    // 因为数据域与下行报文的数据域相同，所以长度不变
    rtn.appzone.functioncode = AFN_INVALID;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamSetDevAddr
*
*   功能说明: 上行帧,回应设置遥测终端地址报文。AFN=10H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevAddr(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr;

    rtn = *frame;
    // 把设置的终端地址写入存储区
    //system_parameter.Taddr = *((pAddrZone)rtn.appzone.userdata);
    sptr = (uint8_t *)rtn.appzone.userdata;
    for (i=0;i<5;i++)
    {
        device_info.addr[i] = *sptr++;
    }
    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);
    rtn.length = 8;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamQryDevaddr
*
*   功能说明: 上行帧,回应查询遥测终端地址报文。AFN=50H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevaddr(pTX101 frame)
{
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);

    rtn.length = 8+5;   // 最基本的加上五个字节的地址
    rtn.appzone.functioncode = AFN_QRY_ADDRESS;
    for (i = 0; i < 5;i++)
    {
        rtnbuf[i] = device_info.addr[i];
    }
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamSetDevRtc
*
*   功能说明: 上行帧,回应设置遥测终端时钟报文。AFN=11H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevRtc(pTX101 frame)
{
    //uint32_t i;
    rtn = *frame;

    SetRTC((uint8_t *)rtn.appzone.userdata);
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_CLOCK;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamQryDevRtc
*
*   功能说明: 上行帧,回应查询遥测终端时钟报文。AFN=51H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevRtc(pTX101 frame)
{
    RtcClk clock;
    uint32_t i;


    rtn = *frame;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8+6;   // 最基本的加上六个字节的时钟
    rtn.appzone.userdata = rtnbuf;
    // 获取RTC时钟
    GetRTC(&clock);

    for (i = 0; i < 6;i++)
    {
        rtnbuf[i] = ((uint8_t *)(&clock))[i];
    }
    rtn.appzone.functioncode = AFN_QRY_CLOCK;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamSetWkMod
*
*   功能说明: 上行帧,回应设置遥测终端工作模式报文。AFN=12H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetWkMod(pTX101 frame)
{

    rtn = *frame;

    // 保存被设置的工作模式参数
    device_info.work_mod = *((uint8_t *)rtn.appzone.userdata);
    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_WORKMODE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryWkMod
*
*   功能说明: 上行帧,回应查询遥测终端工作模式报文。AFN=52H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryWkMod(pTX101 frame)
{
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8+1;   // 最基本的加上1个字节的工作模式
    *rtnbuf = device_info.work_mod;
    rtn.appzone.userdata = rtnbuf;


    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamSetSnsrTyp
*
*   功能说明: 上行帧,回应设置遥测终端的传感器/功能种类报文。AFN=13H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetSnsrTyp(pTX101 frame)
{

    uint32_t i,num;
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的工作模式参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    num =  *sptr++;

    for (i=1;i<num;i++)
    {
        switch (*sptr>>1)   // 协议传感器类型编号映射实际通道号
        {
        #ifdef TOWERBOX
        case PRO_SENS_ROTAT:
            device_info.sensor_en[SENS_ROTAT] = *sptr++;
            break;
        case PRO_SENS_MARGIN:
            device_info.sensor_en[SENS_MARGIN] = *sptr++;
            break;
        case PRO_SENS_TORQUE:
            device_info.sensor_en[SENS_TORQUE] = *sptr++;
            break;
        case PRO_SENS_WALK:
            device_info.sensor_en[SENS_WALK] = *sptr++;
            break;
        #endif
        #ifdef ELIVATOR
        case PRO_SENS_SPEED:
            device_info.sensor_en[SENS_SPEED] = *sptr++;
            break;
        case PRO_SENS_PEOPLE:
            device_info.sensor_en[SENS_PEOPLE] = *sptr++;
            break;
        #endif
        case PRO_SENS_HEIGHT:
            device_info.sensor_en[SENS_HEIGHT] = *sptr++;
            break;
        case PRO_SENS_WIND:
            device_info.sensor_en[SENS_WIND] = *sptr++;//协议编号+ 使能位
            break;
        case PRO_SENS_TILT:
            device_info.sensor_en[SENS_TILT] = *sptr++;
            break;
        case PRO_SENS_WEIGHT:
            device_info.sensor_en[SENS_WEIGHT] = *sptr++;
            break;
        case PRO_SENS_FINGER:
            device_info.sensor_en[SENS_FINGER] = *sptr++;
            break;
        case PRO_SENS_NOISE:
            device_info.sensor_en[SENS_NOISE] = *sptr++;
        default :
            break;
        }
    }

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_SENSORTYPE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQrySnsrTyp
*
*   功能说明: 上行帧,回应查询遥测终端的传感器种类报文。AFN=53H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQrySnsrTyp(pTX101 frame)
{
    uint32_t i;
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;

    //rtnbuf[0] = system_parameter.sensortype.sensornum;
    dptr =  rtnbuf;
    *dptr++ = 9;    //目前塔机黑匣子中有9种传感器类型
    rtn.length += 1;

    for (i=0;i<9;i++)
    {
        //只发送黑匣子实际使用传感器类型
        switch (i)
        {
        case PRO_SENS_WIND:*dptr = (PRO_SENS_WIND<<1)|(device_info.sensor_en[SENS_WIND]&0x01);break;
        case PRO_SENS_HEIGHT:*dptr = (PRO_SENS_HEIGHT<<1)|(device_info.sensor_en[SENS_HEIGHT]&0x01);break;
        case PRO_SENS_WEIGHT:*dptr = (PRO_SENS_WEIGHT<<1)|(device_info.sensor_en[SENS_WEIGHT]&0x01);break;
        case PRO_SENS_TILT:*dptr = (PRO_SENS_TILT<<1)|(device_info.sensor_en[SENS_TILT]&0x01);break;
        case PRO_SENS_FINGER:*dptr = (PRO_SENS_FINGER<<1)|(device_info.sensor_en[SENS_FINGER]&0x01);break;
        case PRO_SENS_NOISE:*dptr = (PRO_SENS_NOISE << 1) | (device_info.sensor_en[SENS_NOISE] & 0x01); break;
        #ifdef TOWERBOX
        case PRO_SENS_ROTAT:*dptr = (PRO_SENS_ROTAT<<1)|(device_info.sensor_en[SENS_ROTAT]&0x01);break;
        case PRO_SENS_MARGIN:*dptr = (PRO_SENS_MARGIN<<1)|(device_info.sensor_en[SENS_MARGIN]&0x01);break;
        case PRO_SENS_TORQUE:*dptr = (PRO_SENS_TORQUE<<1)|(device_info.sensor_en[SENS_TORQUE]&0x01);break;
        case PRO_SENS_WALK:*dptr = (PRO_SENS_WALK<<1)|(device_info.sensor_en[SENS_WALK]&0x01);break;
        #endif
        #ifdef ELIVATOR
        case PRO_SENS_SPEED:
            *dptr = (PRO_SENS_SPEED<<1)|(device_info.sensor_en[SENS_SPEED]&0x01);break;
        case PRO_SENS_PEOPLE:
            *dptr = (PRO_SENS_PEOPLE<<1)|(device_info.sensor_en[SENS_PEOPLE]&0x01);break;
        #endif
        }
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamSetSnsrCfg
*
*   功能说明: 上行帧,回应设置遥测终端的传感器参数报文。AFN=14H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetSnsrCfg(pTX101 frame)
{

    //uint32_t i;
    uint8_t *sptr,type;

    rtn = *frame;

    // 保存被设置的工作模式参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    type = *sptr++;

    switch (type) // sensor type
    {
    #ifdef TOWERBOX
        case PRO_SENS_WIND:
            device_info.wind_scale = *((uint16_t *)sptr) / 1000.0;
            APP_setcali_wind(device_info.wind_scale);
            cali_tbl.flag = CFGSTAT_SAVE;
            cali_tbl.delay = 300;
            break;
        case PRO_SENS_WEIGHT:
            device_info.weigth_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
            break;
        case PRO_SENS_FINGER:
            device_info.finger_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
            break;
        case PRO_SENS_ROTAT:
            device_info.rotat_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
            break;
        case PRO_SENS_HEIGHT:
            break;
        case PRO_SENS_TILT:
            break;
        case PRO_SENS_MARGIN:
            break;
        case PRO_SENS_TORQUE:
            break;
        case PRO_SENS_WALK:
            break;
    #endif
    #ifdef ELIVATOR
        case PRO_SENS_WIND:
    /*
            device_info.wind_scale = *((uint16_t *)sptr)/1000.0;
            APP_setcali_wind(0.0f);
            cali_tbl.flag = CFGSTAT_SAVE;
            cali_tbl.delay = 300;
    */
            break;
        case PRO_SENS_HEIGHT:
            // device_info.height_cfg = ;
            break;
        case PRO_SENS_WEIGHT:
    /*
            device_info.weigth_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_TILT:

            break;
        case PRO_SENS_FINGER:
    /*
            device_info.finger_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_SPEED:
    /*
            device_info.speed_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_PEOPLE:
    /*
            device_info.people_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
    #endif
    #ifdef DUSTMON
        case PRO_SENS_PM25:
            break;
        case PRO_SENS_PM10:
            break;
        case PRO_SENS_TEMP:
            break;
        case PRO_SENS_HMDT:
            break;
        case PRO_SENS_WIND:
            break; // 风速传感器 1float
        case PRO_SENS_NOISE:
            break;
    #endif
	#ifdef UPPLAT
		case PRO_SENS_UPWEIGHT:
			break;
		case PRO_SENS_UPLEFT:
			break;
		case PRO_SENS_UPRIGHT:
			break;
	#endif 

        default:
            return frame_invalid(&rtn);
    }
    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_SENSORPARAM;
    rtn.appzone.userdata = rtnbuf;
    rtnbuf[0] = type;
    rtn.length += 1;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQrySnsrCfg
*
*   功能说明: 上行帧,回应查询遥测终端的传感器参数报文。AFN=54H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQrySnsrCfg(pTX101 frame)
{
    //uint32_t i;
    uint8_t *sptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;

    sptr = (uint8_t *)rtn.appzone.userdata;

    rtn.length += 1;    // for sensor type
    switch (*sptr++)    // 传感器类型
    {
    #ifdef TOWERBOX
        case PRO_SENS_WIND:
            *((uint16_t *)sptr) = device_info.wind_scale * 1000;
            rtn.length += 2;
            break; // 风速传感器 1float
        case PRO_SENS_ROTAT:
            *sptr = device_info.rotat_cfg;
            rtn.length += 1;
            break; // 回转传感器 1Byte
        case PRO_SENS_WEIGHT:
            *sptr = device_info.weigth_cfg;
            rtn.length += 1;
            break; // 称重传感器 1Byte
        case PRO_SENS_FINGER:
            *sptr = device_info.finger_cfg;
            rtn.length += 1;
            break; // 指纹传感器 1Byte
        case PRO_SENS_HEIGHT:
            break;
        case PRO_SENS_TILT:
            break;
        case PRO_SENS_MARGIN:
            break;
        case PRO_SENS_TORQUE:
            break;
        case PRO_SENS_WALK:
            break;
    #endif
    #ifdef ELIVATOR
        case PRO_SENS_WIND:
    /*
            device_info.wind_scale = *((uint16_t *)sptr)/1000.0;
            APP_setcali_wind(0.0f);
            cali_tbl.flag = CFGSTAT_SAVE;
            cali_tbl.delay = 300;
    */
            break;
        case PRO_SENS_HEIGHT:

            break;
        case PRO_SENS_WEIGHT:
    /*
            device_info.weigth_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_TILT:

            break;
        case PRO_SENS_FINGER:
    /*
            device_info.finger_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_SPEED:
    /*
            device_info.speed_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
        case PRO_SENS_PEOPLE:
    /*
            device_info.people_cfg = *sptr;
            device_info.flag = CFGSTAT_SAVE;
            device_info.delay = 300;
    */
            break;
    #endif
    #ifdef DUSTMON
        case PRO_SENS_PM25:
            break;
        case PRO_SENS_PM10:
            break;
        case PRO_SENS_TEMP:
            break;
        case PRO_SENS_HMDT:
            break;
        case PRO_SENS_WIND:
            break; // 风速传感器 1float
        case PRO_SENS_NOISE:
            break;
    #endif
	#ifdef UPPLAT
		case PRO_SENS_UPWEIGHT:
			break;
		case PRO_SENS_UPLEFT:
			break;
		case PRO_SENS_UPRIGHT:
			break;
	#endif 

        default:
            return frame_invalid(&rtn);
    }
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamSetDevIpPort
*
*   功能说明: 上行帧,回应设置遥测终端存储的中心站IP地址和端口号报文。AFN=1FH
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevIpPort(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,*dptr;

    rtn = *frame;

    // 保存被设置的IP参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // ip
    dptr = device_info.ip_port[0];
    for (i=0;i<24;i++)
    {
        *dptr++ = *sptr++;
    }

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_DEVIPPORT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevIpPort
*
*   功能说明: 上行帧,回应遥测终端存储的中心站IP地址和端口号报文。AFN=5FH
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevIpPort(pTX101 frame)
{
    uint8_t *sptr,*dptr;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // IP
    sptr = device_info.ip_port[0];
    for (i=0;i<24;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 24;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetHrtIntvl
*
*   功能说明: 上行帧,回应设置心跳间隔报文。AFN=20H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetHrtIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的心跳间隔参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // 心跳间隔
    device_info.beat_time = UnPackBCD(sptr,2,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_HEARTINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryHrtIntvl
*
*   功能说明: 上行帧,回应设置遥测终端的保护区报文。AFN=60H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryHrtIntvl(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // 心跳间隔
    PackBCD(dptr,device_info.beat_time,2,0);

    rtn.length += 1;
    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevLnkReconIntvl
*
*   功能说明: 上行帧,回应设置终端链路重连间隔报文。AFN=21H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevLnkReconIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的重连间隔参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // 重连间隔
    device_info.recon_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_RECONNECTINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevLnkReconIntvl
*
*   功能说明: 上行帧,回应终端链路重连间隔报文。AFN=61H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevLnkReconIntvl(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // 重连间隔
    PackBCD(dptr,device_info.recon_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevRecIntvl
*
*   功能说明: 上行帧,回应设置历史数据存盘间隔报文。AFN=22H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevRecIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的数据上传间隔参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // 数据存盘间隔
    device_info.datsave_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_DATRECINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevRecIntvl
*
*   功能说明: 上行帧,回应历史数据存盘间隔报文。AFN=62H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevRecIntvl(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // 数据存盘间隔
    PackBCD(dptr,device_info.datsave_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevRTDReptIntvl
*
*   功能说明: 上行帧,回应设置实时数据上报间隔报文。AFN=23H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevRTDReptIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的数据上传间隔参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // 历史数据上传间隔
    device_info.datrpt_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_DATUPLOADINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevRTDReptIntvl
*
*   功能说明: 上行帧,回应实时数据上报间隔报文。AFN=63H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevRTDReptIntvl(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // 数据上传间隔
    PackBCD(dptr,device_info.datrpt_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevUpd
*
*   功能说明: 上行帧,回应设置遥测终端的升级数据报文。AFN=24H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevUpd(pTX101 frame)
{
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 远程更新数据
    // to be continued...
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_UPDATE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevVerInfo
*
*   功能说明: 上行帧,回应查询终端版本信息报文。AFN=64H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevVerInfo(pTX101 frame)
{
    uint8_t *sptr,*dptr;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_VERINFO;
    dptr = rtnbuf;

    // 设备代码 & 版本号
    sptr = device_ver.mfr;
    for (i=0;i<2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    sptr = device_ver.model;
    for (i=0;i<2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    *dptr++ = device_ver.ver_soft;
    *dptr++ = device_ver.ver_prtcl;
    *dptr++ = device_ver.ver_mmi;
    rtn.length += 3;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevPwd
*
*   功能说明: 上行帧,回应设置遥测终端密码报文。AFN=25H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevPwd(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,*dptr,type,Num;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 密码类型
    sptr = (uint8_t *)rtn.appzone.userdata;
    type = *sptr++;
    Num = *sptr++;
    switch (type)
    {
    case 0x00:  // 用户密码
        dptr = device_info.pswd[0];
        break;
    case 0x01:  // 管理员密码/调试密码
        dptr = device_info.pswd[1];
        break;
    case 0x08:
        dptr = device_info.pswd[2];
        break;
    case 0x09:
        dptr = device_info.pswd[3];
        break;
    default:break;
    }
    for (i=0;i<Num;i++)
    {
        *dptr++ = *sptr++;
    }

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_PASSWORD;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryDevPwd
*
*   功能说明: 上行帧,回应遥测终端的密码报文。AFN=65H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevPwd(pTX101 frame)
{
    uint8_t *sptr,*dptr,type,Num;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_PASSWORD;
    dptr = rtnbuf;

    // 提取所需类型密码
    type = *dptr++;
    rtn.length += 1;

    switch (type)
    {
    case 0x00:
        sptr = device_info.pswd[0];
        break;
    case 0x01:
        sptr = device_info.pswd[1];
        break;
    case 0x08:
        sptr = device_info.pswd[2];
        break;
    case 0x09:
        sptr = device_info.pswd[3];
        break;
    default:break;
    }
    *dptr++ = Num = strlen((const char *)sptr); // N
    rtn.length += 1;

    for (i=0;i<Num;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += Num;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

#ifdef TOWERBOX
/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrInfo
*
*   功能说明: 上行帧,回应设置遥测终端的塔机基本参数报文。AFN=16H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrInfo(pTX101 frame)
{

    uint32_t i;
    uint8_t *sptr,*dptr;
    //uint8_t bcd[5];

    rtn = *frame;

    // 保存被设置的塔机基本参数
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 塔机的名称
    dptr = (uint8_t *)&tower_info.name;

    for (i = 0; i < 16; i++)
    {
        *dptr++ = *sptr++;
    }

    // 塔机ID
    tower_info.tower_ID = *sptr++;

    // 塔群ID
    tower_info.group_ID = *sptr++;

    // 坐标X（3B）0.0~9999.91m
    tower_info.org_x = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //坐标Y（3B）0.0~9999.9m
    tower_info.org_y = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //塔机型号代码(2B)
    tower_info.mfr_ID = *sptr++;    // 厂商代码
    tower_info.model_ID = *sptr++;  // 型号代码

    //额定载重（1B）0.1t
    tower_info.rated_load = (*sptr++) * 100.0f;

    //塔机类型/倍率（1B）
    tower_info.ratio = (*sptr)&0xf;
    tower_info.type = *sptr++ >> 4;

    //前臂长度（3B）BCD2float 0.1m
    tower_info.front = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //后臂长度（3B）0.1m
    tower_info.front = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //前拉杆1位置（3B）0.1m
    tower_info.mast_front1 = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //前拉杆2位置（3B）0.1m
    tower_info.mast_front2 = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //后拉杆位置（3B）0.1m
    tower_info.mast_rear = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //塔臂下沿高度（3B）0.1m
    tower_info.height = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //塔臂自身高度（2B）0.1m
    tower_info.thick = UnPacksBCD(sptr,3,1);
    sptr += 2;

    //塔臂上沿到塔尖高度（2B）0.1m
    tower_info.topheight = UnPacksBCD(sptr,3,1);
    sptr += 2;

    //回转惯性系数（1B）1~99
    tower_info.inertia = UnPacksBCD(sptr,2,0);
    sptr += 1;

    //行走角度（2B）0.1度
    tower_info.topheight = UnPackBCD(sptr,4,1);
    sptr += 2;

    tower_info.flag = CFGSTAT_SAVE;
    tower_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_TOWERPARAM;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryTwrInfo
*
*   功能说明: 上行帧,回应查询遥测终端的塔机基本参数报文。AFN=56H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryTwrInfo(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *dptr,*sptr,bcd[5];
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;

    // 读取塔机基本参数
    dptr = (uint8_t *)rtn.appzone.userdata;

    // 塔机名称
    sptr = tower_info.name;
    for (i = 0; i < 16; i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 16;

    // 塔机ID
    *dptr++ = tower_info.tower_ID;
    rtn.length += 1;

    // 塔群ID
    *dptr++ = tower_info.group_ID;
    rtn.length += 1;

    // 坐标X（3B）
    PacksBCD(bcd,tower_info.org_x,5,1);
    sptr = bcd;
    for (i=0;i<3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //坐标Y（3B）
    PacksBCD(bcd,tower_info.org_y,5,1);
    sptr = bcd;
    for (i=0;i<3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //塔机型号代码(2B)
    sptr = (uint8_t *)&tower_info.mfr_ID;   // 厂商代码
    *dptr++ = *sptr++;
    rtn.length += 1;

    sptr = (uint8_t *)&tower_info.model_ID; //型号代码
    *dptr++ = *sptr++;
    rtn.length += 1;

    //额定载重（1B）
    *dptr++ = tower_info.rated_load/100;    // 0.1吨
    rtn.length += 1;

    //塔机类型/倍率（1B）
    *dptr++ = tower_info.ratio|(tower_info.type<<4);
    rtn.length += 1;

    //前臂长度（3B）BCD2float
    PacksBCD(bcd,tower_info.front,5,1); // 1m
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //后臂长度（3B）
    PacksBCD(bcd,tower_info.rear,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //前拉杆1位置（3B）
    PacksBCD(bcd,tower_info.mast_front1,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //前拉杆2位置（3B）
    PacksBCD(bcd,tower_info.mast_front2,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //后拉杆位置（3B）
    PacksBCD(bcd,tower_info.mast_rear,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //塔臂下沿高度（3B）
    PacksBCD(bcd,tower_info.height,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //塔臂自身高度（2B）
    PacksBCD(bcd,tower_info.thick,3,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    //塔臂上沿到塔尖高度（2B）
    PacksBCD(bcd,tower_info.topheight,3,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    //回转惯性系数（1B）
    PacksBCD(bcd,tower_info.inertia,2,0);
    *dptr++ = bcd[i];
    rtn.length += 1;

    //行走角度（2B）
    PackBCD(bcd,tower_info.walk_dir,4,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   函 数 名: ParamSetPrtcZone
*
*   功能说明: 上行帧,回应设置遥测终端的保护区报文。AFN=17H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetPrtcZone(pTX101 frame)
{

    uint32_t i,j;
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的保护区信息参数
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 保护区个数信息
    zone_tbl.tblsize = *sptr++;

    // 保护区信息
    for (i=0;i<zone_tbl.tblsize;i++)
    {
        zone_tbl.zone[i].type = *sptr++;
        zone_tbl.zone[i].id = *sptr++;
        zone_tbl.zone[i].bld_typ = *sptr++;
        zone_tbl.zone[i].height = UnPacksBCD(sptr,5,1);
        sptr += 3;
        zone_tbl.zone[i].tblsize = *sptr++;
        for (j=0;j<zone_tbl.zone[i].tblsize;j++)
        {
            zone_tbl.zone[i].dat[j].type = *sptr++;
            switch (zone_tbl.zone[i].dat[j].type)
            {
            case 0:// 点元素
                zone_tbl.zone[i].dat[j].x = UnPacksBCD(sptr,5,1);
                sptr += 3;
                zone_tbl.zone[i].dat[j].y = UnPacksBCD(sptr,5,1);
                sptr += 3;
                break;
            case 1:// 圆弧,保留
                zone_tbl.zone[i].dat[j].x = UnPacksBCD(sptr,5,1);
                sptr += 3;
                zone_tbl.zone[i].dat[j].y = UnPacksBCD(sptr,5,1);
                sptr += 3;

                zone_tbl.zone[i].dat[j].radius = UnPackBCD(sptr,5,1);
                sptr += 3;
                zone_tbl.zone[i].dat[j].start_angle = UnPackBCD(sptr,3,1);
                sptr += 2;
                zone_tbl.zone[i].dat[j].end_angle = UnPackBCD(sptr,3,1);
                sptr += 2;
                break;
            default:
                break;
            }
        }
    }

    zone_tbl.flag = CFGSTAT_SAVE;
    zone_tbl.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_PROTECTIONZONE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryPrtcZone
*
*   功能说明: 上行帧,回应查询遥测终端的保护区报文。AFN=57H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryPrtcZone(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *sptr,*dptr;
    uint32_t i,j,prot_len=0;
    uint16_t pagenum,lastpage;


    // 保护区信息很可能需要多帧发送
    // 首先计算应用层数据区信息，判断分几帧发送，单帧最长发送数据长度为255-12=243

    // 整合保护区信息到多帧缓冲区
    sptr = mrtnbuffer;
    // 保护区个数
    *sptr++ = zone_tbl.tblsize;
    prot_len += 1;
    // 保护区信息
    for (i=0;i<zone_tbl.tblsize;i++)
    {
        *sptr++ = zone_tbl.zone[i].type;
        *sptr++ = zone_tbl.zone[i].id;
        *sptr++ = zone_tbl.zone[i].bld_typ;
        PacksBCD(sptr,zone_tbl.zone[i].height,5,1);
        sptr += 3;
        *sptr++ = zone_tbl.zone[i].tblsize;
        prot_len += 7;
        for (j=0;j<zone_tbl.zone[i].tblsize;j++)
        {
            switch (zone_tbl.zone[i].type)
            {
            case 0 :
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].x,5,1);
                sptr += 3;
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].y,5,1);
                sptr += 3;
                prot_len += 6;
                break;
            case 1:
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].x,5,1);
                sptr += 3;
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].y,5,1);
                sptr += 3;
                PackBCD(sptr,zone_tbl.zone[i].dat[j].radius,5,1);
                sptr += 3;
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].start_angle,4,1);
                sptr += 2;
                PacksBCD(sptr,zone_tbl.zone[i].dat[j].end_angle,4,1);
                sptr += 2;
                prot_len += 13;
                break;
            default:
                break;
            }
        }
    }
    lastpage = prot_len%243;
    if (lastpage) pagenum = prot_len/243+1;
    else pagenum = prot_len/243;

    // 如果pagenum大于1，为多帧，=1为单帧

    if (pagenum > 1)
    {
        mrtn.mf_flag = 1;
        mrtn.mframe_num = pagenum;

        for (i=0;i<pagenum;i++)
        {
            mrtn.mlen[i] = 243;
            mrtn.frame[i].startb1 = mrtn.frame[i].startb2 = STARTCHAR;
            mrtn.frame[i].endbyte = ENDCHAR;
            mrtn.frame[i].length = mrtn.mlen[i]+12;
            mrtn.frame[i].ctrlzone.dir = 1;
            mrtn.frame[i].ctrlzone.div = 1;
            mrtn.frame[i].ctrlzone.fcb = 3;
            mrtn.frame[i].ctrlzone.func = LFN_DIR1_PARAMRESPON;
            mrtn.frame[i].framenum = pagenum;
            mrtn.frame[i].framecnt = pagenum-i;
            mrtn.frame[i].addrzone = *GetTAddr();
            mrtn.frame[i].appzone.functioncode = AFN_QRY_PROECTIONZONE;
            mrtn.frame[i].appzone.userdata = mrtnbuf[i];
        }
        if (lastpage)
        {
            mrtn.mlen[i] = lastpage;
            mrtn.frame[i].length = mrtn.mlen[i]+12;
        }
        // 往帧应用数据缓冲区中置数

        sptr = mrtnbuffer;
        for (i=0;i<pagenum;i++)
        {
            for (j=0;j<mrtn.mlen[i];j++)
            {
                mrtnbuf[i][j] = *sptr++;
            }
        }

        // 求校验
        GetCRC(1);
        return NULL;
    }
    else    // 单帧
    {
        rtn = *frame;

        link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
        rtn.length = 8;

        rtn.appzone.functioncode = AFN_QRY_PROECTIONZONE;
        rtn.length += 1;

        sptr = mrtnbuffer;
        dptr = rtnbuf;
        for (i=0;i<lastpage;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += lastpage;

        rtn.appzone.userdata = rtnbuf;

        // 重新计算CRC
        rtn.cs = GetCRC(0);
        return &rtn;
    }
    return NULL;    // for return
}

/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrLmt
*
*   功能说明: 上行帧,回应设置塔机限位信息报文。AFN=18H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrLmt(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的限位和保护参数
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 限位参数
    // 回转
    limit_tbl.limit[SENS_ROTAT].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_ROTAT].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_ROTAT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_ROTAT].lowarn = limit_tbl.limit[SENS_ROTAT].hiwarn;

    // 高度
    limit_tbl.limit[SENS_HEIGHT].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_HEIGHT].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_HEIGHT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_HEIGHT].lowarn = limit_tbl.limit[SENS_HEIGHT].hiwarn;

    // 幅度
    limit_tbl.limit[SENS_MARGIN].hilimit = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].lolimit = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].lowarn = limit_tbl.limit[SENS_MARGIN].hiwarn;

    // 行走
    limit_tbl.limit[SENS_WALK].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_WALK].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_WALK].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_WALK].lowarn = limit_tbl.limit[SENS_WALK].hiwarn;

    // 倾角
    limit_tbl.limit[SENS_TILT].hiwarn = UnPacksBCD(sptr,3,1);
    sptr += 2;
    limit_tbl.limit[SENS_TILT].hilimit = UnPacksBCD(sptr,3,1);
    sptr += 2;
/*
    limit_tbl.limit[SENS_TILT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_TILT].lowarn = limit_tbl.limit[SENS_TILT].hiwarn;
*/

    // 风速
    limit_tbl.limit[SENS_WIND].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_WIND].hilimit = UnPackBCD(sptr,4,1);
    sptr += 2;

    limit_tbl.flag = CFGSTAT_SAVE;
    limit_tbl.delay = 300;
/*
    limit_tbl.limit[SENS_WIND].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_WIND].lowarn = limit_tbl.limit[SENS_WIND].hiwarn;
*/

    // 预留12B
    sptr += 12;

    // 保护参数
    // 回转
    protect_tbl.protect[SENS_ROTAT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_ROTAT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // 高度
    protect_tbl.protect[SENS_HEIGHT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_HEIGHT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // 幅度
    protect_tbl.protect[SENS_MARGIN].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_MARGIN].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // 行走
    protect_tbl.protect[SENS_WALK].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_WALK].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // 塔身
    protect_tbl.protect[SENS_BODY].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_BODY].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

/*
    // 倾角
    protect_tbl.protect[SENS_TILT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_TILT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // 风速
    protect_tbl.protect[SENS_WIND].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_WIND].warn = UnPackBCD(sptr,4,1);
    sptr += 2;
*/

    protect_tbl.flag = CFGSTAT_SAVE;
    protect_tbl.delay = 300;

    // 预留12B
    sptr += 12;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_LIMIT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryTwrLmt
*
*   功能说明: 上行帧,回应查询塔机限位信息报文。AFN=58H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryTwrLmt(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *dptr;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;

    // 读取限位和保护参数
    dptr = (uint8_t *)rtn.appzone.userdata;

    // 限位参数
    // 回转
    PacksBCD(dptr,limit_tbl.limit[SENS_ROTAT].hilimit,5,1); // 左限位
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_ROTAT].lolimit,5,1); // 右限位
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_ROTAT].hiwarn,4,1);   // 预警值
    dptr += 2;
    rtn.length += 8;

    // 高度
    PacksBCD(dptr,limit_tbl.limit[SENS_HEIGHT].hilimit,5,1);// 高限位
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_HEIGHT].lolimit,5,1);// 低限位
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_HEIGHT].hiwarn,4,1);  // 预警值
    dptr += 2;
    rtn.length += 8;

    // 幅度
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].hilimit,4,1); // 远限位
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].lolimit,4,1); // 近限位
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].hiwarn,4,1);  // 预警值
    dptr += 2;
    rtn.length += 6;

    // 行走
    PacksBCD(dptr,limit_tbl.limit[SENS_WALK].hilimit,5,1);  // 前限位
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_WALK].lolimit,5,1);  // 后限位
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_WALK].hiwarn,4,1);    // 预警值
    dptr += 2;
    rtn.length += 8;

    // 倾角
    PacksBCD(dptr,limit_tbl.limit[SENS_TILT].hiwarn,3,1);   // 预警值
    dptr += 2;
    PacksBCD(dptr,limit_tbl.limit[SENS_TILT].hilimit,3,1);  // 报警值
    dptr += 2;
/*
    PackBCD(dptr,limit_tbl.limit[SENS_TILT].hiwarn,4,1);
    dptr += 2;
*/
    rtn.length += 4;

    // 风速
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hiwarn,4,1);    // 预警值
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hilimit,4,1);   // 报警值
    dptr += 2;
/*
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hiwarn,4,1);
    dptr += 2;
*/
    rtn.length += 4;

    // 预留12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    // 保护参数
    // 回转
    PackBCD(dptr,protect_tbl.protect[SENS_ROTAT].alarm,4,1);    // 保护值
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_ROTAT].warn,4,1);     // 预警值
    dptr += 2;
    rtn.length += 4;

    // 高度
    PackBCD(dptr,protect_tbl.protect[SENS_HEIGHT].alarm,4,1);   // 保护值
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_HEIGHT].warn,4,1);    // 预警值
    dptr += 2;
    rtn.length += 4;

    // 幅度
    PackBCD(dptr,protect_tbl.protect[SENS_MARGIN].alarm,4,1);   // 保护值
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_MARGIN].warn,4,1);    // 预警值
    dptr += 2;
    rtn.length += 4;

    // 行走
    PackBCD(dptr,protect_tbl.protect[SENS_WALK].alarm,4,1);     // 保护值
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_WALK].warn,4,1);      // 预警值
    dptr += 2;
    rtn.length += 4;


    // 塔身
    PackBCD(dptr,protect_tbl.protect[SENS_BODY].alarm,4,1);     // 保护值
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_BODY].warn,4,1);      // 预警值
    dptr += 2;
    rtn.length += 4;
    /*
    // 倾角
    PackBCD(dptr,protect_tbl.protect[SENS_TILT].alarm,4,1);
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_TILT].warn,4,1);
    dptr += 2;
    rtn.length += 4;

    // 风速
    PackBCD(dptr,protect_tbl.protect[SENS_WIND].alarm,4,1);
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_WIND].warn,4,1);
    dptr += 2;
    rtn.length += 4;
*/

    // 预留12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrTorque
*
*   功能说明: 上行帧,回应设置力矩曲线报文。AFN=19H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrTorque(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr;

    rtn = *frame;

    // 保存被设置的力矩曲线
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 倍率
    tower_info.ratio = *sptr++;

    // 力矩曲线点数
    torque_tbl.tblsize = *sptr++;

    // 每个点的幅度和重量
    for (i=0; i<torque_tbl.tblsize;i++)
    {
        torque_tbl.dat[i].distance = UnPackBCD(sptr,4,1);
        sptr += 2;
        torque_tbl.dat[i].weight = UnPackBCD(sptr,6,3)*1000;//公斤
        sptr += 3;
    }

    torque_tbl.flag = CFGSTAT_SAVE;
    torque_tbl.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8+1;
    rtn.appzone.functioncode = AFN_SET_MOMENTCURVE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryTwrTorque
*
*   功能说明: 上行帧,回应查询力矩曲线报文。AFN=59H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryTwrTorque(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *dptr;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;

    // 读取限位和保护参数
    dptr = (uint8_t *)rtn.appzone.userdata;

    // 倍率
    *dptr++ = tower_info.ratio;
    rtn.length += 1;

    // 力矩曲线点数
    *dptr++ = torque_tbl.tblsize;
    rtn.length += 1;

    // 每个点的幅度和重量
    for (i=0; i<torque_tbl.tblsize;i++)
    {
        PackBCD(dptr,torque_tbl.dat[i].distance,4,1);
        dptr += 2;
        PackBCD(dptr,torque_tbl.dat[i].weight/1000,6,3);
        dptr += 3;
        rtn.length += 5;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrCali
*
*   功能说明: 上行帧,回应设置塔基标定参数报文。AFN=1AH
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrCali(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,sensor_type,Num;
    APP_CALICHDAT_TypeDef *ts;

    rtn = *frame;

    // 保存被设置的标定参数
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 传感器类型
    sensor_type = *sptr++;
    // 传感器点数
    Num = *sptr++;

    switch (sensor_type)
    {
        case PRO_SENS_WIND:
            ts = &cali_tbl.chdat[SENS_WIND];
            break;
        case PRO_SENS_ROTAT:
            ts = &cali_tbl.chdat[SENS_ROTAT];
            break;
        case PRO_SENS_MARGIN:
            ts = &cali_tbl.chdat[SENS_MARGIN];
            break;
        case PRO_SENS_HEIGHT:
            ts = &cali_tbl.chdat[SENS_HEIGHT];
            break;
        case PRO_SENS_WEIGHT:
            ts = &cali_tbl.chdat[SENS_WEIGHT];
            break;
        case PRO_SENS_TORQUE:
            ts = &cali_tbl.chdat[SENS_TORQUE];
            break;
        case PRO_SENS_TILT:
            ts = &cali_tbl.chdat[SENS_TILT];
            break;
        case PRO_SENS_WALK:
            ts = &cali_tbl.chdat[SENS_WALK];
            break;
        case PRO_SENS_FINGER:
            ts = &cali_tbl.chdat[SENS_FINGER];
            break;
        default :
            break;
    }
    ts->tblsize = Num;
    for (i=0;i<Num;i++)
    {
        ts->dat[i].x = UnPackBCD(sptr, 6, 0);
        sptr += 3;
        ts->dat[i].y = UnPackBCD(sptr, 6, 3);
        sptr += 3;
        if (sensor_type == PRO_SENS_HEIGHT)
        {
            ts->dat[i].y = tower_info.height - ts->dat[i].y;
        }
        if (sensor_type == PRO_SENS_ROTAT)
        {
            APP_setcali_rotate(ts->dat[0].x, ts->dat[0].y);
        }
    }
    cali_tbl.flag = CFGSTAT_SAVE;
    cali_tbl.delay = 300;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_CALIBRATPARAM;
    rtn.appzone.userdata = rtnbuf;
    rtnbuf[0] = sensor_type;
    rtn.length += 1;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryTwrCali
*
*   功能说明: 上行帧,回应查询塔机标定参数报文。AFN=5AH
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryTwrCali(pTX101 frame)
{
    uint8_t *dptr,sensor_type;
    uint32_t i;
    APP_CALICHDAT_TypeDef *ts;
    float high;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // sensor type
    sensor_type = rtnbuf[0];
    *dptr++ = sensor_type;
    rtn.length += 1;

    switch (sensor_type)
    {
    case PRO_SENS_WIND:
        ts = &cali_tbl.chdat[SENS_WIND];
        break;
    case PRO_SENS_ROTAT:
        ts = &cali_tbl.chdat[SENS_ROTAT];
        break;
    case PRO_SENS_MARGIN:
        ts = &cali_tbl.chdat[SENS_MARGIN];
        break;
    case PRO_SENS_HEIGHT:
        ts = &cali_tbl.chdat[SENS_HEIGHT];
        break;
    case PRO_SENS_WEIGHT:
        ts = &cali_tbl.chdat[SENS_WEIGHT];
        break;
    case PRO_SENS_TORQUE:
        ts = &cali_tbl.chdat[SENS_TORQUE];
        break;
    case PRO_SENS_TILT:
        ts = &cali_tbl.chdat[SENS_TILT];
        break;
    case PRO_SENS_WALK:
        ts = &cali_tbl.chdat[SENS_WALK];
        break;
    case PRO_SENS_FINGER:
        ts = &cali_tbl.chdat[SENS_FINGER];
        break;
    default :
        break;
    }
    *dptr++ = ts->tblsize;
    rtn.length += 1;

    for (i = 0; i < ts->tblsize;i++)
    {
        PackBCD(dptr, ts->dat[i].x, 6, 0);
        dptr += 3;
        PackBCD(dptr, ts->dat[i].y, 6, 3);
        if (sensor_type == PRO_SENS_HEIGHT)
        {
            high = tower_info.height - ts->dat[i].y;
            PackBCD(dptr, high, 6, 3);
        }
        dptr += 3;
        rtn.length += 6;
    }


    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrLift
*
*   功能说明: 上行帧,回应设置遥测终端的塔机顶升数据报文。AFN=26H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrLift(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 塔机顶升数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    tower_info.last_lift = UnPacksBCD(sptr,3,1);

    tower_info.flag = CFGSTAT_SAVE;
    tower_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_TOWERLIFT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetTwrLmt
*
*   功能说明: 上行帧,回应遥测终端的塔机顶升报文。AFN=66H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryTwrLift(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_TOWERLIFT;
    dptr = rtnbuf;

    // 塔机顶升数据
    PacksBCD(dptr,tower_info.last_lift,3,1);
    dptr += 2;
    rtn.length += 2;

    *dptr++ = tower_info.lift_sec;
    *dptr++ = tower_info.lift_min;
    *dptr++ = tower_info.lift_hour;
    *dptr++ = tower_info.lift_date;
    *dptr++ = tower_info.lift_month;
    *dptr++ = tower_info.lift_year;
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetDevLct
*
*   功能说明: 上行帧,回应设置遥测终端的地理位置/经纬度报文。AFN=15H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetDevLct(pTX101 frame)
{

    uint32_t i;
    uint8_t *sptr,*dptr;

    rtn = *frame;

    // 保存被设置的工作模式参数
    sptr = (uint8_t *)rtn.appzone.userdata;
    // 经度
    dptr = tower_info.longitude;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    // 纬度
    dptr = tower_info.latitude;
    for (i = 0;i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    // 海拔
    dptr = tower_info.altitude;
    for (i = 0;i < 2;i++)
    {
        *dptr++ = *sptr++;
    }

    tower_info.flag = CFGSTAT_SAVE;
    tower_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_LOCATION;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryDevLct
*
*   功能说明: 上行帧,回应查询遥测终端的地理位置/经纬度报文。AFN=55H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryDevLct(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *sptr,*dptr;
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;
    // 经度
    sptr = tower_info.longitude;
    for (i=0;i<6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;
    // 纬度
    sptr = tower_info.latitude;
    for (i=0;i<6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;
    // 海拔
    sptr = tower_info.altitude;
    for (i=0;i<2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}
#endif


#ifdef ELIVATOR
/*
*********************************************************************************************************
*   函 数 名: ParamSetElvtInfo
*
*   功能说明: 设置升降机信息报文。AFN=30H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetElvtInfo(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 升降机基本结构数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    elivator_info.rated_load = UnPackBCD(sptr, 4, 0) / 10.0f;//0~9999kg
    sptr += 2;
    elivator_info.people = *sptr++;//0~255
    elivator_info.midweight = *sptr++;//0~255

    elivator_info.flag = CFGSTAT_SAVE;
    elivator_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_ELVTINFO;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryElvtInfo
*
*   功能说明: 查询升降机信息报文。AFN=70H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryElvtInfo(pTX101 frame)
{
    uint8_t *dptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    dptr = rtnbuf;

    // 升降机数据信息
    PackBCD(dptr,elivator_info.rated_load,4,0);
    dptr += 2;
    rtn.length += 2;
    *dptr++ = elivator_info.people;
    rtn.length += 1;
    *dptr++ = elivator_info.midweight;
    rtn.length += 1;

    rtn.appzone.functioncode = AFN_QRY_ELVTINFO;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetElvtFloor
*
*   功能说明: 设置升降机楼层信息报文。AFN=31H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetElvtFloor(pTX101 frame)
{
    uint8_t *sptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 升降机楼层数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    // 返回系数，高度偏置
    device_info.height_offset = (*sptr++) / 100.0f;

    // 楼层类型N
    floor_tbl.tblsize = *sptr++;

    // 类型N高度、层数
    for (i = 0; i < floor_tbl.tblsize; i++)
    {
        floor_tbl.type[i].height = *sptr++ / 10.0f;
        floor_tbl.type[i].number = *sptr++;
    }

    floor_tbl.flag = CFGSTAT_SAVE;
    floor_tbl.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_ELVTFLOOR;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamQryElvtFloor
*
*   功能说明: 查询升降机楼层信息报文。AFN=71H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryElvtFloor(pTX101 frame)
{
    uint8_t *dptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_TOWERLIFT;
    dptr = rtnbuf;

    // 升降机楼层数据

    // 返回系数，高度偏置
    *dptr++ = device_info.height_offset * 100.0f;
    rtn.length += 1;

    // 楼层类型N
    *dptr++ = floor_tbl.tblsize;
    rtn.length += 1;

    // 类型N高度、层数
    for (i = 0; i < floor_tbl.tblsize; i++)
    {
        *dptr++ = floor_tbl.type[i].height * 10.0f;
        *dptr++ = floor_tbl.type[i].number;
        rtn.length += 2;
    }

    rtn.appzone.functioncode = AFN_QRY_ELVTFLOOR;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif

#ifdef DUSTMON
/*
*********************************************************************************************************
*   函 数 名: ParamSetValveLmt
*
*   功能说明: 设置扬尘在线检测终端电磁阀阈值信息报文。AFN=40H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetValveLmt(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    for (i = 0; i < 4; i++)
    {
        dustmon_info.threshold[i] = UnPacksBCD(sptr, 5, 1); //0~9999.9ug/m3
        sptr += 3;
    }

    dustmon_info.flag = CFGSTAT_SAVE;
    dustmon_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_VALVELMT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryValveLmt
*
*   功能说明: 查询扬尘在线检测终端电磁阀阈值信息报文。AFN=80H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryValveLmt(pTX101 frame)
{
    uint8_t *dptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_VALVELMT;
    dptr = rtnbuf;

    // 4个电磁阀阈值

    for (i = 0; i < 4; i++)
    {
        PacksBCD(dptr, dustmon_info.threshold[i], 5, 1);  // float2BCD
        dptr += 3;
        rtn.length += 3;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamSetValveLmt_Ext
*
*   功能说明: 设置扬尘在线检测终端电磁阀扩展阈值(仅仅针对PM10)信息报文。AFN=40H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetValveLmt_Ext(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    for (i = 0; i < 4; i++)
    {
        dustmon_info.threshold[i+4] = UnPacksBCD(sptr, 5, 1); //0~9999.9ug/m3
        sptr += 3;
    }

    dustmon_info.flag = CFGSTAT_SAVE;
    dustmon_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_VALVELMT_EXT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryValveLmt_Ext
*
*   功能说明: 查询扬尘在线检测终端电磁阀阈值信息报文。AFN=80H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryValveLmt_Ext(pTX101 frame)
{
    uint8_t *dptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_VALVELMT_EXT;
    dptr = rtnbuf;

    // 4个电磁阀阈值

    for (i = 0; i < 4; i++)
    {
        PacksBCD(dptr, dustmon_info.threshold[i+4], 5, 1);  // float2BCD
        dptr += 3;
        rtn.length += 3;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: ParamSetValveMan
*
*   功能说明: 设置扬尘在线检测终端电磁阀手动开合信息报文。AFN=41H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetValveMan(pTX101 frame)
{
    uint8_t *sptr,i,dat_i[4];

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    //
    for (i = 0; i < 4; i++)
    {
        dat_i[i] = *(sptr+i);
        if (dat_i[i] == 1)
        {
            dustmon_info.thrshdflag[i] = FALSE;
            switch (i)
            {
                case 0:valve_stat.valve1_en = (BOOL)((*(sptr+4))&0x01); break;
                case 1:valve_stat.valve2_en = (BOOL)((*(sptr+5))&0x01); break;
                case 2:valve_stat.valve3_en = (BOOL)((*(sptr+6))&0x01); break;
                case 3:valve_stat.valve4_en = (BOOL)((*(sptr+7))&0x01); break;
            }
        }
        else if (dat_i[i] == 0)
        {
            dustmon_info.thrshdflag[i] = TRUE;
        }
    }

    dustmon_info.flag = CFGSTAT_SAVE;
    dustmon_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_MANVALVE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamSetNotice
*
*   功能说明: 设置扬尘在线通知信息报文。AFN=44H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetNotice(pTX101 frame)
{
    uint8_t *sptr,*dptr,i,j;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 数据
    sptr = (uint8_t *)rtn.appzone.userdata;
    dptr = (uint8_t *)&dustmon_notice.oled_dispchar;


    for (i = 0; i < (rtn.length - 8); i++)
    {
        *dptr++ = *sptr++;
    }
    for (j = i; j < 234; j++)
    {
        *dptr++ = 0;
    }
    dustmon_notice.oled_update = TRUE;
    dustmon_notice.flag = CFGSTAT_SAVE;
    dustmon_notice.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_NOTICE;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif
#ifdef UPPLAT
/*
*********************************************************************************************************
*   函 数 名: ParamSetValveLmt
*
*   功能说明: 设置卸料平台阈值信息报文。AFN=45H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamSetUPLmt(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // 数据
    sptr = (uint8_t *)rtn.appzone.userdata;

    limit_tbl.limit[SENS_UPWEIGHT].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_UPWEIGHT].hiwarn = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_UPWEIGHT].lowarn = limit_tbl.limit[SENS_UPWEIGHT].hiwarn;

    limit_tbl.limit[SENS_CABL].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_CABL].hiwarn = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_CABL].lowarn = limit_tbl.limit[SENS_CABL].hiwarn;

    limit_tbl.limit[SENS_CABR].hilimit = limit_tbl.limit[SENS_CABL].hilimit;
    limit_tbl.limit[SENS_CABR].hiwarn = limit_tbl.limit[SENS_CABL].hiwarn;
    limit_tbl.limit[SENS_CABR].lowarn = limit_tbl.limit[SENS_CABR].hiwarn;

    limit_tbl.flag = CFGSTAT_SAVE;
    limit_tbl.delay = 300;

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_UPLMT;
    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: ParamQryValveLmt
*
*   功能说明: 查询扬尘在线检测终端电磁阀阈值信息报文。AFN=80H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 ParamQryUPLmt(pTX101 frame)
{
    uint8_t *dptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_QRY_UPLMT;
    dptr = rtnbuf;

	// 称重
    PacksBCD(dptr, limit_tbl.limit[SENS_UPWEIGHT].hilimit, 5, 1); // 高限位
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_UPWEIGHT].hiwarn,5,1);  // 预警值
    dptr += 3;
	// 斜拉索
    PacksBCD(dptr, limit_tbl.limit[SENS_CABL].hilimit, 5, 1); // 高限位
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_CABL].hiwarn,5,1);  // 预警值
    dptr += 3;

	// 预留
    // 预留12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif 

/*
*********************************************************************************************************
*   函 数 名: ParamSetFngrDat
*
*   功能说明: 下行帧,指纹数据变更报文。AFN=28H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pMultiTX101 ParamSetFngrDat(pMF_TX101 frame)
{
    static uint16_t i,j;
    uint8_t *dptr,*sptr,fingernum,buf[32],rtn;
    static uint32_t opid1,opid2;    // 操作人员ID


    // 识别方式及手指序号（1B）指纹所属人员标识码（4B）已经保存到了第0帧中直接上发.
    if (mrtn.mf_flag && (mrtn.mframe_st == (uint16_t)(pow(2, mrtn.mframe_num) - 1)))
    {   // 表示当前多帧是最后一帧,数据存放在mrtnbuffer缓冲区中,转存至终端相应缓冲区
        // 所有帧都已经收完，开始打包数据，识别方式及手指序号（1B）指纹所属人员标识码（4B） 指纹所属人员汉字名称（12B）+指纹特征码
        dptr = mrtnbuffer;
        sptr = (uint8_t *)mrtn.frame[0].appzone.userdata;
        // 保存识别方式及手指序号（1B）指纹所属人员标识码（4B） 指纹所属人员汉字名称（12B）
        for (i = 0; i < 17; i++)
        {
            *dptr++ = mrtnbuf[0][i];
        }
        // 拼接指纹特征码
        for (i = mrtn.mframe_num; i > 0; i--)
        {
            sptr = (uint8_t *)mrtn.frame[i-1].appzone.userdata + 17;    // 只取指纹特征码
            for (j = 0; j < mrtn.mlen[i-1]-17; j++)
            {
                *dptr++ = *sptr++;
            }
        }

        mrtn.mf_flag = 0; // 数据处理完清除多帧标志

        // 保存指纹数据至指纹模块中
        // 保存指纹所属人员标识码（4B） 指纹所属人员汉字名称（12B）到指纹模块记事本

        // 首先查找是否有匹配的ID号，如果有，覆盖写入或者直接退出
        for (i = 0; i < 16; i++)
        {
            if (FNGR_ReadNotepad(i, buf) != ACK_OK)
            {
                i = 0xff;
                break;
            }
            fingernum = mrtnbuffer[0] & 0x0f; // 获取手指序号1or2
            opid1 = *((uint32_t *)(mrtnbuffer+1));  // 获取人员ID号
            opid2 = *((uint32_t *)buf);
            if (opid1 == opid2)
            {
                for (j = 0; j < 16; j++)
                {
                    buf[j] = mrtnbuffer[j + 1];
                }
                if (FNGR_WriteNotepad(i, buf) != ACK_OK)
                {
                    i = 0xff;
                    break;
                }
                // 保存指纹数据特征码至指纹模块中
                if (FNGR_DowChar(&fngr_struct_send, CHARBUF1, mrtnbuffer + 17, 512) == ACK_OK)
                {
                    rtn = FNGR_Store(&fngr_struct_send, CHARBUF1, 2 * i + fingernum - 1);
                    if (rtn == ACK_OK)
                    {
                        novar_print("finger Store success !\r\n", 24);
                        break;
                    }
                    else
                    {
                        i = 0xff;
                    }
                    break;
                }
                else
                {
                    i = 0xff;
                    break;
                }
            }
        }
        // 如果没有找到匹配，在第一个空白指纹区写入新ID和新指纹
        if (i == 16)
        {
            for (i = 0; i < 16; i++)
            {
                if (FNGR_ReadNotepad(i, buf) != ACK_OK)
                {
                    i = 0xff;
                    break;
                }
                opid1 = *((uint32_t *)(mrtnbuffer+1));  // 获取人员ID号
                opid2 = *((uint32_t *)buf);
                if (!opid2) // 第一个空区域
                {
                    for (j = 0; j < 16; j++)
                    {
                        buf[j] = mrtnbuffer[j + 1];
                    }
                    if (FNGR_WriteNotepad(i, buf) != ACK_OK)
                    {
                        i = 0xff;
                        break;
                    }
                    // 保存指纹数据特征码至指纹模块中
                    if (FNGR_DowChar(&fngr_struct_send, CHARBUF1, mrtnbuffer + 17, 512) == ACK_OK)
                    {
                        rtn = FNGR_Store(&fngr_struct_send, CHARBUF1, 2 * i + fingernum - 1);
                        if (rtn == ACK_OK)
                        {
                            novar_print("finger Store success !\r\n", 24);

                        }
                        else
                        {
                            i = 0xff;
                        }
                        break;
                    }
                    else
                    {
                        i = 0xff;
                        break;
                    }
                }
            }
        }
    }

    // 组织应答帧
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.dir = 1;
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.div = 1;
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.fcb = 3;
    if (i == 0xff || i == 0x16)
    {
        mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.func = LFN_DIR1_FAIL;
    }
    else mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.func = LFN_DIR1_OK;
    //mrtn.frame[mrtn.mframe_cnt-1].framecnt = mrtn.mframe_cnt; // 把当前收到的帧号放入
    //mrtn.frame[mrtn.mframe_cnt-1].addrzone = *GetTAddr();
    //mrtn.frame[mrtn.mframe_cnt-1].version = *GetProtocolVersion();
    //mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode = AFN_CHG_FINGER;

    mrtn.frame[mrtn.mframe_cnt-1].length = 12+5; // Ctrl1+FrameNum2+FrameCnt2+ADDR5+Version1+AFN1+AppDat5
    // 重新计算当前帧的CRC
    GetCRC(mrtn.mframe_cnt);
    mrtn.frame[mrtn.mframe_cnt - 1].endbyte = ENDCHAR;

    return &mrtn.frame[mrtn.mframe_cnt-1];
}
/*
*********************************************************************************************************
*   函 数 名: Param_Del_Finger
*
*   功能说明: 下行帧,删除指纹数据报文。AFN=29H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/

pTX101 Param_Del_FngrDat(pTX101 frame)
{
    uint32_t fngrid,tmp;
    uint8_t buf[32];
    uint16_t i,ret=0;

    // 删除指定所属人员标识码的指纹信息
    fngrid = *((uint32_t *)rtn.appzone.userdata);
    if (!fngrid)    // 如果指纹ID为0，清空指纹库
    {
        if ( FNGR_Empty() == ACK_OK)
        {
            memset(buf, 0, 32);
            for (i = 0; i < 16; i++)
            {
                if (FNGR_WriteNotepad(i, buf) != ACK_OK) ret = 1;
            }
        }
        else
        {
            ret = 1;
        }
    }
    else // 如果指纹ID不为零，搜索指纹模块保存ID
    {
        for (i = 0; i < 16; i++)
        {
            if (FNGR_ReadNotepad(i, buf) == ACK_OK)
            {
                tmp = *((uint32_t *)buf);
                if (tmp == fngrid) // 如果有匹配指纹，删除对应指纹并清空对应ID号和姓名
                {
                    if (FNGR_DeletChar(2 * i, 2) == ACK_OK)// 删除对应ID号的两个指纹
                    {
                        memset(buf, 0, 32);
                        if (FNGR_WriteNotepad(i, buf) != ACK_OK) ret = 1; // 该页第一个字节存放写指针
                    }
                }

            }
        }
    }

    // 组织应答
    rtn = *frame;

    // 没有找到对应的id，回应否认帧
    if (ret)
    {
        link_layer_pack(&rtn, ~frame->ctrlzone.dir, 0, 0, LFN_DIR1_FAIL);
    }
    else
    {
        link_layer_pack(&rtn, ~frame->ctrlzone.dir, 0, 0, LFN_DIR1_OK);
    }

    rtn.length = 8+4;
    rtn.appzone.functioncode = AFN_DEL_FINGER;

    // 重新计算CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: Param_Restart
*
*   功能说明: 下行帧,重启终端报文。AFN=A0H
*
*   形   参: pTX101 frame —— 接收到的下行帧
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 Param_Restart(pTX101 frame)
{
    // 组织应答帧
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_RESTART;

    // 重新计算CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
//************************************ 主动上传数据 ****************************************/
// 产生定时实时数据 调试用

#ifdef NOVAR_TEST
void TowRTDatInit(void)
{
    uint32_t i;

    // 时间
    period_value.sec = RNG_Get_RandomRange(0,59);
    period_value.min = RNG_Get_RandomRange(0,59);
    period_value.hour = RNG_Get_RandomRange(0, 23);
    period_value.date = RNG_Get_RandomRange(1, 30);
    period_value.month = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    period_value.year = RNG_Get_RandomRange(0,15);

    // 包类型
    period_value.attrib = 0;

    // 吊重力0.0-9999.9吨
    period_value.weight_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.weight_alarm = gravwarn[RNG_Get_RandomRange(0,3)];
    period_value.weight_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 高度－9999.9　-　9999.9米
    period_value.height_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.height_alarm = heigwarn[RNG_Get_RandomRange(0,6)];
    if (RNG_Get_RandomRange(0, 1)) period_value.height_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.height_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // 幅度0.0-9999.9米
    period_value.margin_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.margin_alarm = scopewarn[RNG_Get_RandomRange(0,6)];
    period_value.margin_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 回转角度－9999.9　-　9999.9度
    period_value.rotat_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.rotat_alarm = rotarywarn[RNG_Get_RandomRange(0,6)];
    period_value.rotat_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    if (RNG_Get_RandomRange(0, 1)) period_value.rotat_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.rotat_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // 风速0.0 - 9999.9 m/s
    period_value.wind_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.wind_alarm = windwarn[RNG_Get_RandomRange(0,2)];
    period_value.wind_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 倾角0.0-9999.9度
    period_value.tilt_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.tilt_alarm = dipanglewarn[RNG_Get_RandomRange(0,2)];
    period_value.tilt_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 力矩比例0.0%　-　9999.9%
    period_value.torque_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.torque_alarm = momentwarn[RNG_Get_RandomRange(0,3)];
    period_value.torque_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 行走－9999.9　-　9999.9米
    period_value.walk_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.walk_alarm = walkwarn[RNG_Get_RandomRange(0,2)];
    period_value.walk_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    if (RNG_Get_RandomRange(0, 1)) period_value.walk_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.walk_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // 备用4
    period_value.spare1_flag = 0;
    period_value.spare1_alarm = 0;
    period_value.spare1_value = 0;

    period_value.spare2_flag = 0;
    period_value.spare2_alarm = 0;
    period_value.spare2_value = 0;

    period_value.spare3_flag = 0;
    period_value.spare3_alarm = 0;
    period_value.spare3_value = 0;

    period_value.spare4_flag = 0;
    period_value.spare4_alarm = 0;
    period_value.spare4_value = 0;

    // 碰撞代码
    period_value.collision_alarm[0] = RNG_Get_RandomRange(0, 255);  // 对方塔机ID，0表示无报警
    period_value.collision_alarm[1] = RNG_Get_RandomRange(0, 2) |   // 低碰撞
                                                       (0 << 2) |   // 空
                                 RNG_Get_RandomRange(0, 2) << 4 |   // 右碰撞
                                 RNG_Get_RandomRange(0, 2) << 6;    // 左碰撞
    period_value.collision_alarm[2] = RNG_Get_RandomRange(1, 2) |   // 干涉类型
                                                       (0 << 2) |   // 空
                                 RNG_Get_RandomRange(0, 2) << 4 |   // 近碰撞
                                 RNG_Get_RandomRange(0, 2) << 6;    // 远碰撞

    // 障碍物代码
    period_value.obstacle_alarm[0] = RNG_Get_RandomRange(0, 255);   // 对方塔机ID，0表示无报警
    period_value.obstacle_alarm[1] = RNG_Get_RandomRange(0, 2) |    // 低碰撞
                                                       (0 << 2) |   // 空
                                 RNG_Get_RandomRange(0, 2) << 4 |   // 右碰撞
                                 RNG_Get_RandomRange(0, 2) << 6;    // 左碰撞
    period_value.obstacle_alarm[2] = 0 |    // 空
                              (0 << 2) |    // 空
        RNG_Get_RandomRange(0, 2) << 4 |    // 近碰撞
        RNG_Get_RandomRange(0, 2) << 6;     // 远碰撞

    // 禁行区代码
    period_value.forbid_alarm[0] = RNG_Get_RandomRange(0, 255);     // 对方塔机ID，0表示无报警
    period_value.forbid_alarm[1] = 0 |      // 空
                            (0 << 2) |      // 空
      RNG_Get_RandomRange(0, 2) << 4 |      // 右碰撞
      RNG_Get_RandomRange(0, 2) << 6;       // 左碰撞
    period_value.forbid_alarm[2] =   0 |    // 空
                              (0 << 2) |    // 空
        RNG_Get_RandomRange(0, 2) << 4 |    // 近碰撞
        RNG_Get_RandomRange(0, 2) << 6;     // 远碰撞

    // 保留3BX4
    for (i = 0; i < 12;i++)
    {
        period_value.spare_flag[i] = 0;
    }

    // 保留6B
    for (i = 0; i < 6;i++)
    {
        period_value.spare_other[i] = 0;
    }
}

void TowWklpDatInit(void)
{
    uint32_t i;

    // 工作循环编号：2个字节，低字节在前，高字节在后
    workloop.sn = RNG_Get_RandomRange(0, 65535);

    // 开始时间
    workloop.sec_begin = RNG_Get_RandomRange(0,59);
    workloop.min_begin = RNG_Get_RandomRange(0,59);
    workloop.hour_begin = RNG_Get_RandomRange(0, 23);
    workloop.date_begin = RNG_Get_RandomRange(1, 30);
    workloop.month_begin = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    workloop.year_begin = RNG_Get_RandomRange(0,15);
    // 结束时间
    workloop.sec_end = (workloop.sec_begin + 1) % 60;
    workloop.min_end = (workloop.min_begin + 1) % 60;
    workloop.hour_end = workloop.hour_begin;
    workloop.date_end = workloop.date_begin;
    workloop.month_end = workloop.month_begin;
    workloop.year_end = workloop.year_begin;

    // 最大起重，3个字节，0-9999.9吨
    workloop.weight_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最大力矩，3个字节，0-9999.9吨*米
    workloop.torque_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最大高度，3个字节，-9999.9 - 9999.9米
    workloop.height_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最小高度，3个字节，-9999.9 - 9999.9米
    workloop.height_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最大幅度，3个字节，-9999.9 - 9999.9米
    workloop.margin_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最小幅度，3个字节，-9999.9 - 9999.9米
    workloop.margin_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最大回转角度，3个字节，-9999.9 - 9999.9度
    workloop.rotat_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最小回转角度，3个字节，-9999.9 - 9999.9度
    workloop.rotat_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最大行走距离，3个字节，-9999.9 - 9999.9米
    workloop.walk_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 最小行走距离，3个字节，-9999.9 - 9999.9米
    workloop.walk_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 起点角度，3个字节，-9999.9-9999.9度
    workloop.rotat_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 起吊点幅度，3个字节，0-9999.9米
    workloop.margin_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 起吊点高度，3个字节，-9999.9-9999.9度
    workloop.height_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 卸吊点角度，3个字节，-9999.9-9999.9度
    workloop.rotat_end = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 卸吊点幅度，3个字节，0-9999.9米
    workloop.margin_end = RNG_Get_RandomRange(0, 99999) / 10.0;

    // 卸吊点高度，3个字节，-9999.9-9999.9度
    workloop.height_end = RNG_Get_RandomRange(0, 99999) / 10.0;

}

// 报警数据
void TowWrnDatInit()
{

}
// 标定数据
void TowCaliDatInit()
{
    uint32_t i;

    // 时间BCD，6bytes
    cali_dat.sec = RNG_Get_RandomRange(0,59);
    cali_dat.min = RNG_Get_RandomRange(0,59);
    cali_dat.hour = RNG_Get_RandomRange(0, 23);
    cali_dat.date = RNG_Get_RandomRange(1, 30);
    cali_dat.month = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    cali_dat.year = RNG_Get_RandomRange(0,15);

    // 包类型待定
    cali_dat.attrib = 0;

    // 吊重力 3字节BCD码，传感器原始值：0-999999
    cali_dat.weight_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.weight_alarm = 0;
    cali_dat.weight = RNG_Get_RandomRange(0,999999);

    // 高度  3字节BCD码，传感器原始值：0-999999。
    cali_dat.height_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.height_alarm = 0;
    cali_dat.heigh = RNG_Get_RandomRange(0,999999);

    // 幅度  3字节BCD码，传感器原始值：0-999999。
    cali_dat.margin_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.margin_alarm = 0;
    cali_dat.margin = RNG_Get_RandomRange(0,999999);

    // 回转角度   3字节BCD码，传感器原始值：0-999999。
    cali_dat.rotat_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.rotat_alarm = 0;
    cali_dat.rotat = RNG_Get_RandomRange(0,999999);

    // 风速     3字节BCD码，传感器原始值：0-999999。
    cali_dat.wind_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.wind_alarm = 0;
    cali_dat.wind = RNG_Get_RandomRange(0,999999);

    // 倾角   3字节BCD码，传感器原始值：0-999999。
    cali_dat.tilt_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.tilt_alarm = 0;
    cali_dat.tilt = RNG_Get_RandomRange(0,999999);

    // 行走     3字节BCD码，传感器原始值：0-999999。
    cali_dat.walk_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.walk_alarm = 0;
    cali_dat.walk = RNG_Get_RandomRange(0,999999);

    // 预留4bytes   3字节BCD码，传感器原始值：0-999999。
    cali_dat.spare1_flag = 0;
    cali_dat.spare1_alarm =0;
    cali_dat.spare1_dat = 0;

    cali_dat.spare2_flag = 0;
    cali_dat.spare2_alarm =0;
    cali_dat.spare2_dat = 0;

    cali_dat.spare3_flag = 0;
    cali_dat.spare3_alarm =0;
    cali_dat.spare3_dat = 0;

    cali_dat.spare4_flag = 0;
    cali_dat.spare4_alarm =0;
    cali_dat.spare4_dat = 0;
}

#endif

#ifdef TOWERBOX
void GetTowRTDat(void)
{
    uint32_t i;

    // 时间
    realtimedata.clock.sec = period_value.sec;
    realtimedata.clock.min = period_value.min;
    realtimedata.clock.hour = period_value.hour;
    realtimedata.clock.date = period_value.date;
    realtimedata.clock.week_single = (period_value.month >> 5) & 0x07;
    realtimedata.clock.mon_single = period_value.month & 0x1F;
    realtimedata.clock.year = period_value.year;

    // 包类型 待定
    realtimedata.packtype[0] = period_value.attrib&0xff;
    realtimedata.packtype[1] = (period_value.attrib>>8)&0xff;

    // 吊重力0.0-9999.9吨
    realtimedata.gravitylift.sensor_OK = period_value.weight_flag & 0x01;
    realtimedata.gravitylift.sensor_EN = (period_value.weight_flag & 0x02)>>1;
    realtimedata.gravitylift.rev1 = 0;
    realtimedata.gravitylift.warning = period_value.weight_alarm;
    PacksBCD((uint8_t *)&realtimedata.gravitylift.data, period_value.weight_value/1000,5,1);

    // 高度－9999.9　-　9999.9米
    realtimedata.height.sensor_OK = period_value.height_flag & 0x01;
    realtimedata.height.sensor_EN = (period_value.height_flag & 0x02)>>1;
    realtimedata.height.rev1 = 0;
    realtimedata.height.warning = period_value.height_alarm;
    PacksBCD((uint8_t *)&realtimedata.height.data, period_value.height_value,5,1);

    // 幅度0.0-9999.9米
    realtimedata.scope.sensor_OK = period_value.margin_flag & 0x01;
    realtimedata.scope.sensor_EN = (period_value.margin_flag & 0x02)>>1;
    realtimedata.scope.rev1 = 0;
    realtimedata.scope.warning = period_value.margin_alarm;
    PacksBCD((uint8_t *)&realtimedata.scope.data, period_value.margin_value,5,1);

    // 回转角度－9999.9　-　9999.9度
    realtimedata.rotary.sensor_OK = period_value.rotat_flag & 0x01;
    realtimedata.rotary.sensor_EN = (period_value.rotat_flag & 0x02)>>1;
    realtimedata.rotary.rev1 = 0;
    realtimedata.rotary.warning = period_value.rotat_alarm;
    PacksBCD((uint8_t *)&realtimedata.rotary.data, period_value.rotat_value,5,1);

    // 风速0.0 - 9999.9 m/s
    realtimedata.wind.sensor_OK = period_value.wind_flag & 0x01;
    realtimedata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    realtimedata.wind.rev1 = 0;
    realtimedata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&realtimedata.wind.data, period_value.wind_value,5,1);

    // 倾角0.0-9999.9度
    realtimedata.dipangle.sensor_OK = period_value.tilt_flag & 0x01;
    realtimedata.dipangle.sensor_EN = (period_value.tilt_flag & 0x02)>>1;
    realtimedata.dipangle.rev1 = 0;
    realtimedata.dipangle.warning = period_value.tilt_alarm;
    PacksBCD((uint8_t *)&realtimedata.dipangle.data, period_value.tilt_value,5,1);

    // 力矩比例0.0%　-　9999.9%
    realtimedata.moment .sensor_OK = period_value.torque_flag & 0x01;
    realtimedata.moment .sensor_EN = (period_value.torque_flag & 0x02)>>1;
    realtimedata.moment .rev1 = 0;
    realtimedata.moment .warning = period_value.torque_alarm;
    PacksBCD((uint8_t *)&realtimedata.moment .data, period_value.torque_value,5,1);

    // 行走－9999.9　-　9999.9米
    realtimedata.walk.sensor_OK = period_value.walk_flag & 0x01;
    realtimedata.walk.sensor_EN = (period_value.walk_flag & 0x02)>>1;
    realtimedata.walk.rev1 = 0;
    realtimedata.walk.warning = period_value.walk_alarm;
    PacksBCD((uint8_t *)&realtimedata.walk.data, period_value.walk_value,5,1);

    // 备用4
    realtimedata.rev1[0].sensor_OK = period_value.spare1_flag & 0x01;
    realtimedata.rev1[0].sensor_EN = (period_value.spare1_flag & 0x02)>>1;
    realtimedata.rev1[0].rev1 = 0;
    realtimedata.rev1[0].warning = period_value.spare1_alarm;
    PacksBCD((uint8_t *)&realtimedata.rev1[0].data, period_value.spare1_value,5,1);

    realtimedata.rev1[1].sensor_OK = period_value.spare2_flag & 0x01;
    realtimedata.rev1[1].sensor_EN = (period_value.spare2_flag & 0x02)>>1;
    realtimedata.rev1[1].rev1 = 0;
    realtimedata.rev1[1].warning = period_value.spare2_alarm;
    PacksBCD((uint8_t *)&realtimedata.rev1[1].data, period_value.spare2_value,5,1);

    realtimedata.rev1[2].sensor_OK = period_value.spare3_flag & 0x01;
    realtimedata.rev1[2].sensor_EN = (period_value.spare3_flag & 0x02)>>1;
    realtimedata.rev1[2].rev1 = 0;
    realtimedata.rev1[2].warning = period_value.spare3_alarm;
    PacksBCD((uint8_t *)&realtimedata.rev1[2].data, period_value.spare3_value,5,1);

    realtimedata.rev1[3].sensor_OK = period_value.spare4_flag & 0x01;
    realtimedata.rev1[3].sensor_EN = (period_value.spare4_flag & 0x02)>>1;
    realtimedata.rev1[3].rev1 = 0;
    realtimedata.rev1[3].warning = period_value.spare4_alarm;
    PacksBCD((uint8_t *)&realtimedata.rev1[3].data, period_value.spare4_value,5,1);

    // 碰撞代码
    realtimedata.collision.ID = period_value.collision_alarm[0];
    realtimedata.collision.low = period_value.collision_alarm[1]&0x03;
    realtimedata.collision.rev1 = (period_value.collision_alarm[1]>>2)&0x03;
    realtimedata.collision.right = (period_value.collision_alarm[1]>>4)&0x03;
    realtimedata.collision.left = (period_value.collision_alarm[1]>>6)&0x03;
    realtimedata.collision.Inttype = period_value.collision_alarm[2]&0x03;
    realtimedata.collision.rev2 = (period_value.collision_alarm[2]>>2)&0x03;
    realtimedata.collision.near = (period_value.collision_alarm[2]>>4)&0x03;
    realtimedata.collision.far = (period_value.collision_alarm[2]>>6)&0x03;
    realtimedata.collision.rev1 = period_value.collision_alarm[3];
    for (i = 0; i < 12; i++)
    {
        realtimedata.collision.risk_info[i] = period_value.risk_info[i];
    }

    // 障碍物代码
    realtimedata.obstacle.ID = period_value.obstacle_alarm[0];
    realtimedata.obstacle.low = period_value.obstacle_alarm[1]&0x03;
    realtimedata.obstacle.rev1 = (period_value.obstacle_alarm[1]>>2)&0x03;
    realtimedata.obstacle.right = (period_value.obstacle_alarm[1]>>4)&0x03;
    realtimedata.obstacle.left = (period_value.obstacle_alarm[1]>>6)&0x03;
    realtimedata.obstacle.Inttype = period_value.obstacle_alarm[2]&0x03;
    realtimedata.obstacle.rev2 = (period_value.obstacle_alarm[2]>>2)&0x03;
    realtimedata.obstacle.near = (period_value.obstacle_alarm[2]>>4)&0x03;
    realtimedata.obstacle.far = (period_value.obstacle_alarm[2]>>6)&0x03;

    // 禁行区代码
    realtimedata.forbidden.ID = period_value.forbid_alarm[0];
    realtimedata.forbidden.low = period_value.forbid_alarm[1]&0x03;
    realtimedata.forbidden.rev1 = (period_value.forbid_alarm[1]>>2)&0x03;
    realtimedata.forbidden.right = (period_value.forbid_alarm[1]>>4)&0x03;
    realtimedata.forbidden.left = (period_value.forbid_alarm[1]>>6)&0x03;
    realtimedata.forbidden.Inttype = period_value.forbid_alarm[2]&0x03;
    realtimedata.forbidden.rev2 = (period_value.forbid_alarm[2]>>2)&0x03;
    realtimedata.forbidden.near = (period_value.forbid_alarm[2]>>4)&0x03;
    realtimedata.forbidden.far = (period_value.forbid_alarm[2]>>6)&0x03;

    // 保留3BX4
    realtimedata.rev2[0].ID = period_value.spare_flag[0];
    realtimedata.rev2[0].low = period_value.spare_flag[1]&0x03;
    realtimedata.rev2[0].rev1 = (period_value.spare_flag[1]>>2)&0x03;
    realtimedata.rev2[0].right = (period_value.spare_flag[1]>>4)&0x03;
    realtimedata.rev2[0].left = (period_value.spare_flag[1]>>6)&0x03;
    realtimedata.rev2[0].Inttype = period_value.spare_flag[2]&0x03;
    realtimedata.rev2[0].rev2 = (period_value.spare_flag[2]>>2)&0x03;
    realtimedata.rev2[0].near = (period_value.spare_flag[2]>>4)&0x03;
    realtimedata.rev2[0].far = (period_value.spare_flag[2]>>6)&0x03;

    realtimedata.rev2[1].ID = period_value.spare_flag[3];
    realtimedata.rev2[1].low = period_value.spare_flag[4]&0x03;
    realtimedata.rev2[1].rev1 = (period_value.spare_flag[4]>>2)&0x03;
    realtimedata.rev2[1].right = (period_value.spare_flag[4]>>4)&0x03;
    realtimedata.rev2[1].left = (period_value.spare_flag[4]>>6)&0x03;
    realtimedata.rev2[1].Inttype = period_value.spare_flag[5]&0x03;
    realtimedata.rev2[1].rev2 = (period_value.spare_flag[5]>>2)&0x03;
    realtimedata.rev2[1].near = (period_value.spare_flag[5]>>4)&0x03;
    realtimedata.rev2[1].far = (period_value.spare_flag[5]>>6)&0x03;

    realtimedata.rev2[2].ID = period_value.spare_flag[6];
    realtimedata.rev2[2].low = period_value.spare_flag[7]&0x03;
    realtimedata.rev2[2].rev1 = (period_value.spare_flag[7]>>2)&0x03;
    realtimedata.rev2[2].right = (period_value.spare_flag[7]>>4)&0x03;
    realtimedata.rev2[2].left = (period_value.spare_flag[7]>>6)&0x03;
    realtimedata.rev2[2].Inttype = period_value.spare_flag[8]&0x03;
    realtimedata.rev2[2].rev2 = (period_value.spare_flag[8]>>2)&0x03;
    realtimedata.rev2[2].near = (period_value.spare_flag[8]>>4)&0x03;
    realtimedata.rev2[2].far = (period_value.spare_flag[8]>>6)&0x03;

    realtimedata.rev2[3].ID = period_value.spare_flag[9];
    realtimedata.rev2[3].low = period_value.spare_flag[10]&0x03;
    realtimedata.rev2[3].rev1 = (period_value.spare_flag[10]>>2)&0x03;
    realtimedata.rev2[3].right = (period_value.spare_flag[10]>>4)&0x03;
    realtimedata.rev2[3].left = (period_value.spare_flag[10]>>6)&0x03;
    realtimedata.rev2[3].Inttype = period_value.spare_flag[11]&0x03;
    realtimedata.rev2[3].rev2 = (period_value.spare_flag[11]>>2)&0x03;
    realtimedata.rev2[3].near = (period_value.spare_flag[11]>>4)&0x03;
    realtimedata.rev2[3].far = (period_value.spare_flag[11]>>6)&0x03;

    // 保留6B
    for (i = 0; i < 6;i++)
    {
        realtimedata.rev3[i] = period_value.spare_other[i];
    }
}

// 产生工作循环数据 调试用
void GetTowWklpDat(void)
{

    // 工作循环编号：2个字节，低字节在前，高字节在后
    TowWklpDat.workcyclenum[0] = workloop.sn&0xff;
    TowWklpDat.workcyclenum[1] = (workloop.sn>>8)&0xff;

    // 开始时间
    TowWklpDat.starttime.sec = workloop.sec_begin;
    TowWklpDat.starttime.min = workloop.min_begin;
    TowWklpDat.starttime.hour = workloop.hour_begin;
    TowWklpDat.starttime.date = workloop.date_begin;
    TowWklpDat.starttime.week_single = (workloop.month_begin >> 5) & 0x07;
    TowWklpDat.starttime.mon_single = workloop.month_begin & 0x1F;
    TowWklpDat.starttime.year = workloop.year_begin;

    // 结束时间

    TowWklpDat.endtime.sec = workloop.sec_end;
    TowWklpDat.endtime.min = workloop.min_end;
    TowWklpDat.endtime.hour = workloop.hour_end;
    TowWklpDat.endtime.date = workloop.date_end;
    TowWklpDat.endtime.week_single = (workloop.month_begin >> 5) & 0x07;
    TowWklpDat.endtime.mon_single = workloop.month_begin & 0x1F;
    TowWklpDat.endtime.year = workloop.year_end;

    // 最大起重，3个字节，0-9999.9吨
    PacksBCD((uint8_t *)&TowWklpDat.maxlift, workloop.weight_max/1000,5,1);

    // 最大力矩，3个字节，0-9999.9吨*米
    PacksBCD((uint8_t *)&TowWklpDat.maxmoment, workloop.torque_max,5,1);

    // 最大高度，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.maxheight, workloop.height_max,5,1);

    // 最小高度，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.minheight, workloop.height_min,5,1);

    // 最大幅度，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.maxscope, workloop.margin_max,5,1);

    // 最小幅度，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.minscope, workloop.margin_min,5,1);

    // 最大回转角度，3个字节，-9999.9 - 9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.maxrotary, workloop.rotat_max,5,1);

    // 最小回转角度，3个字节，-9999.9 - 9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.minrotary, workloop.rotat_min,5,1);

    // 最大行走距离，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.maxwalk, workloop.walk_max,5,1);

    // 最小行走距离，3个字节，-9999.9 - 9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.minwalk, workloop.walk_min,5,1);

    // 起吊点角度，3个字节，-9999.9-9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.liftpointangle, workloop.rotat_begin,5,1);

    // 起吊点幅度，3个字节，0-9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.liftpointscope, workloop.margin_begin,5,1);

    // 起吊点高度，3个字节，-9999.9-9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.liftpointheight, workloop.height_begin,5,1);

    // 卸吊点角度，3个字节，-9999.9-9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointangle, workloop.rotat_end,5,1);

    // 卸吊点幅度，3个字节，0-9999.9米
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointscope, workloop.margin_end,5,1);

    // 卸吊点高度，3个字节，-9999.9-9999.9度
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointheight, workloop.height_end,5,1);
}

// 产生报警实时数据 调试用
void GetTowWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;
    // 时间
    // 主程序未定义报警信息结构，所以从周期数据中读取时钟，带结构定以后替换
    warningdata.clock.sec = alarm_dat[num].sec;
    warningdata.clock.min = alarm_dat[num].min;
    warningdata.clock.hour = alarm_dat[num].hour;
    warningdata.clock.date = alarm_dat[num].date;
    warningdata.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    warningdata.clock.mon_single = alarm_dat[num].month & 0x1F;
    warningdata.clock.year = alarm_dat[num].year;

    // 包类型
    warningdata.packtype[0] = alarm_dat[num].attrib&0xff;
    warningdata.packtype[1] = (alarm_dat[num].attrib>>8)&0xff;;

    // 吊重力
    warningdata.gravitylift.sensor_OK = alarm_dat[num].weight_flag & 0x01;
    warningdata.gravitylift.sensor_EN = (alarm_dat[num].weight_flag & 0x02)>>1;
    warningdata.gravitylift.rev1 = 0;
    warningdata.gravitylift.warning = alarm_dat[num].weight_alarm;
    PacksBCD((uint8_t *)&warningdata.gravitylift.data,alarm_dat[num].weight_value/1000,5,1);

    // 高度
    warningdata.height.sensor_OK = alarm_dat[num].height_flag & 0x01;
    warningdata.height.sensor_EN = (alarm_dat[num].height_flag & 0x02)>>1;
    warningdata.height.rev1 = 0;
    warningdata.height.warning = alarm_dat[num].height_alarm;
    PacksBCD((uint8_t *)&warningdata.height.data, alarm_dat[num].height_value,5,1);

    // 幅度
    warningdata.scope.sensor_OK = alarm_dat[num].margin_flag & 0x01;
    warningdata.scope.sensor_EN = (alarm_dat[num].margin_flag & 0x02)>>1;
    warningdata.scope.rev1 = 0;
    warningdata.scope.warning = alarm_dat[num].margin_alarm;
    PacksBCD((uint8_t *)&warningdata.scope.data, alarm_dat[num].margin_value,5,1);

    // 回转角度
    warningdata.rotary.sensor_OK = alarm_dat[num].rotat_flag & 0x01;
    warningdata.rotary.sensor_EN = (alarm_dat[num].rotat_flag>>1)&0x01;
    warningdata.rotary.rev1 = 0;
    warningdata.rotary.warning = alarm_dat[num].rotat_alarm;
    PacksBCD((uint8_t *)&warningdata.rotary.data, alarm_dat[num].rotat_value,5,1);

    // 风速
    warningdata.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    warningdata.wind.sensor_EN = (alarm_dat[num].wind_flag>>1)&0x01;
    warningdata.wind.rev1 = 0;
    warningdata.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&warningdata.wind.data, alarm_dat[num].wind_value,5,1);

    // 倾角
    warningdata.dipangle.sensor_OK = alarm_dat[num].tilt_flag & 0x01;
    warningdata.dipangle.sensor_EN = (alarm_dat[num].tilt_flag >> 1) & 0x01;
    warningdata.dipangle.rev1 = 0;
    warningdata.dipangle.warning = alarm_dat[num].tilt_alarm;
    PacksBCD((uint8_t *)&warningdata.dipangle.data, alarm_dat[num].tilt_value,5,1);

    // 力矩比率
    warningdata.moment.sensor_OK = alarm_dat[num].torque_flag & 0x01;
    warningdata.moment.sensor_EN = (alarm_dat[num].torque_flag >> 1) & 0x01;
    warningdata.moment.rev1 = 0;
    warningdata.moment.warning = alarm_dat[num].torque_alarm;
    PacksBCD((uint8_t *)&warningdata.moment.data, alarm_dat[num].torque_value,5,1);

    // 行走
    warningdata.walk.sensor_OK = alarm_dat[num].walk_flag & 0x01;
    warningdata.walk.sensor_EN = (alarm_dat[num].walk_flag >> 1) & 0x01;
    warningdata.walk.rev1 = 0;
    warningdata.walk.warning = alarm_dat[num].walk_alarm;
    PacksBCD((uint8_t *)&warningdata.walk.data, alarm_dat[num].walk_value,5,1);

    // 状态字
    warningdata.status = alarm_dat[num].alarm_stat;

    // 根据状态字添加报警/违章/故障信息
    if (warningdata.status&0x01)
    {
        // 添加报警信息 1条
        dptr = warningdata.warning;
        *dptr++ = alarm_dat[num].alarm_num;
        for (i = 0; i < alarm_dat[num].alarm_num; i++)
        {
            *dptr++ = alarm_dat[num].alarm[i].alarm_code;
            switch (alarm_dat[num].alarm[i].alarm_code)
            {
            case RPT_ALARMID_COLLISION:
                len = 2;
                break;
            case RPT_ALARMID_LIMIT:
                len = 3;
                break;
            case RPT_ALARMID_FORBIDDEN:
            case RPT_ALARMID_OBSTACLE:
            case RPT_ALARMID_WEIGHT:
            case RPT_ALARMID_TORQUE:
            case RPT_ALARMID_WIND:
            case RPT_ALARMID_TILT:
            case RPT_ALARMID_WALK:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].alarm[i].alarm_byte[j];
            }
        }
    }
    if ((warningdata.status>>1)&0x01)
    {
        // 添加违章信息 1条
        dptr = warningdata.illegal;
        *dptr++ = alarm_dat[num].against_num;
        for (i = 0; i < alarm_dat[num].against_num; i++)
        {
            *dptr++ = alarm_dat[num].against[i].against_code;
            switch (alarm_dat[num].against[i].against_code)
            {

            case  RPT_AGAINSTID_COLLISION:
                len = 2;
                break;
            case  RPT_AGAINSTID_FORBIDDEN:
            case  RPT_AGAINSTID_OBSTACLE:
            case  RPT_AGAINSTID_LIMIT:
            case  RPT_AGAINSTID_WEIGHT:
            case  RPT_AGAINSTID_TORQUE:
            case  RPT_AGAINSTID_WIND:
            case  RPT_AGAINSTID_TILT:
            case  RPT_AGAINSTID_IDENTITY:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].against[i].against_byte[j];
            }
        }
    }
    if ((warningdata.status>>2)&0x01)
    {
        // 添加故障信息 1条
        warningdata.illegal[0] = alarm_dat[num].error_num;
        dptr = warningdata.fault;
        *dptr++ = alarm_dat[num].error_num;
        for (i = 0; i < alarm_dat[num].error_num; i++)
        {
            *dptr++ = alarm_dat[num].error[i].error_code;
        }
    }

    // 6 Bytes reserved
    for (i=0;i<6;i++)
    {
        warningdata.rev2[i] = 0;
    }
}


// 产生标定实时数据 调试用
void GetTowCaliDat(void)
{
    // 时间
    TowCaliDat.clock.sec = cali_dat.sec;
    TowCaliDat.clock.min = cali_dat.min;
    TowCaliDat.clock.hour = cali_dat.hour;
    TowCaliDat.clock.date = cali_dat.date;
    TowCaliDat.clock.week_single = (cali_dat.month >> 5) & 0x07;
    TowCaliDat.clock.mon_single = cali_dat.month & 0x1F;
    TowCaliDat.clock.year = cali_dat.year;

    // 包类型 待定
    TowCaliDat.packtype[0] = period_value.attrib&0xff;
    TowCaliDat.packtype[1] = (period_value.attrib>>8)&0xff;

    // 吊重力0.0-9999.9吨
    TowCaliDat.gravitylift.sensor_OK = cali_dat.weight_flag & 0x01;
    TowCaliDat.gravitylift.sensor_EN = (cali_dat.weight_flag & 0x02)>>1;
    TowCaliDat.gravitylift.rev1 = (cali_dat.weight_flag>>2) & 0x3F;
    TowCaliDat.gravitylift.warning = cali_dat.weight_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.gravitylift.data, cali_dat.weight/1000.0,6,0);

    // 高度
    TowCaliDat.height.sensor_OK = cali_dat.height_flag & 0x01;
    TowCaliDat.height.sensor_EN = (cali_dat.height_flag & 0x02)>>1;
    TowCaliDat.height.rev1 = (cali_dat.height_flag>>2) & 0x3F;
    TowCaliDat.height.warning = cali_dat.height_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.height.data, cali_dat.heigh,6,0);

    // 幅度
    TowCaliDat.scope.sensor_OK = cali_dat.margin_flag & 0x01;
    TowCaliDat.scope.sensor_EN = (cali_dat.margin_flag & 0x02)>>1;
    TowCaliDat.scope.rev1 = (cali_dat.margin_flag>>2) & 0x3F;
    TowCaliDat.scope.warning = cali_dat.margin_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.scope.data, cali_dat.margin,6,0);

    // 回转角度
    TowCaliDat.rotary.sensor_OK = cali_dat.rotat_flag & 0x01;
    TowCaliDat.rotary.sensor_EN = (cali_dat.rotat_flag & 0x02)>>1;
    TowCaliDat.rotary.rev1 = (cali_dat.rotat_flag>>2) & 0x3F;
    TowCaliDat.rotary.warning = cali_dat.rotat_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rotary.data, cali_dat.rotat,6,0);

    // 风速
    TowCaliDat.wind.sensor_OK = cali_dat.wind_flag & 0x01;
    TowCaliDat.wind.sensor_EN = (cali_dat.wind_flag & 0x02)>>1;
    TowCaliDat.wind.rev1 = (cali_dat.wind_flag>>2) & 0x3F;
    TowCaliDat.wind.warning = cali_dat.wind_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.wind.data, cali_dat.wind,6,0);

    // 倾角
    TowCaliDat.dipangle.sensor_OK = cali_dat.tilt_flag & 0x01;
    TowCaliDat.dipangle.sensor_EN = (cali_dat.tilt_flag & 0x02)>>1;
    TowCaliDat.dipangle.rev1 = (cali_dat.tilt_flag>>2) & 0x3F;
    TowCaliDat.dipangle.warning = cali_dat.tilt_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.dipangle.data, cali_dat.tilt,6,0);

    // 行走
    TowCaliDat.walk.sensor_OK = cali_dat.walk_flag & 0x01;
    TowCaliDat.walk.sensor_EN = (cali_dat.walk_flag & 0x02)>>1;
    TowCaliDat.walk.rev1 = (cali_dat.walk_flag>>2) & 0x3F;
    TowCaliDat.walk.warning = cali_dat.walk_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.walk.data, cali_dat.walk,6,0);

    // 预留5BX4
    TowCaliDat.rev1[0].sensor_OK = cali_dat.spare1_flag & 0x01;
    TowCaliDat.rev1[0].sensor_EN = (cali_dat.spare1_flag & 0x02)>>1;
    TowCaliDat.rev1[0].rev1 = (cali_dat.spare1_flag>>2) & 0x3F;
    TowCaliDat.rev1[0].warning = cali_dat.spare1_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rev1[0].data, cali_dat.spare1_dat,6,0);

    TowCaliDat.rev1[1].sensor_OK = cali_dat.spare2_flag & 0x01;
    TowCaliDat.rev1[1].sensor_EN = (cali_dat.spare2_flag & 0x02)>>1;
    TowCaliDat.rev1[1].rev1 = (cali_dat.spare2_flag>>2) & 0x3F;
    TowCaliDat.rev1[1].warning = cali_dat.spare2_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rev1[1].data, cali_dat.spare2_dat,6,0);

    TowCaliDat.rev1[2].sensor_OK = cali_dat.spare3_flag & 0x01;
    TowCaliDat.rev1[2].sensor_EN = (cali_dat.spare3_flag & 0x02)>>1;
    TowCaliDat.rev1[2].rev1 = (cali_dat.spare3_flag>>2) & 0x3F;
    TowCaliDat.rev1[2].warning = cali_dat.spare3_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rev1[2].data, cali_dat.spare3_dat,6,0);

    TowCaliDat.rev1[3].sensor_OK = cali_dat.spare3_flag & 0x01;
    TowCaliDat.rev1[3].sensor_EN = (cali_dat.spare3_flag & 0x02)>>1;
    TowCaliDat.rev1[3].rev1 = (cali_dat.spare3_flag>>2) & 0x3F;
    TowCaliDat.rev1[3].warning = cali_dat.spare3_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rev1[3].data, cali_dat.spare3_dat,6,0);
}

/*
*********************************************************************************************************
*   函 数 名: AUFTowRTDat
*
*   功能说明: 自动上传报文：定时实时数据。上行帧,主站响应相同功能码报文。AFN=90H
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFTowRTDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    #ifdef NOVAR_TEST
    TowRTDatInit();
    #endif
    //GetTowRTDat();

    rtn.length = 0;

    link_layer_pack(&rtn, 1,0, StatusFlag.resend_times, LFN_DIR1_TIMINGSTATUS);
    rtn.appzone.functioncode = AFN_SELF_REALTIMEDATA;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&realtimedata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 包类型待定       2B
    sptr = realtimedata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // 吊重力     5B
    sptr = (uint8_t *)&realtimedata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&realtimedata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 幅度       5B
    sptr = (uint8_t *)&realtimedata.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 回转角度   5B
    sptr = (uint8_t *)&realtimedata.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&realtimedata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&realtimedata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 力矩比率   5B
    sptr = (uint8_t *)&realtimedata.moment;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 行走       5B
    sptr = (uint8_t *)&realtimedata.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 预留       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&realtimedata.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }
    //碰撞代码 27B
    sptr = (uint8_t *)&realtimedata.collision;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    for (i = 0; i < 12; i++)
    {
        sptr = (uint8_t *)&realtimedata.collision.risk_info[i];
        for (j = 0; j < 2; j++)
        {
            *dptr++ = *sptr++;
        }
    }
    rtn.length += 24;
    // 障碍物 3B
    sptr = (uint8_t *)&realtimedata.obstacle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //禁行区3B
    sptr = (uint8_t *)&realtimedata.forbidden;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 预留3BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&realtimedata.rev2[j];
        for (i = 0; i < 3;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 3;
    }

    // 预留6B
    sptr = realtimedata.rev3;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFTowWklpDat
*
*   功能说明: 自动上传报文工作循环数据。上行帧,主站响应相同功能码报文。AFN=91H
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFTowWklpDat(void)
{
    uint32_t i;
    uint8_t *sptr,*dptr;

    #ifdef NOVAR_TEST
    TowWklpDatInit();
    #endif
    //GetTowWklpDat();

    rtn.length = 0;

    link_layer_pack(&rtn, 0,0, StatusFlag.resend_times, LFN_DIR1_RANDOMWORK);
    rtn.appzone.functioncode = AFN_SELF_WORKCYCLE;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;
    // 工作循环编号：2个字节，低字节在前，高字节在后
    sptr = TowWklpDat.workcyclenum;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // 开始时间6B
    sptr = (uint8_t *)&TowWklpDat.starttime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 结束时间
    sptr = (uint8_t *)&TowWklpDat.endtime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 最大起重，3个字节，0-9999.9吨
    sptr = (uint8_t *)&TowWklpDat.maxlift;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大力矩，3个字节，0-9999.9吨*米
    sptr = (uint8_t *)&TowWklpDat.maxmoment;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大高度，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.maxheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最小高度，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.minheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大幅度，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.maxscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最小幅度，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.minscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大回转角度，3个字节，-9999.9 - 9999.9度
    sptr = (uint8_t *)&TowWklpDat.maxrotary;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最小回转角度，3个字节，-9999.9 - 9999.9度
    sptr = (uint8_t *)&TowWklpDat.minrotary;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大行走距离，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.maxwalk;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最小行走距离，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&TowWklpDat.minwalk;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 起吊点角度，3个字节，-9999.9-9999.9度
    sptr = (uint8_t *)&TowWklpDat.liftpointangle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 起吊点幅度，3个字节，0-9999.9米
    sptr = (uint8_t *)&TowWklpDat.liftpointscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 起吊点高度，3个字节，-9999.9-9999.9度
    sptr = (uint8_t *)&TowWklpDat.liftpointheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 卸吊点角度，3个字节，-9999.9-9999.9度
    sptr = (uint8_t *)&TowWklpDat.unloadpointangle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 卸吊点幅度，3个字节，0-9999.9米
    sptr = (uint8_t *)&TowWklpDat.unloadpointscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 卸吊点高度，3个字节，-9999.9-9999.9度
    sptr = (uint8_t *)&TowWklpDat.unloadpointheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFTowWrnDat
*
*   功能说明: 自动上传报文工作循环数据。上行帧,主站响应相同功能码报文。AFN=92H
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFTowWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//变位信息数目
    uint8_t vartype;//变位信息类型

    #ifdef NOVAR_TEST
    TowWrnDatInit();
    #endif
    //GetTowWrnDat(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_WARNING;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&warningdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 包类型待定       2B
    sptr = warningdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // 吊重力     5B
    sptr = (uint8_t *)&warningdata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&warningdata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 幅度       5B
    sptr = (uint8_t *)&warningdata.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 回转角度   5B
    sptr = (uint8_t *)&warningdata.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&warningdata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&warningdata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 力矩比率   5B
    sptr = (uint8_t *)&warningdata.moment;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 行走       5B
    sptr = (uint8_t *)&warningdata.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 预留       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&warningdata.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }
    // 变位信息状态字
    *dptr++ = warningdata.status;
    rtn.length += 1;

    // 判断状态查询是否存在变位信息
    // 存在报警
    if (warningdata.status&0x01)
    {
        // 添加报警信息
        sptr = warningdata.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// 限位报警3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警
            case 0x05:  // 起重量报警
            case 0x06:  // 力矩报警
            case 0x07:  // 风速报警
            case 0x08: // 倾斜报警
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在违章
    if ((warningdata.status>>1)&0x01)
    {
        // 添加违章信息
        sptr = warningdata.illegal;
        varnum = *sptr++;   // 违章信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警1Byte
            case 0x04:  // 限位报警1Byte
            case 0x05:  // 起重量报警1Byte
            case 0x06:  // 力矩报警1Byte
            case 0x07:  // 风速报警1Byte
            case 0x08: // 倾斜报警1Byte
            case 0x09:  //身份验证1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在故障
    if ((warningdata.status>>2)&0x01)
    {
        // 添加故障信息
        sptr = warningdata.fault;
        varnum = *sptr++;   // 故障信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //故障类型编码
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // 预留6B
    sptr = warningdata.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFTowCaliDat
*
*   功能说明: 自动上传报文：定时实时数据。上行帧,主站响应相同功能码报文。AFN=98H
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFTowCaliDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    #ifdef NOVAR_TEST
    TowCaliDatInit();
    #endif
    //GetTowCaliDat();

    rtn.length = 0;

    link_layer_pack(&rtn, 0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWORK);
    rtn.appzone.functioncode = AFN_SELF_CALIBRATION;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&TowCaliDat.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 包类型待定       2B
    sptr = TowCaliDat.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // 吊重力     5B
    sptr = (uint8_t *)&TowCaliDat.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&TowCaliDat.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 幅度       5B
    sptr = (uint8_t *)&TowCaliDat.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 回转角度   5B
    sptr = (uint8_t *)&TowCaliDat.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&TowCaliDat.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&TowCaliDat.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 行走       5B
    sptr = (uint8_t *)&TowCaliDat.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 预留       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&TowCaliDat.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
#endif

#ifdef ELIVATOR
void GetElvtRTData(void)
{
    uint32_t i;

    // 时间
    elvtdata.clock.sec = period_value.sec; //period_value.sec;
    elvtdata.clock.min = period_value.min;
    elvtdata.clock.hour = period_value.hour;
    elvtdata.clock.date = period_value.date;
    elvtdata.clock.week_single = (period_value.month >> 5) & 0x07;
    elvtdata.clock.mon_single = period_value.month & 0x1F;
    elvtdata.clock.year = period_value.year;

    // 人员标识码
    elvtdata.name_id = period_value.name_id;

    // 包类型 待定
    elvtdata.packtype[0] = period_value.attrib&0xff;
    elvtdata.packtype[1] = (period_value.attrib>>8)&0xff;

    // 吊重力0.0-9999.9吨
    elvtdata.gravitylift.sensor_OK = period_value.weight_flag & 0x01;
    elvtdata.gravitylift.sensor_EN = (period_value.weight_flag & 0x02)>>1;
    elvtdata.gravitylift.rev1 = 0;
    elvtdata.gravitylift.warning = period_value.weight_alarm;
    PacksBCD((uint8_t *)&elvtdata.gravitylift.data, period_value.weight_value/1000,5,1);

    // 高度－9999.9　-　9999.9米
    elvtdata.height.sensor_OK = period_value.height_flag & 0x01;
    elvtdata.height.sensor_EN = (period_value.height_flag & 0x02)>>1;
    elvtdata.height.rev1 = 0;
    elvtdata.height.warning = period_value.height_alarm;
    PacksBCD((uint8_t *)&elvtdata.height.data, period_value.height_value,5,1);

    // 速度0.0-9999.9米/秒
    elvtdata.speed.sensor_OK = period_value.speed_flag & 0x01;
    elvtdata.speed.sensor_EN = (period_value.speed_flag & 0x02)>>1;
    elvtdata.speed.rev1 = 0;
    elvtdata.speed.warning = period_value.speed_alarm;
    PacksBCD((uint8_t *)&elvtdata.speed.data, period_value.speed_value,5,1);

    // 风速0.0 - 9999.9 m/s
    elvtdata.wind.sensor_OK = period_value.wind_flag & 0x01;
    elvtdata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    elvtdata.wind.rev1 = 0;
    elvtdata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&elvtdata.wind.data, period_value.wind_value,5,1);

    // 倾角0.0-9999.9度
    elvtdata.dipangle.sensor_OK = period_value.tilt_flag & 0x01;
    elvtdata.dipangle.sensor_EN = (period_value.tilt_flag & 0x02)>>1;
    elvtdata.dipangle.rev1 = 0;
    elvtdata.dipangle.warning = period_value.tilt_alarm;
    PacksBCD((uint8_t *)&elvtdata.dipangle.data, period_value.tilt_value,5,1);

    // 电机一
    elvtdata.motor[0].sensor_OK = period_value.motor1_flag & 0x01;
    elvtdata.motor[0].sensor_EN = (period_value.motor1_flag & 0x02)>>1;
    elvtdata.motor[0].rev1 = 0;
    elvtdata.motor[0].warning = period_value.motor1_alarm;

    // 电机二
    elvtdata.motor[1].sensor_OK = period_value.motor2_flag & 0x01;
    elvtdata.motor[1].sensor_EN = (period_value.motor2_flag & 0x02)>>1;
    elvtdata.motor[1].rev1 = 0;
    elvtdata.motor[1].warning = period_value.motor2_alarm;

    // 电机三
    elvtdata.motor[2].sensor_OK = period_value.motor3_flag & 0x01;
    elvtdata.motor[2].sensor_EN = (period_value.motor3_flag & 0x02)>>1;
    elvtdata.motor[2].rev1 = 0;
    elvtdata.motor[2].warning = period_value.motor3_alarm;

    // 人数
    elvtdata.people_flag = period_value.people_flag;
    elvtdata.people_alarm = period_value.people_alarm;
    elvtdata.people_value = period_value.people_value;

    // 楼层
    elvtdata.floor_flag = period_value.floor_flag;
    elvtdata.floor_aligned = period_value.floor_aligned;
    elvtdata.floor_value = period_value.floor_value;

    elvtdata.door_limit = period_value.door_limit;
    // 保留19B
    for (i = 0; i < 19;i++)
    {
        elvtdata.rev[i] = 0;
    }


    // 保留6B
    for (i = 0; i < 6;i++)
    {
        elvtdata.rev1[i] = 0;
    }
}
void GetElvtWklpDat(void)
{
    // 工作循环编号：2个字节，低字节在前，高字节在后
    ElvtWklpDat.workcyclenum[0] = workloop.sn&0xff;
    ElvtWklpDat.workcyclenum[1] = (workloop.sn>>8)&0xff;

    // 人员标识码（4B）
    ElvtWklpDat.name_id = workloop.name_id;

    // 开始时间（6B）
    ElvtWklpDat.starttime.sec = workloop.sec_begin;
    ElvtWklpDat.starttime.min = workloop.min_begin;
    ElvtWklpDat.starttime.hour = workloop.hour_begin;
    ElvtWklpDat.starttime.date = workloop.date_begin;
    ElvtWklpDat.starttime.week_single = (workloop.month_begin >> 5) & 0x07;
    ElvtWklpDat.starttime.mon_single = workloop.month_begin & 0x1F;
    ElvtWklpDat.starttime.year = workloop.year_begin;

    // 结束时间（6B）
    ElvtWklpDat.endtime.sec = workloop.sec_end;
    ElvtWklpDat.endtime.min = workloop.min_end;
    ElvtWklpDat.endtime.hour = workloop.hour_end;
    ElvtWklpDat.endtime.date = workloop.date_end;
    ElvtWklpDat.endtime.week_single = (workloop.month_begin >> 5) & 0x07;
    ElvtWklpDat.endtime.mon_single = workloop.month_begin & 0x1F;
    ElvtWklpDat.endtime.year = workloop.year_end;

    // 最大重量，3个字节，0-9999.9吨
    PacksBCD((uint8_t *)&ElvtWklpDat.maxlift, workloop.weight_max/1000,5,1);

    // 最大人数，1个字节，0-255 人
    ElvtWklpDat.maxpeople = (uint8_t)workloop.people_max;

    // 最大高度，3个字节，0-9999.9米
    PacksBCD((uint8_t *)&ElvtWklpDat.maxheight, workloop.height_max,5,1);

    // 最大楼层，1个字节，0-255 层
    ElvtWklpDat.maxfloor = (uint8_t)workloop.floor_max;
}

void GetElvtWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // 时间
    ElvtWrnData.clock.sec = alarm_dat[num].sec;
    ElvtWrnData.clock.min = alarm_dat[num].min;
    ElvtWrnData.clock.hour = alarm_dat[num].hour;
    ElvtWrnData.clock.date = alarm_dat[num].date;
    ElvtWrnData.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    ElvtWrnData.clock.mon_single = alarm_dat[num].month & 0x1F;
    ElvtWrnData.clock.year = alarm_dat[num].year;

    // 人员标识码
    ElvtWrnData.name_id = alarm_dat[num].name_id;

    // 包类型 待定
    ElvtWrnData.packtype[0] = alarm_dat[num].attrib&0xff;
    ElvtWrnData.packtype[1] = (alarm_dat[num].attrib>>8)&0xff;

    // 吊重力0.0-9999.9吨
    ElvtWrnData.gravitylift.sensor_OK = alarm_dat[num].weight_flag & 0x01;
    ElvtWrnData.gravitylift.sensor_EN = (alarm_dat[num].weight_flag & 0x02)>>1;
    ElvtWrnData.gravitylift.rev1 = 0;
    ElvtWrnData.gravitylift.warning = alarm_dat[num].weight_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.gravitylift.data, alarm_dat[num].weight_value/1000,5,1);

    // 高度－9999.9　-　9999.9米
    ElvtWrnData.height.sensor_OK = alarm_dat[num].height_flag & 0x01;
    ElvtWrnData.height.sensor_EN = (alarm_dat[num].height_flag & 0x02)>>1;
    ElvtWrnData.height.rev1 = 0;
    ElvtWrnData.height.warning = alarm_dat[num].height_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.height.data, alarm_dat[num].height_value,5,1);

    // 速度0.0-9999.9米/秒
    ElvtWrnData.speed.sensor_OK = alarm_dat[num].speed_flag & 0x01;
    ElvtWrnData.speed.sensor_EN = (alarm_dat[num].speed_flag & 0x02)>>1;
    ElvtWrnData.speed.rev1 = 0;
    ElvtWrnData.speed.warning = alarm_dat[num].speed_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.speed.data, alarm_dat[num].speed_value,5,1);

    // 风速0.0 - 9999.9 m/s
    ElvtWrnData.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    ElvtWrnData.wind.sensor_EN = (alarm_dat[num].wind_flag & 0x02)>>1;
    ElvtWrnData.wind.rev1 = 0;
    ElvtWrnData.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.wind.data, alarm_dat[num].wind_value,5,1);

    // 倾角0.0-9999.9度
    ElvtWrnData.dipangle.sensor_OK = alarm_dat[num].tilt_flag & 0x01;
    ElvtWrnData.dipangle.sensor_EN = (alarm_dat[num].tilt_flag & 0x02)>>1;
    ElvtWrnData.dipangle.rev1 = 0;
    ElvtWrnData.dipangle.warning = alarm_dat[num].tilt_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.dipangle.data, alarm_dat[num].tilt_value,5,1);

    // 电机一
    ElvtWrnData.motor[0].sensor_OK = alarm_dat[num].motor1_flag & 0x01;
    ElvtWrnData.motor[0].sensor_EN = (alarm_dat[num].motor1_flag & 0x02)>>1;
    ElvtWrnData.motor[0].rev1 = 0;
    ElvtWrnData.motor[0].warning = alarm_dat[num].motor1_alarm;

    // 电机二
    ElvtWrnData.motor[1].sensor_OK = alarm_dat[num].motor2_flag & 0x01;
    ElvtWrnData.motor[1].sensor_EN = (alarm_dat[num].motor2_flag & 0x02)>>1;
    ElvtWrnData.motor[1].rev1 = 0;
    ElvtWrnData.motor[1].warning = alarm_dat[num].motor2_alarm;

    // 电机三
    ElvtWrnData.motor[2].sensor_OK = alarm_dat[num].motor3_flag & 0x01;
    ElvtWrnData.motor[2].sensor_EN = (alarm_dat[num].motor3_flag & 0x02)>>1;
    ElvtWrnData.motor[2].rev1 = 0;
    ElvtWrnData.motor[2].warning = alarm_dat[num].motor3_alarm;

    // 人数
    ElvtWrnData.people_flag = alarm_dat[num].people_flag;
    ElvtWrnData.people_alarm = alarm_dat[num].people_alarm;
    ElvtWrnData.people_value = alarm_dat[num].people_value;

    // 楼层
    ElvtWrnData.floor_flag = alarm_dat[num].floor_flag;
    ElvtWrnData.floor_aligned = alarm_dat[num].floor_aligned;
    ElvtWrnData.floor_value = alarm_dat[num].floor_value;

    // 门和限位状态
    ElvtWrnData.door_limit = alarm_dat[num].door_limit;

    // 状态字
    ElvtWrnData.status = alarm_dat[num].alarm_stat;

    // 根据状态字添加报警/违章/故障信息
    if (ElvtWrnData.status&0x01)
    {
        // 添加报警信息 1条
        dptr = ElvtWrnData.warning;
        *dptr++ = alarm_dat[num].alarm_num;
        for (i = 0; i < alarm_dat[num].alarm_num; i++)
        {
            *dptr++ = alarm_dat[num].alarm[i].alarm_code;
            switch (alarm_dat[num].alarm[i].alarm_code)
            {
            case RPT_ALARMID_COLLISION:
                len = 2;
                break;
            case RPT_ALARMID_LIMIT:
                len = 3;
                break;
            case RPT_ALARMID_FORBIDDEN:
            case RPT_ALARMID_OBSTACLE:
            case RPT_ALARMID_WEIGHT:
            case RPT_ALARMID_TORQUE:
            case RPT_ALARMID_WIND:
            case RPT_ALARMID_TILT:
            case RPT_ALARMID_WALK:
            case RPT_ALARMID_DOOR:
            case RPT_ALARMID_PEOPLE:
                len = 1;
                break;

            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].alarm[i].alarm_byte[j];
            }
        }
    }
    if ((ElvtWrnData.status>>1)&0x01)
    {
        // 添加违章信息 1条
        dptr = ElvtWrnData.illegal;
        *dptr++ = alarm_dat[num].against_num;
        for (i = 0; i < alarm_dat[num].against_num; i++)
        {
            *dptr++ = alarm_dat[num].against[i].against_code;
            switch (alarm_dat[num].against[i].against_code)
            {

            case  RPT_AGAINSTID_COLLISION:
                len = 2;
                break;
            case  RPT_AGAINSTID_FORBIDDEN:
            case  RPT_AGAINSTID_OBSTACLE:
            case  RPT_AGAINSTID_LIMIT:
            case  RPT_AGAINSTID_WEIGHT:
            case  RPT_AGAINSTID_TORQUE:
            case  RPT_AGAINSTID_WIND:
            case  RPT_AGAINSTID_TILT:
            case  RPT_AGAINSTID_IDENTITY:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].against[i].against_byte[j];
            }
        }
    }
    if ((ElvtWrnData.status>>2)&0x01)
    {
        // 添加故障信息 1条
        ElvtWrnData.illegal[0] = alarm_dat[num].error_num;
        dptr = ElvtWrnData.fault;
        *dptr++ = alarm_dat[num].error_num;
        for (i = 0; i < alarm_dat[num].error_num; i++)
        {
            *dptr++ = alarm_dat[num].error[i].error_code;
        }
    }

    // 6 Bytes reserved
    for (i=0;i<6;i++)
    {
        ElvtWrnData.rev2[i] = 0;
    }
}

void GetElvtCaliData(void)
{
    // 时间
    ElvtCaliDat.clock.sec = cali_dat.sec;
    ElvtCaliDat.clock.min = cali_dat.min;
    ElvtCaliDat.clock.hour = cali_dat.hour;
    ElvtCaliDat.clock.date = cali_dat.date;
    ElvtCaliDat.clock.week_single = (cali_dat.month >> 5) & 0x07;
    ElvtCaliDat.clock.mon_single = cali_dat.month & 0x1F;
    ElvtCaliDat.clock.year = cali_dat.year;

    // 包类型 待定
    ElvtCaliDat.packtype[0] = period_value.attrib&0xff;
    ElvtCaliDat.packtype[1] = (period_value.attrib>>8)&0xff;

    // 吊重力0.0-9999.9吨
    ElvtCaliDat.gravitylift.sensor_OK = cali_dat.weight_flag & 0x01;
    ElvtCaliDat.gravitylift.sensor_EN = (cali_dat.weight_flag & 0x02)>>1;
    ElvtCaliDat.gravitylift.rev1 = (cali_dat.weight_flag>>2) & 0x3F;
    ElvtCaliDat.gravitylift.warning = cali_dat.weight_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.gravitylift.data, cali_dat.weight/1000.0,6,0);

    // 高度
    ElvtCaliDat.height.sensor_OK = cali_dat.height_flag & 0x01;
    ElvtCaliDat.height.sensor_EN = (cali_dat.height_flag & 0x02)>>1;
    ElvtCaliDat.height.rev1 = (cali_dat.height_flag>>2) & 0x3F;
    ElvtCaliDat.height.warning = cali_dat.height_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.height.data, cali_dat.heigh,6,0);

    // 风速
    ElvtCaliDat.wind.sensor_OK = cali_dat.wind_flag & 0x01;
    ElvtCaliDat.wind.sensor_EN = (cali_dat.wind_flag & 0x02)>>1;
    ElvtCaliDat.wind.rev1 = (cali_dat.wind_flag>>2) & 0x3F;
    ElvtCaliDat.wind.warning = cali_dat.wind_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.wind.data, cali_dat.wind,6,0);

    // 倾角
    ElvtCaliDat.dipangle.sensor_OK = cali_dat.tilt_flag & 0x01;
    ElvtCaliDat.dipangle.sensor_EN = (cali_dat.tilt_flag & 0x02)>>1;
    ElvtCaliDat.dipangle.rev1 = (cali_dat.tilt_flag>>2) & 0x3F;
    ElvtCaliDat.dipangle.warning = cali_dat.tilt_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.dipangle.data, cali_dat.tilt,6,0);

    // 预留5BX4
    ElvtCaliDat.rev1[0].sensor_OK = cali_dat.spare1_flag & 0x01;
    ElvtCaliDat.rev1[0].sensor_EN = (cali_dat.spare1_flag & 0x02)>>1;
    ElvtCaliDat.rev1[0].rev1 = (cali_dat.spare1_flag>>2) & 0x3F;
    ElvtCaliDat.rev1[0].warning = cali_dat.spare1_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.rev1[0].data, cali_dat.spare1_dat,6,0);

    ElvtCaliDat.rev1[1].sensor_OK = cali_dat.spare2_flag & 0x01;
    ElvtCaliDat.rev1[1].sensor_EN = (cali_dat.spare2_flag & 0x02)>>1;
    ElvtCaliDat.rev1[1].rev1 = (cali_dat.spare2_flag>>2) & 0x3F;
    ElvtCaliDat.rev1[1].warning = cali_dat.spare2_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.rev1[1].data, cali_dat.spare2_dat,6,0);

    ElvtCaliDat.rev1[2].sensor_OK = cali_dat.spare3_flag & 0x01;
    ElvtCaliDat.rev1[2].sensor_EN = (cali_dat.spare3_flag & 0x02)>>1;
    ElvtCaliDat.rev1[2].rev1 = (cali_dat.spare3_flag>>2) & 0x3F;
    ElvtCaliDat.rev1[2].warning = cali_dat.spare3_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.rev1[2].data, cali_dat.spare3_dat,6,0);

    ElvtCaliDat.rev1[3].sensor_OK = cali_dat.spare3_flag & 0x01;
    ElvtCaliDat.rev1[3].sensor_EN = (cali_dat.spare3_flag & 0x02)>>1;
    ElvtCaliDat.rev1[3].rev1 = (cali_dat.spare3_flag>>2) & 0x3F;
    ElvtCaliDat.rev1[3].warning = cali_dat.spare3_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.rev1[3].data, cali_dat.spare3_dat,6,0);
}
/*
*********************************************************************************************************
*   函 数 名: AUFElvtRTDat
*
*   功能说明: 自动上传报文：升降机定时实时数据。上行帧,主站响应相同功能码报文。AFN=9AH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFElvtRTDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    rtn.length = 0;

    link_layer_pack(&rtn, 1,0, StatusFlag.resend_times, LFN_DIR1_TIMINGSTATUS);
    rtn.appzone.functioncode = AFN_SELF_ELVTRT;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&elvtdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 人员ID
    sptr = (uint8_t *)&elvtdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // 包类型待定       2B
    sptr = elvtdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // 吊重力     5B
    sptr = (uint8_t *)&elvtdata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&elvtdata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 速度       5B
    sptr = (uint8_t *)&elvtdata.speed;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&elvtdata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&elvtdata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 电机一、二、三   2B X 3
    for (i = 0; i < 3; i++)
    {
        sptr = (uint8_t *)&elvtdata.motor[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // 人数
    *dptr++ = elvtdata.people_flag;
    *dptr++ = elvtdata.people_alarm;
    *dptr++ = elvtdata.people_value;
    rtn.length += 3;

    // 楼层
    *dptr++ = elvtdata.floor_flag;
    *dptr++ = elvtdata.floor_aligned;
    *dptr++ = elvtdata.floor_value;
    rtn.length += 3;

    // 门及限位状态
    *dptr++ = elvtdata.door_limit;
    rtn.length += 1;

    // 预留19B
    for (i = 0; i < 19; i++)
    {
        *dptr++ = elvtdata.rev[i];
        rtn.length += 1;
    }

    // 预留6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = elvtdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFElvtWklpDat
*
*   功能说明: 自动上传报文工作循环数据。上行帧,主站响应相同功能码报文。AFN=9BH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFElvtWklpDat(void)
{
    uint32_t i;
    uint8_t *sptr,*dptr;

    #ifdef NOVAR_TEST
    ElvtWklpDatInit();
    #endif
    //GetElvtWklpDat();

    rtn.length = 0;

    link_layer_pack(&rtn, 0,0, StatusFlag.resend_times, LFN_DIR1_RANDOMWORK);
    rtn.appzone.functioncode = AFN_SELF_ELVTWKLP;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;
    // 工作循环编号：2个字节，低字节在前，高字节在后
    sptr = ElvtWklpDat.workcyclenum;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // 人员标识码
    sptr = (uint8_t *)&ElvtWklpDat.name_id;
    for (i = 0; i < 4; i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // 开始时间6B
    sptr = (uint8_t *)&ElvtWklpDat.starttime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 结束时间
    sptr = (uint8_t *)&ElvtWklpDat.endtime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 最大重量，3个字节，0-9999.9吨
    sptr = (uint8_t *)&ElvtWklpDat.maxlift;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大人数，1个字节，0~255
    *dptr++ = ElvtWklpDat.maxpeople;
    rtn.length += 1;

    // 最大高度，3个字节，-9999.9 - 9999.9米
    sptr = (uint8_t *)&ElvtWklpDat.maxheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // 最大楼层，1个字节，0~255
    *dptr++ = ElvtWklpDat.maxfloor;
    rtn.length += 1;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}
/*
*********************************************************************************************************
*   函 数 名: AUFElvtWrnDat
*
*   功能说明: 自动上传报文工作循环数据。上行帧,主站响应相同功能码报文。AFN=9CH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFElvtWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//变位信息数目
    uint8_t vartype;//变位信息类型

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_ELVTWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&ElvtWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;


    // 人员ID
    sptr = (uint8_t *)&ElvtWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // 包类型待定       2B
    sptr = ElvtWrnData.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // 吊重力     5B
    sptr = (uint8_t *)&ElvtWrnData.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&ElvtWrnData.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 速度       5B
    sptr = (uint8_t *)&ElvtWrnData.speed;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&ElvtWrnData.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&ElvtWrnData.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 电机一、二、三   2B X 3
    for (i = 0; i < 3; i++)
    {
        sptr = (uint8_t *)&ElvtWrnData.motor[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // 人数
    *dptr++ = ElvtWrnData.people_flag;
    *dptr++ = ElvtWrnData.people_alarm;
    *dptr++ = ElvtWrnData.people_value;
    rtn.length += 3;

    // 楼层
    *dptr++ = ElvtWrnData.floor_flag;
    *dptr++ = ElvtWrnData.floor_aligned;
    *dptr++ = ElvtWrnData.floor_value;
    rtn.length += 3;

    // 门及限位状态
    *dptr++ = ElvtWrnData.door_limit;
    rtn.length += 1;

    // 预留19B
    for (i = 0; i < 19; i++)
    {
        *dptr++ = ElvtWrnData.rev[i];
        rtn.length += 1;
    }
    // 变位信息状态字
    *dptr++ = ElvtWrnData.status;
    rtn.length += 1;

    // 判断状态查询是否存在变位信息
    // 存在报警
    if (ElvtWrnData.status&0x01)
    {
        // 添加报警信息
        sptr = ElvtWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// 限位报警3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警
            case 0x05:  // 起重量报警
            case 0x06:  // 力矩报警
            case 0x07:  // 风速报警
            case 0x08: // 倾斜报警
            case 0x0a:  // 门报警
            case 0x0b:  // 人数报警
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在违章
    if ((ElvtWrnData.status>>1)&0x01)
    {
        // 添加违章信息
        sptr = ElvtWrnData.illegal;
        varnum = *sptr++;   // 违章信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警1Byte
            case 0x04:  // 限位报警1Byte
            case 0x05:  // 起重量报警1Byte
            case 0x06:  // 力矩报警1Byte
            case 0x07:  // 风速报警1Byte
            case 0x08: // 倾斜报警1Byte
            case 0x09:  //身份验证1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在故障
    if ((ElvtWrnData.status>>2)&0x01)
    {
        // 添加故障信息
        sptr = ElvtWrnData.fault;
        varnum = *sptr++;   // 故障信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //故障类型编码
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // 预留6B
    sptr = ElvtWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFElvtCaliDat
*
*   功能说明: 自动上传报文升降机标定数据。上行帧,主站响应相同功能码报文。AFN=9DH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFElvtCaliDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    rtn.length = 0;

    link_layer_pack(&rtn, 0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWORK);
    rtn.appzone.functioncode = AFN_SELF_ELVTCALI;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&ElvtCaliDat.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // 包类型待定       2B
    sptr = ElvtCaliDat.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // 吊重力     5B
    sptr = (uint8_t *)&ElvtCaliDat.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 高度       5B
    sptr = (uint8_t *)&ElvtCaliDat.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 风速       5B
    sptr = (uint8_t *)&ElvtCaliDat.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 倾角       5B
    sptr = (uint8_t *)&ElvtCaliDat.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 预留       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&ElvtCaliDat.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
#endif

#ifdef DUSTMON
void GetDustRTData(void)
{
    uint32_t i;

    // 时间
    dustdata.clock.sec = period_value.sec; //period_value.sec;
    dustdata.clock.min = period_value.min;
    dustdata.clock.hour = period_value.hour;
    dustdata.clock.date = period_value.date;
    dustdata.clock.week_single = (period_value.month >> 5) & 0x07;
    dustdata.clock.mon_single = period_value.month & 0x1F;
    dustdata.clock.year = period_value.year;

    // 人员标识码
    dustdata.name_id = period_value.name_id;

    // 包类型 待定
    dustdata.packtype[0] = period_value.attrib&0xff;
    dustdata.packtype[1] = (period_value.attrib>>8)&0xff;

    // pm25 0.0-9999.9ug/m3
    dustdata.pm25.sensor_OK = period_value.pm25_flag & 0x01;
    dustdata.pm25.sensor_EN = (period_value.pm25_flag & 0x02)>>1;
    dustdata.pm25.rev1 = 0;
    dustdata.pm25.warning = period_value.pm25_alarm;
    PacksBCD((uint8_t *)&dustdata.pm25.data, period_value.pm25_value, 5, 1);

    // pm10 0.0-9999.9ug/m3
    dustdata.pm10.sensor_OK = period_value.pm10_flag & 0x01;
    dustdata.pm10.sensor_EN = (period_value.pm10_flag & 0x02)>>1;
    dustdata.pm10.rev1 = 0;
    dustdata.pm10.warning = period_value.pm10_alarm;
    PacksBCD((uint8_t *)&dustdata.pm10.data, period_value.pm10_value, 5, 1);

    // temperature
    dustdata.temperature.sensor_OK = period_value.temperature_flag & 0x01;
    dustdata.temperature.sensor_EN = (period_value.temperature_flag & 0x02)>>1;
    dustdata.temperature.rev1 = 0;
    dustdata.temperature.warning = period_value.temperature_alarm;
    PacksBCD((uint8_t *)&dustdata.temperature.data, period_value.temperature_value, 5, 1);

    // humidity
    dustdata.humidity.sensor_OK = period_value.humidity_flag & 0x01;
    dustdata.humidity.sensor_EN = (period_value.humidity_flag & 0x02)>>1;
    dustdata.humidity.rev1 = 0;
    dustdata.humidity.warning = period_value.humidity_alarm;
    PacksBCD((uint8_t *)&dustdata.humidity.data, period_value.humidity_value, 5, 1);

    // 风速0.0 - 9999.9 m/s
    dustdata.wind.sensor_OK = period_value.wind_flag & 0x01;
    dustdata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    dustdata.wind.rev1 = 0;
    dustdata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&dustdata.wind.data, period_value.wind_value,5,1);

    // 电磁阀一
    dustdata.valve[0].sensor_OK = period_value.valve1_flag & 0x01;
    dustdata.valve[0].sensor_EN = (period_value.valve1_flag & 0x02)>>1;
    dustdata.valve[0].rev1 = 0;
    dustdata.valve[0].warning = period_value.valve1_alarm;

    // 电磁阀二
    dustdata.valve[1].sensor_OK = period_value.valve2_flag & 0x01;
    dustdata.valve[1].sensor_EN = (period_value.valve2_flag & 0x02)>>1;
    dustdata.valve[1].rev1 = 0;
    dustdata.valve[1].warning = period_value.valve2_alarm;

    // 电磁阀三
    dustdata.valve[2].sensor_OK = period_value.valve3_flag & 0x01;
    dustdata.valve[2].sensor_EN = (period_value.valve3_flag & 0x02)>>1;
    dustdata.valve[2].rev1 = 0;
    dustdata.valve[2].warning = period_value.valve3_alarm;

    // 电磁阀四
    dustdata.valve[3].sensor_OK = period_value.valve4_flag & 0x01;
    dustdata.valve[3].sensor_EN = (period_value.valve4_flag & 0x02)>>1;
    dustdata.valve[3].rev1 = 0;
    dustdata.valve[3].warning = period_value.valve4_alarm;

    // noise
    dustdata.noise.sensor_OK = period_value.noise_flag & 0x01;
    dustdata.noise.sensor_EN = (period_value.noise_flag & 0x02)>>1;
    dustdata.noise.rev1 = 0;
    dustdata.noise.warning = period_value.noise_alarm;
    PacksBCD((uint8_t *)&dustdata.noise.data, period_value.noise_value, 5, 1); //35 * log10(0.00523*sensor_value[SENS_NOISE].value+4.708) + 25

    // vane
    dustdata.vane.sensor_OK = period_value.vane_flag & 0x01;
    dustdata.vane.sensor_EN = (period_value.vane_flag & 0x02)>>1;
    dustdata.vane.rev1 = 0;
    dustdata.vane.warning = period_value.vane_alarm;
    PacksBCD((uint8_t *)&dustdata.vane.data, period_value.vane_value, 5, 1); //35 * log10(0.00523*sensor_value[SENS_NOISE].value+4.708) + 25
    // 保留5BX2
    for (i = 0; i < 10;i++)
    {
        dustdata.rev[i] = 0;
    }


    // 保留6B
    for (i = 0; i < 6;i++)
    {
        dustdata.rev1[i] = 0;
    }
}

void GetDustWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // 时间
    DustWrnData.clock.sec = alarm_dat[num].sec; //alarm_dat[num].sec;
    DustWrnData.clock.min = alarm_dat[num].min;
    DustWrnData.clock.hour = alarm_dat[num].hour;
    DustWrnData.clock.date = alarm_dat[num].date;
    DustWrnData.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    DustWrnData.clock.mon_single = alarm_dat[num].month & 0x1F;
    DustWrnData.clock.year = alarm_dat[num].year;

    // 人员标识码
    DustWrnData.name_id = alarm_dat[num].name_id;

    // 包类型 待定
    DustWrnData.packtype[0] = alarm_dat[num].attrib&0xff;
    DustWrnData.packtype[1] = (alarm_dat[num].attrib>>8)&0xff;

    // pm25 0.0-9999.9ug/m3
    DustWrnData.pm25.sensor_OK = alarm_dat[num].pm25_flag & 0x01;
    DustWrnData.pm25.sensor_EN = (alarm_dat[num].pm25_flag & 0x02)>>1;
    DustWrnData.pm25.rev1 = 0;
    DustWrnData.pm25.warning = alarm_dat[num].pm25_alarm;
    PacksBCD((uint8_t *)&DustWrnData.pm25.data, alarm_dat[num].pm25_value, 5, 1);

    // pm10 0.0-9999.9ug/m3
    DustWrnData.pm10.sensor_OK = alarm_dat[num].pm10_flag & 0x01;
    DustWrnData.pm10.sensor_EN = (alarm_dat[num].pm10_flag & 0x02)>>1;
    DustWrnData.pm10.rev1 = 0;
    DustWrnData.pm10.warning = alarm_dat[num].pm10_alarm;
    PacksBCD((uint8_t *)&DustWrnData.pm10.data, alarm_dat[num].pm10_value, 5, 1);

    // temperature
    DustWrnData.temperature.sensor_OK = alarm_dat[num].temperature_flag & 0x01;
    DustWrnData.temperature.sensor_EN = (alarm_dat[num].temperature_flag & 0x02)>>1;
    DustWrnData.temperature.rev1 = 0;
    DustWrnData.temperature.warning = alarm_dat[num].temperature_alarm;
    PacksBCD((uint8_t *)&DustWrnData.temperature.data, alarm_dat[num].temperature_value, 5, 1);

    // humidity
    DustWrnData.humidity.sensor_OK = alarm_dat[num].humidity_flag & 0x01;
    DustWrnData.humidity.sensor_EN = (alarm_dat[num].humidity_flag & 0x02)>>1;
    DustWrnData.humidity.rev1 = 0;
    DustWrnData.humidity.warning = alarm_dat[num].humidity_alarm;
    PacksBCD((uint8_t *)&DustWrnData.humidity.data, alarm_dat[num].humidity_value, 5, 1);

    // 风速0.0 - 9999.9 m/s
    DustWrnData.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    DustWrnData.wind.sensor_EN = (alarm_dat[num].wind_flag & 0x02)>>1;
    DustWrnData.wind.rev1 = 0;
    DustWrnData.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&DustWrnData.wind.data, alarm_dat[num].wind_value,5,1);

    // 电磁阀一
    DustWrnData.valve[0].sensor_OK = alarm_dat[num].valve1_flag & 0x01;
    DustWrnData.valve[0].sensor_EN = (alarm_dat[num].valve1_flag & 0x02)>>1;
    DustWrnData.valve[0].rev1 = 0;
    DustWrnData.valve[0].warning = alarm_dat[num].valve1_alarm;

    // 电磁阀二
    DustWrnData.valve[1].sensor_OK = alarm_dat[num].valve2_flag & 0x01;
    DustWrnData.valve[1].sensor_EN = (alarm_dat[num].valve2_flag & 0x02)>>1;
    DustWrnData.valve[1].rev1 = 0;
    DustWrnData.valve[1].warning = alarm_dat[num].valve2_alarm;

    // 电磁阀三
    DustWrnData.valve[2].sensor_OK = alarm_dat[num].valve3_flag & 0x01;
    DustWrnData.valve[2].sensor_EN = (alarm_dat[num].valve3_flag & 0x02)>>1;
    DustWrnData.valve[2].rev1 = 0;
    DustWrnData.valve[2].warning = alarm_dat[num].valve3_alarm;

    // 电磁阀四
    DustWrnData.valve[3].sensor_OK = alarm_dat[num].valve4_flag & 0x01;
    DustWrnData.valve[3].sensor_EN = (alarm_dat[num].valve4_flag & 0x02)>>1;
    DustWrnData.valve[3].rev1 = 0;
    DustWrnData.valve[3].warning = alarm_dat[num].valve4_alarm;

    // 保留5BX4
    for (i = 0; i < 20;i++)
    {
        DustWrnData.rev[i] = 0;
    }


    // 保留6B
    for (i = 0; i < 6;i++)
    {
        DustWrnData.rev1[i] = 0;
    }

    // 状态字
    DustWrnData.status = alarm_dat[num].alarm_stat;

    // 根据状态字添加报警/违章/故障信息
    if (DustWrnData.status&0x01)
    {
        // 添加报警信息 1条
        dptr = DustWrnData.warning;
        *dptr++ = alarm_dat[num].alarm_num;
        for (i = 0; i < alarm_dat[num].alarm_num; i++)
        {
            *dptr++ = alarm_dat[num].alarm[i].alarm_code;
            switch (alarm_dat[num].alarm[i].alarm_code)
            {
            case RPT_ALARMID_COLLISION:
                len = 2;
                break;
            case RPT_ALARMID_LIMIT:
                len = 3;
                break;
            case RPT_ALARMID_FORBIDDEN:
            case RPT_ALARMID_OBSTACLE:
            case RPT_ALARMID_WEIGHT:
            case RPT_ALARMID_TORQUE:
            case RPT_ALARMID_WIND:
            case RPT_ALARMID_TILT:
            case RPT_ALARMID_WALK:
            case RPT_ALARMID_DOOR:
            case RPT_ALARMID_PEOPLE:
            case RPT_ALARMID_VALVE:
                len = 1;
                break;

            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].alarm[i].alarm_byte[j];
            }
        }
    }
    if ((DustWrnData.status>>1)&0x01)
    {
        // 添加违章信息 1条
        dptr = DustWrnData.illegal;
        *dptr++ = alarm_dat[num].against_num;
        for (i = 0; i < alarm_dat[num].against_num; i++)
        {
            *dptr++ = alarm_dat[num].against[i].against_code;
            switch (alarm_dat[num].against[i].against_code)
            {

            case  RPT_AGAINSTID_COLLISION:
                len = 2;
                break;
            case  RPT_AGAINSTID_FORBIDDEN:
            case  RPT_AGAINSTID_OBSTACLE:
            case  RPT_AGAINSTID_LIMIT:
            case  RPT_AGAINSTID_WEIGHT:
            case  RPT_AGAINSTID_TORQUE:
            case  RPT_AGAINSTID_WIND:
            case  RPT_AGAINSTID_TILT:
            case  RPT_AGAINSTID_IDENTITY:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = alarm_dat[num].against[i].against_byte[j];
            }
        }
    }
    if ((DustWrnData.status>>2)&0x01)
    {
        // 添加故障信息 1条
        DustWrnData.illegal[0] = alarm_dat[num].error_num;
        dptr = DustWrnData.fault;
        *dptr++ = alarm_dat[num].error_num;
        for (i = 0; i < alarm_dat[num].error_num; i++)
        {
            *dptr++ = alarm_dat[num].error[i].error_code;
        }
    }

    // 6 Bytes reserved
    for (i=0;i<6;i++)
    {
        DustWrnData.rev2[i] = 0;
    }
}

/*
*********************************************************************************************************
*   函 数 名: AUFElvtRTDat
*
*   功能说明: 自动上传报文：升降机定时实时数据。上行帧,主站响应相同功能码报文。AFN=9AH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFDustRTDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    rtn.length = 0;

    link_layer_pack(&rtn, 1,0, StatusFlag.resend_times, LFN_DIR1_TIMINGSTATUS);
    rtn.appzone.functioncode = AFN_SELF_DUSTRT;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&dustdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // 人员ID
    sptr = (uint8_t *)&dustdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // 包类型待定       2B
    sptr = dustdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // pm2.5     5B
    sptr = (uint8_t *)&dustdata.pm25;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // pm10       5B
    sptr = (uint8_t *)&dustdata.pm10;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // temperature       5B
    sptr = (uint8_t *)&dustdata.temperature;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // humidity       5B
    sptr = (uint8_t *)&dustdata.humidity;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // wind       5B
    sptr = (uint8_t *)&dustdata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 电磁阀一、二、三、四   2B X 4
    for (i = 0; i < 4; i++)
    {
        sptr = (uint8_t *)&dustdata.valve[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }
    // 噪声数据5B
    sptr = (uint8_t *)&dustdata.noise;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    //风向数据5B
    sptr = (uint8_t *)&dustdata.vane;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    // 预留10B
    for (i = 0; i < 10; i++)
    {
        *dptr++ = dustdata.rev[i];
        rtn.length += 1;
    }

    // 预留6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = dustdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: AUFDustWrnDat
*
*   功能说明: 自动上传报文报警数据。上行帧,主站响应相同功能码报文。AFN=9CH
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFDustWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//变位信息数目
    uint8_t vartype;//变位信息类型

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_DUSTWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&DustWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // 人员ID
    sptr = (uint8_t *)&DustWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // 包类型待定       2B
    sptr = DustWrnData.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // pm2.5     5B
    sptr = (uint8_t *)&DustWrnData.pm25;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // pm10       5B
    sptr = (uint8_t *)&DustWrnData.pm10;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // temperature       5B
    sptr = (uint8_t *)&DustWrnData.temperature;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // humidity       5B
    sptr = (uint8_t *)&DustWrnData.humidity;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // wind       5B
    sptr = (uint8_t *)&DustWrnData.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // 电磁阀一、二、三、四   2B X 4
    for (i = 0; i < 4; i++)
    {
        sptr = (uint8_t *)&DustWrnData.valve[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // 噪声数据5B
    sptr = (uint8_t *)&DustWrnData.noise;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    //风向数据5B
    sptr = (uint8_t *)&DustWrnData.vane;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    // 预留10B
    for (i = 0; i < 10; i++)
    {
        *dptr++ = DustWrnData.rev[i];
        rtn.length += 1;
    }

    // 变位信息状态字
    *dptr++ = DustWrnData.status;
    rtn.length += 1;

    // 判断状态查询是否存在变位信息
    // 存在报警
    if (DustWrnData.status&0x01)
    {
        // 添加报警信息
        sptr = DustWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// 限位报警3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警
            case 0x05:  // 起重量报警
            case 0x06:  // 力矩报警
            case 0x07:  // 风速报警
            case 0x08: // 倾斜报警
            case 0x0a:  // 门报警
            case 0x0b:  // 人数报警
            case 0x20:  // 电磁阀报警
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在违章
    if ((DustWrnData.status>>1)&0x01)
    {
        // 添加违章信息
        sptr = DustWrnData.illegal;
        varnum = *sptr++;   // 违章信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警1Byte
            case 0x04:  // 限位报警1Byte
            case 0x05:  // 起重量报警1Byte
            case 0x06:  // 力矩报警1Byte
            case 0x07:  // 风速报警1Byte
            case 0x08: // 倾斜报警1Byte
            case 0x09:  //身份验证1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在故障
    if ((DustWrnData.status>>2)&0x01)
    {
        // 添加故障信息
        sptr = DustWrnData.fault;
        varnum = *sptr++;   // 故障信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //故障类型编码
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // 预留6B
    sptr = DustWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

#endif

#ifdef UPPLAT
void GetUPPlatRTData(void)
{
    uint32_t i;

    // 时间
   upplatdata.clock.sec = upperiod_value.sec; //upperiod_value.sec;
   upplatdata.clock.min = upperiod_value.min;
   upplatdata.clock.hour = upperiod_value.hour;
   upplatdata.clock.date = upperiod_value.date;
   upplatdata.clock.week_single = (upperiod_value.month >> 5) & 0x07;
   upplatdata.clock.mon_single = upperiod_value.month & 0x1F;
   upplatdata.clock.year = upperiod_value.year;

    // 人员标识码
   upplatdata.name_id = upperiod_value.name_id;

    // 包类型 待定
   upplatdata.packtype[0] = upperiod_value.attrib&0xff;
   upplatdata.packtype[1] = (upperiod_value.attrib>>8)&0xff;

   // weight 0.0-9999.9kg
   upplatdata.weight.sensor_OK = upperiod_value.weight_flag & 0x01;
   upplatdata.weight.sensor_EN = (upperiod_value.weight_flag & 0x02)>>1;
   upplatdata.weight.rev1 = 0;
   upplatdata.weight.warning = upperiod_value.weight_alarm;
   PacksBCD((uint8_t *)&upplatdata.weight.data, upperiod_value.weight_value, 5, 1);

    // cableleft 0.0-9999.9kg
   upplatdata.cableleft.sensor_OK = upperiod_value.cableleft_flag & 0x01;
   upplatdata.cableleft.sensor_EN = (upperiod_value.cableleft_flag & 0x02)>>1;
   upplatdata.cableleft.rev1 = 0;
   upplatdata.cableleft.warning = upperiod_value.cableleft_alarm;
   PacksBCD((uint8_t *)&upplatdata.cableleft.data, upperiod_value.cableleft_value, 5, 1);

    // cableright 0.0-9999.9kg
   upplatdata.cableright.sensor_OK = upperiod_value.cableright_flag & 0x01;
   upplatdata.cableright.sensor_EN = (upperiod_value.cableright_flag & 0x02)>>1;
   upplatdata.cableright.rev1 = 0;
   upplatdata.cableright.warning = upperiod_value.cableright_alarm;
   PacksBCD((uint8_t *)&upplatdata.cableright.data, upperiod_value.cableright_value, 5, 1);

   for (i = 0; i < SENSORMAX; i++)
   {
	   upplatdata.weightsensor[i].sensor_OK = upperiod_value.weightsensor_flag[i] & 0x01;
	   upplatdata.weightsensor[i].sensor_EN = (upperiod_value.weightsensor_flag[i] & 0x02)>>1;
	   upplatdata.weightsensor[i].rev1 = 0;
	   upplatdata.weightsensor[i].warning = upperiod_value.weightsensor_alarm[i];
	   PacksBCD((uint8_t *)&upplatdata.weightsensor[i].data, upperiod_value.weightsensor_value[i], 5, 1);
   }

   // 保留5BX4
    for (i = 0; i < 20;i++)
    {
       upplatdata.rev[i] = 0;
    }


    // 保留6B
    for (i = 0; i < 6;i++)
    {
       upplatdata.rev1[i] = 0;
    }
}

pTX101 AUFUPPlatRTDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;

    rtn.length = 0;

    link_layer_pack(&rtn, 1,0, StatusFlag.resend_times, LFN_DIR1_TIMINGSTATUS);
    rtn.appzone.functioncode = AFN_SELF_UPPLATRT;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&upplatdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // 人员ID
    sptr = (uint8_t *)&dustdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // 包类型待定       2B
    sptr = upplatdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

	// weight    5B
    sptr = (uint8_t *)&upplatdata.weight;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

	// cableleft    5B
    sptr = (uint8_t *)&upplatdata.cableleft;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

	// cableright    5B
    sptr = (uint8_t *)&upplatdata.cableright;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    for (i = 0; i < SENSORMAX; i++)
    {
        // weightsensor    5B
        sptr = (uint8_t *)&upplatdata.weightsensor[i];
		for (j = 0; j < 5;j++)
		{
			*dptr++ = *sptr++;
		}
		rtn.length += 5;
    }

    // 预留20B
    for (i = 0; i < 20; i++)
    {
        *dptr++ = upplatdata.rev[i];
        rtn.length += 1;
    }

    // 预留6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = upplatdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

void GetUPPlatWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // 时间
    UPPlatWrnData.clock.sec = upalarm_dat[num].sec; //alarm_dat[num].sec;
    UPPlatWrnData.clock.min = upalarm_dat[num].min;
    UPPlatWrnData.clock.hour = upalarm_dat[num].hour;
    UPPlatWrnData.clock.date = upalarm_dat[num].date;
    UPPlatWrnData.clock.week_single = (upalarm_dat[num].month >> 5) & 0x07;
    UPPlatWrnData.clock.mon_single = upalarm_dat[num].month & 0x1F;
    UPPlatWrnData.clock.year = upalarm_dat[num].year;

    // 人员标识码
    UPPlatWrnData.name_id = upalarm_dat[num].name_id;

    // 包类型 待定
    UPPlatWrnData.packtype[0] = upalarm_dat[num].attrib&0xff;
    UPPlatWrnData.packtype[1] = (upalarm_dat[num].attrib>>8)&0xff;

    // weight 0.0-9999.9ug/m3
    UPPlatWrnData.weight.sensor_OK = upalarm_dat[num].weight_flag & 0x01;
    UPPlatWrnData.weight.sensor_EN = (upalarm_dat[num].weight_flag & 0x02)>>1;
    UPPlatWrnData.weight.rev1 = 0;
    UPPlatWrnData.weight.warning = upalarm_dat[num].weight_alarm;
    PacksBCD((uint8_t *)&UPPlatWrnData.weight.data, upalarm_dat[num].weight_value, 5, 1);

    // cableleft 0.0-9999.9ug/m3
    UPPlatWrnData.cableleft.sensor_OK = upalarm_dat[num].cableleft_flag & 0x01;
    UPPlatWrnData.cableleft.sensor_EN = (upalarm_dat[num].cableleft_flag & 0x02)>>1;
    UPPlatWrnData.cableleft.rev1 = 0;
    UPPlatWrnData.cableleft.warning = upalarm_dat[num].cableleft_alarm;
    PacksBCD((uint8_t *)&UPPlatWrnData.cableleft.data, upalarm_dat[num].cableleft_value, 5, 1);

    // cableright 0.0-9999.9ug/m3
    UPPlatWrnData.cableright.sensor_OK = upalarm_dat[num].cableright_flag & 0x01;
    UPPlatWrnData.cableright.sensor_EN = (upalarm_dat[num].cableright_flag & 0x02)>>1;
    UPPlatWrnData.cableright.rev1 = 0;
    UPPlatWrnData.cableright.warning = upalarm_dat[num].cableright_alarm;
    PacksBCD((uint8_t *)&UPPlatWrnData.cableright.data, upalarm_dat[num].cableright_value, 5, 1);

    for (i = 0; i < SENSORMAX; i++)
    {
		// weight_sensor[i] 0.0-9999.9ug/m3
		UPPlatWrnData.weightsensor[i].sensor_OK = upalarm_dat[num].weightsensor_flag[i] & 0x01;
		UPPlatWrnData.weightsensor[i].sensor_EN = (upalarm_dat[num].weightsensor_flag[i] & 0x02)>>1;
		UPPlatWrnData.weightsensor[i].rev1 = 0;
		UPPlatWrnData.weightsensor[i].warning = upalarm_dat[num].weightsensor_alarm[i];
		PacksBCD((uint8_t *)&UPPlatWrnData.weightsensor[i].data, upalarm_dat[num].weightsensor_value[i], 5, 1);
    }

    // 保留5BX4
    for (i = 0; i < 20;i++)
    {
        UPPlatWrnData.rev[i] = 0;
    }


    // 保留6B
    for (i = 0; i < 6;i++)
    {
        UPPlatWrnData.rev1[i] = 0;
    }

    // 状态字
    UPPlatWrnData.status = upalarm_dat[num].alarm_stat;

    // 根据状态字添加报警/违章/故障信息
    if (UPPlatWrnData.status&0x01)
    {
        // 添加报警信息 1条
        dptr = UPPlatWrnData.warning;
        *dptr++ = upalarm_dat[num].alarm_num;
        for (i = 0; i < upalarm_dat[num].alarm_num; i++)
        {
            *dptr++ = upalarm_dat[num].alarm[i].alarm_code;
            switch (upalarm_dat[num].alarm[i].alarm_code)
            {
            case RPT_ALARMID_COLLISION:
                len = 2;
                break;
            case RPT_ALARMID_LIMIT:
                len = 3;
                break;
            case RPT_ALARMID_FORBIDDEN:
            case RPT_ALARMID_OBSTACLE:
            case RPT_ALARMID_WEIGHT:
            case RPT_ALARMID_TORQUE:
            case RPT_ALARMID_WIND:
            case RPT_ALARMID_TILT:
            case RPT_ALARMID_WALK:
            case RPT_ALARMID_DOOR:
            case RPT_ALARMID_PEOPLE:
            case RPT_ALARMID_VALVE:
			case RPT_ALARMID_UPPLAT:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = upalarm_dat[num].alarm[i].alarm_byte[j];
            }
        }
    }
    if ((UPPlatWrnData.status>>1)&0x01)
    {
        // 添加违章信息 1条
        dptr = UPPlatWrnData.illegal;
        *dptr++ = upalarm_dat[num].against_num;
        for (i = 0; i < upalarm_dat[num].against_num; i++)
        {
            *dptr++ = upalarm_dat[num].against[i].against_code;
            switch (upalarm_dat[num].against[i].against_code)
            {

            case  RPT_AGAINSTID_COLLISION:
                len = 2;
                break;
            case  RPT_AGAINSTID_FORBIDDEN:
            case  RPT_AGAINSTID_OBSTACLE:
            case  RPT_AGAINSTID_LIMIT:
            case  RPT_AGAINSTID_WEIGHT:
            case  RPT_AGAINSTID_TORQUE:
            case  RPT_AGAINSTID_WIND:
            case  RPT_AGAINSTID_TILT:
            case  RPT_AGAINSTID_IDENTITY:
                len = 1;
                break;
            default:
                // 未处理错误状态
                break;
            }
            for (j = 0; j < len; j++)
            {
                *dptr++ = upalarm_dat[num].against[i].against_byte[j];
            }
        }
    }
    if ((UPPlatWrnData.status>>2)&0x01)
    {
        // 添加故障信息 1条
        UPPlatWrnData.illegal[0] = upalarm_dat[num].error_num;
        dptr = UPPlatWrnData.fault;
        *dptr++ = upalarm_dat[num].error_num;
        for (i = 0; i < upalarm_dat[num].error_num; i++)
        {
            *dptr++ = upalarm_dat[num].error[i].error_code;
        }
    }

    // 6 Bytes reserved
    for (i=0;i<6;i++)
    {
        UPPlatWrnData.rev2[i] = 0;
    }
}

pTX101 AUFUPPlatWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//变位信息数目
    uint8_t vartype;//变位信息类型

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_UPPLATWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&UPPlatWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // 人员ID
    sptr = (uint8_t *)&UPPlatWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // 包类型待定       2B
    sptr = UPPlatWrnData.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

	// weight    5B
    sptr = (uint8_t *)&UPPlatWrnData.weight;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

	// cableleft     5B
    sptr = (uint8_t *)&UPPlatWrnData.cableleft;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

	// cableright     5B
    sptr = (uint8_t *)&UPPlatWrnData.cableright;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    for (i = 0; i < SENSORMAX; i++)
    {
        // weightsensor    5B
        sptr = (uint8_t *)&UPPlatWrnData.weightsensor[i];
		for (j = 0; j < 5;j++)
		{
			*dptr++ = *sptr++;
		}
		rtn.length += 5;
    }

    // 预留20B
    for (i = 0; i < 20; i++)
    {
        *dptr++ = UPPlatWrnData.rev[i];
        rtn.length += 1;
    }

    // 变位信息状态字
    *dptr++ = UPPlatWrnData.status;
    rtn.length += 1;

    // 判断状态查询是否存在变位信息
    // 存在报警
    if (UPPlatWrnData.status&0x01)
    {
        // 添加报警信息
        sptr = UPPlatWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// 限位报警3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警
            case 0x05:  // 起重量报警
            case 0x06:  // 力矩报警
            case 0x07:  // 风速报警
            case 0x08: // 倾斜报警
            case 0x0a:  // 门报警
            case 0x0b:  // 人数报警
			case 0x20:  // 电磁阀报警
			case 0x21:	// 卸料平台报警
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在违章
    if ((UPPlatWrnData.status>>1)&0x01)
    {
        // 添加违章信息
        sptr = UPPlatWrnData.illegal;
        varnum = *sptr++;   // 违章信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //变位信息类型编码
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// 相互干涉报警2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // 禁行区保护报警1Byte
            case 0x03:  // 障碍物碰撞报警1Byte
            case 0x04:  // 限位报警1Byte
            case 0x05:  // 起重量报警1Byte
            case 0x06:  // 力矩报警1Byte
            case 0x07:  // 风速报警1Byte
            case 0x08: // 倾斜报警1Byte
            case 0x09:  //身份验证1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // 存在故障
    if ((UPPlatWrnData.status>>2)&0x01)
    {
        // 添加故障信息
        sptr = UPPlatWrnData.fault;
        varnum = *sptr++;   // 故障信息数目
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //故障类型编码
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // 预留6B
    sptr = UPPlatWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}
#endif

/*
*********************************************************************************************************
*   函 数 名: AUFFngrDat
*
*   功能说明: 自动上传报文：定时实时数据。上行帧,主站响应相同功能码报文。AFN=98H
*
*   形   参: none
*
*   返 回 值: 返回即将发送的上行帧指针
*
*********************************************************************************************************
*/
pTX101 AUFFngrDat(void)
{
    uint32_t i;
    RtcClk clock;
    uint8_t *sptr,*dptr;

    rtn.length = 0;

    link_layer_pack(&rtn, 0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWORK);
    rtn.appzone.functioncode = AFN_SELF_FINGER;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8，userzone

    dptr = rtnbuf;

    // 时标 6B
    GetRTC(&clock);

    for (i = 0; i < 6;i++)
    {
        *dptr++ = ((uint8_t *)(&clock))[i];
    }
    rtn.length += 6;
    // 人员ID 4B
    sptr = (uint8_t *)&fingerdata.staffid;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    rtn.appzone.userdata = rtnbuf;

    // 重新计算CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}

/*
*********************************************************************************************************
*   函 数 名: link_layer_unpack
*
*   功能说明: 判断接收帧的链路层解析是否正确。判断依据主要有，起始字符、长度、地址域、校验码以及结束字符
*
*   形   参: pTX101 pframe —— 接收到的完整下行帧
*
*   返 回 值: uint8_t —— 链路层解码没有错误返回0，如有错误返回为>1错误码
*                       1 —— 该帧不是下行帧
*                       2 —— 链路层功能码无效
*                       3 —— 下发地址和终端地址不匹配
*
*********************************************************************************************************
*/
uint8_t link_layer_unpack(pTX101 pframe)
{
    AddrZone addr;

    // 判断控制域和地址域的合理性，在获取完整帧的时候其他链路信息已经判断
    //
    // 判断控制域方向位是否下行
    // if (pframe->ctrlzone.dir != 0) return 1;

    // 判断控制域功能码是否有意义
    switch (pframe->ctrlzone.func)
    {
    case 1: // 发送/确认 —— 下发命令
    case 2: // 发送/无回答 —— 用户数据
    case 3: // 查询/响应 —— 链路测试
    case 4: // 查询/响应 —— 被测参数
    case 13:// 响应帧 —— 报警或状态参数
        break;
    default:    // 备用或者无意义功能码
        return 2;
    }

    // 判断下行帧地址是否与本机地址相符
    addr = *GetTAddr();
    if (addr.a1_low != pframe->addrzone.a1_low ||
        addr.a1_high == pframe->addrzone.a1_high ||
        addr.a2_low == pframe->addrzone.a2_low ||
        addr.a2_middle == pframe->addrzone.a2_middle ||
        addr.a2_high == pframe->addrzone.a2_high) return 3;

    // 链路层正确，返回0
    return 0;
}

/*
*********************************************************************************************************
*   函 数 名: link_layer_pack
*
*   功能说明: 当上行回复时，对链路层的打包，该函数实现公共数据(起始字符、结束字符、终端地址，协议版本，控制域方向位，CS校验位)
*            的操作，个性数据(控制域DIV，FCB，功能码，帧长，CS校验位)另行操作
*
*   形   参: pTX101 pframe —— 接收到的完整下行帧
*
*   返 回 值: uint8_t —— 链路层解码没有错误返回0，如有错误返回为>1错误码
*
*********************************************************************************************************
*/
void link_layer_pack(pTX101 pframe,uint8_t dir,uint8_t div,uint8_t fcb,uint8_t linkfuncode)
{
    pframe->startb1 = pframe->startb2 = STARTCHAR;
    pframe->endbyte = ENDCHAR;
    pframe->addrzone = *GetTAddr();
    pframe->version = *GetProtocolVersion();
    pframe->ctrlzone.dir = dir; // 上行
    pframe->ctrlzone.div = div;
    pframe->ctrlzone.fcb = fcb;
    pframe->ctrlzone.func = linkfuncode;
}


/*
*********************************************************************************************************
*   函 数 名: GetTAddr
*
*   功能说明: 获取存储区中终端地址
*
*   形   参: none
*
*   返 回 值: pAddrZone —— 地址结构指针
*
*********************************************************************************************************
*/
pAddrZone GetTAddr(void)
{
    tempA.a1_low = device_info.addr[0];
    tempA.a1_high = device_info.addr[1];
    tempA.a2_low = device_info.addr[2];
    tempA.a2_middle = device_info.addr[3];
    tempA.a2_high = device_info.addr[4];

    return &tempA;
}


/*
*********************************************************************************************************
*   函 数 名: GetVersion
*
*   功能说明: 获取协议版本号
*
*   形   参: none
*
*   返 回 值: pVerInfo —— 协议版本的结构指针
*
*********************************************************************************************************
*/
pVerNo GetProtocolVersion(void)
{

    tempV.firstver = device_ver.ver_prtcl&0x0f;
    tempV.secondver = (device_ver.ver_prtcl>>4)&0x0f;

    return &tempV;
}

/*
*********************************************************************************************************
*   函 数 名: GetIPHost
*
*   功能说明: 获取协议版本号
*
*   形   参: num —— IP组号，共四组，默认第一组
*
*   返 回 值: pVerInfo —— 协议版本的结构指针
*
*********************************************************************************************************
*/
void GetIPHost(uint8_t num, uint8_t *str)
{
    __IO uint16_t i,port,len;
    uint8_t strnum[4][4],strport[6];

    if (num > 4) num = 0;
    for (i = 0; i < 4; i++)
    {
        GET_ARRAY_LEN(strnum[i], len);
        //memset(strnum[i], 0, len);
        sprintf((char *)strnum[i], "%d", device_info.ip_port[num][i]);
    }
    GET_ARRAY_LEN(strport, len);
    //memset(strport, 0, len);
    port = device_info.ip_port[num][4]<<8 | device_info.ip_port[num][5];
    sprintf((char *)strport, "%d", port);

    //memset(str, 0, 38);

    sprintf((char *)str, "AT+ZIPSETUPU=1,%s.%s.%s.%s,%s\r\n", strnum[0], strnum[1], strnum[2], strnum[3], strport);
}

/*
*********************************************************************************************************
*   函 数 名: SetRTC
*
*   功能说明: 设置实时时钟
*
*   形   参: 时钟结构
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SetRTC(uint8_t *rtc)
{
    RTC_TimeTypeDef sT;
    RTC_DateTypeDef sD;

    sT.Seconds = *rtc++;
    sT.Minutes = *rtc++;
    sT.Hours = *rtc++;
    sT.TimeFormat = RTC_HOURFORMAT12_AM;
    sT.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sT.StoreOperation = RTC_STOREOPERATION_RESET;
    sD.Date = *rtc++;
    sD.Month = (*rtc)&0x1F;
    sD.WeekDay = ((*rtc++)>>5)&0x07;
    sD.Year = *rtc;

    // 获取下行帧中的时间信息，并放入RTC
    //system_parameter.clk = *((pRtcClk)rtn.appzone.userdata);
    HAL_RTC_SetTime(&hrtc, &sT, FORMAT_BCD);
    HAL_RTC_SetDate(&hrtc, &sD, FORMAT_BCD);
}

/*
*********************************************************************************************************
*   函 数 名: GetRTC
*
*   功能说明: 获取实时时钟
*
*   形   参: 时钟结构
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GetRTC(pRtcClk realtime)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    // 获取RTC时钟
    HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

    realtime->sec = sTime.Seconds;
    realtime->min = sTime.Minutes;
    realtime->hour = sTime.Hours;
    realtime->date = sDate.Date;
    realtime->week_single = sDate.WeekDay;
    realtime->mon_single = sDate.Month;
    realtime->year = sDate.Year;
}
/*
*********************************************************************************************************
*   函 数 名: GetCRC
*
*   功能说明: 获取完整帧结构中的CRC校验字节
*
*   形   参: 帧类型 0 —— 单帧
*                  >0 —— 多帧，帧号为(frametype-1)
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint8_t GetCRC(uint8_t frametype)
{
    uint32_t i;
    uint8_t *ptr1,*ptr2;

    // 重新计算CRC
    if (frametype) // 多帧
    {
        //for (i=0;i<mrtn.mframe_num;i++)
        {
            ptr1 = crcbuf;
            ptr2 = (uint8_t *)&mrtn.frame[frametype-1].ctrlzone;
            *ptr1++ = *ptr2;
            ptr2 = (uint8_t *)&mrtn.frame[frametype-1].framenum;
            for (i = 0; i < 2; i++)
            {
                *ptr1++ = *ptr2++;
            }
            ptr2 = (uint8_t *)&mrtn.frame[frametype-1].framecnt;
            for (i = 0; i < 2; i++)
            {
                *ptr1++ = *ptr2++;
            }
            *ptr1++ = mrtn.frame[frametype-1].addrzone.a1_low;
            *ptr1++ = mrtn.frame[frametype-1].addrzone.a1_high;
            *ptr1++ = mrtn.frame[frametype-1].addrzone.a2_low;
            *ptr1++ = mrtn.frame[frametype-1].addrzone.a2_middle;
            *ptr1++ = mrtn.frame[frametype-1].addrzone.a2_high;
            ptr2 = (uint8_t *)&mrtn.frame[frametype-1].version;
            *ptr1++ = *ptr2;
            *ptr1++ = mrtn.frame[frametype-1].appzone.functioncode;
            ptr2 = (uint8_t *)(mrtn.frame[frametype-1].appzone.userdata);
            for (i = 0; i < mrtn.frame[frametype - 1].length - 12; i++)
            {
                *ptr1++ = *ptr2++;
            }
            mrtn.frame[frametype-1].cs = GetCRC7ByLeftByTable(crcbuf, mrtn.frame[frametype-1].length);
        }
        return 1;
    }
    else    // 单帧
    {// 拷贝控制域1、地址域5、版本1、功能码
        ptr1 = crcbuf;
        //*ptr1++ = rtn.startb1;
        //*ptr1++ = rtn.length;
        //*ptr1++ = rtn.startb2;
        ptr2 = (uint8_t *)&rtn.ctrlzone;
        *ptr1++ = *ptr2;
        *ptr1++ = rtn.addrzone.a1_low;
        *ptr1++ = rtn.addrzone.a1_high;
        *ptr1++ = rtn.addrzone.a2_low;
        *ptr1++ = rtn.addrzone.a2_middle;
        *ptr1++ = rtn.addrzone.a2_high;
        ptr2 = (uint8_t *)&rtn.version;
        *ptr1++ = *ptr2;
        *ptr1++ = rtn.appzone.functioncode;
        if (rtn.length > 8)
        {
            ptr2 = (uint8_t *)(rtn.appzone.userdata);
            for (i = 0; i < rtn.length-8;i++)
            {
                *ptr1++ = *ptr2++;
            }
        }
        rtn.cs = GetCRC7ByLeftByTable(crcbuf, rtn.length);

        return rtn.cs;
    }
}

/*
*********************************************************************************************************
*   函 数 名: RNG_Get_RandomRange
*
*   功能说明: 获取给出范围内的随机整数
*
*   形   参: uint32_t xmin, uint32_t xmax —— 随机数包含区间
*
*   返 回 值: 随机整数
*
*********************************************************************************************************
*/
uint32_t RNG_Get_RandomRange(uint32_t xmin, uint32_t xmax)
{
    uint32_t random;
    float tmp;

    random = HAL_RNG_GetRandomNumber(&hrng);
    tmp = (random / pow(2 , 32))*(xmax-xmin)+xmin+0.5;
    return (uint32_t)tmp;
}

/*
*********************************************************************************************************
*   函 数 名: rtn2frame
*
*   功能说明: 把帧结构转换成数组
*
*   形   参:  pTX101 rptr —— 帧结构指针
*             uint8_t *pframe —— 数组
*
*   返 回 值: 帧长度
*
*********************************************************************************************************
*/
uint16_t rtn2frame(pTX101 rptr,uint8_t *pframe)
{
    static uint8_t i,*sptr,*dptr;

    if (!rptr) return 0;

    dptr = pframe;
    //sptr = rtn;

    *dptr++ = rptr->startb1;
    *dptr++ = rptr->length;
    *dptr++ = rptr->startb2;
    sptr = (uint8_t *)&rptr->ctrlzone;
    *dptr++ = *sptr;
    sptr = (uint8_t *)&rptr->addrzone;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    sptr = (uint8_t *)&rptr->version;
    *dptr++ = *sptr++;
    *dptr++ = rptr->appzone.functioncode;
    sptr = rptr->appzone.userdata;
    for (i = 0; i < rptr->length - 8;i++)
    {
        *dptr++ = *sptr++;
    }
    *dptr++ = rptr->cs;
    *dptr++ = rptr->endbyte;

    return (rptr->length + 5);
}

/*
*********************************************************************************************************
*   函 数 名: rtn2frame
*
*   功能说明: 把帧结构转换成数组
*
*   形   参:  pTX101 rptr —— 帧结构指针
*             uint8_t *pframe —— 数组
*
*   返 回 值: 帧长度
*
*********************************************************************************************************
*/
uint16_t mrtn2frame(pMultiTX101 rptr,uint8_t *pframe)
{
    static uint8_t i,*sptr,*dptr;

    if (!rptr) return 0;

    dptr = pframe;
    //sptr = rtn;

    *dptr++ = rptr->startb1;
    *dptr++ = rptr->length;
    *dptr++ = rptr->startb2;
    sptr = (uint8_t *)&rptr->ctrlzone;
    *dptr++ = *sptr++;
    sptr = (uint8_t *)&rptr->framenum;
    *dptr++ = *sptr++;
    *dptr++ = *sptr++;
    sptr = (uint8_t *)&rptr->framecnt;
    *dptr++ = *sptr++;
    *dptr++ = *sptr++;
    sptr = (uint8_t *)&rptr->addrzone;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    sptr = (uint8_t *)&rptr->version;
    *dptr++ = *sptr++;
    *dptr++ = rptr->appzone.functioncode;
    sptr = rptr->appzone.userdata;
    for (i = 0; i < rptr->length - 12;i++)
    {
        *dptr++ = *sptr++;
    }
    *dptr++ = rptr->cs;
    *dptr++ = rptr->endbyte;

    return (rptr->length + 5);
}
// follow function for GPRS Tx Queue
//  make frame which will be send
/*
*********************************************************************************************************
*   函 数 名: FrmLnkLog
*
*   功能说明: 组织链路登陆发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmLnkLog(uint8_t *buf)
{
    TIP_login();
    return rtn2frame(&rtn, buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmHrtDat
*
*   功能说明: 组织心跳包发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmHrtDat(uint8_t *buf)
{
    frame_link_chk(DATAFIELD_ONLINE);    // 发送登录帧
    return rtn2frame(&rtn, buf);
}

#ifdef TOWERBOX
/*
*********************************************************************************************************
*   函 数 名: FrmDevLctSet
*
*   功能说明: 设置终端地理位置/经纬度并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevLctSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevLct(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevLctQry
*
*   功能说明: 查询终端地理位置/经纬度并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevLctQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevLct(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTowRtDat
*
*   功能说明: 组织实时定时数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmTowRtDat(uint8_t *buf)
{
    if (period_value.flag != BUFSTAT_READY) return 0;
    GetTowRTDat();
    period_value.flag = BUFSTAT_NULL;

    return rtn2frame(AUFTowRTDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTowWklpDat
*
*   功能说明: 组织工作循环数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmTowWklpDat(uint8_t *buf)
{
    if (workloop.flag != BUFSTAT_READY) return 0;
    GetTowWklpDat();
    workloop.flag = BUFSTAT_NULL;
    return rtn2frame(AUFTowWklpDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTowWrnDat
*
*   功能说明: 组织实时报警数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmTowWrnDat(uint8_t *buf)
{
    uint32_t i;
    for (i = 0; i < RPT_ALARM_BUFSIZE; i++)
    {
        if (alarm_dat[i].flag == BUFSTAT_READY)
        {
            GetTowWrnDat(i);
            alarm_dat[i].flag = BUFSTAT_NULL;
            break;
        }
    }
    if (i > 5) return 0;
    else return rtn2frame(AUFTowWrnDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTowCaliDat
*
*   功能说明: 组织实时标定发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmTowCaliDat(uint8_t *buf)
{
    if (cali_dat.flag != BUFSTAT_READY) return 0;
    GetTowCaliDat();
    cali_dat.flag = BUFSTAT_NULL;

    return rtn2frame(AUFTowCaliDat(), buf);
}
#endif
/*
*********************************************************************************************************
*   函 数 名: FrmFngrDat
*
*   功能说明: 组织指纹上报发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmFngrDat(uint8_t *buf)
{
    if (fingerdata.flag != BUFSTAT_READY) return 0;
    fingerdata.flag = BUFSTAT_NULL;

    return rtn2frame(AUFFngrDat(), buf);
}

#ifdef ELIVATOR
/*
*********************************************************************************************************
*   函 数 名: FrmElvtRtDat
*
*   功能说明: 组织升降机实时定时数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmElvtRtDat(uint8_t *buf)
{
    if (period_value.flag != BUFSTAT_READY) return 0;
    GetElvtRTData();
    period_value.flag = BUFSTAT_NULL;

    return rtn2frame(AUFElvtRTDat(), buf);
}

/*
*********************************************************************************************************
*   函 数 名: FrmTowWklpDat
*
*   功能说明: 组织工作循环数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmElvtWklpDat(uint8_t *buf)
{
    if (workloop.flag != BUFSTAT_READY) return 0;
    GetElvtWklpDat();
    workloop.flag = BUFSTAT_NULL;
    return rtn2frame(AUFElvtWklpDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmElvtCaliDat
*
*   功能说明: 组织实时标定发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmElvtCaliDat(uint8_t *buf)
{
    if (cali_dat.flag != BUFSTAT_READY) return 0;
    GetElvtCaliData();
    cali_dat.flag = BUFSTAT_NULL;

    return rtn2frame(AUFElvtCaliDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmElvtWrnDat
*
*   功能说明: 组织升降机实时报警数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmElvtWrnDat(uint8_t *buf)
{
    uint32_t i;
    for (i = 0; i < RPT_ALARM_BUFSIZE; i++)
    {
        if (alarm_dat[i].flag == BUFSTAT_READY)
        {
            GetElvtWrnDat(i);
            alarm_dat[i].flag = BUFSTAT_NULL;
            break;
        }
    }
    if (i > 5) return 0;
    else return rtn2frame(AUFElvtWrnDat(), buf);
}
#endif
#ifdef DUSTMON

/*
*********************************************************************************************************
*   函 数 名: FrmDustRtDat
*
*   功能说明: 组织扬尘监测实时定时数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmDustRtDat(uint8_t *buf)
{
    if (period_value.flag != BUFSTAT_READY) return 0;
    GetDustRTData();
    period_value.flag = BUFSTAT_NULL;

    return rtn2frame(AUFDustRTDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDustWrnDat
*
*   功能说明: 组织扬尘监测实时报警数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmDustWrnDat(uint8_t *buf)
{
    uint32_t i;
    for (i = 0; i < RPT_ALARM_BUFSIZE; i++)
    {
        if (alarm_dat[i].flag == BUFSTAT_READY)
        {
            GetDustWrnDat(i);
            alarm_dat[i].flag = BUFSTAT_NULL;
            break;
        }
    }
    if (i > 5) return 0;
    else return rtn2frame(AUFDustWrnDat(), buf);
}
#endif
#ifdef UPPLAT
/*
*********************************************************************************************************
*   函 数 名: FrmUPPlatRtDat
*
*   功能说明: 组织卸料平台实时定时数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmUPPlatRtDat(uint8_t *buf)
{
    if (upperiod_value.flag != BUFSTAT_READY) return 0;
    GetUPPlatRTData();
    upperiod_value.flag = BUFSTAT_NULL;

    return rtn2frame(AUFUPPlatRTDat(), buf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmUPPlatWrnDat
*
*   功能说明: 组织卸料平台报警数据发送帧
*
*   形   参: uint8_t
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmUPPlatWrnDat(uint8_t *buf)
{
    uint32_t i;
    for (i = 0; i < RPT_ALARM_BUFSIZE; i++)
    {
        if (upalarm_dat[i].flag == BUFSTAT_READY)
        {
            GetUPPlatWrnDat(i);
            upalarm_dat[i].flag = BUFSTAT_NULL;
            break;
        }
    }
    if (i > 5) return 0;
    else return rtn2frame(AUFUPPlatWrnDat(), buf);
}

#endif 

/*
*********************************************************************************************************
*   函 数 名: FrmDevAddrDatSet
*
*   功能说明: 设置终端地址发送帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevAddrDatSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetDevAddr(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevAddrDatQry
*
*   功能说明: 查询终端地址并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevAddrDatQry(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamQryDevaddr(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevRtcDatSet
*
*   功能说明: 设置终端时钟并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevRtcDatSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetDevRtc(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevRtcQry
*
*   功能说明: 查询终端时钟并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevRtcQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRtc(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevWkModSet
*
*   功能说明: 设置终端工作模式并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevWkModSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetWkMod(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevWkModQry
*
*   功能说明: 查询终端工作模式并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevWkModQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryWkMod(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevSnsrTypSet
*
*   功能说明: 设置终端传感器类型并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrTypSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetSnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevSnsrTypQry
*
*   功能说明: 查询终端传感器类型并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrTypQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQrySnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevSnsrCfgSet
*
*   功能说明: 设置终端传感器参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrCfgSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetSnsrCfg(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevSnsrCfgQry
*
*   功能说明: 查询终端传感器参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrCfgQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQrySnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevIpPortSet
*
*   功能说明: 设置终端存储的中心站IP地址和端口号并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevIpPortSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevIpPort(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevIpPortQry
*
*   功能说明: 查询终端存储的中心站IP地址和端口号并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevIpPortQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevIpPort(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevHrtIntvlSet
*
*   功能说明: 设置终端心跳间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevHrtIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetHrtIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevHrtIntvlQry
*
*   功能说明: 查询终端心跳间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevHrtIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryHrtIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevLnkReconIntvlSet
*
*   功能说明: 设置终端链路重连间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevLnkReconIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevLnkReconIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevLnkReconIntvlQry
*
*   功能说明: 查询终端链路重连间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevLnkReconIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevLnkReconIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevLnkReconIntvlSet
*
*   功能说明: 设置终端历史数据存盘间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSavIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevRecIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevSavIntvlQry
*
*   功能说明: 查询终端历史数据存盘间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevSavIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRecIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevRtdRptIntvlSet
*
*   功能说明: 设置终端实时数据上报间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevRtdRptIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevRTDReptIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevRtdRptIntvlQry
*
*   功能说明: 查询终端实时数据上报间隔并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevRtdRptIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRTDReptIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevUpdSet
*
*   功能说明: 设置终端升级数据并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevUpdSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevUpd(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevVerInfoQry
*
*   功能说明: 查询终端版本信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevVerInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevVerInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevPwdSet
*
*   功能说明: 设置终端密码并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevPwdSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevPwd(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmDevPwdQry
*
*   功能说明: 查询终端终端密码并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmDevPwdQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevPwd(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmFngrDatSet
*
*   功能说明: 设置指纹数据变更并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmFngrDatSet(uint8_t *rf,uint8_t *sf)
{
    return mrtn2frame(ParamSetFngrDat(&mrtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmFngrDatDel
*
*   功能说明: 删除指纹数据并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmFngrDatDel(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(Param_Del_FngrDat(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmRestart
*
*   功能说明: 服务器下行重启终端命令，死循环等待看门狗超时复位
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t FrmRestart(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(Param_Restart(&rtn), sf);
}

#ifdef TOWERBOX
/*
*********************************************************************************************************
*   函 数 名: FrmTwrInfoSet
*
*   功能说明: 设置塔机基本信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrInfoSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrInfoQry
*
*   功能说明: 查询塔机基本信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrPrtcZoneSet
*
*   功能说明: 设置塔机保护区并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrPrtcZoneSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetPrtcZone(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrPrtcZoneQry
*
*   功能说明: 查询塔机保护区并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrPrtcZoneQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryPrtcZone(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrLmtSet
*
*   功能说明: 设置塔机限位信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrLmt(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrLmtQry
*
*   功能说明: 查询塔机限位信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrLmtQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrLmt(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrTorqSet
*
*   功能说明: 设置塔机力矩信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrTorqSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrTorque(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrTorqQry
*
*   功能说明: 查询塔机力矩信息并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrTorqQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrTorque(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrCaliSet
*
*   功能说明: 设置塔机标定参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrCaliSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrCali(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrCaliQry
*
*   功能说明: 查询塔机标定参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrCaliQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrCali(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrLiftSet
*
*   功能说明: 设置塔机顶升数据并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrLiftSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrLift(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmTwrLiftQry
*
*   功能说明: 查询塔机顶升数据并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmTwrLiftQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrLift(&rtn), sf);
}
#endif

#ifdef ELIVATOR
/*
*********************************************************************************************************
*   函 数 名: FrmElvtInfoSet
*
*   功能说明: 设置升降机基本结构参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmElvtInfoSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetElvtInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmElvtInfoQry
*
*   功能说明: 查询设置升降机基本结构参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmElvtInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryElvtInfo(&rtn), sf);
}


/*
*********************************************************************************************************
*   函 数 名: FrmElvtFloorSet
*
*   功能说明: 设置升降机基本结构参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmElvtFloorSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetElvtFloor(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmElvtFloorQry
*
*   功能说明: 查询设置升降机基本结构参数并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmElvtFloorQry(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamQryElvtFloor(&rtn), sf);
}
#endif

#ifdef DUSTMON
/*
*********************************************************************************************************
*   函 数 名: FrmValveLmtSet
*
*   功能说明: 设置扬尘监测电磁阀阈值PM2.5并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetValveLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   函 数 名: FrmValveLmtSet_Ext
*
*   功能说明: 设置扬尘监测电磁阀PM10阈值并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtSet_Ext(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetValveLmt_Ext(&rtn), sf);
}

/*
*********************************************************************************************************
*   函 数 名: FrmValveLmtQry
*
*   功能说明: 查询扬尘监测电磁阀PM2.5阈值并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryValveLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   函 数 名: FrmValveLmtQry_Ext
*
*   功能说明: 查询扬尘监测电磁阀PM10阈值并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtQry_Ext(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryValveLmt_Ext(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmValveManual
*
*   功能说明: 设置扬尘监测电磁阀手动开合并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmValveManual(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetValveMan(&rtn), sf);
}
/*
*********************************************************************************************************
*   函 数 名: FrmNotice
*
*   功能说明: 设置扬尘监测OLED信息下发并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmNotice(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetNotice(&rtn), sf);
}
#endif
#ifdef UPPLAT
/*
*********************************************************************************************************
*   函 数 名: FrmUPLmtSet
*
*   功能说明: 设置卸料平台阈值并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmUPLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetUPLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   函 数 名: FrmValveLmtQry
*
*   功能说明: 查卸料平台阈值并组织应答帧
*
*   形   参: uint8_t *rf —— 收到的下行帧
*            uint8_t *sf —— 应答的上行帧
*
*   返 回 值: 应答帧长
*
*********************************************************************************************************
*/
uint16_t FrmUPLmtQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryUPLmt(&rtn), sf);
}
#endif 

#undef _LOCAL_PRTC
/********************************* endline **********************************/
