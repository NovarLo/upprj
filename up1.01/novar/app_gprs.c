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
uint8_t strbuf[256];    // �����ݻ�����������֯AT����֡
extern uint8_t data;

uint8_t *port = "8086";
uint8_t *hostip = "122.114.22.87"; //��˾SXJIANSHE.COM
uint8_t or_At[] = "AT\r\n"; //4
uint8_t or_Ate0[] = "ATE0\r\n"; //6
uint8_t or_Anode[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n"; //27
uint8_t or_AstatU[] = "AT+ZIPSTATUSU=1\r\n";//17
uint8_t or_Acloseu[] = "AT+ZIPCLOSEU=1\r\n";//16
uint8_t or_Acgatt[] = "AT+CGATT=1\r\n"; //12
uint8_t or_Aopen[] = "AT+ZPPPOPEN\r\n"; //13
uint8_t or_Aipset[38] = "AT+ZIPSETUPU=1,122.114.22.87,8086\r\n"; //���38��AT+ZIPSETUPU=1,255.255.255.255,65535\r\n
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
*   �� �� ��: GPRS_2639_POWON
*
*   ����˵��: GPRS 2639 �ϵ���򣬴򿪹��磬ģ�ⰴ���ϵ�
*
*   ��   ��: none
*
*   �� �� ֵ:uint8_t GPRS_POWON_SUCCESS GPRS�ϵ�ɹ�
*                   GPRS_POWON_FAIL GPRS�ϵ�ʧ��
*
*********************************************************************************************************
*/
uint8_t gprs_2639_powon(void)
{
    uint8_t cnt = 0;

    while (!_IS_GPRS_ON())
    {
        // ��Դ��
        _VGPRS_ON();
        //���°���
        _GPRS_PWKEY_PUSH();
        //��ʱ5S
        HAL_Delay(5000);
        //�ͷŰ���
        _GPRS_PWKEY_RELEASE();

        cnt++;
        if (cnt > 2) return GPRS_POWON_FAIL;
    }

    return GPRS_POWON_SUCCESS;
}
/*
*********************************************************************************************************
*   �� �� ��: GPRS_2639_INIT
*
*   ����˵��: GPRS 2639 ��ʼ������
*
*   ��   ��: none
*
*   �� �� ֵ:uint8_t GPRS_POWON_SUCCESS     GPRS�ϵ�ɹ�
*                   GPRS_POWON_FAIL        GPRS�ϵ�ʧ��
*                   GPRS_RETURN_FAIL       GPRS��Ӧ��
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*   �� �� ��: gprs_power_reset
*
*   ����˵��: gprsģ��Ӳ����λ
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
*   �� �� ��: statemachine_init
*
*   ����˵��: ״̬����־��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
*   �� �� ��: gprs_pow_on
*
*   ����˵��: �������ϵ��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
        // ��Դ��
        _VGPRS_ON();
        //���°���
        _GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 4;
        break;
    case 4:
        //��ʱ5S
        if (HAL_GetTick() - tick > 5000)
        {
            //�ͷŰ���
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
*   �� �� ��: gprs_pow_off
*
*   ����˵��: �������ػ�
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
        // ��Դ��
        _VGPRS_ON();
        //���°���
        _GPRS_PWKEY_PUSH();
        tick = HAL_GetTick();
        step = 2;
        break;
    case 2:
        //��ʱ5S
        if (HAL_GetTick() - tick > 5000)
        {
            //�ͷŰ���
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
*   �� �� ��: gprs_udp_init
*
*   ����˵��: udp��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: �ɹ�����ʧ��
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
        rdat = recvfifo_5.in & (recvfifo_5.size - 1) - recvfifo_5.out & (recvfifo_5.size - 1);  // ʵ��δ��ȡ���ݳ���
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
        rdat = recvfifo_5.in & (recvfifo_5.size - 1) - recvfifo_5.out & (recvfifo_5.size - 1);  // ʵ��δ��ȡ���ݳ���
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
            } else    // �лظ������ǲ�����Ҫ��
            {
                if (HAL_GetTick() - tick > 5000)
                {
                    step = 0;
                    tick = HAL_GetTick();
                }
            }
        } else    // û�лظ�����ʱ�˳��ط�
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
*   �� �� ��: gprs_sendu
*
*   ����˵��: GPRS 2639 UDP����ָ����������
*
*   ��   ��: uint8_t *buffer ,uint8_t length
*
*   �� �� ֵ:uint8_t GPRS_POWON_SUCCESS     GPRS�ϵ�ɹ�
*                   GPRS_POWON_FAIL        GPRS�ϵ�ʧ��
*                   GPRS_RETURN_FAIL       GPRS��Ӧ��
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
                {// ��ʱδ���յ�'>'�������ʹ������ݣ���ֹ��ʧ���ַ�����GPRSģ������
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
*   �� �� ��: GPRS_INIT
*
*   ����˵��: GPRS������ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
*   �� �� ��: GPRS_MAIN
*
*   ����˵��: GPRS��������main����ѭ���е���
*
*   ��   ��: NONE
*
*   �� �� ֵ: none
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
*   �� �� ��: UDP_AT_INIT
*
*   ����˵��: UDP��ʼ������AT����
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void UDP_AT_INIT(void)
{
    COMSERARRAY cr;

    // UDP ��ʼ�����ֱ���˳�
    if (smflag.UDP_INIT_OK) return;

    HAL_UART_Receive_IT(HDL_UART_GPRS, recvbuf_5, QUEUE_REV_SIZE);
    gprs_pow_on();

    if (!smflag.IS_POW_ON) return;  // �ϵ粻�ɹ�ֱ���˳�

    cr.SerType = COMTYPE_AT;
    cr.overtime = 250;
    cr.par1 = (uint32_t)or_At;  //  ��Ҫ���͵�AT�����׵�ַ
    cr.par2 = (uint32_t)re_ok;  //  �ȴ��ظ���AT���������׵�ַ
    cr.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&cr, 1);

    smflag.UDP_INIT_OK = 1;
}

/*
*********************************************************************************************************
*   �� �� ��: CMD_AT_ACK
*
*   ����˵��: ���շ�����յ�AT\r\n����ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_AT_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = 250;  // ��ʱ250*20=5S
    rt.par1 = (uint32_t)or_Ate0;
    rt.par2 = (uint32_t)re_ok;
    rt.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: CMD_NODE_ACK
*
*   ����˵��: ���շ�����յ�ATE0����ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_ATE0_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;

    rt.SerType = COMTYPE_AT;
    rt.overtime = 750;  // ��ʱ250*20=5S
    rt.par1 = (uint32_t)or_Aopen;
    rt.par2 = (uint32_t)re_Aopen;
    rt.par3 = GPRS_SENDU_ASCII;
    Add2TxSerArray(&rt,0);
}

/*
*********************************************************************************************************
*   �� �� ��: CMD_NODE_ACK
*
*   ����˵��: ���շ�����յ�AT+CGDCONT=1�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_NODE_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   �� �� ��: CMD_CGATT_ACK
*
*   ����˵��: ���շ�����յ�AT+CGATT=1�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_CGATT_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   �� �� ��: CMD_STATU_ACK
*
*   ����˵��: ���շ�����յ�AT+ZIPSTATUSU=1�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_STATU_ACK(uint32_t par1, uint32_t par2)
{
}
/*
*********************************************************************************************************
*   �� �� ��: CMD_CLOSEU_ACK
*
*   ����˵��: ���շ�����յ�AT+ZIPCLOSEU=1�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_CLOSEU_ACK(uint32_t par1, uint32_t par2)
{
}

/*
*********************************************************************************************************
*   �� �� ��: CMD_OPENU_ACK
*
*   ����˵��: ���շ�����յ�AT+ZPPPOPEN�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
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
*   �� �� ��: CMD_IPSET_ACK
*
*   ����˵��: ���շ�����յ�AT+ZIPSETUPU=1,122.114.22.87,8086�ɹ��ظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_IPSET_ACK(uint32_t par1, uint32_t par2)
{
    //COMSERARRAY rt;
    //uint8_t strnum[4];

    smflag.UDP_LNK_OK = 1;

    // ��֯��·��¼֡
    extparam.SerType = COMTYPE_TICK;
    //extparam.pBuf = extparam.buf;         // ��·��¼������
    extparam.len = FrmLnkLog(extparam.buf); // Ӧ�ò����ݣ���ʾ��¼֡
    extparam.timeout = 250; // 250X20ms=5S
    extparam.lock = 0;  //��ʼ���ź���Ϊ����״̬

    zipsendu(0);
}

/*
*********************************************************************************************************
*   �� �� ��: CMD_SENDU_ACK
*
*   ����˵��: ���շ�����յ�AT+ZIPSENDU����Ļظ�����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void CMD_SENDU_ACK(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;
    //param_ext *ptr;

    //ptr =  (param_ext *)ComSerCtr.TxOpoint->par2;
    // ��ӷ�����������֡��
    rt.SerType = extparam.SerType;  // ��¼����
    rt.overtime = extparam.timeout;         // ��ʱ250*20=5S
    rt.par1 = (uint32_t)(extparam.buf);     // ȡ��������AFN
    rt.par2 = (uint32_t)(extparam.len); // ȡ���������¼ʶ����
    rt.par3 = GPRS_SENDU_HEX;
    Add2TxSerArray(&rt,1);
}

/*
*********************************************************************************************************
*   �� �� ��: GPRS_SENDU_ASCII
*
*   ����˵��: GPRS���ͺ��������Ͳ���ָ����ַ�ͳ��ȵĻ���������
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
*   �� �� ��: GPRS_SENDU_ASCII
*
*   ����˵��: GPRS���ͺ��������Ͳ���ָ����ַ�ͳ��ȵĻ���������
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
*   �� �� ��: my_strstr
*
*   ����˵��: strstr������ԭ�ͣ���S1�ַ������ҵ�s2�ַ����״γ��ֵĵ�ַ
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
char * my_strstr(const char *s1,const char *s2)
{
   if (*s1 == 0)//��� ����s1��û���ַ������֣�s2�����ַ������ؿգ���������ַ�����Ϊ�գ�����s1�׵�ַ��Ҳ���ҵ�ƥ��
    {
      if (*s2)
        return (char *) NULL;
      return (char *) s1;
    }
   while (*s1)// ֱ��s1�н���������
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
*   �� �� ��: my_arraystr
*
*   ����˵��: strstr�����ĸĽ��ͣ���S1�������ҵ�s2�����״γ��ֵĵ�ַ
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
uint8_t* my_arrayarray(uint8_t *a1, uint32_t a1_len, uint8_t *a2, uint32_t a2_len)
{

    uint32_t i,j;
    uint8_t *k=NULL,ret=0;

    if (a1_len < a2_len) return NULL;   // ����������С�ڲ��������˳�

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
*   �� �� ��: cmp_char
*
*   ����˵��: �ַ����ȽϺ�������ѯ���Ƚ��ַ������Ƿ���ڱȽ��ַ���
*
*   ��   ��: uint8_t *m ���� ���Ƚ��ַ���
*            uint8_t *s ���� �Ƚ��ַ���
*            uint32_t num ���� ���Ƚ��ַ�������
*
*   �� �� ֵ: 0,��ƥ�䣻ƥ�䴮��ʼ��ַ
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
*   �� �� ��: GPRS_RECV
*
*   ����˵��: GPRS���մ���������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ATӦ������
*                           ���� ����֡Ӧ��
*                           ���� ��������������֡������֡/��ѯ֡��
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GPRS_RECV(void)
{
    // ���ջ������������˳�
    if (recvfifo_5.in == recvfifo_5.out) return;

    // ����ATӦ��֡
    GPRS_RECV_ATACK();

    // ��������֡
    GPRS_RECV_DATAFRAME();
}

/*
*********************************************************************************************************
*   �� �� ��: GPRS_CHKRPT
*
*   ����˵��: GPRS����Ա���Ϣ���ĺ�������Ҫ���ڲ�ѯ�Ƿ��������ϴ����ݱ�־������У���ӳ�Ա�����Ͷ���
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GPRS_CHKRPT(void)
{
    uint16_t len;

    if (!StatusFlag.STAT_LINK_OK)
    {
    // ����������
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
    if (ComSerCtr.TxRpoint == ComSerCtr.TxOpoint) return;//������
    // �����ǰ��������������ֱ���˳�
    if (extparam.lock) return;

    #ifdef TOWERBOX
    // ��ѯ�������ݱ�־
    len = FrmTowWrnDat(extparam.buf);
    if (len)    // ��ʾ��
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

        return;
    }

    // ��ѯ����ѭ����־
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

    // ��ѯ�궨���ݱ�־
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

    // ��ѯʵʱ���ݱ�־
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
    // ��ѯ������ʵʱ���ݱ�־
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

    // ��ѯ����������ѭ����־
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

    // ��ѯ�������ݱ�־
    len = FrmElvtWrnDat(extparam.buf);
    if (len)    // ��ʾ��
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

        return;
    }

    // ��ѯ�������궨���ݱ�־
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
    // ��ѯ�ﳾ���ʵʱ���ݱ�־
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

    // ��ѯ�������ݱ�־
    len = FrmDustWrnDat(extparam.buf);
    if (len)    // ��ʾ��
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = 500; // 500X20ms=10S

        zipsendu(0);

        return;
    }
    #endif

    // ��ѯ�Ƿ���ָ�Ƶ�¼�����Ϸ�
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
*   �� �� ��: GPRS_RECV_ACK
*
*   ����˵��: GPRS���պ�������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ATӦ������
*                           ���� ����֡Ӧ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GPRS_RECV_ACK(void)
{
    // �޵ȴ�����Ӧ��ֱ���˳�
    if (ComSerCtr.TxOpoint != ComSerCtr.TxRpoint) return;

    // �����AT����
    if (ComSerCtr.TxOpoint->SerType == COMTYPE_AT)
    {
        GPRS_RECV_ATACK();
    }
    // ��AT����Ӧ�� (�Ա�����Ӧ��֡/��������Ӧ��֡)
    if (ComSerCtr.TxOpoint->SerType == COMTYPE_REPORT ||
        ComSerCtr.TxOpoint->SerType == COMTYPE_TICK)
    {
        //GPRS_RECV_PRTCACK();
    }
}

/*
*********************************************************************************************************
*   �� �� ��: GPRS_RECV_ATACK
*
*   ����˵��: GPRS���պ�������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ATӦ������
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GPRS_RECV_ATACK(void)
{
    uint8_t *dptr,*dptr1;
    COMSERARRAY rt;
    uint8_t again = 0;
    uint32_t a,b=0;
    void (*func)(uint32_t par1,uint32_t par2)=NULL; //ִ�к���

    // �޵ȴ�����Ӧ��ֱ���˳�
    if (ComSerCtr.TxOpoint != ComSerCtr.TxRpoint) return;

    // �������AT���ֱ���˳�
    if (ComSerCtr.TxOpoint->SerType != COMTYPE_AT) return;

    // �ж�������AT����
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

    // recvfifo_5.inδ��ͷ
    b = cmp_char(&recvfifo_5, dptr);
    if (b)
    {
        // ������͵����ն�����
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
            // ������͵����ն�����
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

    if (a)  // ��ʾ�յ�+ZIPRECVU:ָ��
    {
        #ifdef PRINT_UART1
        novar_print(dptr, (uint16_t)strlen((const char *)dptr));
        #endif
        // ��ȡ��Ч֡
        if (TIP_frame_get(&recvfifo_5, RFULL_frame))  // ��ѯ���ջ������Ƿ�������֡
        {
            a = 0;
            if (!mrtn.mf_flag) afn = rtn.appzone.functioncode;
            else afn = mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode;

            // �ж�֡����
            if (afn >= AFN_SET_ADDRESS && afn < AFN_SELF_REALTIMEDATA)
            {
                GPRS_RECV_DOWN(afn,RFULL_frame);// ����Э�����ò�ѯ֡
            }
            else
            {
                GPRS_RECV_PRTCACK(afn,RFULL_frame);// ����Э��Ӧ��֡
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
*   �� �� ��: GPRS_RECV_PRTCACK
*
*   ����˵��: GPRS���պ�������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ����֡Ӧ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
    // Ӧ���ĵĻص�����ֻ����Ӧ���͵����ã�����ص��������д���
    rt.par3 = Fnull;
    Add2RxSerArray(&rt);
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}

/*
*********************************************************************************************************
*   �� �� ��: GPRS_RECV_DOWN
*
*   ����˵��: GPRS���պ�������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ��������������֡������֡/��ѯ֡��
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GPRS_RECV_DOWN(uint8_t appfuncode,uint8_t *buf)
{
    // ����֡�ֵ�֡�Ͷ�֡
    switch (appfuncode)
    {
        case AFN_SET_ADDRESS:// ����ң���ն˵�ַ
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevAddrDatSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ADDRESS:// ��ѯң���ն˵�ַ
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevAddrDatQry(buf,extparam.buf);
        break;
        case AFN_SET_CLOCK:// ����ң���ն�ʱ��
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevRtcDatSet(buf,extparam.buf);
        break;
        case AFN_QUERY_CLOCK:// ��ѯң���ն�ʱ��
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevRtcQry(buf,extparam.buf);
        break;
        case AFN_SET_WORKMODE:// ����ң���ն˹���ģʽ
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevWkModSet(buf,extparam.buf);
        break;
        case AFN_QUERY_WORKMODE:// ��ѯң���ն˹���ģʽ
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevWkModQry(buf,extparam.buf);
        break;
        case AFN_SET_SENSORTYPE:// ����ң���ն˵Ĵ���������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSnsrTypSet(buf,extparam.buf);
        break;
        case AFN_QUERY_SENSORTYPE:// ��ѯң���ն˵Ĵ���������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSnsrTypQry(buf,extparam.buf);
        break;
        case AFN_SET_SENSORPARAM:// ����ң���ն˵Ĵ���������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSnsrCfgSet(buf,extparam.buf);
        break;
        case AFN_QUERY_SENSORPARAM:// ��ѯң���ն˵Ĵ���������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSnsrCfgQry(buf,extparam.buf);
        break;
        case AFN_SET_DEVIPPORT:// ����ң���ն˴洢������վIP��ַ�Ͷ˿ں�
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevIpPortSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DEVIPPORT:// ��ѯң���ն˴洢������վIP��ַ�Ͷ˿ں�
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevIpPortQry(buf,extparam.buf);
        break;
        case AFN_SET_HEARTINTERVAL:// ����ң���ն���·�������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevHrtIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_HEARTINTERVAL:// ��ѯң���ն���·�������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevHrtIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_RECONNECTINTERVAL:// ����ң���ն���·�������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevLnkReconIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_RECONNECTINTERVAL:// ��ѯң���ն���·�������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevLnkReconIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_DATRECINTERVAL:// ����ң���ն���ʷ���ݴ��̼��
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevSavIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DATRECINTERVAL:// ��ѯң���ն���ʷ���ݴ��̼��
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevSavIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_DATUPLOADINTERVAL:// ����ң���ն˵�ʵʱ�����ϱ����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevRtdRptIntvlSet(buf,extparam.buf);
        break;
        case AFN_QUERY_DATUPLOADINTERVAL:// ��ѯң���ն˵�ʵʱ�����ϱ����
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevRtdRptIntvlQry(buf,extparam.buf);
        break;
        case AFN_SET_UPDATE:// ����ң���ն˵���������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevUpdSet(buf,extparam.buf);
        break;
        case AFN_QUERY_VERINFO:// ��ѯң���ն˵İ汾��Ϣ
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevVerInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_PASSWORD:// ����ң���ն˵�����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevPwdSet(buf,extparam.buf);
        break;
        case AFN_QUERY_PASSWORD:// ��ѯң���ն˵�����
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevPwdQry(buf,extparam.buf);
        break;
        case AFN_CHG_FINGER:    // ָ�����ݱ��
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmFngrDatSet(buf,extparam.buf);
        break;
        case AFN_DEL_FINGER:// ɾ��ָ������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmFngrDatDel(buf,extparam.buf);
        break;
        case AFN_RESTART:       // ң���ն�����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmRestart(buf,extparam.buf);
            while (1);
        #ifdef TOWERBOX
        case AFN_SET_LOCATION:// ����ң���ն˵���λ��/��γ��
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmDevLctSet(buf,extparam.buf);
        break;
        case AFN_QUERY_LOCATION:// ��ѯң���ն�GPSλ��
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmDevLctQry(buf,extparam.buf);
        break;
        case AFN_SET_TOWERPARAM:// ����������̬�ṹ����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrInfoSet(buf,extparam.buf);
        break;
        case AFN_QUERY_TOWERPARAM:// ��ѯ������̬�ṹ����
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_PROTECTIONZONE:// ��������������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrPrtcZoneSet(buf,extparam.buf);
        break;
        case AFN_QUERY_PROECTIONZONE:// ��ѯ����������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrPrtcZoneQry(buf,extparam.buf);
        break;
        case AFN_SET_LIMIT:// ����������λ��Ϣ
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrLmtSet(buf,extparam.buf);
        break;
        case AFN_QUERY_LIMIT:// ��ѯ������λ��Ϣ
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrLmtQry(buf,extparam.buf);
        break;
        case AFN_SET_MOMENTCURVE:// ����������������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrTorqSet(buf,extparam.buf);
        break;
        case AFN_QUERY_MOMENTCURVE:// ��ѯ������������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrTorqQry(buf,extparam.buf);
        break;
        case AFN_SET_CALIBRATPARAM:// ���������궨����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrCaliSet(buf,extparam.buf);
        break;
        case AFN_QUERY_CALIBRATPARAM:// ��ѯ�����궨����
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrCaliQry(buf,extparam.buf);
        break;
        case AFN_SET_TOWERLIFT:// ����������������
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmTwrLiftSet(buf,extparam.buf);
        break;
        case AFN_QUERY_TOWERLIFT:// ��ѯ������������
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmTwrLiftQry(buf,extparam.buf);
        break;
        #endif
        #ifdef ELIVATOR
        case AFN_SET_ELVTINFO:// ���������������ṹ����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmElvtInfoSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ELVTINFO:// ��ѯ�����������ṹ����
            extparam.SerType = COMTYPE_QUERY;
            extparam.len = FrmElvtInfoQry(buf,extparam.buf);
        break;
        case AFN_SET_ELVTFLOOR:// ����������¥�����
            extparam.SerType = COMTYPE_SET;
            extparam.len = FrmElvtFloorSet(buf,extparam.buf);
        break;
        case AFN_QUERY_ELVTFLOOR:// ��ѯ������¥�����
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
    // �������б��ĵĻص�����Ϊ����Ӧ��֡������׼��

    // ��֯Ӧ��֡
    extparam.timeout = 250; // 250X20ms=5S
    extparam.lock = 1;  // �������ݣ����б������ȼ����������ϱ�����
    zipsendu(1);    // ����ϴ�Ӧ��δ�ɹ��յ��������Ӧ���ͻ��ʹ�üӼ��������֮ǰ���Ͷ�����δ����֡
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
    // ��֯��·��¼֡

    len = extparam.len;
    memset(or_Asendu, 0, 25);
    sprintf((char *)strnum, "%d", len);
    sprintf((char *)or_Asendu, "AT+ZIPSENDU=1,%s\r\n", strnum);

    // ��ӷ�����������֡
    tp.SerType = COMTYPE_AT;
    tp.overtime = 500; // ��ʱ500*20=10S
    tp.par1 = (uint32_t)or_Asendu;
    tp.par2 = (uint32_t)&extparam;  // ������·��¼֡��Ϣ
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
