#define _LOCAL_GPRS

#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "app_user.h"
#include "app_sensor.h"
#include "app_comser.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "app_network.h"
#include "app_mg2639d.h"
#include "app_sim7600ce.h"

/**************************************************************/
uint8_t strbuf[256];    // 该数据缓冲区用来组织AT命令帧
extern uint8_t data;

uint8_t *port = "8086";
uint8_t *hostip = "122.114.22.87"; //公司SXJIANSHE.COM
uint8_t or_At[] = "AT\r\n"; //4
uint8_t or_Ate0[] = "ATE0\r\n"; //6
uint8_t or_Anode[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n"; //27
uint8_t or_AstatU[] = "AT+ZIPSTATUSU=1\r\n";//17
uint8_t or_Acloseu[] = "AT+ZIPCLOSEU=1\r\n";//16
uint8_t or_Acgatt[] = "AT+CGATT=1\r\n"; //12
uint8_t or_Aopen[] = "AT+ZPPPOPEN\r\n"; //13
uint8_t or_Aipset[38] = "AT+ZIPSETUPU=1,122.114.22.87,8086\r\n"; //最大38：AT+ZIPSETUPU=1,255.255.255.255,65535\r\n
uint8_t or_Asendu[25] = "AT+ZIPSENDU=1,";
uint8_t re_ok[] = "\r\nOK\r\n"; //6
uint8_t re_Atok[] = "\r\nAT\r\n\r\nOK\r\n"; //10
uint8_t re_Ateok[] = "\r\nATE0\r\n\r\nOK\r\n"; //12
uint8_t re_AstatU[] = "\r\n+ZIPSTATUSU: ESTABLISHED\r\n";
uint8_t re_Aclostu[] = "\r\n+ZIPCLOSEU:OK\rn";//15
uint8_t re_Aopen[] = "\r\n+ZPPPOPEN:CONNECTED\r\n\r\nOK\r\n"; //29
uint8_t re_Aopen2[] = "\r\n+ZPPPOPEN:ESTABLISHED\r\n\r\nOK\r\n";
uint8_t re_Aipset[] =  "\r\n+ZIPSETUPU:CONNECTED\r\n\r\nOK\r\n"; //30
uint8_t re_Aipset2[] = "\r\n+ZIPSETUPU:ESTABLISHED\r\n\r\nOK\r\n"; //31
uint8_t re_Arecv[] = "\r\n+ZIPRECVU:";
uint8_t re_Asendu[] = "\r\n>";
uint8_t or_re1[30];

/*
*********************************************************************************************************
*   函 数 名: GPRS_2639_POWON
*
*   功能说明: GPRS 2639 上电程序，打开供电，模拟按键上电
*
*   形   参: none
*
*   返 回 值:uint8_t GPRS_POWON_SUCCESS GPRS上电成功
*                   GPRS_POWON_FAIL GPRS上电失败
*
*********************************************************************************************************
*/
uint8_t gprs_2639_powon(void)
{
    uint8_t cnt = 0;

    while (!_IS_GPRS_ON())
    {
        // 电源打开
        _VGPRS_ON();
        //按下按键
        _GPRS_PWKEY_PUSH();
        //延时5S
        HAL_Delay(5000);
        //释放按键
        _GPRS_PWKEY_RELEASE();

        cnt++;
        if (cnt > 2) return GPRS_POWON_FAIL;
    }

    return GPRS_POWON_SUCCESS;
}
/*
*********************************************************************************************************
*   函 数 名: GPRS_2639_INIT
*
*   功能说明: GPRS 2639 初始化程序
*
*   形   参: none
*
*   返 回 值:uint8_t GPRS_POWON_SUCCESS     GPRS上电成功
*                   GPRS_POWON_FAIL        GPRS上电失败
*                   GPRS_RETURN_FAIL       GPRS无应答
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*   函 数 名: gprs_power_reset
*
*   功能说明: gprs模块硬件复位
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL gprs_power_reset(void)
{
    static uint8_t step = 0;
    static uint32_t tick;

    BOOL ret = (BOOL)FALSE;
    switch (step)
    {
    case 0 :    // entrance
        if (_IS_GPRS_ON())
        {
            step = 1;
            tick = HAL_GetTick();
            _GPRS_PWKEY_PUSH();
        } else
        {
            step = 2;
        }
        break;

    case 1 :
        if ((HAL_GetTick() - tick) >= 5000)    // duration of key-down
        {
            step = 2;
            tick = HAL_GetTick();
            _GPRS_PWKEY_RELEASE();
        }
        break;

    case 2 :
        if ((HAL_GetTick() - tick) >= 2000)    // duration of power-down
        {
            step = 3;
            tick = HAL_GetTick();
            _GPRS_PWKEY_PUSH();
        }
        break;

    case 3 :
        if ((HAL_GetTick() - tick) >= 5000)    // duration of key-down
        {
            step = 0;
            tick = HAL_GetTick();
            _GPRS_PWKEY_RELEASE();

            ret = (BOOL)TRUE;
        }
        break;
    default:
        step = 0;
        ret = (BOOL)FALSE;
        break;
    }

    return (ret);
}


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
}
/*
*********************************************************************************************************
*   函 数 名: gprs_pow_on
*
*   功能说明: 非阻塞上电初始化
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL gprs_pow_on(void)
{
    static uint32_t tick = 0;
    static uint8_t step = 0;

    BOOL ret = (BOOL)FALSE;

    if (smflag.IS_POW_ON) return (BOOL)TRUE;
    switch (step)
    {
    case 0:
        _VGPRS_OFF();
        tick = HAL_GetTick();
        step = 1;
        break;
    case 1:
        if (HAL_GetTick() - tick > 2000)
        {
            step = 2;
        }
        break;
    case 2:
        if (_IS_GPRS_ON())
        {
            step = 0;
            tick = 0;
            ret = (BOOL)TRUE;
            smflag.IS_POW_ON = 1;
        }
        else step = 3;
        break;
    case 3:
        // 电源打开
        _VGPRS_ON();
        //按下按键
        _GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 4;
        break;
    case 4:
        //延时5S
        if (HAL_GetTick() - tick > 5000)
        {
            //释放按键
            _GPRS_PWKEY_RELEASE();
            if (_IS_GPRS_ON())
            {
                step = 0;
                tick = 0;
                ret = (BOOL)TRUE;
                smflag.IS_POW_ON = 1;
            }
        }
        break;
    }

    return ret;

}

/*
*********************************************************************************************************
*   函 数 名: gprs_pow_off
*
*   功能说明: 非阻塞关机
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL gprs_pow_off(void)
{
    static uint32_t tick = 0;
    static uint8_t step = 0;

    BOOL ret = (BOOL)FALSE;

    switch (step)
    {
    case 0:
        if (!_IS_GPRS_ON())
        {
            step = 0;
            tick = 0;
            ret = (BOOL)TRUE;
        }
        else step = 1;
        break;
    case 1:
        // 电源打开
        _VGPRS_ON();
        //按下按键
        _GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 2;
        break;
    case 2:
        //延时5S
        if (HAL_GetTick() - tick > 5000)
        {
            //释放按键
            _GPRS_PWKEY_RELEASE();
            if (!_IS_GPRS_ON())
            {
                step = 0;
                tick = 0;
                ret = (BOOL)TRUE;
                smflag.IS_POW_ON = 0;
            }
        }
        break;
    }

    return ret;

}

/*
*********************************************************************************************************
*   函 数 名: gprs_udp_init
*
*   功能说明: udp初始化
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL gprs_udp_init(void)
{
    //static uint32_t tick = 0;
    static uint8_t step = 0;

    BOOL ret = (BOOL)FALSE;

    switch (step)
    {
    case 0:
        if (gprs_at_cycle(or_At, re_ok,re_Atok)) step = 1;
        break;
    case 1:
        if (gprs_at_cycle(or_Ate0, re_ok,re_Ateok)) step = 2;
        break;
    case 2:
        if (gprs_at_cycle(or_Aopen, re_Aopen,re_Aopen2)) step = 3;
        break;
    case 3:
        if (gprs_at_cycle(or_Aipset, re_Aipset,re_Aipset2))
        {
            step = 0;
            //tick = 0;
            ret = (BOOL)TRUE;
        }
        break;
    }
/*
    switch (step)
    {
    case 0:
        if (gprs_pow_off()) step = 1;
        break;
    case 1:
        if (gprs_pow_on()) step = 2;
        break;
    case 2:
        if (gprs_at_cycle(or_At, re_Atok)) step = 4;
        else step = 3;
        break;
    case 3:
        if (gprs_at_cycle(or_At, re_ok)) step = 4;
        else step = 2;
        break;
    case 4:
        if (gprs_at_cycle(or_Ate0, re_Ateok)) step = 6;
        else step = 5;
        break;
    case 5:
        if (gprs_at_cycle(or_Ate0, re_ok)) step = 6;
        else step = 4;
        break;
    case 6:
        if (gprs_at_cycle(or_AstatU,re_AstatU))
        {
            step = 0;
            tick = 0;
            ret = TRUE;
        }
        else
        {
            step = 7;
        }
        break;
    case 7:
        if (gprs_at_cycle(or_Aopen, re_Aopen)) step = 9;
        else step = 8;
        break;
    case 8:
        if (gprs_at_cycle(or_Aopen, re_Aopen2)) step = 9;
        else step = 7;
        break;
    case 9:
        if (gprs_at_cycle(or_Aipset, re_Aipset))
        {
            step = 0;
            tick = 0;
            ret = TRUE;
        }
        else step= 10;
        break;
    case 10:
        if (gprs_at_cycle(or_Aipset, re_Aipset2))
        {
            step = 0;
            tick = 0;
            ret = TRUE;
        }
        else step = 9;
        break;
    default:
        step = 0;
        tick = 0;
        ret = FALSE;
        break;
    }
*/
    return ret;
}

