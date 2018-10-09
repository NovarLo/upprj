/*******************************************************************************
    ComSer.c
ͨѶ����1�����ͷ�����У�1�����շ������
���ͻ��ƣ�1.����Ҫ����ʱ���Ͷ����м���1������(�����ȼ�)
          2.�������鷢�Ͷ����Ƿ�������,��ִ�лص�����(�˺���Ϊ׼���������ݡ����������ж�),
            ��Ϊ��������(��ȴ�Ӧ��)��������ʱ��ʱ��
���ջ��ƣ�1.�����ж��յ�1֡��Ч���ݺ�����ն����м���1������,��������յ�������
          2.����������ն����Ƿ�������,��ִ�лص�����(�˺�����������,���������û��ѯ������1��Ӧ������д�뷢�Ͷ�����)
          3.���ȴ�Ӧ��������ն����м���1����ʱ����(������ص�����ִ��:���ط���������,�������Ͷ����мӼ�д��1����·��¼����;
            δ���򽫵�ǰ���͵��������¼�д�뷢�Ͷ�����)
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

/*********ȫ�ֱ���**********************************************************/
// COMSERCTR  ComSerCtr;

void ComSer_Main(void)
{
    ComRxSeverExc();
    ComTxSeverExc();
}
/****************************************************
    ComSeverInit
������г�ʼ��
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
���������Ͷ��м���1�������͵�����,��Ҫ���жϺ����е���
���룺*point���������Ϣָ��
      priority�����������ȼ�(0=����,1=�Ӽ�)
���أ�0=�ɹ�,-1=����
********************************************************************/
int16_t Add2TxSerArray(COMSERARRAY *point,uint8_t priority)
{
    if((ComSerCtr.TxWpoint==ComSerCtr.TxRpoint+TXSERARRAYNUM-1)||(ComSerCtr.TxRpoint==ComSerCtr.TxWpoint+1))
        return -1;    //������

    //����
    //��������Ϣ
    ComSerCtr.TxWpoint->SerType = point->SerType;
    ComSerCtr.TxWpoint->overtime = point->overtime;
    ComSerCtr.TxWpoint->par1 = point->par1;
    ComSerCtr.TxWpoint->par2 = point->par2;
    ComSerCtr.TxWpoint->par3 = point->par3;
    // ����Ӽ�������ԭ��δ�����֡����ָ��ֱ��ָ���¼����֡
    if (priority) ComSerCtr.TxRpoint = ComSerCtr.TxWpoint;
    //дָ��+1
    ComSerCtr.TxWpoint++;
    if (ComSerCtr.TxWpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
        ComSerCtr.TxWpoint = ComSerCtr.TxArray;


/*
    //����������
    if(priority)
    {//�Ӽ�
        if(ComSerCtr.TxRpoint==ComSerCtr.TxArray)
        {//��ָ���ڶ��ף���ָ���β
            ComSerCtr.TxRpoint = ComSerCtr.TxArray+TXSERARRAYNUM-1;
        }
        else
        {//��ָ��-1
            ComSerCtr.TxRpoint--;
        }
        //��������Ϣ
        ComSerCtr.TxRpoint->SerType = point->SerType;
        ComSerCtr.TxRpoint->overtime = point->overtime;
        ComSerCtr.TxRpoint->par1 = point->par1;
        ComSerCtr.TxRpoint->par2 = point->par2;
        ComSerCtr.TxRpoint->par3 = point->par3;
    }
    else
    {//����
        //��������Ϣ
        ComSerCtr.TxWpoint->SerType = point->SerType;
        ComSerCtr.TxWpoint->overtime = point->overtime;
        ComSerCtr.TxWpoint->par1 = point->par1;
        ComSerCtr.TxWpoint->par2 = point->par2;
        ComSerCtr.TxWpoint->par3 = point->par3;
        //дָ��+1
        ComSerCtr.TxWpoint++;
        if (ComSerCtr.TxWpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
            ComSerCtr.TxWpoint = ComSerCtr.TxArray;
    }
*/
    return 0;
}
/********************************************************************
    ComTxSeverExc
���������ͷ���,main()ѭ������
*******************************************************************/
void ComTxSeverExc(void)
{
    if(ComSerCtr.TxWpoint==ComSerCtr.TxRpoint)
    {//���п�
        return;
    }
    if(ComSerCtr.TxRpoint==ComSerCtr.TxOpoint)
    {//������
        return;
    }
    //��ʼ����
    ComSerCtr.TxOpoint = ComSerCtr.TxRpoint;

    //ִ�з��ʹ���ص�����(��������ʱ�ó�ʱ��־,�ó�ʱ��������ֵ
    (*ComSerCtr.TxOpoint->par3)(ComSerCtr.TxOpoint->par1,ComSerCtr.TxOpoint->par2);

    if(ComSerCtr.TxOpoint->overtime >0)
    {//�������������������Ӧ����������ʱ��ʱ
        ComSerCtr.TxOverDlyCnt = ComSerCtr.TxOpoint->overtime;
        // ����250ms��ʱ��ʱ��
        if (ComSerCtr.TxOpoint->SerType != COMTYPE_SET && ComSerCtr.TxOpoint->SerType != COMTYPE_QUERY)
        {
            ComSerCtr.TxOverDlyFlag = 1;
            StartTimeoutCnt();
        }
        else //�ظ�����֡����
        {
            extparam.lock = 0;  // ���ݽ������
            ComSerCtr.TxRpoint++;   //���Ͷ�ָ��+1
            ComSerCtr.TxOpoint=NULL;
            FProtocolAck(0, 0);
            if(ComSerCtr.TxRpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
                ComSerCtr.TxRpoint = ComSerCtr.TxArray;
        }
    }
}

/*******************************************************************
    Add2RxSerArray
����������ն��м���1�������͵������˳��д�������ȼ�
      �ɽ����жϳ������,ÿ�յ�1���������������д��
���룺*point���������Ϣָ��
���أ�0=�ɹ�,-1=����
********************************************************************/
int16_t Add2RxSerArray(COMSERARRAY *point)
{
    if((ComSerCtr.RxWpoint==ComSerCtr.RxRpoint+RXSERARRAYNUM-1)||(ComSerCtr.RxRpoint==ComSerCtr.RxWpoint+1))
        return -1;    //������
    //��������Ϣ
    ComSerCtr.RxWpoint->SerType = point->SerType;
    ComSerCtr.RxWpoint->par1 = point->par1;
    ComSerCtr.RxWpoint->par2 = point->par2;
    ComSerCtr.RxWpoint->par3 = point->par3;
    //дָ��+1
    ComSerCtr.RxWpoint++;
    if (ComSerCtr.RxWpoint>=ComSerCtr.RxArray+RXSERARRAYNUM)
        ComSerCtr.RxWpoint = ComSerCtr.RxArray;
    return 0;
}
/********************************************************************
    ComRxSeverExc
���������շ���,main()ѭ������
*******************************************************************/
void ComRxSeverExc(void)
{
    if(ComSerCtr.RxWpoint==ComSerCtr.RxRpoint)
    {//���п�
        return;
    }
    ComSerCtr.RxOpoint = ComSerCtr.RxRpoint;
    //�����Ͷ���ָ�룺1.��ǰ�з��ͣ����յ��ľ��Ƿ��͵�Ӧ��,������յ���������һ�ν��������Ͷ�ָ ��+1
    //                  2.��ǰ�з��ͣ����յ��Ĳ��Ƿ��͵�Ӧ��(�������������),������յ�������ϴν��� δ��ɣ����Ͷ�ָ�벻��
    //                  3.��ǰ�޷��ͣ����յ��������������յ�������
    if((ComSerCtr.TxWpoint!=ComSerCtr.TxRpoint)&&(ComSerCtr.RxOpoint->SerType == ComSerCtr.TxOpoint->SerType))
    {//�����1�����Ͷ��в������յ�Ӧ�����һ�ν���
        ComSerCtr.TxRpoint++;   //���Ͷ�ָ��+1
        ComSerCtr.TxOpoint=NULL;
        if(ComSerCtr.TxRpoint>=ComSerCtr.TxArray+TXSERARRAYNUM)
            ComSerCtr.TxRpoint = ComSerCtr.TxArray;
    }
    //ִ�н��մ���ص�����
    (*ComSerCtr.RxOpoint->par3)(ComSerCtr.RxOpoint->par1,ComSerCtr.RxOpoint->par2);
    //���ն�ָ��+1
    ComSerCtr.RxRpoint++;
    if (ComSerCtr.RxRpoint>=ComSerCtr.RxArray+RXSERARRAYNUM)
        ComSerCtr.RxRpoint = ComSerCtr.RxArray;
}
/********************************************
    Fnull
�յĻص�����
*********************************************/
void Fnull(uint32_t par1, uint32_t par2)
{
    ;
}

/********************************************
    FProtocolAck
���յ�Э��Ӧ��֡�Ļص�����
*********************************************/
void FProtocolAck(uint32_t par1, uint32_t par2)
{
    ComSerCtr.TxOverDlyFlag = 0;   //�峬ʱ��־
    HAL_TIM_Base_Stop_IT(&htim14);  // �رն�ʱ��
    ComSerCtr.TxRepTimsEn = 1;
    ComSerCtr.TxRepTims = REPTXTIMS;
    ComSerCtr.TxOpoint=NULL;
}

/********************************************
    Freset
��λ�Ļص�����
*********************************************/
void Freset(uint32_t par1, uint32_t par2)
{
    while(1);
}

/*************************************************
    RxAnsOverErr
����������Ӧ��ʱ��������
���룺par1,par2:������,����ص�������ʽ
*************************************************/
static void RxAnsOverErr(uint32_t par1, uint32_t par2)
{
    COMSERARRAY tp;

    //������־����

    if(ComSerCtr.TxRepTims==0)
    {//�ط�������
        // ComSerCtr.TxRepTimsEn = 1;
        ComSerCtr.TxRepTims = REPTXTIMS;

        gprs_rssi = 0; // dtu��ʾ����

        NETWORK_INIT();
        ComSerCtr.RxRpoint--;
    }
    else
    {//����ǰ���͵��������¼Ӽ�д�뷢�Ͷ���
        // ��ǰΪAT��������д������ָ��
        if (ComSerCtr.TxOpoint->SerType == COMTYPE_AT)
        {
            tp.SerType = ComSerCtr.TxOpoint->SerType;
            tp.overtime = ComSerCtr.TxOpoint->overtime;
            tp.par1 = ComSerCtr.TxOpoint->par1;
            tp.par2 = ComSerCtr.TxOpoint->par2;
            tp.par3 = ComSerCtr.TxOpoint->par3;
            Add2TxSerArray(&tp,1);
        }
        else    // ��ǰ����AT���д�뷢������ATָ��
        {
            NETWORK_AT_SENDDATA(1);
        }
    }
    ComSerCtr.TxOpoint = NULL;
}
/*************************************************
    RxOverDly
����Ӧ��ʱ,�ɶ�ʱ�жϳ������
*************************************************/
void RxOverDly(void)
{
    COMSERARRAY rp;
    if(ComSerCtr.TxOverDlyFlag)
    {
        if(--ComSerCtr.TxOverDlyCnt==0)
        {
            HAL_TIM_Base_Stop_IT(&htim14);  // �رն�ʱ��
            //��ʱ����
            ComSerCtr.TxOverDlyFlag = 0;
            if (ComSerCtr.TxRepTims > 0)
            {
                ComSerCtr.TxRepTimsEn = 0;
                ComSerCtr.TxRepTims--;
            }
            // ������ջ�����δ���������
            recvfifo_5.out = recvfifo_5.in;
            //����ն�����д��1�����ճ�ʱ����
            rp.SerType = COMTYPE_ERR;
            rp.par1 = 0;
            rp.par2 = 0;
            //ִ�к���
            rp.par3 = RxAnsOverErr;
            Add2RxSerArray(&rp);
        }
    }
}

// unit��S
// ����20ms��ʱ��
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
