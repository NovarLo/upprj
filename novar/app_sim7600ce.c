#define _LOCAL_SIM7600CE

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
/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_pow_on
*
*   功能说明: 非阻塞上电初始化
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL SIM7600CE_pow_on(void)
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
        /*if (_IS_GPRS_ON())
        {
            step = 0;
            tick = 0;
            ret = (BOOL)TRUE;
            smflag.IS_POW_ON = 1;
        }
        else step = 3;*/
        step = 3;
        break;
    case 3:
        // 电源打开
        _VGPRS_ON();
        //按下按键
        //_GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 4;
        break;
    case 4:
        //延时25S
        if (HAL_GetTick() - tick > 25000)
        {
            //释放按键
            //_SIM7600CE_PWKEY_RELEASE();
            //if (_IS_SIM7600CE_ON())
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

////////////////////// for ComSer function ///////////////////
///                                                        ///
///                                                        ///
/////////////////////////////////////////////////////////////

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_ATE0
*
*   功能说明: 把发送帧“AT+ATE0\r\n”放入发送缓冲区
*             关闭AT命令回显
*
*   形   参: uint32_t par1, 传递AT命令
*            uint32_t par2, 传递AT命令应答
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_ATE0(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    gprs_rssi = 1;  // AT命令正常响应

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPCCFG
*
*   功能说明: AT命令为“AT+CIPCCFG\r\n”
*             配置socket参数为默认值
*
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPCCFG(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPCCFG;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPSRIP
*
*   功能说明: 下一帧AT命令为“AT+CIPSRIP=0\r\n”放入发送缓冲区
*             接收数据时不显示提示“RECV FROM:<IP ADDRESS>:<PORT>”
*
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPSRIP(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPSRIP;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPHEAD
*
*   功能说明: 下一帧AT命令为“AT+CIPHEAD=1\r\n”放入发送缓冲区
*             接收数据时加上一个IP头“+IPD(data length)
*
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPHEAD(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPHEAD;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPMODE
*
*   功能说明: 下一帧AT命令为“AT+CIPMODE=0\r\n”放入发送缓冲区
*             选择TCPIP 应用方式
*
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPMODE(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPMODE;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_NETOPEN
*
*   功能说明: 下一帧AT命令为“AT+NETOPEN\r\n”放入发送缓冲区
*             打开网络
*
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_NETOPEN(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_NETOPEN;
    rt.par2 = (uint32_t)re_NETOPEN;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPRXGET
*
*   功能说明: 下一帧AT命令为“AT+CIPRXGET=0\r\n”放入发送缓冲区
*             设置为自动接收
* 
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPRXGET(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPRXGET;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPOPEN
*
*   功能说明: 下一帧AT命令为“AT+CIPOPEN=1,"UDP",,,8086\r\n”放入发送缓冲区
*             打开UDP，8086为本机端口号，可以任意输入
* 
*   形   参: uint32_t par1, 传递AT命令     
*            uint32_t par2, 传递AT命令应答 
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPOPEN(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时ATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPOPEN;
    rt.par2 = (uint32_t)re_CIPOPEN;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: SIM7600CE_SENDAT_CIPSEND
*
*   功能说明: 使用UDP的AT命令发送数据帧
*
*   形    参: uint8_t priority,优先级
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPSEND(uint8_t priority)
{
    __IO uint16_t i,port,len;
    uint8_t strnum[4][4],strport[6];
    uint8_t len1,lennum[10];
    COMSERARRAY tp;

    for (i = 0; i < 4; i++)
    {
        GET_ARRAY_LEN(strnum[i], len);
        sprintf((char *)strnum[i], "%d", device_info.ip_port[0][i]);
    }
    GET_ARRAY_LEN(strport, len);
    port = device_info.ip_port[0][4]<<8 | device_info.ip_port[0][5];
    sprintf((char *)strport, "%d", port);

    // 组织协议帧

    len1 = extparam.len;
    memset(or_CIPSEND, 0, 40);
    sprintf((char *)lennum, "%d", len1);
    sprintf((char *)or_CIPSEND, "AT+CIPSEND=1,%s,\"%s.%s.%s.%s\",%s\r\n", lennum, strnum[0], strnum[1], strnum[2], strnum[3], strport);

    // 添加发送数据启动帧
    tp.SerType = COMTYPE_AT;
    //tp.SerType = extparam.SerType;
    tp.overtime = extparam.timeout; // 超时500*20=10S
    tp.par1 = (uint32_t)or_CIPSEND;
    tp.par2 = (uint32_t)&extparam;  // 传递链路登录帧信息
    tp.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&tp, priority);
}


/*
*********************************************************************************************************
*   函 数 名: LOADFRM_LOGIN
*
*   功能说明: 接收服务接收到成功回复后调用的回调函数
*             把登录帧放入发送缓冲区中
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void LOADFRM_LOGIN(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    gprs_rssi = 2;  // 网络正常打开

    smflag.UDP_LNK_OK = 1;

    // 组织链路登录帧
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // 链路登录功能码
    extparam.len = FrmLnkLog(extparam.buf); // 应用层数据，表示登录帧
    extparam.timeout = device_info.link_timeout * 1000 / 20; //RXOVERTM; // ATOVERTM * 20ms
    extparam.lock = 0;  //初始化信号量为解锁状态

    NETWORK_SENDAT_CIPSEND(0);
}
#undef  _LOCAL_SIM7600CE

/*****************************  END OF FILE  *********************************/
