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
*   �� �� ��: SIM7600CE_pow_on
*
*   ����˵��: �������ϵ��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
        // ��Դ��
        _VGPRS_ON();
        //���°���
        //_GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 4;
        break;
    case 4:
        //��ʱ25S
        if (HAL_GetTick() - tick > 25000)
        {
            //�ͷŰ���
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
*   �� �� ��: SIM7600CE_SENDAT_ATE0
*
*   ����˵��: �ѷ���֡��AT+ATE0\r\n�����뷢�ͻ�����
*             �ر�AT�������
*
*   ��   ��: uint32_t par1, ����AT����
*            uint32_t par2, ����AT����Ӧ��
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_ATE0(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    gprs_rssi = 1;  // AT����������Ӧ

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPCCFG
*
*   ����˵��: AT����Ϊ��AT+CIPCCFG\r\n��
*             ����socket����ΪĬ��ֵ
*
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPCCFG(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPCCFG;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPSRIP
*
*   ����˵��: ��һ֡AT����Ϊ��AT+CIPSRIP=0\r\n�����뷢�ͻ�����
*             ��������ʱ����ʾ��ʾ��RECV FROM:<IP ADDRESS>:<PORT>��
*
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPSRIP(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPSRIP;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPHEAD
*
*   ����˵��: ��һ֡AT����Ϊ��AT+CIPHEAD=1\r\n�����뷢�ͻ�����
*             ��������ʱ����һ��IPͷ��+IPD(data length)
*
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPHEAD(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPHEAD;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPMODE
*
*   ����˵��: ��һ֡AT����Ϊ��AT+CIPMODE=0\r\n�����뷢�ͻ�����
*             ѡ��TCPIP Ӧ�÷�ʽ
*
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPMODE(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPMODE;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_NETOPEN
*
*   ����˵��: ��һ֡AT����Ϊ��AT+NETOPEN\r\n�����뷢�ͻ�����
*             ������
*
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_NETOPEN(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_NETOPEN;
    rt.par2 = (uint32_t)re_NETOPEN;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPRXGET
*
*   ����˵��: ��һ֡AT����Ϊ��AT+CIPRXGET=0\r\n�����뷢�ͻ�����
*             ����Ϊ�Զ�����
* 
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPRXGET(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPRXGET;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPOPEN
*
*   ����˵��: ��һ֡AT����Ϊ��AT+CIPOPEN=1,"UDP",,,8086\r\n�����뷢�ͻ�����
*             ��UDP��8086Ϊ�����˿ںţ�������������
* 
*   ��   ��: uint32_t par1, ����AT����     
*            uint32_t par2, ����AT����Ӧ�� 
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void SIM7600CE_SENDAT_CIPOPEN(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = ATOVERTM;  // ��ʱATOVERTM * 20ms
    rt.par1 = (uint32_t)or_CIPOPEN;
    rt.par2 = (uint32_t)re_CIPOPEN;
    rt.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: SIM7600CE_SENDAT_CIPSEND
*
*   ����˵��: ʹ��UDP��AT���������֡
*
*   ��    ��: uint8_t priority,���ȼ�
*
*   �� �� ֵ: none
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

    // ��֯Э��֡

    len1 = extparam.len;
    memset(or_CIPSEND, 0, 40);
    sprintf((char *)lennum, "%d", len1);
    sprintf((char *)or_CIPSEND, "AT+CIPSEND=1,%s,\"%s.%s.%s.%s\",%s\r\n", lennum, strnum[0], strnum[1], strnum[2], strnum[3], strport);

    // ��ӷ�����������֡
    tp.SerType = COMTYPE_AT;
    //tp.SerType = extparam.SerType;
    tp.overtime = extparam.timeout; // ��ʱ500*20=10S
    tp.par1 = (uint32_t)or_CIPSEND;
    tp.par2 = (uint32_t)&extparam;  // ������·��¼֡��Ϣ
    tp.par3 = NETWORK_SENDU_ASCII;
    Add2TxSerArray(&tp, priority);
}


/*
*********************************************************************************************************
*   �� �� ��: LOADFRM_LOGIN
*
*   ����˵��: ���շ�����յ��ɹ��ظ�����õĻص�����
*             �ѵ�¼֡���뷢�ͻ�������
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void LOADFRM_LOGIN(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    gprs_rssi = 2;  // ����������

    smflag.UDP_LNK_OK = 1;

    // ��֯��·��¼֡
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // ��·��¼������
    extparam.len = FrmLnkLog(extparam.buf); // Ӧ�ò����ݣ���ʾ��¼֡
    extparam.timeout = device_info.link_timeout * 1000 / 20; //RXOVERTM; // ATOVERTM * 20ms
    extparam.lock = 0;  //��ʼ���ź���Ϊ����״̬

    NETWORK_SENDAT_CIPSEND(0);
}
#undef  _LOCAL_SIM7600CE

/*****************************  END OF FILE  *********************************/