BOOL gprs_at_cycle(uint8_t *at_cmd, uint8_t *ack,uint8_t *ack1)
{
    static uint32_t tick = 0;
    static uint8_t step = 0;
    uint32_t rdat = 0, length = 0,length1 = 0;
    char *res;

    BOOL ret = (BOOL)FALSE;
    switch (step)
    {
    case 0:
        if (!smflag.UART_TX_BUSY)
        {
            HAL_UART_Transmit_IT(HDL_UART_GPRS, at_cmd, (uint16_t)strlen((const char *)at_cmd));
            smflag.UART_TX_BUSY  = 1;
            //HAL_UART_Receive_IT(HDL_UART_GPRS, recvfifo_5.buffer, QUEUE_REV_SIZE);
#ifdef PRINT_UART1
            //HAL_UART_Transmit_IT(HDL_UART_232, at_cmd, (uint16_t)strlen((const char *)at_cmd));
            novar_print(at_cmd, (uint16_t)strlen((const char *)at_cmd));
#endif
            step = 1;
            tick = HAL_GetTick();
        }
        break;
    case 1:
        rdat = recvfifo_5.in & (recvfifo_5.size - 1) - recvfifo_5.out & (recvfifo_5.size - 1);  // 实际未读取数据长度
        length = strlen((const char *)ack);
        if (rdat >= length)
        {
            res = strstr((const char *)&recvfifo_5.buffer[recvfifo_5.out & (recvfifo_5.size - 1)], (const char *)ack);
            if (res)
            {
                rdat = strlen((const char *)ack);
                if (((uint32_t)res - (uint32_t)recvfifo_5.buffer + rdat) > (recvfifo_5.in & (recvfifo_5.size - 1)))
                {
                    step = 2;
                    break;
                }
                recvfifo_5.out += (uint32_t)res - (uint32_t)&recvfifo_5.buffer[recvfifo_5.out & (recvfifo_5.size - 1)] + strlen((const char *)ack);
                step = 0;
                tick = 0;
                ret = (BOOL)TRUE;
                #ifdef PRINT_UART1
                    novar_print( ack, (uint16_t)strlen((const char *)ack));
                #endif
            }
        }
        step = 2;
        break;
    case 2:
        rdat = recvfifo_5.in & (recvfifo_5.size - 1) - recvfifo_5.out & (recvfifo_5.size - 1);  // 实际未读取数据长度
        length1 = strlen((const char *)ack1);
        if (rdat >= length1)
        {
            res = strstr((const char *)&recvfifo_5.buffer[recvfifo_5.out & (recvfifo_5.size - 1)], (const char *)ack1);
            if (res)
            {
                rdat = strlen((const char *)ack1);
                if (((uint32_t)res - (uint32_t)recvfifo_5.buffer + rdat) > (recvfifo_5.in & (recvfifo_5.size - 1)))
                {
                    break;
                }
                recvfifo_5.out += (uint32_t)res - (uint32_t)&recvfifo_5.buffer[recvfifo_5.out & (recvfifo_5.size - 1)] + strlen((const char *)ack1);
                step = 0;
                tick = 0;
                ret = (BOOL)TRUE;
                #ifdef PRINT_UART1
                    novar_print(ack1, (uint16_t)strlen((const char *)ack1));
                #endif
            } else    // 有回复，但是不是需要的
            {
                if (HAL_GetTick() - tick > 5000)
                {
                    step = 0;
                    tick = HAL_GetTick();
                }
            }
        } else    // 没有回复，超时退出重发
        {
            if (HAL_GetTick() - tick > 15000)
            {
                step = 0;
                tick = HAL_GetTick();
            }
        }
        break;
    }
    return ret;
}

