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
*   �� �� ��: MG2639D_pow_on
*
*   ����˵��: �������ϵ��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
        // ��Դ��
        _VGPRS_ON();
        //���°���
        _GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 4;
        break;
    case 4:
        //��ʱ25S
        if (HAL_GetTick() - tick > 25000)
        {
            //�ͷŰ���
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
*   �� �� ��: MG2639DCMD_AT_ACK
*
*   ����˵��: ���շ�����յ�AT\r\n����ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void MG2639DCMD_AT_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱ250*20=5S
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: MG2639DCMD_NODE_ACK
*
*   ����˵��: ���շ�����յ�ATE0����ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void MG2639DCMD_ATE0_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱ250*20=5S
    rt.par1 = (uint32_t)or_Aopen;
    rt.par2 = (uint32_t)re_Aopen;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}
/*
*********************************************************************************************************
*   �� �� ��: MG2639DCMD_OPENU_ACK
*
*   ����˵��: ���շ�����յ�AT+ZPPPOPEN�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
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
*   �� �� ��: MG2639D_SENDAT_CIPSEND
*
*   ����˵��: ʹ��UDP��AT���������֡
*
*   ��    ��: uint8_t priority,���ȼ�
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void MG2639D_SENDAT_CIPSEND(uint8_t priority)
{
    uint8_t len,strnum[10];
    COMSERARRAY tp;
    // ��֯��·��¼֡

    len = extparam.len;
    memset(or_Asendu, 0, 25);
    sprintf((char *)strnum, "%d", len);
    sprintf((char *)or_Asendu, "AT+ZIPSENDU=1,%s\r\n", strnum);

    // ��ӷ�����������֡
    tp.SerType = COMTYPE_AT;
    tp.overtime = extparam.timeout; // ��ʱ500*20=10S
    tp.par1 = (uint32_t)or_Asendu;
    tp.par2 = (uint32_t)&extparam;  // ������·��¼֡��Ϣ
    tp.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&tp, priority);
}

/*
*********************************************************************************************************
*   �� �� ��: MG2639DCMD_IPSET_ACK
*
*   ����˵��: ���շ�����յ�AT+ZIPSETUPU=1,122.114.22.87,8086�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void MG2639DCMD_IPSET_ACK(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    smflag.UDP_LNK_OK = 1;

    // ��֯��·��¼֡
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // ��·��¼������
    extparam.len = FrmLnkLog(extparam.buf); // Ӧ�ò����ݣ���ʾ��¼֡
    extparam.timeout = device_info.link_timeout * 1000 / 20; //RXOVERTM; // ATOVERTM * 20ms
    extparam.lock = 0;  //��ʼ���ź���Ϊ����״̬

    MG2639D_SENDAT_CIPSEND(0);
}
#undef  _LOCAL_MG2639D

/*****************************  END OF FILE  *********************************/
