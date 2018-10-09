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
uint8_t *hostip = "122.114.22.87"; //��˾SXJIANSHE.COM
uint8_t or_At[] = "AT\r\n"; //4
uint8_t or_Ate0[] = "ATE1\r\n"; //6
								//
// MG2639Dר��
uint8_t or_Anode[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n"; //27
uint8_t or_AstatU[] = "AT+ZIPSTATUSU=1\r\n"; //17
uint8_t or_Acloseu[] = "AT+ZIPCLOSEU=1\r\n"; //16
uint8_t or_Acgatt[] = "AT+CGATT=1\r\n"; //12
uint8_t or_Aopen[] = "AT+ZPPPOPEN\r\n"; //13
uint8_t or_Aipset[38] = "AT+ZIPSETUPU=1,122.114.22.87,8086\r\n"; //���38��AT+ZIPSETUPU=1,255.255.255.255,65535\r\n
uint8_t or_Asendu[25] = "AT+ZIPSENDU=1,";

// SIM7600Cר��
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

uint8_t re_NETOPEN[] = "+NETOPEN: 0\r\n";   // �򿪳ɹ�
uint8_t re_NETCLOSE[] = "+NETCLOSE:";
uint8_t re_CIPOPEN[] = "+CIPOPEN: 1,0\r\n";
uint8_t re_CIPSEND[] = "OK\r\n\r\n+CIPSEND:";
uint8_t re_CIPCLOSE[] = "+CIPCLOSE: 1,";
uint8_t re_Arcv[] = "+IPD"; 
uint8_t re_Enter[] = "\r\n";

// AT������ַ�
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
	DTU_rssi = 99;
}