/*
*********************************************************************************************************
*   函 数 名: gprs_sendu
*
*   功能说明: GPRS 2639 UDP发送指定长度数据
*
*   形   参: uint8_t *buffer ,uint8_t length
*
*   返 回 值:uint8_t GPRS_POWON_SUCCESS     GPRS上电成功
*                   GPRS_POWON_FAIL        GPRS上电失败
*                   GPRS_RETURN_FAIL       GPRS无应答
*
*********************************************************************************************************
*/
BOOL gprs_sendu(uint8_t *buffer, uint8_t length)
{
    uint8_t strnum[10], re=0, cnt = 0;
    uint16_t i;
    uint32_t tick;

    BOOL ret = (BOOL)FALSE;

    if (length != buffer[1]+5) return ret;

    for (i = 0; i < 10; i++) strnum[i] = 0;

    sprintf((char *)strnum, "%d", length);
    sprintf((char *)strbuf, "AT+ZIPSENDU=1,%s\r\n", strnum);

    do
    {
        if (!StatusFlag.STAT_GPRSSEND_BUSY)
        {
            HAL_UART_Transmit_IT(HDL_UART_GPRS, strbuf, (uint16_t)strlen((const char *)strbuf));
            StatusFlag.STAT_GPRSSEND_BUSY = 1;
            //HAL_Delay(10);

            //HAL_UART_Receive_IT(HDL_UART_GPRS, &data, 1);
            //cnt1++;
            //if (cnt1>3) return ret;
        }
#ifdef PRINT_UART1
        novar_print(strbuf, (uint16_t)strlen((const char *)strbuf));
#endif
        tick = HAL_GetTick();
        do
        {
            __ring_buffer_get(&recvfifo_5, &re, 1);

            if (HAL_GetTick() - tick > 5000)
            {
                cnt++;
                tick = HAL_GetTick();
                //if (cnt >= 3)
                {// 超时未接收到'>'继续发送待发数据，防止丢失该字符导致GPRS模块阻塞
                    if (!StatusFlag.STAT_GPRSSEND_BUSY)
                    {
                        HAL_UART_Transmit_IT(HDL_UART_GPRS, buffer, (uint16_t)length);
                        StatusFlag.STAT_GPRSSEND_BUSY = 1;
                        //HAL_UART_Receive_IT(HDL_UART_GPRS, &data, 1);
                        #ifdef PRINT_UART1
                        novar_print(buffer, (uint16_t)length);
                        #endif
                        ret = (BOOL)TRUE;
                    }
                    return ret;
                }
                //break;
            }

        } while (re != '>');
        #ifdef PRINT_UART1
        HAL_UART_Transmit(HDL_UART_232, &re,1, 100);
        #endif

    } while (re != '>');
    if (!StatusFlag.STAT_GPRSSEND_BUSY)
    {
        HAL_UART_Transmit_IT(HDL_UART_GPRS, buffer, (uint16_t)length);
        StatusFlag.STAT_GPRSSEND_BUSY = 1;
#ifdef PRINT_UART1
        novar_print(buffer, (uint16_t)length);
#endif
    }
    ret = (BOOL)TRUE;
    //return GPRS_POWON_SUCCESS;
    return ret;
}

