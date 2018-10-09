/*******************************************************************************
    ComSer.c
通讯服务：1个发送服务队列，1个接收服务队列
发送机制：1.当需要发送时向发送队列中加入1条命令(分优先级)
          2.主程序检查发送队列是否有内容,有执行回调函数(此函数为准备发送数据、启动发送中断),
            若为主动发送(需等待应答)则启动超时定时。
接收机制：1.接收中断收到1帧有效数据后向接收队列中加入1条命令,并保存接收到的数据
          2.主程序检查接收队列是否有内容,有执行回调函数(此函数分析数据,若下行设置或查询则生成1条应答命令写入发送队列中)
          3.若等待应答超则向接收队列中加入1条超时命令(该命令回调函数执行:判重发次数到否,到则向发送队列中加急写入1条链路登录命令;
            未到则将当前发送的命令重新急写入发送队列中)
*******************************************************************************/
#define _LOCAL_COMSER

#include "stm32f4xx_hal.h"
#include "app_user.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "app_sensor.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "app_network.h"
#include "app_comser.h"

/*********全局变量**********************************************************/
// COMSERCTR  ComSerCtr;

void ComSer_Main(void)
{
    ComRxSeverExc();
    ComTxSeverExc();
}
/****************************************************
    ComSeverInit
服务队列初始化
*****************************************************/
void ComSeverInit(void)
{
    ComSerCtr.TxRpoint = ComSerCtr.TxArray;
    ComSerCtr.TxWpoint = ComSerCtr.TxArray;
    ComSerCtr.TxOpoint = NULL;
    ComSerCtr.RxRpoint = ComSerCtr.RxArray;
    ComSerCtr.RxWpoint = ComSerCtr.RxArray;
    ComSerCtr.RxOpoint = NULL;
    ComSerCtr.TxOverDlyCnt = RXOVERTM;
    ComSerCtr.TxRepTimsEn = 1;
    ComSerCtr.TxRepTims = REPTXTIMS;
    ComSerCtr.TxOverDlyFlag = 0;
}
/*******************************************************************
    Add2TxSerArray
描述：向发送队列加入1条待发送的命令,不要在中断函数中调用
输入：*point带加入的信息指针
      priority该条命令优先级(0=正常,1=加急)
返回：0=成功,-1=错误
********************************************************************/
int16_t Add2TxSerArray(COMSERARRAY *point,uint8_t priority)
{
    if((ComSerCtr.TxWpoint==ComSerCtr.TxRpoint+TXSERARRAYNUM-1)||(ComSerCtr.TxRpoint==ComSerCtr.TxWpoint+1))
        return -1;    //队列满

    //正常
    //加入新消息
    ComSerCtr.TxWpoint->SerType = point->SerType;
    ComSerCtr.TxWpoint->overtime = point->overtime;
    ComSerCtr.TxWpoint->par1 = point->par1;
    ComSerCtr.TxWpoint->par2 = point->par2;
    ComSerCtr.TxWpoint->par3 = point->par3;
    // 如果加急，丢弃原来未处理的帧，读指针直接指向新加入的帧
    if (priority) ComSerCtr.TxRpoint = ComSerCtr.TxWpoint;
    //写指针+1
    ComSerCtr.TxWpoint++;
    if (ComSerCtr.TxWpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
        ComSerCtr.TxWpoint = ComSerCtr.TxArray;


/*
    //加入新命令
    if(priority)
    {//加急
        if(ComSerCtr.TxRpoint==ComSerCtr.TxArray)
        {//读指针在队首，置指向队尾
            ComSerCtr.TxRpoint = ComSerCtr.TxArray+TXSERARRAYNUM-1;
        }
        else
        {//读指针-1
            ComSerCtr.TxRpoint--;
        }
        //加入新消息
        ComSerCtr.TxRpoint->SerType = point->SerType;
        ComSerCtr.TxRpoint->overtime = point->overtime;
        ComSerCtr.TxRpoint->par1 = point->par1;
        ComSerCtr.TxRpoint->par2 = point->par2;
        ComSerCtr.TxRpoint->par3 = point->par3;
    }
    else
    {//正常
        //加入新消息
        ComSerCtr.TxWpoint->SerType = point->SerType;
        ComSerCtr.TxWpoint->overtime = point->overtime;
        ComSerCtr.TxWpoint->par1 = point->par1;
        ComSerCtr.TxWpoint->par2 = point->par2;
        ComSerCtr.TxWpoint->par3 = point->par3;
        //写指针+1
        ComSerCtr.TxWpoint++;
        if (ComSerCtr.TxWpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
            ComSerCtr.TxWpoint = ComSerCtr.TxArray;
    }
*/
    return 0;
}
/********************************************************************
    ComTxSeverExc
描述：发送服务,main()循环调用
*******************************************************************/
void ComTxSeverExc(void)
{
    if(ComSerCtr.TxWpoint==ComSerCtr.TxRpoint)
    {//队列空
        return;
    }
    if(ComSerCtr.TxRpoint==ComSerCtr.TxOpoint)
    {//处理中
        return;
    }
    //开始处理
    ComSerCtr.TxOpoint = ComSerCtr.TxRpoint;

    //执行发送处理回调函数(启动发送时置超时标志,置超时计数器初值
    (*ComSerCtr.TxOpoint->par3)(ComSerCtr.TxOpoint->par1,ComSerCtr.TxOpoint->par2);

    if(ComSerCtr.TxOpoint->overtime >0)
    {//若是主动的上行命令，有应答则启动超时定时
        ComSerCtr.TxOverDlyCnt = ComSerCtr.TxOpoint->overtime;
        // 启动250ms超时定时器
        if (ComSerCtr.TxOpoint->SerType != COMTYPE_SET && ComSerCtr.TxOpoint->SerType != COMTYPE_QUERY)
        {
            ComSerCtr.TxOverDlyFlag = 1;
            StartTimeoutCnt();
        }
        else //回复下行帧结束
        {
            extparam.lock = 0;  // 数据解除锁定
            ComSerCtr.TxRpoint++;   //发送读指针+1
            ComSerCtr.TxOpoint=NULL;
            FProtocolAck(0, 0);
            if(ComSerCtr.TxRpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
                ComSerCtr.TxRpoint = ComSerCtr.TxArray;
        }
    }
}

/*******************************************************************
    Add2RxSerArray
描述：向接收队列加入1条待发送的命令，按顺序写入无优先级
      由接收中断程序调用,每收到1条完整数据向队列写入
输入：*point带加入的信息指针
返回：0=成功,-1=错误
********************************************************************/
int16_t Add2RxSerArray(COMSERARRAY *point)
{
    if((ComSerCtr.RxWpoint==ComSerCtr.RxRpoint+RXSERARRAYNUM-1)||(ComSerCtr.RxRpoint==ComSerCtr.RxWpoint+1))
        return -1;    //队列满
    //加入新消息
    ComSerCtr.RxWpoint->SerType = point->SerType;
    ComSerCtr.RxWpoint->par1 = point->par1;
    ComSerCtr.RxWpoint->par2 = point->par2;
    ComSerCtr.RxWpoint->par3 = point->par3;
    //写指针+1
    ComSerCtr.RxWpoint++;
    if (ComSerCtr.RxWpoint>=ComSerCtr.RxArray+RXSERARRAYNUM)
        ComSerCtr.RxWpoint = ComSerCtr.RxArray;
    return 0;
}
/********************************************************************
    ComRxSeverExc
描述：接收服务,main()循环调用
*******************************************************************/
void ComRxSeverExc(void)
{
    if(ComSerCtr.RxWpoint==ComSerCtr.RxRpoint)
    {//队列空
        return;
    }
    ComSerCtr.RxOpoint = ComSerCtr.RxRpoint;
    //处理发送队列指针：1.当前有发送，接收到的就是发送的应答,则处理接收到的命令，完成一次交互，发送读指 针+1
    //                  2.当前有发送，接收到的不是发送的应答(有下行命令插入),则处理接收到的命令，上次交互 未完成，发送读指针不动
    //                  3.当前无发送，接收到下行命令，则处理接收到的命令
    if((ComSerCtr.TxWpoint!=ComSerCtr.TxRpoint)&&(ComSerCtr.RxOpoint->SerType == ComSerCtr.TxOpoint->SerType))
    {//是情况1：发送队列不空且收到应答，完成一次交互
        ComSerCtr.TxRpoint++;   //发送读指针+1
        ComSerCtr.TxOpoint=NULL;
        if(ComSerCtr.TxRpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
            ComSerCtr.TxRpoint = ComSerCtr.TxArray;
    }
    //执行接收处理回调函数
    (*ComSerCtr.RxOpoint->par3)(ComSerCtr.RxOpoint->par1,ComSerCtr.RxOpoint->par2);
    //接收读指针+1
    ComSerCtr.RxRpoint++;
    if (ComSerCtr.RxRpoint>=ComSerCtr.RxArray+RXSERARRAYNUM)
        ComSerCtr.RxRpoint = ComSerCtr.RxArray;
}
/********************************************
    Fnull
空的回调函数
*********************************************/
void Fnull(uint32_t par1, uint32_t par2)
{
    ;
}

/********************************************
    FProtocolAck
接收到协议应答帧的回调函数
*********************************************/
void FProtocolAck(uint32_t par1, uint32_t par2)
{
    ComSerCtr.TxOverDlyFlag = 0;   //清超时标志
    HAL_TIM_Base_Stop_IT(&htim14);  // 关闭定时器
    ComSerCtr.TxRepTimsEn = 1;
    ComSerCtr.TxRepTims = REPTXTIMS;
    ComSerCtr.TxOpoint=NULL;
}

/********************************************
    Freset
复位的回调函数
*********************************************/
void Freset(uint32_t par1, uint32_t par2)
{
    while(1);
}

/*************************************************
    RxAnsOverErr
描述：接收应答超时处理函数。
输入：par1,par2:无意义,满足回调函数格式
*************************************************/
static void RxAnsOverErr(uint32_t par1, uint32_t par2)
{
    COMSERARRAY tp;

    //其他标志处理

    if(ComSerCtr.TxRepTims==0)
    {//重发次数到
        // ComSerCtr.TxRepTimsEn = 1;
        ComSerCtr.TxRepTims = REPTXTIMS;

        gprs_rssi = 0; // dtu显示掉线

        NETWORK_INIT();
        ComSerCtr.RxRpoint--;
    }
    else
    {//将当前发送的命令重新加急写入发送队列
        // 当前为AT命令重新写入在线指针
        if (ComSerCtr.TxOpoint->SerType == COMTYPE_AT)
        {
            tp.SerType = ComSerCtr.TxOpoint->SerType;
            tp.overtime = ComSerCtr.TxOpoint->overtime;
            tp.par1 = ComSerCtr.TxOpoint->par1;
            tp.par2 = ComSerCtr.TxOpoint->par2;
            tp.par3 = ComSerCtr.TxOpoint->par3;
            Add2TxSerArray(&tp,1);
        }
        else    // 当前不是AT命令，写入发送数据AT指令
        {
            NETWORK_AT_SENDDATA(1);
        }
    }
    ComSerCtr.TxOpoint = NULL;
}
/*************************************************
    RxOverDly
接收应答超时,由定时中断程序调用
*************************************************/
void RxOverDly(void)
{
    COMSERARRAY rp;
    if(ComSerCtr.TxOverDlyFlag)
    {
        if(--ComSerCtr.TxOverDlyCnt==0)
        {
            HAL_TIM_Base_Stop_IT(&htim14);  // 关闭定时器
            //超时处理
            ComSerCtr.TxOverDlyFlag = 0;
            if (ComSerCtr.TxRepTims > 0)
            {
                ComSerCtr.TxRepTimsEn = 0;
                ComSerCtr.TxRepTims--;
            }
            // 清除接收缓存中未处理的数据
            recvfifo_5.out = recvfifo_5.in;
            //向接收队列中写入1条接收超时命令
            rp.SerType = COMTYPE_ERR;
            rp.par1 = 0;
            rp.par2 = 0;
            //执行函数
            rp.par3 = RxAnsOverErr;
            Add2RxSerArray(&rp);
        }
    }
}

// unit：S
// 启动20ms定时器
void StartTimeoutCnt(void)
{
    //timeoutcnt = time;
    htim14.Init.Prescaler = 7999;
    htim14.Init.Period = 200;
    HAL_TIM_Base_Init(&htim14);
    __HAL_TIM_SetCounter(&htim14, 0);
    __HAL_TIM_CLEAR_FLAG(&htim14, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim14);
}
#undef _LOCAL_COMSER