/*
*********************************************************************************************************
*   �� �� ��: NETWORK_INIT
*
*   ����˵��: �������������ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
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

	if (!smflag.IS_POW_ON) return;  // �ϵ粻�ɹ�ֱ���˳�

	cr.SerType = COMTYPE_AT;
	cr.overtime = ATOVERTM;
	cr.par1 = (uint32_t)or_At;  //  ��Ҫ���͵�AT�����׵�ַ
	cr.par2 = (uint32_t)re_ok;  //  �ȴ��ظ���AT���������׵�ַ
	cr.par3 = NETWORK_SENDU_ASCII;

	Add2TxSerArray(&cr, 1);

	smflag.UDP_INIT_OK = 1;
}

/*
*********************************************************************************************************
*   �� �� ��: NETWORK_TASK
*
*   ����˵��: ������������main����ѭ���е���
*
*   ��   ��: NONE
*
*   �� �� ֵ: none
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
*   �� �� ��: NETWORK_SENDU_ASCII
*
*   ����˵��: GPRS���ͺ��������Ͳ���ָ����ַ�ͳ��ȵĻ���������
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
*   �� �� ��: NETWORK_SENDU_ASCII
*
*   ����˵��: GPRS���ͺ��������Ͳ���ָ����ַ�ͳ��ȵĻ���������
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
*   �� �� ��: NETWORK_AT_SENDDATA
*
*   ����˵��: UARTͨ��AT�������������
*
*   ��   ��: uint8_t priority�����ȼ�
*
*   �� �� ֵ: none
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
*   �� �� ��: NETWORK_SENDDATA_PROTOCOL
*
*   ����˵��: ���շ�����յ�"AT+ZIPSENDU"/"AT+CIPSEND"����Ļظ�">"����õĻص�����
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void NETWORK_SENDDATA_PROTOCOL(uint32_t par1, uint32_t par2)
{
    COMSERARRAY rt;
    //param_ext *ptr;

    //ptr =  (param_ext *)ComSerCtr.TxOpoint->par2;
    // ��ӷ�����������֡��
    rt.SerType = extparam.SerType;  // ��¼����
    rt.overtime = extparam.timeout;         // ��ʱ250*20=5S
    rt.par1 = (uint32_t)(extparam.buf);     // ȡ��������AFN
    rt.par2 = (uint32_t)(extparam.len); // ȡ���������¼ʶ����
	rt.par3 = NETWORK_SENDU_HEX;

    Add2TxSerArray(&rt,1);
}
/*
*********************************************************************************************************
*   �� �� ��: NETWORK_RECV
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
void NETWORK_RECV(void)
{
    COMSERARRAY rt;

    // ���ջ������������˳�
    //if (recvfifo.in == recvfifo.out)
    {
        // �ڴ˴������ѯ�ź�����AT�����Ӱ������ATͨ��
        if (StatusFlag.STAT_CSQ_CHK && (ComSerCtr.TxWpoint==ComSerCtr.TxRpoint))
        {
                StatusFlag.STAT_CSQ_CHK = 0;
                rt.SerType = COMTYPE_AT;
                rt.overtime = ATOVERTM;
                rt.par1 = (uint32_t)or_CSQ;  //  ��Ҫ���͵�AT�����׵�ַ
                rt.par2 = (uint32_t)re_CSQ;  //  �ȴ��ظ���AT���������׵�ַ
                rt.par3 = NETWORK_SENDU_ASCII;
                Add2TxSerArray(&rt, 0);
                return;
		}

    }
	// ����AT����ظ�
	NETWORK_RECV_ATACK();
	// ��������֡
	NETWORK_RECV_DATAFRAME();
}
/*
*********************************************************************************************************
*   �� �� ��: NETWORK_SENDAT_CIPSEND
*
*   ����˵��: ���緢������֡
*
*   ��   ��: priority�����ȼ���0��������1�����
*
*   �� �� ֵ: none
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
*   �� �� ��: NETWORK_CHKRPT
*
*   ����˵��: GPRS����Ա���Ϣ���ĺ�������Ҫ���ڲ�ѯ�Ƿ��������ϴ����ݱ�־������У���ӳ�Ա�����Ͷ���
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void NETWORK_CHKRPT(void)
{
	uint16_t len;
    uint32_t relink_time;

    relink_time = device_info.link_timeout * 1000 / 20;	//ת������

	if (ComSerCtr.TxRpoint == ComSerCtr.TxOpoint) return;//������
	// �����ǰ��������������ֱ���˳�
	if (extparam.lock) return;

    // ��ѯ�Ƿ�������֡��Ҫ�ϴ�
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
    // ��ѯ�������ݱ�־
	len = FrmTowWrnDat(extparam.buf);
	if (len)	// ��ʾ��
	{	
		extparam.len = len;
		extparam.SerType = COMTYPE_REPORT;
		//extparam.pBuf = extparam.buf;
		extparam.timeout = relink_time;	// 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);

		return;
	}

	// ��ѯ����ѭ����־
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

	// ��ѯ�궨���ݱ�־
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

	// ��ѯʵʱ���ݱ�־
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
    // ��ѯ������ʵʱ���ݱ�־
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

    // ��ѯ����������ѭ����־
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

    // ��ѯ�������ݱ�־
    len = FrmElvtWrnDat(extparam.buf);
    if (len)    // ��ʾ��
    {
        extparam.len = len;
        extparam.SerType = COMTYPE_REPORT;
        //extparam.pBuf = extparam.buf;
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);

        return;
    }

    // ��ѯ�������궨���ݱ�־
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
    // ��ѯ�ﳾ���ʵʱ���ݱ�־
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

    // ��ѯ�������ݱ�־
    len = FrmDustWrnDat(extparam.buf);
    if (len)    // ��ʾ��
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
                                                       
    // ��ѯ�������ݱ�־                                
                                                       
    len = FrmUPPlatWrnDat(extparam.buf);               
    if (len)    // ��ʾ��                              
    {                                                  
        extparam.len = len;                            
        extparam.SerType = COMTYPE_REPORT;             
        //extparam.pBuf = extparam.buf;                
        extparam.timeout = relink_time; // 500X20ms=10S
                                                       
        NETWORK_SENDAT_CIPSEND(0);                     
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
        extparam.timeout = relink_time; // 500X20ms=10S

        NETWORK_SENDAT_CIPSEND(0);                           
        return;                                
	}
}
/*
*********************************************************************************************************
*   �� �� ��: NETWORK_RECV_DATAFRAME
*
*   ����˵��: ������յ�������֡��Э��֡�Ĵ������
*
*   ��   ��: none
*
*   �� �� ֵ: none
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

    if (a)  // ��ʾ�յ�"+ZIPRECVU:1,"����"+IPD"
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
                
                // ��ȡ��Ч֡
                if (TIP_frame_get(&recvfifo_5, RFULL_frame))  // ��ѯ���ջ������Ƿ�������֡
                {
                    if (!mrtn.mf_flag) afn = rtn.appzone.functioncode;
                    else afn = mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode;

                    // �ж�֡����
					if (afn >= AFN_SET_ADDRESS && afn < AFN_SELF_REALTIMEDATA)
					{
						NETWORK_RECV_DOWN(afn, RFULL_frame); // ����Э�����ò�ѯ֡
					}
					else
					{
						NETWORK_RECV_PRTCACK(afn, RFULL_frame); // ����Э��Ӧ��֡
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
*   �� �� ��: NETWORK_RECV_ATACK
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
void NETWORK_RECV_ATACK(void)
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
	


    // recvfifo.inδ��ͷ
    b = cmp_char(&recvfifo_5, dptr);
    if (b)
    {
        // ������͵����ն�����
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
            // ������͵����ն�����
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
*   �� �� ��: NETWORK_RECV_PRTCACK
*
*   ����˵��: ������պ�������Ҫ���ڲ�ѯ���ڻ��������յ������ݷ��ദ��
*                           ���� ����֡Ӧ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
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
    // Ӧ���ĵĻص�����ֻ����Ӧ���͵����ã�����ص��������д���
    rt.par3 = FProtocolAck;
    Add2RxSerArray(&rt);
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
}
/*
*********************************************************************************************************
*   �� �� ��: NETWORK_RECV_DOWN
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
void NETWORK_RECV_DOWN(uint8_t appfuncode,uint8_t *buf)
{
	// ����֡�ֵ�֡�Ͷ�֡
	switch (appfuncode)
	{
		case AFN_SET_ADDRESS:// ����ң���ն˵�ַ  
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevAddrDatSet(buf,extparam.buf);
		break;                                                   
		case AFN_QRY_ADDRESS:// ��ѯң���ն˵�ַ   
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevAddrDatQry(buf,extparam.buf);
		break;                                                   
		case AFN_SET_CLOCK:// ����ң���ն�ʱ��                                      
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevRtcDatSet(buf,extparam.buf);
		break;                                                 
		case AFN_QRY_CLOCK:// ��ѯң���ն�ʱ��                                
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevRtcQry(buf,extparam.buf);
		break;                                         
		case AFN_SET_WORKMODE:// ����ң���ն˹���ģʽ                               
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevWkModSet(buf,extparam.buf);
		break;                                        
		case AFN_QRY_WORKMODE:// ��ѯң���ն˹���ģʽ                            
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevWkModQry(buf,extparam.buf);
		break;                                                
		case AFN_SET_SENSORTYPE:// ����ң���ն˵Ĵ���������
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSnsrTypSet(buf,extparam.buf);
		break;
		case AFN_QRY_SENSORTYPE:// ��ѯң���ն˵Ĵ���������  
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSnsrTypQry(buf,extparam.buf);
		break;
		case AFN_SET_SENSORPARAM:// ����ң���ն˵Ĵ���������
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSnsrCfgSet(buf,extparam.buf);
		break;
		case AFN_QRY_SENSORPARAM:// ��ѯң���ն˵Ĵ���������
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSnsrCfgQry(buf,extparam.buf);
		break;
		case AFN_SET_DEVIPPORT:// ����ң���ն˴洢������վIP��ַ�Ͷ˿ں�
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevIpPortSet(buf,extparam.buf);
		break;
		case AFN_QRY_DEVIPPORT:// ��ѯң���ն˴洢������վIP��ַ�Ͷ˿ں�
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevIpPortQry(buf,extparam.buf);
		break;
		case AFN_SET_HEARTINTERVAL:// ����ң���ն���·�������
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevHrtIntvlSet(buf,extparam.buf);
		break;          
		case AFN_QRY_HEARTINTERVAL:// ��ѯң���ն���·�������
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevHrtIntvlQry(buf,extparam.buf);
		break;			
		case AFN_SET_RECONNECTINTERVAL:// ����ң���ն���·�������
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevLnkReconIntvlSet(buf,extparam.buf);
		break;  
		case AFN_QRY_RECONNECTINTERVAL:// ��ѯң���ն���·�������
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevLnkReconIntvlQry(buf,extparam.buf);
		break; 
		case AFN_SET_DATRECINTERVAL:// ����ң���ն���ʷ���ݴ��̼��
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevSavIntvlSet(buf,extparam.buf);
		break;    
		case AFN_QRY_DATRECINTERVAL:// ��ѯң���ն���ʷ���ݴ��̼��
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevSavIntvlQry(buf,extparam.buf);
		break;    
		case AFN_SET_DATUPLOADINTERVAL:// ����ң���ն˵�ʵʱ�����ϱ����
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevRtdRptIntvlSet(buf,extparam.buf);
		break;    
		case AFN_QRY_DATUPLOADINTERVAL:// ��ѯң���ն˵�ʵʱ�����ϱ����
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevRtdRptIntvlQry(buf,extparam.buf);
		break;    
		case AFN_SET_UPDATE:// ����ң���ն˵���������
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevUpdSet(buf,extparam.buf);
		break;    
		case AFN_QRY_VERINFO:// ��ѯң���ն˵İ汾��Ϣ
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevVerInfoQry(buf,extparam.buf);
		break;    
		case AFN_SET_PASSWORD:// ����ң���ն˵�����
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmDevPwdSet(buf,extparam.buf);
		break;    
		case AFN_QRY_PASSWORD:// ��ѯң���ն˵�����
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevPwdQry(buf,extparam.buf);
		break;    
		case AFN_CHG_FINGER:// ָ�����ݱ��
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
        case AFN_QRY_LOCATION:// ��ѯң���ն�GPSλ��
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmDevLctQry(buf,extparam.buf);
		break;    
		case AFN_SET_TOWERPARAM:// ����������̬�ṹ����
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrInfoSet(buf,extparam.buf);
		break;    
		case AFN_QRY_TOWERPARAM:// ��ѯ������̬�ṹ����
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrInfoQry(buf,extparam.buf);
		break;    
		case AFN_SET_PROTECTIONZONE:// ��������������
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrPrtcZoneSet(buf,extparam.buf);
		break;    
		case AFN_QRY_PROECTIONZONE:// ��ѯ����������
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrPrtcZoneQry(buf,extparam.buf);
		break;    
		case AFN_SET_LIMIT:// ����������λ��Ϣ
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrLmtSet(buf,extparam.buf);
		break;    
		case AFN_QRY_LIMIT:// ��ѯ������λ��Ϣ
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrLmtQry(buf,extparam.buf);
		break;    
		case AFN_SET_MOMENTCURVE:// ����������������
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrTorqSet(buf,extparam.buf);
		break;    
		case AFN_QRY_MOMENTCURVE:// ��ѯ������������
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrTorqQry(buf,extparam.buf);
		break;    
		case AFN_SET_CALIBRATPARAM:// ���������궨����
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrCaliSet(buf,extparam.buf);
		break;    
		case AFN_QRY_CALIBRATPARAM:// ��ѯ�����궨����
            extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrCaliQry(buf,extparam.buf);
		break;    
		case AFN_SET_TOWERLIFT:// ����������������
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmTwrLiftSet(buf,extparam.buf);
		break;    
		case AFN_QRY_TOWERLIFT:// ��ѯ������������
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmTwrLiftQry(buf,extparam.buf);
		break;
        #endif
        #ifdef ELIVATOR
        case AFN_SET_ELVTINFO:// ���������������ṹ����
			extparam.SerType = COMTYPE_SET;
			extparam.len = FrmElvtInfoSet(buf,extparam.buf);
		break;    
		case AFN_QRY_ELVTINFO:// ��ѯ�����������ṹ����
			extparam.SerType = COMTYPE_QUERY;
			extparam.len = FrmElvtInfoQry(buf,extparam.buf);
		break;	  
		case AFN_SET_ELVTFLOOR:	// ����������¥�����
            extparam.SerType = COMTYPE_SET;
			extparam.len = FrmElvtFloorSet(buf,extparam.buf);
		break;    
		case AFN_QRY_ELVTFLOOR:// ��ѯ������¥�����
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
	// �������б��ĵĻص�����Ϊ����Ӧ��֡������׼��

    // ��֯Ӧ��֡
    extparam.timeout = ATOVERTM; // ATOVERTMX20ms=5S
    extparam.lock = 1;  // �������ݣ����б������ȼ����������ϱ�����
    NETWORK_SENDAT_CIPSEND(1);    // ����ϴ�Ӧ��δ�ɹ��յ��������Ӧ���ͻ��ʹ�üӼ��������֮ǰ���Ͷ�����δ����֡
    //Add2RxSerArray(&rt);

    gprs_rssi = 3;
    _LED_COM_ON();
    #ifdef PRINT_UART1
    novar_print(extparam.buf, buf[1]+5);
    #endif
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
char* my_strstr(const char *s1, const char *s2)
{
	if (*s1 == 0) //��� ����s1��û���ַ������֣�s2�����ַ������ؿգ���������ַ�����Ϊ�գ�����s1�׵�ַ��Ҳ���ҵ�ƥ��
	{
		if (*s2) return (char *)NULL;
		return (char *)s1;
	}
	while (*s1) // ֱ��s1�н���������
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

	uint32_t i, j;
	uint8_t *k = NULL, ret = 0;

	if (a1_len < a2_len) return NULL;   // ����������С�ڲ��������˳�

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
*   �� �� ��: GET_CSQ
*
*   ����˵��: ���շ�����յ�"+CSQ: "����õĻص�������
* 
*             ��ȡ�ź�ǿ��ֵ������
*
*   ��   ��: uint32_t par1, uint32_t par2
*
*   �� �� ֵ: none
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
*����         :   uint32_t GSM_StringToHex(char *pStr, uint8_t NumDigits) 
*����         :   ��16������ʽ�ַ���ת��Ϊ16����������(���뱣֤�ַ�����ĸ���Ǵ�д) 
*����         :   pStr:�ַ�����ʼָ�� 
*                   NumDigits:����λ��,16��������λ�� 
*����         :   ת��������� 
*����         :   �� 
*����         :   cp1300@139.com 
*ʱ��         :   2013-04-30 
*����޸�ʱ�� :   2013-10-17 
*˵��         :   �����ַ���"A865"ת����Ϊ0xA865,λ��Ϊ4λ 
					���뱣֤�ַ�����ĸ���Ǵ�д 
*************************************************************************************************************************/
uint32_t GSM_StringToHex(char *pStr, uint8_t NumDigits)
{
	uint8_t temp;
	uint32_t HEX = 0;
	uint8_t i;

	NumDigits = (NumDigits > 8) ? 8 : NumDigits; //���֧��8λ16������

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
*����         :   void GSM_HexToString(uint32_t HexNum,c har *pStr, uint8_t NumDigits) 
*����         :   ����������ת��Ϊ16������ʽ�ַ���(��ĸΪ��д,����������) 
*����         :   HexNum:16�������� 
					pStr:�ַ�������ָ�� 
*                   NumDigits:����λ��,16��������λ�� 
*����         :   �� 
*����         :   �� 
*����         :   cp1300@139.com 
*ʱ��         :   2013-04-30 
*����޸�ʱ�� :   2013-04-30 
*˵��         :   �����ַ���0xA865ת����Ϊ"A865",λ��Ϊ4λ 
*************************************************************************************************************************/
void GSM_HexToString(uint32_t HexNum, char *pStr, uint8_t NumDigits)
{
	uint8_t temp;
	uint8_t i;

	NumDigits = (NumDigits > 8) ? 8 : NumDigits; //���֧��8λ16������

	for (i = 0; i < NumDigits; i++)
	{
		temp = 0x0f & (HexNum >> (4 * (NumDigits - 1 - i)));
		temp = (temp > 0x09) ? (temp - 0x0A + 'A') : (temp + '0');
		pStr[i] = temp;
	}
}




/************************************************************************************************************************* 
*����         :   uint32_t GSM_StringToDec(char *pStr, uint8_t NumDigits) 
*����         :   ��10������ʽ�ַ���ת��Ϊ������(���뱣֤��ȫΪ�����ַ�) 
*����         :   pStr:�ַ�����ʼָ�� 
*                   NumDigits:����λ��,10��������λ�� 
*����         :   ת��������� 
*����         :   �� 
*����         :   cp1300@139.com 
*ʱ��         :   2013-04-30 
*����޸�ʱ�� :   2013-04-30 
*˵��         :   �����ַ���"1865"ת����Ϊ1865,λ��Ϊ4λ 
					���뱣֤��ȫΪ�����ַ� 
*************************************************************************************************************************/
uint32_t GSM_StringToDec(char *pStr, uint8_t NumDigits)
{
	uint32_t temp;
	uint32_t DEC = 0;
	uint8_t i;
	uint8_t j;

	NumDigits = (NumDigits > 10) ? 10 : NumDigits;   //���֧��10λ10������

	for (i = 0; i < NumDigits; i++)
	{
		temp = pStr[i] - '0';
		if (temp > 9)         //ֻ�������ַ�Χ
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