////////////////////// for ComSer function ///////////////////
///                                                        ///
///                                                        ///
/////////////////////////////////////////////////////////////

/*
*********************************************************************************************************
*   函 数 名: GPRS_INIT
*
*   功能说明: GPRS变量初始化
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_INIT(void)
{
    statemachine_init();
    TIP_init();
    ComSeverInit();
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_MAIN
*
*   功能说明: GPRS主任务，在main工作循环中调用
*
*   形   参: NONE
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_TASK(void)
{
    UDP_AT_INIT();
    if (!smflag.UDP_INIT_OK) return;
    GPRS_RECV();
    ComSer_Main();
    GPRS_CHKRPT();
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
    gprs_pow_on();

    if (!smflag.IS_POW_ON) return;  // 上电不成功直接退出

    cr.SerType = COMTYPE_AT;
    cr.overtime = 250;
    cr.par1 = (uint32_t)or_At;  //  需要发送的AT命令首地址
    cr.par2 = (uint32_t)re_ok;  //  等待回复的AT返回命令首地址
    cr.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&cr, 1);

    smflag.UDP_INIT_OK = 1;
}

/*
*********************************************************************************************************
*   函 数 名: CMD_AT_ACK
*
*   功能说明: 接收服务接收到AT\r\n命令成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_AT_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = 250;  // 超时250*20=5S
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: CMD_NODE_ACK
*
*   功能说明: 接收服务接收到ATE0命令成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_ATE0_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = 750;  // 超时250*20=5S
    rt.par1 = (uint32_t)or_Aopen;
    rt.par2 = (uint32_t)re_Aopen;
    rt.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: CMD_NODE_ACK
*
*   功能说明: 接收服务接收到AT+CGDCONT=1成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_NODE_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   函 数 名: CMD_CGATT_ACK
*
*   功能说明: 接收服务接收到AT+CGATT=1成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_CGATT_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   函 数 名: CMD_STATU_ACK
*
*   功能说明: 接收服务接收到AT+ZIPSTATUSU=1成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_STATU_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   函 数 名: CMD_CLOSEU_ACK
*
*   功能说明: 接收服务接收到AT+ZIPCLOSEU=1成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_CLOSEU_ACK(uint32_t par1, uint32_t par2)
{
}

/*
*********************************************************************************************************
*   函 数 名: CMD_OPENU_ACK
*
*   功能说明: 接收服务接收到AT+ZPPPOPEN成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_OPENU_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    GetIPHost(0, or_Aipset);
    rt.SerType = COMTYPE_AT;
    rt.overtime = 750;  // 250*20=5s
    rt.par1 = (uint32_t)or_Aipset;
    rt.par2 = (uint32_t)re_Aipset;
    rt.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: CMD_IPSET_ACK
*
*   功能说明: 接收服务接收到AT+ZIPSETUPU=1,122.114.22.87,8086成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_IPSET_ACK(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    smflag.UDP_LNK_OK = 1;

    // 组织链路登录帧
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // 链路登录功能码
    extparam.len = FrmLnkLog(extparam.buf); // 应用层数据，表示登录帧
    extparam.timeout = 250; // 250X20ms=5S
    extparam.lock = 0;  //初始化信号量为解锁状态

    zipsendu(0);
}

/*
*********************************************************************************************************
*   函 数 名: CMD_SENDU_ACK
*
*   功能说明: 接收服务接收到AT+ZIPSENDU命令的回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void CMD_SENDU_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;
    //param_ext *ptr;

    //ptr =  (param_ext *)ComSerCtr.TxOpoint->par2;
    // 添加发送数据启动帧，
    rt.SerType = extparam.SerType;  // 登录类型
    rt.overtime = extparam.timeout;         // 超时250*20=5S
    rt.par1 = (uint32_t)(extparam.buf);     // 取出功能码AFN
    rt.par2 = (uint32_t)(extparam.len); // 取出数据域登录识别码
    rt.par3 = GPRS_SENDU_HEX;
    Add2TxSerArray(&rt,1);
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_SENDU_ASCII
*
*   功能说明: GPRS发送函数，发送参数指定地址和长度的缓冲区数据
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_SENDU_ASCII(uint32_t par1, uint32_t par2)
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
*   函 数 名: GPRS_SENDU_ASCII
*
*   功能说明: GPRS发送函数，发送参数指定地址和长度的缓冲区数据
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_SENDU_HEX(uint32_t par1, uint32_t par2)
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
char * my_strstr(const char *s1,const char *s2)
{
   if (*s1 == 0)//如果 数组s1中没有字符串出现，s2中有字符串返回空，如果两个字符串均为空，返回s1首地址，也算找到匹配
    {
      if (*s2)
        return (char *) NULL;
      return (char *) s1;
    }
   while (*s1)// 直到s1中结束符出现
   {
      size_t i;
      i = 0;
      while (1)
      {
        if (s2[i] == 0)
        {
           return (char *) s1;
        }
        if (s2[i] != s1[i])
        {
           break;
        }
        i++;
      }
      s1++;
   }
   return (char *) NULL;
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

    uint32_t i,j;
    uint8_t *k=NULL,ret=0;

    if (a1_len < a2_len) return NULL;   // 被查找数组小于查找数组退出

    for (j = 0; j < a1_len-a2_len+1; j++)
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
char* my_strstr(const char *s1, uint32_t len, const char *s2)
{
    uint32_t i,j=0;

    if (*s2) return (char *) NULL;

    while (j<len)
    {
      i = 0;

      while (1)
      {
        if (s2[i] == 0)
        {
           return (char *) s1;
        }
        if (s2[i] != s1[i])
        {
           break;
        }
        i++;
      }
      s1++;
      j++;
    }
    return (char *) NULL;
}
*/
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
    uint8_t *ss;

    if (m->in == m->out) return 0;

    if (m->in > m->out)
    {
        size = m->in - m->out;
        ss = (uint8_t *)malloc((int)size);
        memcpy(ss, &m->buffer[m->out&(m->size-1)], size);
    }
    else
    {
        size = (m->size - m->out - 1) + m->in;
        ss = (uint8_t *)malloc((int)size);
        memcpy(ss, &m->buffer[m->out & (m->size - 1)], m->size - m->out - 1);
        memcpy(ss + m->size - m->out - 1, m->buffer, m->in);
    }

    //res = my_strstr((const char *)ss, (const char *)s);
    res = (char *)my_arrayarray(ss, size, s, (uint32_t)strlen((const char *)s));;
    if (res)
    {
        ret = ((uint32_t)res - (uint32_t)ss + (uint32_t)strlen((const char *)s));
        m->out += ret;
        m->out &= (m->size - 1);
    }
    else ret = 0;

    free(ss);

    return ret;
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_RECV
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
void GPRS_RECV(void)
{
    // 接收缓冲区无数据退出
    if (recvfifo_5.in == recvfifo_5.out) return;

    // 接收AT应答帧
    GPRS_RECV_ATACK();

    // 接收数据帧
    GPRS_RECV_DATAFRAME();
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_CHKRPT
*
*   功能说明: GPRS检测自报信息报文函数，主要用于查询是否有主动上传数据标志，如果有，添加成员至发送队列
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_CHKRPT(void)
{
    uint16_t len;

    if (!StatusFlag.STAT_LINK_OK)
    {
    // 发送心跳包
        if (StatusFlag.STAT_HEART_OV && smflag.UDP_INIT_OK)
        {
            StatusFlag.STAT_HEART_OV = 0;
            len = FrmHrtDat(extparam.buf);
            extparam.SerType = COMTYPE_TICK;
            extparam.timeout = 250;

            zipsendu(0);
        }
        return;
    }
    if (ComSerCtr.TxRpoint == ComSerCtr.TxOpoint) return;//处理中
    // 如果当前发送数据区上锁直接退出
    if (extparam.lock) return;

    #ifdef TOWERBOX
    // 查询报警数据标志
    len = FrmTowWrnDat(extparam.buf);
    if (len)    // 表示有
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

        return;
    }

    // 查询工作循环标志
    len = FrmTowWklpDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }

    // 查询标定数据标志
    len = FrmTowCaliDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }

    // 查询实时数据标志
    len = FrmTowRtDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
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
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }

    // 查询升降机工作循环标志
    len = FrmElvtWklpDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }

    // 查询报警数据标志
    len = FrmElvtWrnDat(extparam.buf);
    if (len)    // 表示有
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

        return;
    }

    // 查询升降机标定数据标志
    len = FrmElvtCaliDat(extparam.buf);
    if (len)
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
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
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }

    // 查询报警数据标志
    len = FrmDustWrnDat(extparam.buf);
    if (len)    // 表示有
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

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
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);
        return;
    }
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_RECV_ACK
*
*   功能说明: GPRS接收函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— AT应答命令
*                           —— 数据帧应答
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_RECV_ACK(void)
{
    // 无等待发送应答直接退出
    if (ComSerCtr.TxOpoint != ComSerCtr.TxRpoint) return;

    // 如果是AT命令
    if (ComSerCtr.TxOpoint->SerType == COMTYPE_AT)
    {
        GPRS_RECV_ATACK();
    }
    // 非AT命令应答 (自报报文应答帧/心跳报文应答帧)
    if (ComSerCtr.TxOpoint->SerType == COMTYPE_REPORT ||
        ComSerCtr.TxOpoint->SerType == COMTYPE_TICK)
    {
        //GPRS_RECV_PRTCACK();
    }
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_RECV_ATACK
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
void GPRS_RECV_ATACK(void)
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
    if (a == (uint32_t)or_At)
    {
        dptr = re_ok;
        func = CMD_AT_ACK;
    }
    else if (a == (uint32_t)or_Ate0)
    {
        dptr = re_ok;
        func = CMD_ATE0_ACK;
    }
    else if (a == (uint32_t)or_Anode)
    {
        dptr = re_ok;
        func = CMD_NODE_ACK;
    }
    else if (a == (uint32_t)or_Acgatt)
    {
        dptr = re_ok;
        func = CMD_CGATT_ACK;
    }
    else if (a == (uint32_t)or_AstatU)
    {
        dptr = re_AstatU;
        func = CMD_STATU_ACK;
    }
    else if (a == (uint32_t)or_Acloseu)
    {
        dptr = re_Aclostu;
        func = CMD_CLOSEU_ACK;
    }
    else if (a == (uint32_t)or_Aopen)
    {
        dptr = re_Aopen;
        dptr1 = re_Aopen2;
        again = 1;
        func = CMD_OPENU_ACK;
    }
    else if (a == (uint32_t)or_Aipset)
    {
        dptr = re_Aipset;
        dptr1 = re_Aipset2;
        again = 1;
        func = CMD_IPSET_ACK;
    }
    else if (a == (uint32_t)or_Asendu)
    {
        dptr = re_Asendu;
        func = CMD_SENDU_ACK;
    }
    else
    {
        dptr = NULL;
        dptr1  = NULL;
        func = Fnull;
        return;
    }

    // recvfifo_5.in未掉头
    b = cmp_char(&recvfifo_5, dptr);
    if (b)
    {
        // 添加类型到接收队列中
        rt.SerType = COMTYPE_AT;
        rt.overtime = 0;
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
            rt.overtime = 0;
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

void GPRS_RECV_DATAFRAME(void)
{
    uint8_t afn,*dptr;
    uint32_t a;

    dptr = re_Arecv;

    a = cmp_char(&recvfifo_5, dptr);

    if (a)  // 表示收到+ZIPRECVU:指令
    {
        #ifdef PRINT_UART1
        novar_print(dptr, (uint16_t)strlen((const char *)dptr));
        #endif
        // 获取有效帧
        if (TIP_frame_get(&recvfifo_5, RFULL_frame))  // 查询接收缓冲区是否有完整帧
        {
            a = 0;
            if (!mrtn.mf_flag) afn = rtn.appzone.functioncode;
            else afn = mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode;

            // 判断帧类型
            if (afn >= AFN_SET_ADDRESS && afn < AFN_SELF_REALTIMEDATA)
            {
                GPRS_RECV_DOWN(afn,RFULL_frame);// 接收协议设置查询帧
            }
            else
            {
                GPRS_RECV_PRTCACK(afn,RFULL_frame);// 接收协议应答帧
            }
        }
        else
        {
            if (recvfifo_5.out > a)recvfifo_5.out -= a;
            else recvfifo_5.out += recvfifo_5.size - a;
        }
    }
}
/*
*********************************************************************************************************
*   函 数 名: GPRS_RECV_PRTCACK
*
*   功能说明: GPRS接收函数，主要用于查询串口缓冲区中收到的数据分类处理
*                           —— 数据帧应答
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void GPRS_RECV_PRTCACK(uint8_t appfuncode,uint8_t *buf)
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
        case AFN_SELF_FINGER:
            rt.SerType = COMTYPE_REPORT;
            rt.par2 = 0;    //
            break;
        default:return;
    }
    gprs_rssi = 0;
    _LED_COM_ON();
    // 应答报文的回调函数只是起到应答发送的作用，无需回调函数进行处理
    rt.par3 = Fnull;
    Add2RxSerArray(&rt);
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}

/*
*********************************************************************************************************
*   函 数 名: GPRS_RECV_DOWN
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
void GPRS_RECV_DOWN(uint8_t appfuncode,uint8_t *buf)
{
    // 下行帧分单帧和多帧
    switch (appfuncode)
    {
        case AFN_SET_ADDRESS:// 设置遥测终端地址
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevAddrDatSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ADDRESS:// 查询遥测终端地址
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevAddrDatQry(buf,extparam.buf);
        break;
        case AFN_SET_CLOCK:// 设置遥测终端时钟
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevRtcDatSet(buf,extparam.buf);
        break;
        case AFN_QUERY_CLOCK:// 查询遥测终端时钟
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevRtcQry(buf,extparam.buf);
        break;
        case AFN_SET_WORKMODE:// 设置遥测终端工作模式
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevWkModSet(buf,extparam.buf);
        break;
        case AFN_QUERY_WORKMODE:// 查询遥测终端工作模式
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevWkModQry(buf,extparam.buf);
        break;
        case AFN_SET_SENSORTYPE:// 设置遥测终端的传感器种类
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSnsrTypSet(buf,extparam.buf);
        break;
        case AFN_QUERY_SENSORTYPE:// 查询遥测终端的传感器种类
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSnsrTypQry(buf,extparam.buf);
        break;
        case AFN_SET_SENSORPARAM:// 设置遥测终端的传感器参数
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSnsrCfgSet(buf,extparam.buf);
        break;
        case AFN_QUERY_SENSORPARAM:// 查询遥测终端的传感器参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSnsrCfgQry(buf,extparam.buf);
        break;
        case AFN_SET_DEVIPPORT:// 设置遥测终端存储的中心站IP地址和端口号
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevIpPortSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DEVIPPORT:// 查询遥测终端存储的中心站IP地址和端口号
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevIpPortQry(buf,extparam.buf);
        break;
        case AFN_SET_HEARTINTERVAL:// 设置遥测终端链路心跳间隔
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevHrtIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_HEARTINTERVAL:// 查询遥测终端链路心跳间隔
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevHrtIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_RECONNECTINTERVAL:// 设置遥测终端链路重连间隔
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevLnkReconIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_RECONNECTINTERVAL:// 查询遥测终端链路重连间隔
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevLnkReconIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_DATRECINTERVAL:// 设置遥测终端历史数据存盘间隔
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSavIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DATRECINTERVAL:// 查询遥测终端历史数据存盘间隔
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSavIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_DATUPLOADINTERVAL:// 设置遥测终端的实时数据上报间隔
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevRtdRptIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DATUPLOADINTERVAL:// 查询遥测终端的实时数据上报间隔
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevRtdRptIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_UPDATE:// 设置遥测终端的升级数据
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevUpdSet(buf,extparam.buf);
        break;
        case AFN_QUERY_VERINFO:// 查询遥测终端的版本信息
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevVerInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_PASSWORD:// 设置遥测终端的密码
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevPwdSet(buf,extparam.buf);
        break;
        case AFN_QUERY_PASSWORD:// 查询遥测终端的密码
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevPwdQry(buf,extparam.buf);
        break;
        case AFN_CHG_FINGER:    // 指纹数据变更
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
        case AFN_QUERY_LOCATION:// 查询遥测终端GPS位置
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevLctQry(buf,extparam.buf);
        break;
        case AFN_SET_TOWERPARAM:// 设置塔机静态结构参数
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrInfoSet(buf,extparam.buf);
        break;
        case AFN_QUERY_TOWERPARAM:// 查询塔机静态结构参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_PROTECTIONZONE:// 设置塔机保护区
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrPrtcZoneSet(buf,extparam.buf);
        break;
        case AFN_QUERY_PROECTIONZONE:// 查询塔机保护区
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrPrtcZoneQry(buf,extparam.buf);
        break;
        case AFN_SET_LIMIT:// 设置塔机限位信息
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrLmtSet(buf,extparam.buf);
        break;
        case AFN_QUERY_LIMIT:// 查询塔机限位信息
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrLmtQry(buf,extparam.buf);
        break;
        case AFN_SET_MOMENTCURVE:// 设置塔机力矩曲线
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrTorqSet(buf,extparam.buf);
        break;
        case AFN_QUERY_MOMENTCURVE:// 查询塔机力矩曲线
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrTorqQry(buf,extparam.buf);
        break;
        case AFN_SET_CALIBRATPARAM:// 设置塔机标定参数
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrCaliSet(buf,extparam.buf);
        break;
        case AFN_QUERY_CALIBRATPARAM:// 查询塔机标定参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrCaliQry(buf,extparam.buf);
        break;
        case AFN_SET_TOWERLIFT:// 设置塔机顶升数据
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrLiftSet(buf,extparam.buf);
        break;
        case AFN_QUERY_TOWERLIFT:// 查询塔机顶升数据
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrLiftQry(buf,extparam.buf);
        break;
        #endif
        #ifdef ELIVATOR
        case AFN_SET_ELVTINFO:// 设置升降机基本结构参数
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmElvtInfoSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ELVTINFO:// 查询升降机基本结构参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmElvtInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_ELVTFLOOR:// 设置升降机楼层参数
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmElvtFloorSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ELVTFLOOR:// 查询升降机楼层参数
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmElvtFloorQry(buf,extparam.buf);
        break;
        #endif
        #ifdef DUSTMON
        case AFN_SET_VALVELMT:
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmValveLmtSet(buf, extparam.buf);
            break;
        case AFN_QUERY_VALVELMT:
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmValveLmtQry(buf, extparam.buf);
            break;
        case AFN_SET_MANVALVE:
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmValveManual(buf, extparam.buf);
        break;
        case AFN_SET_VALVELMT_EXT:
            extparam.SerType = COMTYPE_SET;
            FrmValveLmtSet_Ext(buf, extparam.buf);
        break;
        case AFN_QUERY_VALVELMT_EXT:
            extparam.SerType = COMTYPE_QUERY;
            FrmValveLmtQry_Ext(buf, extparam.buf);
        break;
        case AFN_SET_NOTICE:
            extparam.SerType = COMTYPE_QUERY;
            FrmNotice(buf, extparam.buf);
        break;
        #endif
        default:
            return;
    }
    // 主动下行报文的回调函数为上行应答帧启动做准备

    // 组织应答帧
    extparam.timeout = 250; // 250X20ms=5S
    extparam.lock = 1;  // 锁定数据，下行报文优先级高于主动上报报文
    zipsendu(1);    // 如果上次应答未成功收到，避免和应答冲突，使用加急命令，丢弃之前发送队列中未处理帧
    //Add2RxSerArray(&rt);

    gprs_rssi = 0;
    _LED_COM_ON();
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}

void zipsendu(uint8_t priority)
{
    uint8_t len,strnum[10];
    COMSERARRAY tp;
    // 组织链路登录帧

    len = extparam.len;
    memset(or_Asendu, 0, 25);
    sprintf((char *)strnum, "%d", len);
    sprintf((char *)or_Asendu, "AT+ZIPSENDU=1,%s\r\n", strnum);

    // 添加发送数据启动帧
    tp.SerType = COMTYPE_AT;
    tp.overtime = 500; // 超时500*20=10S
    tp.par1 = (uint32_t)or_Asendu;
    tp.par2 = (uint32_t)&extparam;  // 传递链路登录帧信息
    tp.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&tp, priority);
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
#undef  _LOCAL_GPRS

/*****************************  END OF FILE  *********************************/
