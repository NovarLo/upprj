#define _LOCAL_MG2639D

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
*   函 数 名: MG2639D_pow_on
*
*   功能说明: 非阻塞上电初始化
*
*   形   参: none
*
*   返 回 值: 成功或者失败
*
*********************************************************************************************************
*/
BOOL MG2639D_pow_on(void)
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
        //延时25S
        if (HAL_GetTick() - tick > 25000)
        {
            //释放按键
            _GPRS_PWKEY_RELEASE();
            if (_IS_GPRS_ON())
            {
                step = 0;
                tick = HAL_GetTick();
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
*   函 数 名: MG2639DCMD_AT_ACK
*
*   功能说明: 接收服务接收到AT\r\n命令成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void MG2639DCMD_AT_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时250*20=5S
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: MG2639DCMD_NODE_ACK
*
*   功能说明: 接收服务接收到ATE0命令成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void MG2639DCMD_ATE0_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 超时250*20=5S
    rt.par1 = (uint32_t)or_Aopen;
    rt.par2 = (uint32_t)re_Aopen;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}
/*
*********************************************************************************************************
*   函 数 名: MG2639DCMD_OPENU_ACK
*
*   功能说明: 接收服务接收到AT+ZPPPOPEN成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void MG2639DCMD_OPENU_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    GetIPHost(0, or_Aipset);
    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // 250*20=5s
    rt.par1 = (uint32_t)or_Aipset;
    rt.par2 = (uint32_t)re_Aipset;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   函 数 名: MG2639D_SENDAT_CIPSEND
*
*   功能说明: 使用UDP的AT命令发送数据帧
*
*   形    参: uint8_t priority,优先级
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void MG2639D_SENDAT_CIPSEND(uint8_t priority)
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
    tp.overtime = extparam.timeout; // 超时500*20=10S
    tp.par1 = (uint32_t)or_Asendu;
    tp.par2 = (uint32_t)&extparam;  // 传递链路登录帧信息
    tp.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&tp, priority);
}

/*
*********************************************************************************************************
*   函 数 名: MG2639DCMD_IPSET_ACK
*
*   功能说明: 接收服务接收到AT+ZIPSETUPU=1,122.114.22.87,8086成功回复后调用的回调函数
*
*   形   参: uint32_t par1, uint32_t par2
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void MG2639DCMD_IPSET_ACK(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    smflag.UDP_LNK_OK = 1;

    // 组织链路登录帧
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // 链路登录功能码
    extparam.len = FrmLnkLog(extparam.buf); // 应用层数据，表示登录帧
    extparam.timeout = device_info.link_timeout * 1000 / 20; //RXOVERTM; // ATOVERTM * 20ms
    extparam.lock = 0;  //初始化信号量为解锁状态

    MG2639D_SENDAT_CIPSEND(0);
}
#undef  _LOCAL_MG2639D

/*****************************  END OF FILE  *********************************/
