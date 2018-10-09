#define _LOCAL_PRTC
/*TIP ���� tower information protocol */

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
*   �� �� ��: TIP_init
*
*   ����˵��: ��ʼ��
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void TIP_init(void)
{

    // ��ʼ�����ͺͽ��ջ��λ�����
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

    // ��ʼ����֡Ӧ������FIFO
    mfappfifo_5.buffer = mfappbuf_5;
    memset(mfappfifo_5.buffer, 0, BUFFERSIZE);
    mfappfifo_5.size = BUFFERSIZE;
    mfappfifo_5.in = 0;
    mfappfifo_5.out = 0;

    // ��ʼ����־λ
    StatusFlag.STAT_PRINT_BUSY = 0;
    StatusFlag.STAT_GPRSSEND_BUSY = 0;  // ������
    StatusFlag.STAT_GPRS_ZPPP = 1;  // ��ʼ��GPRS δ����
    StatusFlag.STAT_HEART_OV = 0;   // ������������
    StatusFlag.STAT_CSQ_CHK =1;     // 
    StatusFlag.STAT_LINK_OK = 0;    // ���ӳɹ���־����ʼ��Ϊδ����
    StatusFlag.resend_start = HAL_GetTick();    // �ط����Ƶ���ʼ�����ڷ�����֡�ṹ��ʼ��ֵ
    StatusFlag.resend_interval = RESEND_INTERVAL;   // �ط����Ƶ�ʱ��������Ϊ����
    StatusFlag.resend_times = RESEND_TIMES; // ����ط�����Ϊ3
    StatusFlag.mframecnt = 0;   // Ĭ�ϵ�֡
    StatusFlag.STAT_LINK_LOG = 1;   // =1��ʾ��·��Ҫ��ʼ��
    StatusFlag.STAT_WAIT_ACK = 0;   // =1��ʾ�ȴ�Ӧ��
    StatusFlag.STAT_NO_INIT = 0;
    mrtn.mf_flag = 0;

    // ��ʱ��ʼ������֡�ã���ʽ�汾ɾ��
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
*   �� �� ��: TIP_frame_get
*
*   ����˵��: �ӽ���FIFO��ȡ��һ֡������֡
*
*   ��   ��: pRingBuf ringbuf ���� Դ���ջ������ṹָ��
*           void *buffer ���� Ŀ�����֡���黺�����׵�ַ
*           pTX101 pframe ���� Ŀ��ṹָ��
*
*   �� �� ֵ: 0����֡��������·����Ҫ�󣬶�����
*            1����֡Ϊ������·����Ҫ�������֡������
*
*********************************************************************************************************
*/
uint8_t TIP_frame_get(pRingBuf ringbuf,uint8_t *buffer)
{
	uint8_t bytebuf, bytebuf1, *sptr, *dptr;
	uint32_t i, len = 0, in, out;

	sptr = buffer;
	// ����ȡ���ؼ��ֽڿ��Ƿ����֡��ʽҪ��
	// ��ʼ�ַ�

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

	// ����пɶ����ݣ�ֱ���ҵ������ַ�
	for (i = out; i < in; i++)
	{
		bytebuf = ringbuf->buffer[i & (ringbuf->size - 1)];
		if (bytebuf == STARTCHAR) break;
	}
	if ((i + 2) > in) return TIP_FAIL;

	bytebuf1 = ringbuf->buffer[((i + 2) & (ringbuf->size - 1))];

	if (bytebuf1 != STARTCHAR) return TIP_FAIL;

	// �û����ݳ���
	len = ringbuf->buffer[((i + 1) & (ringbuf->size - 1))];

	if ((in - i) < len + 5) return TIP_FAIL;

	if (ringbuf->buffer[((i + len + 4) & (ringbuf->size - 1))] != ENDCHAR) return TIP_FAIL; 

	// ��������֡��Ϣ
	ringbuf->out = i;
	__ring_buffer_get(ringbuf, sptr, len + 5);

	// �ж�CRCУ��λ
	if (sptr[len + 3] != GetCRC7ByLeftByTable(&buffer[3], len)) return TIP_FAIL;

	// ��Ч֡���ݷ���֡�ṹ
	// ��·����
	sptr = buffer;
	if ((buffer[3] >> 6) & 0x01)    // ��֡����
	{
		// �ж��ǵڼ�֡
		mrtn.mframe_num = *((uint16_t *)(&buffer[4]));
		mrtn.mframe_cnt = *((uint16_t *)(&buffer[6]));
		if (mrtn.mframe_num == mrtn.mframe_cnt) // ��֡����֡������ȱ�ʾ�ǵ�һ֡����
		{
			mrtn.mf_flag = 1;
			//StatusFlag.mframecnt = 1;	// ��֡��־
			mrtn.mframe_st = 1 << (mrtn.mframe_cnt - 1);   // ֡��Ӧ״̬λ��λ

		}
		else
		{
			mrtn.mframe_st |= 1 << (mrtn.mframe_cnt - 1);   // ֡��Ӧ״̬λ��λ
		}

		if (mrtn.mframe_cnt == 1 && mrtn.mframe_st != (uint16_t)(pow(2, mrtn.mframe_num) - 1)) // ��֡���һ֡û��������ֱ���˳�����ظ�
		{
			mrtn.mf_flag = 0;   // ��ʾ��֡����ʧ��
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

		// Ӧ�ò����ݲ���(���������͸�����)
		dptr = mrtn.frame[mrtn.mframe_cnt - 1].appzone.userdata;
		for (i = 0; i < mrtn.frame[mrtn.mframe_cnt - 1].length - 12; i++)
		{
			*dptr++ = *sptr++;
		}
		mrtn.mlen[mrtn.mframe_cnt - 1] = mrtn.frame[mrtn.mframe_cnt - 1].length - 12; //��֡Ӧ�ò㻺����ռ�ó���

		mrtn.frame[mrtn.mframe_cnt - 1].cs = *sptr++;
		mrtn.frame[mrtn.mframe_cnt - 1].endbyte = *sptr;
	}
	else    // ��֡����
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

		// Ӧ�ò����ݲ���(���������͸�����)
		dptr = rtn.appzone.userdata = rtnbuf;
		for (i = 11; i < rtn.length + 3; i++)    // 8:������1+��ַ��5+Э��汾1+Ӧ�ò㹦����
		{
			*dptr++ = *sptr++;
		}
		// CS+�����ַ�
		rtn.cs = *sptr++;
		rtn.endbyte = *sptr;
		//StatusFlag.mframecnt = 0;
		//mrtn.mf_flag=0;	// ���յ���һ��֡����֡��־����
	}
	return len + 5;
}

/*
*********************************************************************************************************
*   �� �� ��: TIP_login
*
*   ����˵��: Э���¼����¼���ȴ�ȷ�ϣ���ɵ�¼
*
*   ��   ��: none
*
*   �� �� ֵ: ���ص�¼�ɹ�����ʧ��
*
*********************************************************************************************************
*/
void TIP_login(void)
{
    frame_link_chk(DATAFIELD_LOGIN);    // ���͵�¼֡

    // �յ�ȷ��֡������޸ĵ�¼״̬Ϊ��¼�ɹ�
}

/*
*********************************************************************************************************
*   �� �� ��: frame_link_chk
*
*   ����˵��: ����֡,��Ӧ��·��ⱨ�ġ�AFN=03H
*           ������1���ֽڣ�F0��¼��F1�˳���¼��F2���߱���
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*           uint8 answer ���� 1/0 ȷ��/����
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 frame_link_chk(uint8_t datazone)
{
    //uint8_t i;

    rtn.length = 9;
    link_layer_pack(&rtn,1,0,0,LFN_DIR0_LNKRESPON);

    // ��Ϊ�����������б��ĵ���������ͬ�����Գ��Ȳ���
    rtn.appzone.functioncode = AFN_LINKCHK;
    rtn.appzone.userdata = rtnbuf;
    rtnbuf[0] = datazone;
    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: frame_invalid
*
*   ����˵��: ����֡,��Ӧ��Ч�����ġ�AFN=02H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 frame_invalid(pTX101 frame)
{
    //uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_DENYRESPON);

    // δ������·������

    // ��Ϊ�����������б��ĵ���������ͬ�����Գ��Ȳ���
    rtn.appzone.functioncode = AFN_INVALID;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevAddr
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵�ַ���ġ�AFN=10H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevAddr(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr;

    rtn = *frame;
    // �����õ��ն˵�ַд��洢��
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevaddr
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵�ַ���ġ�AFN=50H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamQryDevaddr(pTX101 frame)
{
    uint32_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);

    rtn.length = 8+5;   // ������ļ�������ֽڵĵ�ַ
    rtn.appzone.functioncode = AFN_QRY_ADDRESS;
    for (i = 0; i < 5;i++)
    {
        rtnbuf[i] = device_info.addr[i];
    }
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevRtc
*
*   ����˵��: ����֡,��Ӧ����ң���ն�ʱ�ӱ��ġ�AFN=11H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevRtc
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն�ʱ�ӱ��ġ�AFN=51H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamQryDevRtc(pTX101 frame)
{
    RtcClk clock;
    uint32_t i;


    rtn = *frame;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8+6;   // ������ļ��������ֽڵ�ʱ��
    rtn.appzone.userdata = rtnbuf;
    // ��ȡRTCʱ��
    GetRTC(&clock);

    for (i = 0; i < 6;i++)
    {
        rtnbuf[i] = ((uint8_t *)(&clock))[i];
    }
    rtn.appzone.functioncode = AFN_QRY_CLOCK;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamSetWkMod
*
*   ����˵��: ����֡,��Ӧ����ң���ն˹���ģʽ���ġ�AFN=12H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetWkMod(pTX101 frame)
{

    rtn = *frame;

    // ���汻���õĹ���ģʽ����
    device_info.work_mod = *((uint8_t *)rtn.appzone.userdata);
    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_WORKMODE;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryWkMod
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˹���ģʽ���ġ�AFN=52H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamQryWkMod(pTX101 frame)
{
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_PARAMRESPON);
    rtn.length = 8+1;   // ������ļ���1���ֽڵĹ���ģʽ
    *rtnbuf = device_info.work_mod;
    rtn.appzone.userdata = rtnbuf;


    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamSetSnsrTyp
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵Ĵ�����/�������౨�ġ�AFN=13H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetSnsrTyp(pTX101 frame)
{

    uint32_t i,num;
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õĹ���ģʽ����
    sptr = (uint8_t *)rtn.appzone.userdata;
    num =  *sptr++;

    for (i=1;i<num;i++)
    {
        switch (*sptr>>1)   // Э�鴫�������ͱ��ӳ��ʵ��ͨ����
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
            device_info.sensor_en[SENS_WIND] = *sptr++;//Э����+ ʹ��λ
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQrySnsrTyp
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵Ĵ��������౨�ġ�AFN=53H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    *dptr++ = 9;    //Ŀǰ������ϻ������9�ִ���������
    rtn.length += 1;

    for (i=0;i<9;i++)
    {
        //ֻ���ͺ�ϻ��ʵ��ʹ�ô���������
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamSetSnsrCfg
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵Ĵ������������ġ�AFN=14H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetSnsrCfg(pTX101 frame)
{

    //uint32_t i;
    uint8_t *sptr,type;

    rtn = *frame;

    // ���汻���õĹ���ģʽ����
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
            break; // ���ٴ����� 1float
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQrySnsrCfg
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵Ĵ������������ġ�AFN=54H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    switch (*sptr++)    // ����������
    {
    #ifdef TOWERBOX
        case PRO_SENS_WIND:
            *((uint16_t *)sptr) = device_info.wind_scale * 1000;
            rtn.length += 2;
            break; // ���ٴ����� 1float
        case PRO_SENS_ROTAT:
            *sptr = device_info.rotat_cfg;
            rtn.length += 1;
            break; // ��ת������ 1Byte
        case PRO_SENS_WEIGHT:
            *sptr = device_info.weigth_cfg;
            rtn.length += 1;
            break; // ���ش����� 1Byte
        case PRO_SENS_FINGER:
            *sptr = device_info.finger_cfg;
            rtn.length += 1;
            break; // ָ�ƴ����� 1Byte
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
            break; // ���ٴ����� 1float
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevIpPort
*
*   ����˵��: ����֡,��Ӧ����ң���ն˴洢������վIP��ַ�Ͷ˿ںű��ġ�AFN=1FH
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevIpPort(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,*dptr;

    rtn = *frame;

    // ���汻���õ�IP����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevIpPort
*
*   ����˵��: ����֡,��Ӧң���ն˴洢������վIP��ַ�Ͷ˿ںű��ġ�AFN=5FH
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetHrtIntvl
*
*   ����˵��: ����֡,��Ӧ��������������ġ�AFN=20H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetHrtIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ������������
    sptr = (uint8_t *)rtn.appzone.userdata;
    // �������
    device_info.beat_time = UnPackBCD(sptr,2,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_HEARTINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryHrtIntvl
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵ı��������ġ�AFN=60H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // �������
    PackBCD(dptr,device_info.beat_time,2,0);

    rtn.length += 1;
    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevLnkReconIntvl
*
*   ����˵��: ����֡,��Ӧ�����ն���·����������ġ�AFN=21H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevLnkReconIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ������������
    sptr = (uint8_t *)rtn.appzone.userdata;
    // �������
    device_info.recon_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_RECONNECTINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevLnkReconIntvl
*
*   ����˵��: ����֡,��Ӧ�ն���·����������ġ�AFN=61H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // �������
    PackBCD(dptr,device_info.recon_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevRecIntvl
*
*   ����˵��: ����֡,��Ӧ������ʷ���ݴ��̼�����ġ�AFN=22H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevRecIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ������ϴ��������
    sptr = (uint8_t *)rtn.appzone.userdata;
    // ���ݴ��̼��
    device_info.datsave_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_DATRECINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevRecIntvl
*
*   ����˵��: ����֡,��Ӧ��ʷ���ݴ��̼�����ġ�AFN=62H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ���ݴ��̼��
    PackBCD(dptr,device_info.datsave_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevRTDReptIntvl
*
*   ����˵��: ����֡,��Ӧ����ʵʱ�����ϱ�������ġ�AFN=23H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevRTDReptIntvl(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ������ϴ��������
    sptr = (uint8_t *)rtn.appzone.userdata;
    // ��ʷ�����ϴ����
    device_info.datrpt_time = UnPackBCD(sptr,4,0);

    device_info.flag = CFGSTAT_SAVE;
    device_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_DATUPLOADINTERVAL;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevRTDReptIntvl
*
*   ����˵��: ����֡,��Ӧʵʱ�����ϱ�������ġ�AFN=63H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // �����ϴ����
    PackBCD(dptr,device_info.datrpt_time,4,0);

    rtn.length += 2;

    // aux nouse
    rtn.length += 7;    // PW+Tp

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevUpd
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵��������ݱ��ġ�AFN=24H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevUpd(pTX101 frame)
{
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // Զ�̸�������
    // to be continued...
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_UPDATE;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevVerInfo
*
*   ����˵��: ����֡,��Ӧ��ѯ�ն˰汾��Ϣ���ġ�AFN=64H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // �豸���� & �汾��
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevPwd
*
*   ����˵��: ����֡,��Ӧ����ң���ն����뱨�ġ�AFN=25H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevPwd(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,*dptr,type,Num;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ��������
    sptr = (uint8_t *)rtn.appzone.userdata;
    type = *sptr++;
    Num = *sptr++;
    switch (type)
    {
    case 0x00:  // �û�����
        dptr = device_info.pswd[0];
        break;
    case 0x01:  // ����Ա����/��������
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevPwd
*
*   ����˵��: ����֡,��Ӧң���ն˵����뱨�ġ�AFN=65H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ��ȡ������������
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

#ifdef TOWERBOX
/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrInfo
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵����������������ġ�AFN=16H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrInfo(pTX101 frame)
{

    uint32_t i;
    uint8_t *sptr,*dptr;
    //uint8_t bcd[5];

    rtn = *frame;

    // ���汻���õ�������������
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ����������
    dptr = (uint8_t *)&tower_info.name;

    for (i = 0; i < 16; i++)
    {
        *dptr++ = *sptr++;
    }

    // ����ID
    tower_info.tower_ID = *sptr++;

    // ��ȺID
    tower_info.group_ID = *sptr++;

    // ����X��3B��0.0~9999.91m
    tower_info.org_x = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //����Y��3B��0.0~9999.9m
    tower_info.org_y = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //�����ͺŴ���(2B)
    tower_info.mfr_ID = *sptr++;    // ���̴���
    tower_info.model_ID = *sptr++;  // �ͺŴ���

    //����أ�1B��0.1t
    tower_info.rated_load = (*sptr++) * 100.0f;

    //��������/���ʣ�1B��
    tower_info.ratio = (*sptr)&0xf;
    tower_info.type = *sptr++ >> 4;

    //ǰ�۳��ȣ�3B��BCD2float 0.1m
    tower_info.front = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //��۳��ȣ�3B��0.1m
    tower_info.front = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //ǰ����1λ�ã�3B��0.1m
    tower_info.mast_front1 = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //ǰ����2λ�ã�3B��0.1m
    tower_info.mast_front2 = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //������λ�ã�3B��0.1m
    tower_info.mast_rear = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //�������ظ߶ȣ�3B��0.1m
    tower_info.height = UnPacksBCD(sptr,5,1);
    sptr += 3;

    //��������߶ȣ�2B��0.1m
    tower_info.thick = UnPacksBCD(sptr,3,1);
    sptr += 2;

    //�������ص�����߶ȣ�2B��0.1m
    tower_info.topheight = UnPacksBCD(sptr,3,1);
    sptr += 2;

    //��ת����ϵ����1B��1~99
    tower_info.inertia = UnPacksBCD(sptr,2,0);
    sptr += 1;

    //���߽Ƕȣ�2B��0.1��
    tower_info.topheight = UnPackBCD(sptr,4,1);
    sptr += 2;

    tower_info.flag = CFGSTAT_SAVE;
    tower_info.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_TOWERPARAM;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryTwrInfo
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵����������������ġ�AFN=56H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ��ȡ������������
    dptr = (uint8_t *)rtn.appzone.userdata;

    // ��������
    sptr = tower_info.name;
    for (i = 0; i < 16; i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 16;

    // ����ID
    *dptr++ = tower_info.tower_ID;
    rtn.length += 1;

    // ��ȺID
    *dptr++ = tower_info.group_ID;
    rtn.length += 1;

    // ����X��3B��
    PacksBCD(bcd,tower_info.org_x,5,1);
    sptr = bcd;
    for (i=0;i<3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //����Y��3B��
    PacksBCD(bcd,tower_info.org_y,5,1);
    sptr = bcd;
    for (i=0;i<3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //�����ͺŴ���(2B)
    sptr = (uint8_t *)&tower_info.mfr_ID;   // ���̴���
    *dptr++ = *sptr++;
    rtn.length += 1;

    sptr = (uint8_t *)&tower_info.model_ID; //�ͺŴ���
    *dptr++ = *sptr++;
    rtn.length += 1;

    //����أ�1B��
    *dptr++ = tower_info.rated_load/100;    // 0.1��
    rtn.length += 1;

    //��������/���ʣ�1B��
    *dptr++ = tower_info.ratio|(tower_info.type<<4);
    rtn.length += 1;

    //ǰ�۳��ȣ�3B��BCD2float
    PacksBCD(bcd,tower_info.front,5,1); // 1m
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //��۳��ȣ�3B��
    PacksBCD(bcd,tower_info.rear,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //ǰ����1λ�ã�3B��
    PacksBCD(bcd,tower_info.mast_front1,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //ǰ����2λ�ã�3B��
    PacksBCD(bcd,tower_info.mast_front2,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //������λ�ã�3B��
    PacksBCD(bcd,tower_info.mast_rear,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //�������ظ߶ȣ�3B��
    PacksBCD(bcd,tower_info.height,5,1);
    for (i=0;i<3;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 3;

    //��������߶ȣ�2B��
    PacksBCD(bcd,tower_info.thick,3,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    //�������ص�����߶ȣ�2B��
    PacksBCD(bcd,tower_info.topheight,3,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    //��ת����ϵ����1B��
    PacksBCD(bcd,tower_info.inertia,2,0);
    *dptr++ = bcd[i];
    rtn.length += 1;

    //���߽Ƕȣ�2B��
    PackBCD(bcd,tower_info.walk_dir,4,1);
    for (i=0;i<2;i++)
    {
        *dptr++ = bcd[i];
    }
    rtn.length += 2;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}


/*
*********************************************************************************************************
*   �� �� ��: ParamSetPrtcZone
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵ı��������ġ�AFN=17H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetPrtcZone(pTX101 frame)
{

    uint32_t i,j;
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õı�������Ϣ����
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ������������Ϣ
    zone_tbl.tblsize = *sptr++;

    // ��������Ϣ
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
            case 0:// ��Ԫ��
                zone_tbl.zone[i].dat[j].x = UnPacksBCD(sptr,5,1);
                sptr += 3;
                zone_tbl.zone[i].dat[j].y = UnPacksBCD(sptr,5,1);
                sptr += 3;
                break;
            case 1:// Բ��,����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryPrtcZone
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵ı��������ġ�AFN=57H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamQryPrtcZone(pTX101 frame)
{
    //uint8_t addr[5],
    uint8_t *sptr,*dptr;
    uint32_t i,j,prot_len=0;
    uint16_t pagenum,lastpage;


    // ��������Ϣ�ܿ�����Ҫ��֡����
    // ���ȼ���Ӧ�ò���������Ϣ���жϷּ�֡���ͣ���֡��������ݳ���Ϊ255-12=243

    // ���ϱ�������Ϣ����֡������
    sptr = mrtnbuffer;
    // ����������
    *sptr++ = zone_tbl.tblsize;
    prot_len += 1;
    // ��������Ϣ
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

    // ���pagenum����1��Ϊ��֡��=1Ϊ��֡

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
        // ��֡Ӧ�����ݻ�����������

        sptr = mrtnbuffer;
        for (i=0;i<pagenum;i++)
        {
            for (j=0;j<mrtn.mlen[i];j++)
            {
                mrtnbuf[i][j] = *sptr++;
            }
        }

        // ��У��
        GetCRC(1);
        return NULL;
    }
    else    // ��֡
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

        // ���¼���CRC
        rtn.cs = GetCRC(0);
        return &rtn;
    }
    return NULL;    // for return
}

/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrLmt
*
*   ����˵��: ����֡,��Ӧ����������λ��Ϣ���ġ�AFN=18H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrLmt(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ���λ�ͱ�������
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ��λ����
    // ��ת
    limit_tbl.limit[SENS_ROTAT].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_ROTAT].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_ROTAT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_ROTAT].lowarn = limit_tbl.limit[SENS_ROTAT].hiwarn;

    // �߶�
    limit_tbl.limit[SENS_HEIGHT].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_HEIGHT].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_HEIGHT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_HEIGHT].lowarn = limit_tbl.limit[SENS_HEIGHT].hiwarn;

    // ����
    limit_tbl.limit[SENS_MARGIN].hilimit = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].lolimit = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_MARGIN].lowarn = limit_tbl.limit[SENS_MARGIN].hiwarn;

    // ����
    limit_tbl.limit[SENS_WALK].hilimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_WALK].lolimit = UnPacksBCD(sptr,5,1);
    sptr += 3;
    limit_tbl.limit[SENS_WALK].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_WALK].lowarn = limit_tbl.limit[SENS_WALK].hiwarn;

    // ���
    limit_tbl.limit[SENS_TILT].hiwarn = UnPacksBCD(sptr,3,1);
    sptr += 2;
    limit_tbl.limit[SENS_TILT].hilimit = UnPacksBCD(sptr,3,1);
    sptr += 2;
/*
    limit_tbl.limit[SENS_TILT].hiwarn = UnPackBCD(sptr,4,1);
    sptr += 2;
    limit_tbl.limit[SENS_TILT].lowarn = limit_tbl.limit[SENS_TILT].hiwarn;
*/

    // ����
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

    // Ԥ��12B
    sptr += 12;

    // ��������
    // ��ת
    protect_tbl.protect[SENS_ROTAT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_ROTAT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // �߶�
    protect_tbl.protect[SENS_HEIGHT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_HEIGHT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // ����
    protect_tbl.protect[SENS_MARGIN].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_MARGIN].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // ����
    protect_tbl.protect[SENS_WALK].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_WALK].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // ����
    protect_tbl.protect[SENS_BODY].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_BODY].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

/*
    // ���
    protect_tbl.protect[SENS_TILT].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_TILT].warn = UnPackBCD(sptr,4,1);
    sptr += 2;

    // ����
    protect_tbl.protect[SENS_WIND].alarm = UnPackBCD(sptr,4,1);
    sptr += 2;
    protect_tbl.protect[SENS_WIND].warn = UnPackBCD(sptr,4,1);
    sptr += 2;
*/

    protect_tbl.flag = CFGSTAT_SAVE;
    protect_tbl.delay = 300;

    // Ԥ��12B
    sptr += 12;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_LIMIT;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryTwrLmt
*
*   ����˵��: ����֡,��Ӧ��ѯ������λ��Ϣ���ġ�AFN=58H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ��ȡ��λ�ͱ�������
    dptr = (uint8_t *)rtn.appzone.userdata;

    // ��λ����
    // ��ת
    PacksBCD(dptr,limit_tbl.limit[SENS_ROTAT].hilimit,5,1); // ����λ
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_ROTAT].lolimit,5,1); // ����λ
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_ROTAT].hiwarn,4,1);   // Ԥ��ֵ
    dptr += 2;
    rtn.length += 8;

    // �߶�
    PacksBCD(dptr,limit_tbl.limit[SENS_HEIGHT].hilimit,5,1);// ����λ
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_HEIGHT].lolimit,5,1);// ����λ
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_HEIGHT].hiwarn,4,1);  // Ԥ��ֵ
    dptr += 2;
    rtn.length += 8;

    // ����
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].hilimit,4,1); // Զ��λ
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].lolimit,4,1); // ����λ
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_MARGIN].hiwarn,4,1);  // Ԥ��ֵ
    dptr += 2;
    rtn.length += 6;

    // ����
    PacksBCD(dptr,limit_tbl.limit[SENS_WALK].hilimit,5,1);  // ǰ��λ
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_WALK].lolimit,5,1);  // ����λ
    dptr += 3;
    PackBCD(dptr,limit_tbl.limit[SENS_WALK].hiwarn,4,1);    // Ԥ��ֵ
    dptr += 2;
    rtn.length += 8;

    // ���
    PacksBCD(dptr,limit_tbl.limit[SENS_TILT].hiwarn,3,1);   // Ԥ��ֵ
    dptr += 2;
    PacksBCD(dptr,limit_tbl.limit[SENS_TILT].hilimit,3,1);  // ����ֵ
    dptr += 2;
/*
    PackBCD(dptr,limit_tbl.limit[SENS_TILT].hiwarn,4,1);
    dptr += 2;
*/
    rtn.length += 4;

    // ����
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hiwarn,4,1);    // Ԥ��ֵ
    dptr += 2;
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hilimit,4,1);   // ����ֵ
    dptr += 2;
/*
    PackBCD(dptr,limit_tbl.limit[SENS_WIND].hiwarn,4,1);
    dptr += 2;
*/
    rtn.length += 4;

    // Ԥ��12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    // ��������
    // ��ת
    PackBCD(dptr,protect_tbl.protect[SENS_ROTAT].alarm,4,1);    // ����ֵ
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_ROTAT].warn,4,1);     // Ԥ��ֵ
    dptr += 2;
    rtn.length += 4;

    // �߶�
    PackBCD(dptr,protect_tbl.protect[SENS_HEIGHT].alarm,4,1);   // ����ֵ
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_HEIGHT].warn,4,1);    // Ԥ��ֵ
    dptr += 2;
    rtn.length += 4;

    // ����
    PackBCD(dptr,protect_tbl.protect[SENS_MARGIN].alarm,4,1);   // ����ֵ
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_MARGIN].warn,4,1);    // Ԥ��ֵ
    dptr += 2;
    rtn.length += 4;

    // ����
    PackBCD(dptr,protect_tbl.protect[SENS_WALK].alarm,4,1);     // ����ֵ
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_WALK].warn,4,1);      // Ԥ��ֵ
    dptr += 2;
    rtn.length += 4;


    // ����
    PackBCD(dptr,protect_tbl.protect[SENS_BODY].alarm,4,1);     // ����ֵ
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_BODY].warn,4,1);      // Ԥ��ֵ
    dptr += 2;
    rtn.length += 4;
    /*
    // ���
    PackBCD(dptr,protect_tbl.protect[SENS_TILT].alarm,4,1);
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_TILT].warn,4,1);
    dptr += 2;
    rtn.length += 4;

    // ����
    PackBCD(dptr,protect_tbl.protect[SENS_WIND].alarm,4,1);
    dptr += 2;
    PackBCD(dptr,protect_tbl.protect[SENS_WIND].warn,4,1);
    dptr += 2;
    rtn.length += 4;
*/

    // Ԥ��12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrTorque
*
*   ����˵��: ����֡,��Ӧ�����������߱��ġ�AFN=19H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrTorque(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr;

    rtn = *frame;

    // ���汻���õ���������
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ����
    tower_info.ratio = *sptr++;

    // �������ߵ���
    torque_tbl.tblsize = *sptr++;

    // ÿ����ķ��Ⱥ�����
    for (i=0; i<torque_tbl.tblsize;i++)
    {
        torque_tbl.dat[i].distance = UnPackBCD(sptr,4,1);
        sptr += 2;
        torque_tbl.dat[i].weight = UnPackBCD(sptr,6,3)*1000;//����
        sptr += 3;
    }

    torque_tbl.flag = CFGSTAT_SAVE;
    torque_tbl.delay = 300;
    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8+1;
    rtn.appzone.functioncode = AFN_SET_MOMENTCURVE;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryTwrTorque
*
*   ����˵��: ����֡,��Ӧ��ѯ�������߱��ġ�AFN=59H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ��ȡ��λ�ͱ�������
    dptr = (uint8_t *)rtn.appzone.userdata;

    // ����
    *dptr++ = tower_info.ratio;
    rtn.length += 1;

    // �������ߵ���
    *dptr++ = torque_tbl.tblsize;
    rtn.length += 1;

    // ÿ����ķ��Ⱥ�����
    for (i=0; i<torque_tbl.tblsize;i++)
    {
        PackBCD(dptr,torque_tbl.dat[i].distance,4,1);
        dptr += 2;
        PackBCD(dptr,torque_tbl.dat[i].weight/1000,6,3);
        dptr += 3;
        rtn.length += 5;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrCali
*
*   ����˵��: ����֡,��Ӧ���������궨�������ġ�AFN=1AH
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrCali(pTX101 frame)
{
    uint32_t i;
    uint8_t *sptr,sensor_type,Num;
    APP_CALICHDAT_TypeDef *ts;

    rtn = *frame;

    // ���汻���õı궨����
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ����������
    sensor_type = *sptr++;
    // ����������
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryTwrCali
*
*   ����˵��: ����֡,��Ӧ��ѯ�����궨�������ġ�AFN=5AH
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrLift
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵������������ݱ��ġ�AFN=26H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetTwrLift(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ������������
    sptr = (uint8_t *)rtn.appzone.userdata;

    tower_info.last_lift = UnPacksBCD(sptr,3,1);

    tower_info.flag = CFGSTAT_SAVE;
    tower_info.delay = 300;
    rtn.length = 8;
    rtn.appzone.functioncode = AFN_SET_TOWERLIFT;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetTwrLmt
*
*   ����˵��: ����֡,��Ӧң���ն˵������������ġ�AFN=66H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ������������
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetDevLct
*
*   ����˵��: ����֡,��Ӧ����ң���ն˵ĵ���λ��/��γ�ȱ��ġ�AFN=15H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetDevLct(pTX101 frame)
{

    uint32_t i;
    uint8_t *sptr,*dptr;

    rtn = *frame;

    // ���汻���õĹ���ģʽ����
    sptr = (uint8_t *)rtn.appzone.userdata;
    // ����
    dptr = tower_info.longitude;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    // γ��
    dptr = tower_info.latitude;
    for (i = 0;i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryDevLct
*
*   ����˵��: ����֡,��Ӧ��ѯң���ն˵ĵ���λ��/��γ�ȱ��ġ�AFN=55H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    // ����
    sptr = tower_info.longitude;
    for (i=0;i<6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;
    // γ��
    sptr = tower_info.latitude;
    for (i=0;i<6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;
    // ����
    sptr = tower_info.altitude;
    for (i=0;i<2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //

    return &rtn;
}
#endif


#ifdef ELIVATOR
/*
*********************************************************************************************************
*   �� �� ��: ParamSetElvtInfo
*
*   ����˵��: ������������Ϣ���ġ�AFN=30H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetElvtInfo(pTX101 frame)
{
    uint8_t *sptr;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // �����������ṹ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryElvtInfo
*
*   ����˵��: ��ѯ��������Ϣ���ġ�AFN=70H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ������������Ϣ
    PackBCD(dptr,elivator_info.rated_load,4,0);
    dptr += 2;
    rtn.length += 2;
    *dptr++ = elivator_info.people;
    rtn.length += 1;
    *dptr++ = elivator_info.midweight;
    rtn.length += 1;

    rtn.appzone.functioncode = AFN_QRY_ELVTINFO;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetElvtFloor
*
*   ����˵��: ����������¥����Ϣ���ġ�AFN=31H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetElvtFloor(pTX101 frame)
{
    uint8_t *sptr;
    uint16_t i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ������¥������
    sptr = (uint8_t *)rtn.appzone.userdata;

    // ����ϵ�����߶�ƫ��
    device_info.height_offset = (*sptr++) / 100.0f;

    // ¥������N
    floor_tbl.tblsize = *sptr++;

    // ����N�߶ȡ�����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamQryElvtFloor
*
*   ����˵��: ��ѯ������¥����Ϣ���ġ�AFN=71H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // ������¥������

    // ����ϵ�����߶�ƫ��
    *dptr++ = device_info.height_offset * 100.0f;
    rtn.length += 1;

    // ¥������N
    *dptr++ = floor_tbl.tblsize;
    rtn.length += 1;

    // ����N�߶ȡ�����
    for (i = 0; i < floor_tbl.tblsize; i++)
    {
        *dptr++ = floor_tbl.type[i].height * 10.0f;
        *dptr++ = floor_tbl.type[i].number;
        rtn.length += 2;
    }

    rtn.appzone.functioncode = AFN_QRY_ELVTFLOOR;
    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif

#ifdef DUSTMON
/*
*********************************************************************************************************
*   �� �� ��: ParamSetValveLmt
*
*   ����˵��: �����ﳾ���߼���ն˵�ŷ���ֵ��Ϣ���ġ�AFN=40H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetValveLmt(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryValveLmt
*
*   ����˵��: ��ѯ�ﳾ���߼���ն˵�ŷ���ֵ��Ϣ���ġ�AFN=80H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // 4����ŷ���ֵ

    for (i = 0; i < 4; i++)
    {
        PacksBCD(dptr, dustmon_info.threshold[i], 5, 1);  // float2BCD
        dptr += 3;
        rtn.length += 3;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamSetValveLmt_Ext
*
*   ����˵��: �����ﳾ���߼���ն˵�ŷ���չ��ֵ(�������PM10)��Ϣ���ġ�AFN=40H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetValveLmt_Ext(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryValveLmt_Ext
*
*   ����˵��: ��ѯ�ﳾ���߼���ն˵�ŷ���ֵ��Ϣ���ġ�AFN=80H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

    // 4����ŷ���ֵ

    for (i = 0; i < 4; i++)
    {
        PacksBCD(dptr, dustmon_info.threshold[i+4], 5, 1);  // float2BCD
        dptr += 3;
        rtn.length += 3;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: ParamSetValveMan
*
*   ����˵��: �����ﳾ���߼���ն˵�ŷ��ֶ�������Ϣ���ġ�AFN=41H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetValveMan(pTX101 frame)
{
    uint8_t *sptr,i,dat_i[4];

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamSetNotice
*
*   ����˵��: �����ﳾ����֪ͨ��Ϣ���ġ�AFN=44H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetNotice(pTX101 frame)
{
    uint8_t *sptr,*dptr,i,j;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif
#ifdef UPPLAT
/*
*********************************************************************************************************
*   �� �� ��: ParamSetValveLmt
*
*   ����˵��: ����ж��ƽ̨��ֵ��Ϣ���ġ�AFN=45H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 ParamSetUPLmt(pTX101 frame)
{
    uint8_t *sptr,i;

    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    // ����
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: ParamQryValveLmt
*
*   ����˵��: ��ѯ�ﳾ���߼���ն˵�ŷ���ֵ��Ϣ���ġ�AFN=80H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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

	// ����
    PacksBCD(dptr, limit_tbl.limit[SENS_UPWEIGHT].hilimit, 5, 1); // ����λ
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_UPWEIGHT].hiwarn,5,1);  // Ԥ��ֵ
    dptr += 3;
	// б����
    PacksBCD(dptr, limit_tbl.limit[SENS_CABL].hilimit, 5, 1); // ����λ
    dptr += 3;
    PacksBCD(dptr,limit_tbl.limit[SENS_CABL].hiwarn,5,1);  // Ԥ��ֵ
    dptr += 3;

	// Ԥ��
    // Ԥ��12B
    for (i=0;i<12;i++)
    {
        *dptr++ = 0;
    }
    rtn.length += 12;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    //
    return &rtn;
}
#endif 

/*
*********************************************************************************************************
*   �� �� ��: ParamSetFngrDat
*
*   ����˵��: ����֡,ָ�����ݱ�����ġ�AFN=28H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pMultiTX101 ParamSetFngrDat(pMF_TX101 frame)
{
    static uint16_t i,j;
    uint8_t *dptr,*sptr,fingernum,buf[32],rtn;
    static uint32_t opid1,opid2;    // ������ԱID


    // ʶ��ʽ����ָ��ţ�1B��ָ��������Ա��ʶ�루4B���Ѿ����浽�˵�0֡��ֱ���Ϸ�.
    if (mrtn.mf_flag && (mrtn.mframe_st == (uint16_t)(pow(2, mrtn.mframe_num) - 1)))
    {   // ��ʾ��ǰ��֡�����һ֡,���ݴ����mrtnbuffer��������,ת�����ն���Ӧ������
        // ����֡���Ѿ����꣬��ʼ������ݣ�ʶ��ʽ����ָ��ţ�1B��ָ��������Ա��ʶ�루4B�� ָ��������Ա�������ƣ�12B��+ָ��������
        dptr = mrtnbuffer;
        sptr = (uint8_t *)mrtn.frame[0].appzone.userdata;
        // ����ʶ��ʽ����ָ��ţ�1B��ָ��������Ա��ʶ�루4B�� ָ��������Ա�������ƣ�12B��
        for (i = 0; i < 17; i++)
        {
            *dptr++ = mrtnbuf[0][i];
        }
        // ƴ��ָ��������
        for (i = mrtn.mframe_num; i > 0; i--)
        {
            sptr = (uint8_t *)mrtn.frame[i-1].appzone.userdata + 17;    // ֻȡָ��������
            for (j = 0; j < mrtn.mlen[i-1]-17; j++)
            {
                *dptr++ = *sptr++;
            }
        }

        mrtn.mf_flag = 0; // ���ݴ����������֡��־

        // ����ָ��������ָ��ģ����
        // ����ָ��������Ա��ʶ�루4B�� ָ��������Ա�������ƣ�12B����ָ��ģ����±�

        // ���Ȳ����Ƿ���ƥ���ID�ţ�����У�����д�����ֱ���˳�
        for (i = 0; i < 16; i++)
        {
            if (FNGR_ReadNotepad(i, buf) != ACK_OK)
            {
                i = 0xff;
                break;
            }
            fingernum = mrtnbuffer[0] & 0x0f; // ��ȡ��ָ���1or2
            opid1 = *((uint32_t *)(mrtnbuffer+1));  // ��ȡ��ԱID��
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
                // ����ָ��������������ָ��ģ����
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
        // ���û���ҵ�ƥ�䣬�ڵ�һ���հ�ָ����д����ID����ָ��
        if (i == 16)
        {
            for (i = 0; i < 16; i++)
            {
                if (FNGR_ReadNotepad(i, buf) != ACK_OK)
                {
                    i = 0xff;
                    break;
                }
                opid1 = *((uint32_t *)(mrtnbuffer+1));  // ��ȡ��ԱID��
                opid2 = *((uint32_t *)buf);
                if (!opid2) // ��һ��������
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
                    // ����ָ��������������ָ��ģ����
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

    // ��֯Ӧ��֡
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.dir = 1;
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.div = 1;
    mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.fcb = 3;
    if (i == 0xff || i == 0x16)
    {
        mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.func = LFN_DIR1_FAIL;
    }
    else mrtn.frame[mrtn.mframe_cnt-1].ctrlzone.func = LFN_DIR1_OK;
    //mrtn.frame[mrtn.mframe_cnt-1].framecnt = mrtn.mframe_cnt; // �ѵ�ǰ�յ���֡�ŷ���
    //mrtn.frame[mrtn.mframe_cnt-1].addrzone = *GetTAddr();
    //mrtn.frame[mrtn.mframe_cnt-1].version = *GetProtocolVersion();
    //mrtn.frame[mrtn.mframe_cnt-1].appzone.functioncode = AFN_CHG_FINGER;

    mrtn.frame[mrtn.mframe_cnt-1].length = 12+5; // Ctrl1+FrameNum2+FrameCnt2+ADDR5+Version1+AFN1+AppDat5
    // ���¼��㵱ǰ֡��CRC
    GetCRC(mrtn.mframe_cnt);
    mrtn.frame[mrtn.mframe_cnt - 1].endbyte = ENDCHAR;

    return &mrtn.frame[mrtn.mframe_cnt-1];
}
/*
*********************************************************************************************************
*   �� �� ��: Param_Del_Finger
*
*   ����˵��: ����֡,ɾ��ָ�����ݱ��ġ�AFN=29H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/

pTX101 Param_Del_FngrDat(pTX101 frame)
{
    uint32_t fngrid,tmp;
    uint8_t buf[32];
    uint16_t i,ret=0;

    // ɾ��ָ��������Ա��ʶ���ָ����Ϣ
    fngrid = *((uint32_t *)rtn.appzone.userdata);
    if (!fngrid)    // ���ָ��IDΪ0�����ָ�ƿ�
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
    else // ���ָ��ID��Ϊ�㣬����ָ��ģ�鱣��ID
    {
        for (i = 0; i < 16; i++)
        {
            if (FNGR_ReadNotepad(i, buf) == ACK_OK)
            {
                tmp = *((uint32_t *)buf);
                if (tmp == fngrid) // �����ƥ��ָ�ƣ�ɾ����Ӧָ�Ʋ���ն�ӦID�ź�����
                {
                    if (FNGR_DeletChar(2 * i, 2) == ACK_OK)// ɾ����ӦID�ŵ�����ָ��
                    {
                        memset(buf, 0, 32);
                        if (FNGR_WriteNotepad(i, buf) != ACK_OK) ret = 1; // ��ҳ��һ���ֽڴ��дָ��
                    }
                }

            }
        }
    }

    // ��֯Ӧ��
    rtn = *frame;

    // û���ҵ���Ӧ��id����Ӧ����֡
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: Param_Restart
*
*   ����˵��: ����֡,�����ն˱��ġ�AFN=A0H
*
*   ��   ��: pTX101 frame ���� ���յ�������֡
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 Param_Restart(pTX101 frame)
{
    // ��֯Ӧ��֡
    rtn = *frame;

    link_layer_pack(&rtn,~frame->ctrlzone.dir,0,0,LFN_DIR1_OK);

    rtn.length = 8;
    rtn.appzone.functioncode = AFN_RESTART;

    // ���¼���CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
//************************************ �����ϴ����� ****************************************/
// ������ʱʵʱ���� ������

#ifdef NOVAR_TEST
void TowRTDatInit(void)
{
    uint32_t i;

    // ʱ��
    period_value.sec = RNG_Get_RandomRange(0,59);
    period_value.min = RNG_Get_RandomRange(0,59);
    period_value.hour = RNG_Get_RandomRange(0, 23);
    period_value.date = RNG_Get_RandomRange(1, 30);
    period_value.month = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    period_value.year = RNG_Get_RandomRange(0,15);

    // ������
    period_value.attrib = 0;

    // ������0.0-9999.9��
    period_value.weight_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.weight_alarm = gravwarn[RNG_Get_RandomRange(0,3)];
    period_value.weight_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // �߶ȣ�9999.9��-��9999.9��
    period_value.height_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.height_alarm = heigwarn[RNG_Get_RandomRange(0,6)];
    if (RNG_Get_RandomRange(0, 1)) period_value.height_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.height_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // ����0.0-9999.9��
    period_value.margin_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.margin_alarm = scopewarn[RNG_Get_RandomRange(0,6)];
    period_value.margin_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ��ת�Ƕȣ�9999.9��-��9999.9��
    period_value.rotat_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.rotat_alarm = rotarywarn[RNG_Get_RandomRange(0,6)];
    period_value.rotat_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    if (RNG_Get_RandomRange(0, 1)) period_value.rotat_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.rotat_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // ����0.0 - 9999.9 m/s
    period_value.wind_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.wind_alarm = windwarn[RNG_Get_RandomRange(0,2)];
    period_value.wind_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ���0.0-9999.9��
    period_value.tilt_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.tilt_alarm = dipanglewarn[RNG_Get_RandomRange(0,2)];
    period_value.tilt_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ���ر���0.0%��-��9999.9%
    period_value.torque_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.torque_alarm = momentwarn[RNG_Get_RandomRange(0,3)];
    period_value.torque_value = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ���ߣ�9999.9��-��9999.9��
    period_value.walk_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    period_value.walk_alarm = walkwarn[RNG_Get_RandomRange(0,2)];
    period_value.walk_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    if (RNG_Get_RandomRange(0, 1)) period_value.walk_value = RNG_Get_RandomRange(0, 99999) / 10.0;
    else period_value.walk_value = -RNG_Get_RandomRange(0, 99999) / 10.0;

    // ����4
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

    // ��ײ����
    period_value.collision_alarm[0] = RNG_Get_RandomRange(0, 255);  // �Է�����ID��0��ʾ�ޱ���
    period_value.collision_alarm[1] = RNG_Get_RandomRange(0, 2) |   // ����ײ
                                                       (0 << 2) |   // ��
                                 RNG_Get_RandomRange(0, 2) << 4 |   // ����ײ
                                 RNG_Get_RandomRange(0, 2) << 6;    // ����ײ
    period_value.collision_alarm[2] = RNG_Get_RandomRange(1, 2) |   // ��������
                                                       (0 << 2) |   // ��
                                 RNG_Get_RandomRange(0, 2) << 4 |   // ����ײ
                                 RNG_Get_RandomRange(0, 2) << 6;    // Զ��ײ

    // �ϰ������
    period_value.obstacle_alarm[0] = RNG_Get_RandomRange(0, 255);   // �Է�����ID��0��ʾ�ޱ���
    period_value.obstacle_alarm[1] = RNG_Get_RandomRange(0, 2) |    // ����ײ
                                                       (0 << 2) |   // ��
                                 RNG_Get_RandomRange(0, 2) << 4 |   // ����ײ
                                 RNG_Get_RandomRange(0, 2) << 6;    // ����ײ
    period_value.obstacle_alarm[2] = 0 |    // ��
                              (0 << 2) |    // ��
        RNG_Get_RandomRange(0, 2) << 4 |    // ����ײ
        RNG_Get_RandomRange(0, 2) << 6;     // Զ��ײ

    // ����������
    period_value.forbid_alarm[0] = RNG_Get_RandomRange(0, 255);     // �Է�����ID��0��ʾ�ޱ���
    period_value.forbid_alarm[1] = 0 |      // ��
                            (0 << 2) |      // ��
      RNG_Get_RandomRange(0, 2) << 4 |      // ����ײ
      RNG_Get_RandomRange(0, 2) << 6;       // ����ײ
    period_value.forbid_alarm[2] =   0 |    // ��
                              (0 << 2) |    // ��
        RNG_Get_RandomRange(0, 2) << 4 |    // ����ײ
        RNG_Get_RandomRange(0, 2) << 6;     // Զ��ײ

    // ����3BX4
    for (i = 0; i < 12;i++)
    {
        period_value.spare_flag[i] = 0;
    }

    // ����6B
    for (i = 0; i < 6;i++)
    {
        period_value.spare_other[i] = 0;
    }
}

void TowWklpDatInit(void)
{
    uint32_t i;

    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    workloop.sn = RNG_Get_RandomRange(0, 65535);

    // ��ʼʱ��
    workloop.sec_begin = RNG_Get_RandomRange(0,59);
    workloop.min_begin = RNG_Get_RandomRange(0,59);
    workloop.hour_begin = RNG_Get_RandomRange(0, 23);
    workloop.date_begin = RNG_Get_RandomRange(1, 30);
    workloop.month_begin = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    workloop.year_begin = RNG_Get_RandomRange(0,15);
    // ����ʱ��
    workloop.sec_end = (workloop.sec_begin + 1) % 60;
    workloop.min_end = (workloop.min_begin + 1) % 60;
    workloop.hour_end = workloop.hour_begin;
    workloop.date_end = workloop.date_begin;
    workloop.month_end = workloop.month_begin;
    workloop.year_end = workloop.year_begin;

    // ������أ�3���ֽڣ�0-9999.9��
    workloop.weight_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ������أ�3���ֽڣ�0-9999.9��*��
    workloop.torque_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ���߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.height_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ��С�߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.height_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // �����ȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.margin_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ��С���ȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.margin_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ����ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.rotat_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ��С��ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    workloop.rotat_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ������߾��룬3���ֽڣ�-9999.9 - 9999.9��
    workloop.walk_max = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ��С���߾��룬3���ֽڣ�-9999.9 - 9999.9��
    workloop.walk_min = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ���Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    workloop.rotat_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // �������ȣ�3���ֽڣ�0-9999.9��
    workloop.margin_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // �����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    workloop.height_begin = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ж����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    workloop.rotat_end = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ж������ȣ�3���ֽڣ�0-9999.9��
    workloop.margin_end = RNG_Get_RandomRange(0, 99999) / 10.0;

    // ж����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    workloop.height_end = RNG_Get_RandomRange(0, 99999) / 10.0;

}

// ��������
void TowWrnDatInit()
{

}
// �궨����
void TowCaliDatInit()
{
    uint32_t i;

    // ʱ��BCD��6bytes
    cali_dat.sec = RNG_Get_RandomRange(0,59);
    cali_dat.min = RNG_Get_RandomRange(0,59);
    cali_dat.hour = RNG_Get_RandomRange(0, 23);
    cali_dat.date = RNG_Get_RandomRange(1, 30);
    cali_dat.month = (RNG_Get_RandomRange(1,7)<<5)|RNG_Get_RandomRange(1,12);
    cali_dat.year = RNG_Get_RandomRange(0,15);

    // �����ʹ���
    cali_dat.attrib = 0;

    // ������ 3�ֽ�BCD�룬������ԭʼֵ��0-999999
    cali_dat.weight_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.weight_alarm = 0;
    cali_dat.weight = RNG_Get_RandomRange(0,999999);

    // �߶�  3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.height_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.height_alarm = 0;
    cali_dat.heigh = RNG_Get_RandomRange(0,999999);

    // ����  3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.margin_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.margin_alarm = 0;
    cali_dat.margin = RNG_Get_RandomRange(0,999999);

    // ��ת�Ƕ�   3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.rotat_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.rotat_alarm = 0;
    cali_dat.rotat = RNG_Get_RandomRange(0,999999);

    // ����     3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.wind_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.wind_alarm = 0;
    cali_dat.wind = RNG_Get_RandomRange(0,999999);

    // ���   3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.tilt_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.tilt_alarm = 0;
    cali_dat.tilt = RNG_Get_RandomRange(0,999999);

    // ����     3�ֽ�BCD�룬������ԭʼֵ��0-999999��
    cali_dat.walk_flag = RNG_Get_RandomRange(0, 1) | (RNG_Get_RandomRange(0, 1) << 1);
    cali_dat.walk_alarm = 0;
    cali_dat.walk = RNG_Get_RandomRange(0,999999);

    // Ԥ��4bytes   3�ֽ�BCD�룬������ԭʼֵ��0-999999��
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

    // ʱ��
    realtimedata.clock.sec = period_value.sec;
    realtimedata.clock.min = period_value.min;
    realtimedata.clock.hour = period_value.hour;
    realtimedata.clock.date = period_value.date;
    realtimedata.clock.week_single = (period_value.month >> 5) & 0x07;
    realtimedata.clock.mon_single = period_value.month & 0x1F;
    realtimedata.clock.year = period_value.year;

    // ������ ����
    realtimedata.packtype[0] = period_value.attrib&0xff;
    realtimedata.packtype[1] = (period_value.attrib>>8)&0xff;

    // ������0.0-9999.9��
    realtimedata.gravitylift.sensor_OK = period_value.weight_flag & 0x01;
    realtimedata.gravitylift.sensor_EN = (period_value.weight_flag & 0x02)>>1;
    realtimedata.gravitylift.rev1 = 0;
    realtimedata.gravitylift.warning = period_value.weight_alarm;
    PacksBCD((uint8_t *)&realtimedata.gravitylift.data, period_value.weight_value/1000,5,1);

    // �߶ȣ�9999.9��-��9999.9��
    realtimedata.height.sensor_OK = period_value.height_flag & 0x01;
    realtimedata.height.sensor_EN = (period_value.height_flag & 0x02)>>1;
    realtimedata.height.rev1 = 0;
    realtimedata.height.warning = period_value.height_alarm;
    PacksBCD((uint8_t *)&realtimedata.height.data, period_value.height_value,5,1);

    // ����0.0-9999.9��
    realtimedata.scope.sensor_OK = period_value.margin_flag & 0x01;
    realtimedata.scope.sensor_EN = (period_value.margin_flag & 0x02)>>1;
    realtimedata.scope.rev1 = 0;
    realtimedata.scope.warning = period_value.margin_alarm;
    PacksBCD((uint8_t *)&realtimedata.scope.data, period_value.margin_value,5,1);

    // ��ת�Ƕȣ�9999.9��-��9999.9��
    realtimedata.rotary.sensor_OK = period_value.rotat_flag & 0x01;
    realtimedata.rotary.sensor_EN = (period_value.rotat_flag & 0x02)>>1;
    realtimedata.rotary.rev1 = 0;
    realtimedata.rotary.warning = period_value.rotat_alarm;
    PacksBCD((uint8_t *)&realtimedata.rotary.data, period_value.rotat_value,5,1);

    // ����0.0 - 9999.9 m/s
    realtimedata.wind.sensor_OK = period_value.wind_flag & 0x01;
    realtimedata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    realtimedata.wind.rev1 = 0;
    realtimedata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&realtimedata.wind.data, period_value.wind_value,5,1);

    // ���0.0-9999.9��
    realtimedata.dipangle.sensor_OK = period_value.tilt_flag & 0x01;
    realtimedata.dipangle.sensor_EN = (period_value.tilt_flag & 0x02)>>1;
    realtimedata.dipangle.rev1 = 0;
    realtimedata.dipangle.warning = period_value.tilt_alarm;
    PacksBCD((uint8_t *)&realtimedata.dipangle.data, period_value.tilt_value,5,1);

    // ���ر���0.0%��-��9999.9%
    realtimedata.moment .sensor_OK = period_value.torque_flag & 0x01;
    realtimedata.moment .sensor_EN = (period_value.torque_flag & 0x02)>>1;
    realtimedata.moment .rev1 = 0;
    realtimedata.moment .warning = period_value.torque_alarm;
    PacksBCD((uint8_t *)&realtimedata.moment .data, period_value.torque_value,5,1);

    // ���ߣ�9999.9��-��9999.9��
    realtimedata.walk.sensor_OK = period_value.walk_flag & 0x01;
    realtimedata.walk.sensor_EN = (period_value.walk_flag & 0x02)>>1;
    realtimedata.walk.rev1 = 0;
    realtimedata.walk.warning = period_value.walk_alarm;
    PacksBCD((uint8_t *)&realtimedata.walk.data, period_value.walk_value,5,1);

    // ����4
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

    // ��ײ����
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

    // �ϰ������
    realtimedata.obstacle.ID = period_value.obstacle_alarm[0];
    realtimedata.obstacle.low = period_value.obstacle_alarm[1]&0x03;
    realtimedata.obstacle.rev1 = (period_value.obstacle_alarm[1]>>2)&0x03;
    realtimedata.obstacle.right = (period_value.obstacle_alarm[1]>>4)&0x03;
    realtimedata.obstacle.left = (period_value.obstacle_alarm[1]>>6)&0x03;
    realtimedata.obstacle.Inttype = period_value.obstacle_alarm[2]&0x03;
    realtimedata.obstacle.rev2 = (period_value.obstacle_alarm[2]>>2)&0x03;
    realtimedata.obstacle.near = (period_value.obstacle_alarm[2]>>4)&0x03;
    realtimedata.obstacle.far = (period_value.obstacle_alarm[2]>>6)&0x03;

    // ����������
    realtimedata.forbidden.ID = period_value.forbid_alarm[0];
    realtimedata.forbidden.low = period_value.forbid_alarm[1]&0x03;
    realtimedata.forbidden.rev1 = (period_value.forbid_alarm[1]>>2)&0x03;
    realtimedata.forbidden.right = (period_value.forbid_alarm[1]>>4)&0x03;
    realtimedata.forbidden.left = (period_value.forbid_alarm[1]>>6)&0x03;
    realtimedata.forbidden.Inttype = period_value.forbid_alarm[2]&0x03;
    realtimedata.forbidden.rev2 = (period_value.forbid_alarm[2]>>2)&0x03;
    realtimedata.forbidden.near = (period_value.forbid_alarm[2]>>4)&0x03;
    realtimedata.forbidden.far = (period_value.forbid_alarm[2]>>6)&0x03;

    // ����3BX4
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

    // ����6B
    for (i = 0; i < 6;i++)
    {
        realtimedata.rev3[i] = period_value.spare_other[i];
    }
}

// ��������ѭ������ ������
void GetTowWklpDat(void)
{

    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    TowWklpDat.workcyclenum[0] = workloop.sn&0xff;
    TowWklpDat.workcyclenum[1] = (workloop.sn>>8)&0xff;

    // ��ʼʱ��
    TowWklpDat.starttime.sec = workloop.sec_begin;
    TowWklpDat.starttime.min = workloop.min_begin;
    TowWklpDat.starttime.hour = workloop.hour_begin;
    TowWklpDat.starttime.date = workloop.date_begin;
    TowWklpDat.starttime.week_single = (workloop.month_begin >> 5) & 0x07;
    TowWklpDat.starttime.mon_single = workloop.month_begin & 0x1F;
    TowWklpDat.starttime.year = workloop.year_begin;

    // ����ʱ��

    TowWklpDat.endtime.sec = workloop.sec_end;
    TowWklpDat.endtime.min = workloop.min_end;
    TowWklpDat.endtime.hour = workloop.hour_end;
    TowWklpDat.endtime.date = workloop.date_end;
    TowWklpDat.endtime.week_single = (workloop.month_begin >> 5) & 0x07;
    TowWklpDat.endtime.mon_single = workloop.month_begin & 0x1F;
    TowWklpDat.endtime.year = workloop.year_end;

    // ������أ�3���ֽڣ�0-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.maxlift, workloop.weight_max/1000,5,1);

    // ������أ�3���ֽڣ�0-9999.9��*��
    PacksBCD((uint8_t *)&TowWklpDat.maxmoment, workloop.torque_max,5,1);

    // ���߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.maxheight, workloop.height_max,5,1);

    // ��С�߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.minheight, workloop.height_min,5,1);

    // �����ȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.maxscope, workloop.margin_max,5,1);

    // ��С���ȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.minscope, workloop.margin_min,5,1);

    // ����ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.maxrotary, workloop.rotat_max,5,1);

    // ��С��ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.minrotary, workloop.rotat_min,5,1);

    // ������߾��룬3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.maxwalk, workloop.walk_max,5,1);

    // ��С���߾��룬3���ֽڣ�-9999.9 - 9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.minwalk, workloop.walk_min,5,1);

    // �����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.liftpointangle, workloop.rotat_begin,5,1);

    // �������ȣ�3���ֽڣ�0-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.liftpointscope, workloop.margin_begin,5,1);

    // �����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.liftpointheight, workloop.height_begin,5,1);

    // ж����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointangle, workloop.rotat_end,5,1);

    // ж������ȣ�3���ֽڣ�0-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointscope, workloop.margin_end,5,1);

    // ж����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    PacksBCD((uint8_t *)&TowWklpDat.unloadpointheight, workloop.height_end,5,1);
}

// ��������ʵʱ���� ������
void GetTowWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;
    // ʱ��
    // ������δ���屨����Ϣ�ṹ�����Դ����������ж�ȡʱ�ӣ����ṹ���Ժ��滻
    warningdata.clock.sec = alarm_dat[num].sec;
    warningdata.clock.min = alarm_dat[num].min;
    warningdata.clock.hour = alarm_dat[num].hour;
    warningdata.clock.date = alarm_dat[num].date;
    warningdata.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    warningdata.clock.mon_single = alarm_dat[num].month & 0x1F;
    warningdata.clock.year = alarm_dat[num].year;

    // ������
    warningdata.packtype[0] = alarm_dat[num].attrib&0xff;
    warningdata.packtype[1] = (alarm_dat[num].attrib>>8)&0xff;;

    // ������
    warningdata.gravitylift.sensor_OK = alarm_dat[num].weight_flag & 0x01;
    warningdata.gravitylift.sensor_EN = (alarm_dat[num].weight_flag & 0x02)>>1;
    warningdata.gravitylift.rev1 = 0;
    warningdata.gravitylift.warning = alarm_dat[num].weight_alarm;
    PacksBCD((uint8_t *)&warningdata.gravitylift.data,alarm_dat[num].weight_value/1000,5,1);

    // �߶�
    warningdata.height.sensor_OK = alarm_dat[num].height_flag & 0x01;
    warningdata.height.sensor_EN = (alarm_dat[num].height_flag & 0x02)>>1;
    warningdata.height.rev1 = 0;
    warningdata.height.warning = alarm_dat[num].height_alarm;
    PacksBCD((uint8_t *)&warningdata.height.data, alarm_dat[num].height_value,5,1);

    // ����
    warningdata.scope.sensor_OK = alarm_dat[num].margin_flag & 0x01;
    warningdata.scope.sensor_EN = (alarm_dat[num].margin_flag & 0x02)>>1;
    warningdata.scope.rev1 = 0;
    warningdata.scope.warning = alarm_dat[num].margin_alarm;
    PacksBCD((uint8_t *)&warningdata.scope.data, alarm_dat[num].margin_value,5,1);

    // ��ת�Ƕ�
    warningdata.rotary.sensor_OK = alarm_dat[num].rotat_flag & 0x01;
    warningdata.rotary.sensor_EN = (alarm_dat[num].rotat_flag>>1)&0x01;
    warningdata.rotary.rev1 = 0;
    warningdata.rotary.warning = alarm_dat[num].rotat_alarm;
    PacksBCD((uint8_t *)&warningdata.rotary.data, alarm_dat[num].rotat_value,5,1);

    // ����
    warningdata.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    warningdata.wind.sensor_EN = (alarm_dat[num].wind_flag>>1)&0x01;
    warningdata.wind.rev1 = 0;
    warningdata.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&warningdata.wind.data, alarm_dat[num].wind_value,5,1);

    // ���
    warningdata.dipangle.sensor_OK = alarm_dat[num].tilt_flag & 0x01;
    warningdata.dipangle.sensor_EN = (alarm_dat[num].tilt_flag >> 1) & 0x01;
    warningdata.dipangle.rev1 = 0;
    warningdata.dipangle.warning = alarm_dat[num].tilt_alarm;
    PacksBCD((uint8_t *)&warningdata.dipangle.data, alarm_dat[num].tilt_value,5,1);

    // ���ر���
    warningdata.moment.sensor_OK = alarm_dat[num].torque_flag & 0x01;
    warningdata.moment.sensor_EN = (alarm_dat[num].torque_flag >> 1) & 0x01;
    warningdata.moment.rev1 = 0;
    warningdata.moment.warning = alarm_dat[num].torque_alarm;
    PacksBCD((uint8_t *)&warningdata.moment.data, alarm_dat[num].torque_value,5,1);

    // ����
    warningdata.walk.sensor_OK = alarm_dat[num].walk_flag & 0x01;
    warningdata.walk.sensor_EN = (alarm_dat[num].walk_flag >> 1) & 0x01;
    warningdata.walk.rev1 = 0;
    warningdata.walk.warning = alarm_dat[num].walk_alarm;
    PacksBCD((uint8_t *)&warningdata.walk.data, alarm_dat[num].walk_value,5,1);

    // ״̬��
    warningdata.status = alarm_dat[num].alarm_stat;

    // ����״̬����ӱ���/Υ��/������Ϣ
    if (warningdata.status&0x01)
    {
        // ��ӱ�����Ϣ 1��
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
                // δ�������״̬
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
        // ���Υ����Ϣ 1��
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
                // δ�������״̬
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
        // ��ӹ�����Ϣ 1��
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


// �����궨ʵʱ���� ������
void GetTowCaliDat(void)
{
    // ʱ��
    TowCaliDat.clock.sec = cali_dat.sec;
    TowCaliDat.clock.min = cali_dat.min;
    TowCaliDat.clock.hour = cali_dat.hour;
    TowCaliDat.clock.date = cali_dat.date;
    TowCaliDat.clock.week_single = (cali_dat.month >> 5) & 0x07;
    TowCaliDat.clock.mon_single = cali_dat.month & 0x1F;
    TowCaliDat.clock.year = cali_dat.year;

    // ������ ����
    TowCaliDat.packtype[0] = period_value.attrib&0xff;
    TowCaliDat.packtype[1] = (period_value.attrib>>8)&0xff;

    // ������0.0-9999.9��
    TowCaliDat.gravitylift.sensor_OK = cali_dat.weight_flag & 0x01;
    TowCaliDat.gravitylift.sensor_EN = (cali_dat.weight_flag & 0x02)>>1;
    TowCaliDat.gravitylift.rev1 = (cali_dat.weight_flag>>2) & 0x3F;
    TowCaliDat.gravitylift.warning = cali_dat.weight_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.gravitylift.data, cali_dat.weight/1000.0,6,0);

    // �߶�
    TowCaliDat.height.sensor_OK = cali_dat.height_flag & 0x01;
    TowCaliDat.height.sensor_EN = (cali_dat.height_flag & 0x02)>>1;
    TowCaliDat.height.rev1 = (cali_dat.height_flag>>2) & 0x3F;
    TowCaliDat.height.warning = cali_dat.height_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.height.data, cali_dat.heigh,6,0);

    // ����
    TowCaliDat.scope.sensor_OK = cali_dat.margin_flag & 0x01;
    TowCaliDat.scope.sensor_EN = (cali_dat.margin_flag & 0x02)>>1;
    TowCaliDat.scope.rev1 = (cali_dat.margin_flag>>2) & 0x3F;
    TowCaliDat.scope.warning = cali_dat.margin_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.scope.data, cali_dat.margin,6,0);

    // ��ת�Ƕ�
    TowCaliDat.rotary.sensor_OK = cali_dat.rotat_flag & 0x01;
    TowCaliDat.rotary.sensor_EN = (cali_dat.rotat_flag & 0x02)>>1;
    TowCaliDat.rotary.rev1 = (cali_dat.rotat_flag>>2) & 0x3F;
    TowCaliDat.rotary.warning = cali_dat.rotat_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.rotary.data, cali_dat.rotat,6,0);

    // ����
    TowCaliDat.wind.sensor_OK = cali_dat.wind_flag & 0x01;
    TowCaliDat.wind.sensor_EN = (cali_dat.wind_flag & 0x02)>>1;
    TowCaliDat.wind.rev1 = (cali_dat.wind_flag>>2) & 0x3F;
    TowCaliDat.wind.warning = cali_dat.wind_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.wind.data, cali_dat.wind,6,0);

    // ���
    TowCaliDat.dipangle.sensor_OK = cali_dat.tilt_flag & 0x01;
    TowCaliDat.dipangle.sensor_EN = (cali_dat.tilt_flag & 0x02)>>1;
    TowCaliDat.dipangle.rev1 = (cali_dat.tilt_flag>>2) & 0x3F;
    TowCaliDat.dipangle.warning = cali_dat.tilt_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.dipangle.data, cali_dat.tilt,6,0);

    // ����
    TowCaliDat.walk.sensor_OK = cali_dat.walk_flag & 0x01;
    TowCaliDat.walk.sensor_EN = (cali_dat.walk_flag & 0x02)>>1;
    TowCaliDat.walk.rev1 = (cali_dat.walk_flag>>2) & 0x3F;
    TowCaliDat.walk.warning = cali_dat.walk_alarm;
    PacksBCD((uint8_t *)&TowCaliDat.walk.data, cali_dat.walk,6,0);

    // Ԥ��5BX4
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
*   �� �� ��: AUFTowRTDat
*
*   ����˵��: �Զ��ϴ����ģ���ʱʵʱ���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=90H
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&realtimedata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // �����ʹ���       2B
    sptr = realtimedata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // ������     5B
    sptr = (uint8_t *)&realtimedata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&realtimedata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&realtimedata.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ��ת�Ƕ�   5B
    sptr = (uint8_t *)&realtimedata.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&realtimedata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&realtimedata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���ر���   5B
    sptr = (uint8_t *)&realtimedata.moment;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&realtimedata.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // Ԥ��       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&realtimedata.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }
    //��ײ���� 27B
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
    // �ϰ��� 3B
    sptr = (uint8_t *)&realtimedata.obstacle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    //������3B
    sptr = (uint8_t *)&realtimedata.forbidden;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // Ԥ��3BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&realtimedata.rev2[j];
        for (i = 0; i < 3;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 3;
    }

    // Ԥ��6B
    sptr = realtimedata.rev3;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFTowWklpDat
*
*   ����˵��: �Զ��ϴ����Ĺ���ѭ�����ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=91H
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;
    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    sptr = TowWklpDat.workcyclenum;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // ��ʼʱ��6B
    sptr = (uint8_t *)&TowWklpDat.starttime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // ����ʱ��
    sptr = (uint8_t *)&TowWklpDat.endtime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // ������أ�3���ֽڣ�0-9999.9��
    sptr = (uint8_t *)&TowWklpDat.maxlift;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ������أ�3���ֽڣ�0-9999.9��*��
    sptr = (uint8_t *)&TowWklpDat.maxmoment;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ���߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.maxheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ��С�߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.minheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // �����ȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.maxscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ��С���ȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.minscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ����ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.maxrotary;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ��С��ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.minrotary;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ������߾��룬3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.maxwalk;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ��С���߾��룬3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&TowWklpDat.minwalk;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // �����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    sptr = (uint8_t *)&TowWklpDat.liftpointangle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // �������ȣ�3���ֽڣ�0-9999.9��
    sptr = (uint8_t *)&TowWklpDat.liftpointscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // �����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    sptr = (uint8_t *)&TowWklpDat.liftpointheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ж����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    sptr = (uint8_t *)&TowWklpDat.unloadpointangle;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ж������ȣ�3���ֽڣ�0-9999.9��
    sptr = (uint8_t *)&TowWklpDat.unloadpointscope;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ж����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    sptr = (uint8_t *)&TowWklpDat.unloadpointheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFTowWrnDat
*
*   ����˵��: �Զ��ϴ����Ĺ���ѭ�����ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=92H
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 AUFTowWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//��λ��Ϣ��Ŀ
    uint8_t vartype;//��λ��Ϣ����

    #ifdef NOVAR_TEST
    TowWrnDatInit();
    #endif
    //GetTowWrnDat(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_WARNING;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&warningdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // �����ʹ���       2B
    sptr = warningdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // ������     5B
    sptr = (uint8_t *)&warningdata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&warningdata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&warningdata.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ��ת�Ƕ�   5B
    sptr = (uint8_t *)&warningdata.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&warningdata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&warningdata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���ر���   5B
    sptr = (uint8_t *)&warningdata.moment;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&warningdata.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // Ԥ��       5BX4
    for (j = 0; j < 4;j++)
    {
        sptr = (uint8_t *)&warningdata.rev1[j];
        for (i = 0; i < 5;i++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 5;
    }
    // ��λ��Ϣ״̬��
    *dptr++ = warningdata.status;
    rtn.length += 1;

    // �ж�״̬��ѯ�Ƿ���ڱ�λ��Ϣ
    // ���ڱ���
    if (warningdata.status&0x01)
    {
        // ��ӱ�����Ϣ
        sptr = warningdata.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// ��λ����3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����
            case 0x05:  // ����������
            case 0x06:  // ���ر���
            case 0x07:  // ���ٱ���
            case 0x08: // ��б����
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ����Υ��
    if ((warningdata.status>>1)&0x01)
    {
        // ���Υ����Ϣ
        sptr = warningdata.illegal;
        varnum = *sptr++;   // Υ����Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����1Byte
            case 0x04:  // ��λ����1Byte
            case 0x05:  // ����������1Byte
            case 0x06:  // ���ر���1Byte
            case 0x07:  // ���ٱ���1Byte
            case 0x08: // ��б����1Byte
            case 0x09:  //�����֤1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ���ڹ���
    if ((warningdata.status>>2)&0x01)
    {
        // ��ӹ�����Ϣ
        sptr = warningdata.fault;
        varnum = *sptr++;   // ������Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //�������ͱ���
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // Ԥ��6B
    sptr = warningdata.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFTowCaliDat
*
*   ����˵��: �Զ��ϴ����ģ���ʱʵʱ���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=98H
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&TowCaliDat.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // �����ʹ���       2B
    sptr = TowCaliDat.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // ������     5B
    sptr = (uint8_t *)&TowCaliDat.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&TowCaliDat.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&TowCaliDat.scope;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ��ת�Ƕ�   5B
    sptr = (uint8_t *)&TowCaliDat.rotary;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&TowCaliDat.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&TowCaliDat.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&TowCaliDat.walk;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // Ԥ��       5BX4
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
#endif

#ifdef ELIVATOR
void GetElvtRTData(void)
{
    uint32_t i;

    // ʱ��
    elvtdata.clock.sec = period_value.sec; //period_value.sec;
    elvtdata.clock.min = period_value.min;
    elvtdata.clock.hour = period_value.hour;
    elvtdata.clock.date = period_value.date;
    elvtdata.clock.week_single = (period_value.month >> 5) & 0x07;
    elvtdata.clock.mon_single = period_value.month & 0x1F;
    elvtdata.clock.year = period_value.year;

    // ��Ա��ʶ��
    elvtdata.name_id = period_value.name_id;

    // ������ ����
    elvtdata.packtype[0] = period_value.attrib&0xff;
    elvtdata.packtype[1] = (period_value.attrib>>8)&0xff;

    // ������0.0-9999.9��
    elvtdata.gravitylift.sensor_OK = period_value.weight_flag & 0x01;
    elvtdata.gravitylift.sensor_EN = (period_value.weight_flag & 0x02)>>1;
    elvtdata.gravitylift.rev1 = 0;
    elvtdata.gravitylift.warning = period_value.weight_alarm;
    PacksBCD((uint8_t *)&elvtdata.gravitylift.data, period_value.weight_value/1000,5,1);

    // �߶ȣ�9999.9��-��9999.9��
    elvtdata.height.sensor_OK = period_value.height_flag & 0x01;
    elvtdata.height.sensor_EN = (period_value.height_flag & 0x02)>>1;
    elvtdata.height.rev1 = 0;
    elvtdata.height.warning = period_value.height_alarm;
    PacksBCD((uint8_t *)&elvtdata.height.data, period_value.height_value,5,1);

    // �ٶ�0.0-9999.9��/��
    elvtdata.speed.sensor_OK = period_value.speed_flag & 0x01;
    elvtdata.speed.sensor_EN = (period_value.speed_flag & 0x02)>>1;
    elvtdata.speed.rev1 = 0;
    elvtdata.speed.warning = period_value.speed_alarm;
    PacksBCD((uint8_t *)&elvtdata.speed.data, period_value.speed_value,5,1);

    // ����0.0 - 9999.9 m/s
    elvtdata.wind.sensor_OK = period_value.wind_flag & 0x01;
    elvtdata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    elvtdata.wind.rev1 = 0;
    elvtdata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&elvtdata.wind.data, period_value.wind_value,5,1);

    // ���0.0-9999.9��
    elvtdata.dipangle.sensor_OK = period_value.tilt_flag & 0x01;
    elvtdata.dipangle.sensor_EN = (period_value.tilt_flag & 0x02)>>1;
    elvtdata.dipangle.rev1 = 0;
    elvtdata.dipangle.warning = period_value.tilt_alarm;
    PacksBCD((uint8_t *)&elvtdata.dipangle.data, period_value.tilt_value,5,1);

    // ���һ
    elvtdata.motor[0].sensor_OK = period_value.motor1_flag & 0x01;
    elvtdata.motor[0].sensor_EN = (period_value.motor1_flag & 0x02)>>1;
    elvtdata.motor[0].rev1 = 0;
    elvtdata.motor[0].warning = period_value.motor1_alarm;

    // �����
    elvtdata.motor[1].sensor_OK = period_value.motor2_flag & 0x01;
    elvtdata.motor[1].sensor_EN = (period_value.motor2_flag & 0x02)>>1;
    elvtdata.motor[1].rev1 = 0;
    elvtdata.motor[1].warning = period_value.motor2_alarm;

    // �����
    elvtdata.motor[2].sensor_OK = period_value.motor3_flag & 0x01;
    elvtdata.motor[2].sensor_EN = (period_value.motor3_flag & 0x02)>>1;
    elvtdata.motor[2].rev1 = 0;
    elvtdata.motor[2].warning = period_value.motor3_alarm;

    // ����
    elvtdata.people_flag = period_value.people_flag;
    elvtdata.people_alarm = period_value.people_alarm;
    elvtdata.people_value = period_value.people_value;

    // ¥��
    elvtdata.floor_flag = period_value.floor_flag;
    elvtdata.floor_aligned = period_value.floor_aligned;
    elvtdata.floor_value = period_value.floor_value;

    elvtdata.door_limit = period_value.door_limit;
    // ����19B
    for (i = 0; i < 19;i++)
    {
        elvtdata.rev[i] = 0;
    }


    // ����6B
    for (i = 0; i < 6;i++)
    {
        elvtdata.rev1[i] = 0;
    }
}
void GetElvtWklpDat(void)
{
    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    ElvtWklpDat.workcyclenum[0] = workloop.sn&0xff;
    ElvtWklpDat.workcyclenum[1] = (workloop.sn>>8)&0xff;

    // ��Ա��ʶ�루4B��
    ElvtWklpDat.name_id = workloop.name_id;

    // ��ʼʱ�䣨6B��
    ElvtWklpDat.starttime.sec = workloop.sec_begin;
    ElvtWklpDat.starttime.min = workloop.min_begin;
    ElvtWklpDat.starttime.hour = workloop.hour_begin;
    ElvtWklpDat.starttime.date = workloop.date_begin;
    ElvtWklpDat.starttime.week_single = (workloop.month_begin >> 5) & 0x07;
    ElvtWklpDat.starttime.mon_single = workloop.month_begin & 0x1F;
    ElvtWklpDat.starttime.year = workloop.year_begin;

    // ����ʱ�䣨6B��
    ElvtWklpDat.endtime.sec = workloop.sec_end;
    ElvtWklpDat.endtime.min = workloop.min_end;
    ElvtWklpDat.endtime.hour = workloop.hour_end;
    ElvtWklpDat.endtime.date = workloop.date_end;
    ElvtWklpDat.endtime.week_single = (workloop.month_begin >> 5) & 0x07;
    ElvtWklpDat.endtime.mon_single = workloop.month_begin & 0x1F;
    ElvtWklpDat.endtime.year = workloop.year_end;

    // ���������3���ֽڣ�0-9999.9��
    PacksBCD((uint8_t *)&ElvtWklpDat.maxlift, workloop.weight_max/1000,5,1);

    // ���������1���ֽڣ�0-255 ��
    ElvtWklpDat.maxpeople = (uint8_t)workloop.people_max;

    // ���߶ȣ�3���ֽڣ�0-9999.9��
    PacksBCD((uint8_t *)&ElvtWklpDat.maxheight, workloop.height_max,5,1);

    // ���¥�㣬1���ֽڣ�0-255 ��
    ElvtWklpDat.maxfloor = (uint8_t)workloop.floor_max;
}

void GetElvtWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // ʱ��
    ElvtWrnData.clock.sec = alarm_dat[num].sec;
    ElvtWrnData.clock.min = alarm_dat[num].min;
    ElvtWrnData.clock.hour = alarm_dat[num].hour;
    ElvtWrnData.clock.date = alarm_dat[num].date;
    ElvtWrnData.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    ElvtWrnData.clock.mon_single = alarm_dat[num].month & 0x1F;
    ElvtWrnData.clock.year = alarm_dat[num].year;

    // ��Ա��ʶ��
    ElvtWrnData.name_id = alarm_dat[num].name_id;

    // ������ ����
    ElvtWrnData.packtype[0] = alarm_dat[num].attrib&0xff;
    ElvtWrnData.packtype[1] = (alarm_dat[num].attrib>>8)&0xff;

    // ������0.0-9999.9��
    ElvtWrnData.gravitylift.sensor_OK = alarm_dat[num].weight_flag & 0x01;
    ElvtWrnData.gravitylift.sensor_EN = (alarm_dat[num].weight_flag & 0x02)>>1;
    ElvtWrnData.gravitylift.rev1 = 0;
    ElvtWrnData.gravitylift.warning = alarm_dat[num].weight_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.gravitylift.data, alarm_dat[num].weight_value/1000,5,1);

    // �߶ȣ�9999.9��-��9999.9��
    ElvtWrnData.height.sensor_OK = alarm_dat[num].height_flag & 0x01;
    ElvtWrnData.height.sensor_EN = (alarm_dat[num].height_flag & 0x02)>>1;
    ElvtWrnData.height.rev1 = 0;
    ElvtWrnData.height.warning = alarm_dat[num].height_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.height.data, alarm_dat[num].height_value,5,1);

    // �ٶ�0.0-9999.9��/��
    ElvtWrnData.speed.sensor_OK = alarm_dat[num].speed_flag & 0x01;
    ElvtWrnData.speed.sensor_EN = (alarm_dat[num].speed_flag & 0x02)>>1;
    ElvtWrnData.speed.rev1 = 0;
    ElvtWrnData.speed.warning = alarm_dat[num].speed_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.speed.data, alarm_dat[num].speed_value,5,1);

    // ����0.0 - 9999.9 m/s
    ElvtWrnData.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    ElvtWrnData.wind.sensor_EN = (alarm_dat[num].wind_flag & 0x02)>>1;
    ElvtWrnData.wind.rev1 = 0;
    ElvtWrnData.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.wind.data, alarm_dat[num].wind_value,5,1);

    // ���0.0-9999.9��
    ElvtWrnData.dipangle.sensor_OK = alarm_dat[num].tilt_flag & 0x01;
    ElvtWrnData.dipangle.sensor_EN = (alarm_dat[num].tilt_flag & 0x02)>>1;
    ElvtWrnData.dipangle.rev1 = 0;
    ElvtWrnData.dipangle.warning = alarm_dat[num].tilt_alarm;
    PacksBCD((uint8_t *)&ElvtWrnData.dipangle.data, alarm_dat[num].tilt_value,5,1);

    // ���һ
    ElvtWrnData.motor[0].sensor_OK = alarm_dat[num].motor1_flag & 0x01;
    ElvtWrnData.motor[0].sensor_EN = (alarm_dat[num].motor1_flag & 0x02)>>1;
    ElvtWrnData.motor[0].rev1 = 0;
    ElvtWrnData.motor[0].warning = alarm_dat[num].motor1_alarm;

    // �����
    ElvtWrnData.motor[1].sensor_OK = alarm_dat[num].motor2_flag & 0x01;
    ElvtWrnData.motor[1].sensor_EN = (alarm_dat[num].motor2_flag & 0x02)>>1;
    ElvtWrnData.motor[1].rev1 = 0;
    ElvtWrnData.motor[1].warning = alarm_dat[num].motor2_alarm;

    // �����
    ElvtWrnData.motor[2].sensor_OK = alarm_dat[num].motor3_flag & 0x01;
    ElvtWrnData.motor[2].sensor_EN = (alarm_dat[num].motor3_flag & 0x02)>>1;
    ElvtWrnData.motor[2].rev1 = 0;
    ElvtWrnData.motor[2].warning = alarm_dat[num].motor3_alarm;

    // ����
    ElvtWrnData.people_flag = alarm_dat[num].people_flag;
    ElvtWrnData.people_alarm = alarm_dat[num].people_alarm;
    ElvtWrnData.people_value = alarm_dat[num].people_value;

    // ¥��
    ElvtWrnData.floor_flag = alarm_dat[num].floor_flag;
    ElvtWrnData.floor_aligned = alarm_dat[num].floor_aligned;
    ElvtWrnData.floor_value = alarm_dat[num].floor_value;

    // �ź���λ״̬
    ElvtWrnData.door_limit = alarm_dat[num].door_limit;

    // ״̬��
    ElvtWrnData.status = alarm_dat[num].alarm_stat;

    // ����״̬����ӱ���/Υ��/������Ϣ
    if (ElvtWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ 1��
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
                // δ�������״̬
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
        // ���Υ����Ϣ 1��
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
                // δ�������״̬
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
        // ��ӹ�����Ϣ 1��
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
    // ʱ��
    ElvtCaliDat.clock.sec = cali_dat.sec;
    ElvtCaliDat.clock.min = cali_dat.min;
    ElvtCaliDat.clock.hour = cali_dat.hour;
    ElvtCaliDat.clock.date = cali_dat.date;
    ElvtCaliDat.clock.week_single = (cali_dat.month >> 5) & 0x07;
    ElvtCaliDat.clock.mon_single = cali_dat.month & 0x1F;
    ElvtCaliDat.clock.year = cali_dat.year;

    // ������ ����
    ElvtCaliDat.packtype[0] = period_value.attrib&0xff;
    ElvtCaliDat.packtype[1] = (period_value.attrib>>8)&0xff;

    // ������0.0-9999.9��
    ElvtCaliDat.gravitylift.sensor_OK = cali_dat.weight_flag & 0x01;
    ElvtCaliDat.gravitylift.sensor_EN = (cali_dat.weight_flag & 0x02)>>1;
    ElvtCaliDat.gravitylift.rev1 = (cali_dat.weight_flag>>2) & 0x3F;
    ElvtCaliDat.gravitylift.warning = cali_dat.weight_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.gravitylift.data, cali_dat.weight/1000.0,6,0);

    // �߶�
    ElvtCaliDat.height.sensor_OK = cali_dat.height_flag & 0x01;
    ElvtCaliDat.height.sensor_EN = (cali_dat.height_flag & 0x02)>>1;
    ElvtCaliDat.height.rev1 = (cali_dat.height_flag>>2) & 0x3F;
    ElvtCaliDat.height.warning = cali_dat.height_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.height.data, cali_dat.heigh,6,0);

    // ����
    ElvtCaliDat.wind.sensor_OK = cali_dat.wind_flag & 0x01;
    ElvtCaliDat.wind.sensor_EN = (cali_dat.wind_flag & 0x02)>>1;
    ElvtCaliDat.wind.rev1 = (cali_dat.wind_flag>>2) & 0x3F;
    ElvtCaliDat.wind.warning = cali_dat.wind_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.wind.data, cali_dat.wind,6,0);

    // ���
    ElvtCaliDat.dipangle.sensor_OK = cali_dat.tilt_flag & 0x01;
    ElvtCaliDat.dipangle.sensor_EN = (cali_dat.tilt_flag & 0x02)>>1;
    ElvtCaliDat.dipangle.rev1 = (cali_dat.tilt_flag>>2) & 0x3F;
    ElvtCaliDat.dipangle.warning = cali_dat.tilt_alarm;
    PacksBCD((uint8_t *)&ElvtCaliDat.dipangle.data, cali_dat.tilt,6,0);

    // Ԥ��5BX4
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
*   �� �� ��: AUFElvtRTDat
*
*   ����˵��: �Զ��ϴ����ģ���������ʱʵʱ���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9AH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&elvtdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // ��ԱID
    sptr = (uint8_t *)&elvtdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // �����ʹ���       2B
    sptr = elvtdata.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // ������     5B
    sptr = (uint8_t *)&elvtdata.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&elvtdata.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �ٶ�       5B
    sptr = (uint8_t *)&elvtdata.speed;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&elvtdata.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&elvtdata.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���һ��������   2B X 3
    for (i = 0; i < 3; i++)
    {
        sptr = (uint8_t *)&elvtdata.motor[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // ����
    *dptr++ = elvtdata.people_flag;
    *dptr++ = elvtdata.people_alarm;
    *dptr++ = elvtdata.people_value;
    rtn.length += 3;

    // ¥��
    *dptr++ = elvtdata.floor_flag;
    *dptr++ = elvtdata.floor_aligned;
    *dptr++ = elvtdata.floor_value;
    rtn.length += 3;

    // �ż���λ״̬
    *dptr++ = elvtdata.door_limit;
    rtn.length += 1;

    // Ԥ��19B
    for (i = 0; i < 19; i++)
    {
        *dptr++ = elvtdata.rev[i];
        rtn.length += 1;
    }

    // Ԥ��6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = elvtdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFElvtWklpDat
*
*   ����˵��: �Զ��ϴ����Ĺ���ѭ�����ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9BH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;
    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    sptr = ElvtWklpDat.workcyclenum;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // ��Ա��ʶ��
    sptr = (uint8_t *)&ElvtWklpDat.name_id;
    for (i = 0; i < 4; i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // ��ʼʱ��6B
    sptr = (uint8_t *)&ElvtWklpDat.starttime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // ����ʱ��
    sptr = (uint8_t *)&ElvtWklpDat.endtime;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // ���������3���ֽڣ�0-9999.9��
    sptr = (uint8_t *)&ElvtWklpDat.maxlift;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ���������1���ֽڣ�0~255
    *dptr++ = ElvtWklpDat.maxpeople;
    rtn.length += 1;

    // ���߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    sptr = (uint8_t *)&ElvtWklpDat.maxheight;
    for (i = 0; i < 3;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 3;

    // ���¥�㣬1���ֽڣ�0~255
    *dptr++ = ElvtWklpDat.maxfloor;
    rtn.length += 1;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}
/*
*********************************************************************************************************
*   �� �� ��: AUFElvtWrnDat
*
*   ����˵��: �Զ��ϴ����Ĺ���ѭ�����ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9CH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 AUFElvtWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//��λ��Ϣ��Ŀ
    uint8_t vartype;//��λ��Ϣ����

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_ELVTWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&ElvtWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;


    // ��ԱID
    sptr = (uint8_t *)&ElvtWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    // �����ʹ���       2B
    sptr = ElvtWrnData.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;
    // ������     5B
    sptr = (uint8_t *)&ElvtWrnData.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&ElvtWrnData.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �ٶ�       5B
    sptr = (uint8_t *)&ElvtWrnData.speed;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&ElvtWrnData.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&ElvtWrnData.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���һ��������   2B X 3
    for (i = 0; i < 3; i++)
    {
        sptr = (uint8_t *)&ElvtWrnData.motor[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // ����
    *dptr++ = ElvtWrnData.people_flag;
    *dptr++ = ElvtWrnData.people_alarm;
    *dptr++ = ElvtWrnData.people_value;
    rtn.length += 3;

    // ¥��
    *dptr++ = ElvtWrnData.floor_flag;
    *dptr++ = ElvtWrnData.floor_aligned;
    *dptr++ = ElvtWrnData.floor_value;
    rtn.length += 3;

    // �ż���λ״̬
    *dptr++ = ElvtWrnData.door_limit;
    rtn.length += 1;

    // Ԥ��19B
    for (i = 0; i < 19; i++)
    {
        *dptr++ = ElvtWrnData.rev[i];
        rtn.length += 1;
    }
    // ��λ��Ϣ״̬��
    *dptr++ = ElvtWrnData.status;
    rtn.length += 1;

    // �ж�״̬��ѯ�Ƿ���ڱ�λ��Ϣ
    // ���ڱ���
    if (ElvtWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ
        sptr = ElvtWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// ��λ����3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����
            case 0x05:  // ����������
            case 0x06:  // ���ر���
            case 0x07:  // ���ٱ���
            case 0x08: // ��б����
            case 0x0a:  // �ű���
            case 0x0b:  // ��������
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ����Υ��
    if ((ElvtWrnData.status>>1)&0x01)
    {
        // ���Υ����Ϣ
        sptr = ElvtWrnData.illegal;
        varnum = *sptr++;   // Υ����Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����1Byte
            case 0x04:  // ��λ����1Byte
            case 0x05:  // ����������1Byte
            case 0x06:  // ���ر���1Byte
            case 0x07:  // ���ٱ���1Byte
            case 0x08: // ��б����1Byte
            case 0x09:  //�����֤1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ���ڹ���
    if ((ElvtWrnData.status>>2)&0x01)
    {
        // ��ӹ�����Ϣ
        sptr = ElvtWrnData.fault;
        varnum = *sptr++;   // ������Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //�������ͱ���
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // Ԥ��6B
    sptr = ElvtWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFElvtCaliDat
*
*   ����˵��: �Զ��ϴ������������궨���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9DH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&ElvtCaliDat.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    // �����ʹ���       2B
    sptr = ElvtCaliDat.packtype;
    for (i = 0; i < 2;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 2;

    // ������     5B
    sptr = (uint8_t *)&ElvtCaliDat.gravitylift;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // �߶�       5B
    sptr = (uint8_t *)&ElvtCaliDat.height;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ����       5B
    sptr = (uint8_t *)&ElvtCaliDat.wind;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // ���       5B
    sptr = (uint8_t *)&ElvtCaliDat.dipangle;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;
    // Ԥ��       5BX4
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

    // ���¼���CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}
#endif

#ifdef DUSTMON
void GetDustRTData(void)
{
    uint32_t i;

    // ʱ��
    dustdata.clock.sec = period_value.sec; //period_value.sec;
    dustdata.clock.min = period_value.min;
    dustdata.clock.hour = period_value.hour;
    dustdata.clock.date = period_value.date;
    dustdata.clock.week_single = (period_value.month >> 5) & 0x07;
    dustdata.clock.mon_single = period_value.month & 0x1F;
    dustdata.clock.year = period_value.year;

    // ��Ա��ʶ��
    dustdata.name_id = period_value.name_id;

    // ������ ����
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

    // ����0.0 - 9999.9 m/s
    dustdata.wind.sensor_OK = period_value.wind_flag & 0x01;
    dustdata.wind.sensor_EN = (period_value.wind_flag & 0x02)>>1;
    dustdata.wind.rev1 = 0;
    dustdata.wind.warning = period_value.wind_alarm;
    PacksBCD((uint8_t *)&dustdata.wind.data, period_value.wind_value,5,1);

    // ��ŷ�һ
    dustdata.valve[0].sensor_OK = period_value.valve1_flag & 0x01;
    dustdata.valve[0].sensor_EN = (period_value.valve1_flag & 0x02)>>1;
    dustdata.valve[0].rev1 = 0;
    dustdata.valve[0].warning = period_value.valve1_alarm;

    // ��ŷ���
    dustdata.valve[1].sensor_OK = period_value.valve2_flag & 0x01;
    dustdata.valve[1].sensor_EN = (period_value.valve2_flag & 0x02)>>1;
    dustdata.valve[1].rev1 = 0;
    dustdata.valve[1].warning = period_value.valve2_alarm;

    // ��ŷ���
    dustdata.valve[2].sensor_OK = period_value.valve3_flag & 0x01;
    dustdata.valve[2].sensor_EN = (period_value.valve3_flag & 0x02)>>1;
    dustdata.valve[2].rev1 = 0;
    dustdata.valve[2].warning = period_value.valve3_alarm;

    // ��ŷ���
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
    // ����5BX2
    for (i = 0; i < 10;i++)
    {
        dustdata.rev[i] = 0;
    }


    // ����6B
    for (i = 0; i < 6;i++)
    {
        dustdata.rev1[i] = 0;
    }
}

void GetDustWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // ʱ��
    DustWrnData.clock.sec = alarm_dat[num].sec; //alarm_dat[num].sec;
    DustWrnData.clock.min = alarm_dat[num].min;
    DustWrnData.clock.hour = alarm_dat[num].hour;
    DustWrnData.clock.date = alarm_dat[num].date;
    DustWrnData.clock.week_single = (alarm_dat[num].month >> 5) & 0x07;
    DustWrnData.clock.mon_single = alarm_dat[num].month & 0x1F;
    DustWrnData.clock.year = alarm_dat[num].year;

    // ��Ա��ʶ��
    DustWrnData.name_id = alarm_dat[num].name_id;

    // ������ ����
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

    // ����0.0 - 9999.9 m/s
    DustWrnData.wind.sensor_OK = alarm_dat[num].wind_flag & 0x01;
    DustWrnData.wind.sensor_EN = (alarm_dat[num].wind_flag & 0x02)>>1;
    DustWrnData.wind.rev1 = 0;
    DustWrnData.wind.warning = alarm_dat[num].wind_alarm;
    PacksBCD((uint8_t *)&DustWrnData.wind.data, alarm_dat[num].wind_value,5,1);

    // ��ŷ�һ
    DustWrnData.valve[0].sensor_OK = alarm_dat[num].valve1_flag & 0x01;
    DustWrnData.valve[0].sensor_EN = (alarm_dat[num].valve1_flag & 0x02)>>1;
    DustWrnData.valve[0].rev1 = 0;
    DustWrnData.valve[0].warning = alarm_dat[num].valve1_alarm;

    // ��ŷ���
    DustWrnData.valve[1].sensor_OK = alarm_dat[num].valve2_flag & 0x01;
    DustWrnData.valve[1].sensor_EN = (alarm_dat[num].valve2_flag & 0x02)>>1;
    DustWrnData.valve[1].rev1 = 0;
    DustWrnData.valve[1].warning = alarm_dat[num].valve2_alarm;

    // ��ŷ���
    DustWrnData.valve[2].sensor_OK = alarm_dat[num].valve3_flag & 0x01;
    DustWrnData.valve[2].sensor_EN = (alarm_dat[num].valve3_flag & 0x02)>>1;
    DustWrnData.valve[2].rev1 = 0;
    DustWrnData.valve[2].warning = alarm_dat[num].valve3_alarm;

    // ��ŷ���
    DustWrnData.valve[3].sensor_OK = alarm_dat[num].valve4_flag & 0x01;
    DustWrnData.valve[3].sensor_EN = (alarm_dat[num].valve4_flag & 0x02)>>1;
    DustWrnData.valve[3].rev1 = 0;
    DustWrnData.valve[3].warning = alarm_dat[num].valve4_alarm;

    // ����5BX4
    for (i = 0; i < 20;i++)
    {
        DustWrnData.rev[i] = 0;
    }


    // ����6B
    for (i = 0; i < 6;i++)
    {
        DustWrnData.rev1[i] = 0;
    }

    // ״̬��
    DustWrnData.status = alarm_dat[num].alarm_stat;

    // ����״̬����ӱ���/Υ��/������Ϣ
    if (DustWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ 1��
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
                // δ�������״̬
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
        // ���Υ����Ϣ 1��
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
                // δ�������״̬
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
        // ��ӹ�����Ϣ 1��
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
*   �� �� ��: AUFElvtRTDat
*
*   ����˵��: �Զ��ϴ����ģ���������ʱʵʱ���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9AH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&dustdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // ��ԱID
    sptr = (uint8_t *)&dustdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // �����ʹ���       2B
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
    // ��ŷ�һ������������   2B X 4
    for (i = 0; i < 4; i++)
    {
        sptr = (uint8_t *)&dustdata.valve[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }
    // ��������5B
    sptr = (uint8_t *)&dustdata.noise;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    //��������5B
    sptr = (uint8_t *)&dustdata.vane;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    // Ԥ��10B
    for (i = 0; i < 10; i++)
    {
        *dptr++ = dustdata.rev[i];
        rtn.length += 1;
    }

    // Ԥ��6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = dustdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: AUFDustWrnDat
*
*   ����˵��: �Զ��ϴ����ı������ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=9CH
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
*
*********************************************************************************************************
*/
pTX101 AUFDustWrnDat(void)
{
    uint32_t i,j;
    uint8_t *sptr,*dptr;
    uint8_t varnum;//��λ��Ϣ��Ŀ
    uint8_t vartype;//��λ��Ϣ����

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_DUSTWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&DustWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // ��ԱID
    sptr = (uint8_t *)&DustWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // �����ʹ���       2B
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
    // ��ŷ�һ������������   2B X 4
    for (i = 0; i < 4; i++)
    {
        sptr = (uint8_t *)&DustWrnData.valve[i];
        for (j = 0; j < 2;j++)
        {
            *dptr++ = *sptr++;
        }
        rtn.length += 2;
    }

    // ��������5B
    sptr = (uint8_t *)&DustWrnData.noise;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    //��������5B
    sptr = (uint8_t *)&DustWrnData.vane;
    for (i = 0; i < 5;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 5;

    // Ԥ��10B
    for (i = 0; i < 10; i++)
    {
        *dptr++ = DustWrnData.rev[i];
        rtn.length += 1;
    }

    // ��λ��Ϣ״̬��
    *dptr++ = DustWrnData.status;
    rtn.length += 1;

    // �ж�״̬��ѯ�Ƿ���ڱ�λ��Ϣ
    // ���ڱ���
    if (DustWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ
        sptr = DustWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// ��λ����3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����
            case 0x05:  // ����������
            case 0x06:  // ���ر���
            case 0x07:  // ���ٱ���
            case 0x08: // ��б����
            case 0x0a:  // �ű���
            case 0x0b:  // ��������
            case 0x20:  // ��ŷ�����
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ����Υ��
    if ((DustWrnData.status>>1)&0x01)
    {
        // ���Υ����Ϣ
        sptr = DustWrnData.illegal;
        varnum = *sptr++;   // Υ����Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����1Byte
            case 0x04:  // ��λ����1Byte
            case 0x05:  // ����������1Byte
            case 0x06:  // ���ر���1Byte
            case 0x07:  // ���ٱ���1Byte
            case 0x08: // ��б����1Byte
            case 0x09:  //�����֤1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ���ڹ���
    if ((DustWrnData.status>>2)&0x01)
    {
        // ��ӹ�����Ϣ
        sptr = DustWrnData.fault;
        varnum = *sptr++;   // ������Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //�������ͱ���
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // Ԥ��6B
    sptr = DustWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

#endif

#ifdef UPPLAT
void GetUPPlatRTData(void)
{
    uint32_t i;

    // ʱ��
   upplatdata.clock.sec = upperiod_value.sec; //upperiod_value.sec;
   upplatdata.clock.min = upperiod_value.min;
   upplatdata.clock.hour = upperiod_value.hour;
   upplatdata.clock.date = upperiod_value.date;
   upplatdata.clock.week_single = (upperiod_value.month >> 5) & 0x07;
   upplatdata.clock.mon_single = upperiod_value.month & 0x1F;
   upplatdata.clock.year = upperiod_value.year;

    // ��Ա��ʶ��
   upplatdata.name_id = upperiod_value.name_id;

    // ������ ����
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

   // ����5BX4
    for (i = 0; i < 20;i++)
    {
       upplatdata.rev[i] = 0;
    }


    // ����6B
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&upplatdata.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // ��ԱID
    sptr = (uint8_t *)&dustdata.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // �����ʹ���       2B
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

    // Ԥ��20B
    for (i = 0; i < 20; i++)
    {
        *dptr++ = upplatdata.rev[i];
        rtn.length += 1;
    }

    // Ԥ��6B
    for (i = 0; i < 6;i++)
    {
        *dptr++ = upplatdata.rev1[i];
        rtn.length += 1;
    }

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}

void GetUPPlatWrnDat(uint8_t num)
{
    uint32_t i,j;
    uint8_t *dptr,len=0;

    // ʱ��
    UPPlatWrnData.clock.sec = upalarm_dat[num].sec; //alarm_dat[num].sec;
    UPPlatWrnData.clock.min = upalarm_dat[num].min;
    UPPlatWrnData.clock.hour = upalarm_dat[num].hour;
    UPPlatWrnData.clock.date = upalarm_dat[num].date;
    UPPlatWrnData.clock.week_single = (upalarm_dat[num].month >> 5) & 0x07;
    UPPlatWrnData.clock.mon_single = upalarm_dat[num].month & 0x1F;
    UPPlatWrnData.clock.year = upalarm_dat[num].year;

    // ��Ա��ʶ��
    UPPlatWrnData.name_id = upalarm_dat[num].name_id;

    // ������ ����
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

    // ����5BX4
    for (i = 0; i < 20;i++)
    {
        UPPlatWrnData.rev[i] = 0;
    }


    // ����6B
    for (i = 0; i < 6;i++)
    {
        UPPlatWrnData.rev1[i] = 0;
    }

    // ״̬��
    UPPlatWrnData.status = upalarm_dat[num].alarm_stat;

    // ����״̬����ӱ���/Υ��/������Ϣ
    if (UPPlatWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ 1��
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
                // δ�������״̬
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
        // ���Υ����Ϣ 1��
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
                // δ�������״̬
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
        // ��ӹ�����Ϣ 1��
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
    uint8_t varnum;//��λ��Ϣ��Ŀ
    uint8_t vartype;//��λ��Ϣ����

    //GetElvtWrnData(0);

    rtn.length = 0; //  C1+A5+V1+appFunc1

    link_layer_pack(&rtn,0, 0, StatusFlag.resend_times, LFN_DIR1_RANDOMWARN);
    rtn.appzone.functioncode = AFN_SELF_UPPLATWARN;
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone


    dptr = rtnbuf;

    // clock 6B
    sptr = (uint8_t *)&UPPlatWrnData.clock;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

/*
    // ��ԱID
    sptr = (uint8_t *)&UPPlatWrnData.name_id;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;
*/

    // �����ʹ���       2B
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

    // Ԥ��20B
    for (i = 0; i < 20; i++)
    {
        *dptr++ = UPPlatWrnData.rev[i];
        rtn.length += 1;
    }

    // ��λ��Ϣ״̬��
    *dptr++ = UPPlatWrnData.status;
    rtn.length += 1;

    // �ж�״̬��ѯ�Ƿ���ڱ�λ��Ϣ
    // ���ڱ���
    if (UPPlatWrnData.status&0x01)
    {
        // ��ӱ�����Ϣ
        sptr = UPPlatWrnData.warning;
        varnum = *sptr++;
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x04:// ��λ����3Bytes
                for(j=0;j<3;j++){*dptr++ = *sptr++;}
                rtn.length += 3;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����
            case 0x05:  // ����������
            case 0x06:  // ���ر���
            case 0x07:  // ���ٱ���
            case 0x08: // ��б����
            case 0x0a:  // �ű���
            case 0x0b:  // ��������
			case 0x20:  // ��ŷ�����
			case 0x21:	// ж��ƽ̨����
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ����Υ��
    if ((UPPlatWrnData.status>>1)&0x01)
    {
        // ���Υ����Ϣ
        sptr = UPPlatWrnData.illegal;
        varnum = *sptr++;   // Υ����Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++)
        {
            vartype = *sptr++;  //��λ��Ϣ���ͱ���
            *dptr++ = vartype;
            rtn.length += 1;
            switch (vartype)
            {
            case 0x01:// �໥���汨��2Bytes
                for(j=0;j<2;j++){*dptr++ = *sptr++;}
                rtn.length += 2;
                break;
            case 0x02:  // ��������������1Byte
            case 0x03:  // �ϰ�����ײ����1Byte
            case 0x04:  // ��λ����1Byte
            case 0x05:  // ����������1Byte
            case 0x06:  // ���ر���1Byte
            case 0x07:  // ���ٱ���1Byte
            case 0x08: // ��б����1Byte
            case 0x09:  //�����֤1Byte
                *dptr++ = *sptr++;
                rtn.length += 1;
                break;
            }
        }
    }
    // ���ڹ���
    if ((UPPlatWrnData.status>>2)&0x01)
    {
        // ��ӹ�����Ϣ
        sptr = UPPlatWrnData.fault;
        varnum = *sptr++;   // ������Ϣ��Ŀ
        *dptr++ = varnum;
        rtn.length += 1;

        for (i=0;i<varnum;i++) //�������ͱ���
        {
            *dptr++ = *sptr++;
            rtn.length += 1;
        }
    }

    // Ԥ��6B
    sptr = UPPlatWrnData.rev2;
    for (i = 0; i < 6;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 6;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);

    return &rtn;
}
#endif

/*
*********************************************************************************************************
*   �� �� ��: AUFFngrDat
*
*   ����˵��: �Զ��ϴ����ģ���ʱʵʱ���ݡ�����֡,��վ��Ӧ��ͬ�����뱨�ġ�AFN=98H
*
*   ��   ��: none
*
*   �� �� ֵ: ���ؼ������͵�����ָ֡��
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
    rtn.length += 8;//C1+Address5+ProtocolVer1+AFN1=8��userzone

    dptr = rtnbuf;

    // ʱ�� 6B
    GetRTC(&clock);

    for (i = 0; i < 6;i++)
    {
        *dptr++ = ((uint8_t *)(&clock))[i];
    }
    rtn.length += 6;
    // ��ԱID 4B
    sptr = (uint8_t *)&fingerdata.staffid;
    for (i = 0; i < 4;i++)
    {
        *dptr++ = *sptr++;
    }
    rtn.length += 4;

    rtn.appzone.userdata = rtnbuf;

    // ���¼���CRC
    rtn.cs = GetCRC(0);
    return &rtn;
}

/*
*********************************************************************************************************
*   �� �� ��: link_layer_unpack
*
*   ����˵��: �жϽ���֡����·������Ƿ���ȷ���ж�������Ҫ�У���ʼ�ַ������ȡ���ַ��У�����Լ������ַ�
*
*   ��   ��: pTX101 pframe ���� ���յ�����������֡
*
*   �� �� ֵ: uint8_t ���� ��·�����û�д��󷵻�0�����д��󷵻�Ϊ>1������
*                       1 ���� ��֡��������֡
*                       2 ���� ��·�㹦������Ч
*                       3 ���� �·���ַ���ն˵�ַ��ƥ��
*
*********************************************************************************************************
*/
uint8_t link_layer_unpack(pTX101 pframe)
{
    AddrZone addr;

    // �жϿ�����͵�ַ��ĺ����ԣ��ڻ�ȡ����֡��ʱ��������·��Ϣ�Ѿ��ж�
    //
    // �жϿ�������λ�Ƿ�����
    // if (pframe->ctrlzone.dir != 0) return 1;

    // �жϿ����������Ƿ�������
    switch (pframe->ctrlzone.func)
    {
    case 1: // ����/ȷ�� ���� �·�����
    case 2: // ����/�޻ش� ���� �û�����
    case 3: // ��ѯ/��Ӧ ���� ��·����
    case 4: // ��ѯ/��Ӧ ���� �������
    case 13:// ��Ӧ֡ ���� ������״̬����
        break;
    default:    // ���û��������幦����
        return 2;
    }

    // �ж�����֡��ַ�Ƿ��뱾����ַ���
    addr = *GetTAddr();
    if (addr.a1_low != pframe->addrzone.a1_low ||
        addr.a1_high == pframe->addrzone.a1_high ||
        addr.a2_low == pframe->addrzone.a2_low ||
        addr.a2_middle == pframe->addrzone.a2_middle ||
        addr.a2_high == pframe->addrzone.a2_high) return 3;

    // ��·����ȷ������0
    return 0;
}

/*
*********************************************************************************************************
*   �� �� ��: link_layer_pack
*
*   ����˵��: �����лظ�ʱ������·��Ĵ�����ú���ʵ�ֹ�������(��ʼ�ַ��������ַ����ն˵�ַ��Э��汾����������λ��CSУ��λ)
*            �Ĳ�������������(������DIV��FCB�������룬֡����CSУ��λ)���в���
*
*   ��   ��: pTX101 pframe ���� ���յ�����������֡
*
*   �� �� ֵ: uint8_t ���� ��·�����û�д��󷵻�0�����д��󷵻�Ϊ>1������
*
*********************************************************************************************************
*/
void link_layer_pack(pTX101 pframe,uint8_t dir,uint8_t div,uint8_t fcb,uint8_t linkfuncode)
{
    pframe->startb1 = pframe->startb2 = STARTCHAR;
    pframe->endbyte = ENDCHAR;
    pframe->addrzone = *GetTAddr();
    pframe->version = *GetProtocolVersion();
    pframe->ctrlzone.dir = dir; // ����
    pframe->ctrlzone.div = div;
    pframe->ctrlzone.fcb = fcb;
    pframe->ctrlzone.func = linkfuncode;
}


/*
*********************************************************************************************************
*   �� �� ��: GetTAddr
*
*   ����˵��: ��ȡ�洢�����ն˵�ַ
*
*   ��   ��: none
*
*   �� �� ֵ: pAddrZone ���� ��ַ�ṹָ��
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
*   �� �� ��: GetVersion
*
*   ����˵��: ��ȡЭ��汾��
*
*   ��   ��: none
*
*   �� �� ֵ: pVerInfo ���� Э��汾�Ľṹָ��
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
*   �� �� ��: GetIPHost
*
*   ����˵��: ��ȡЭ��汾��
*
*   ��   ��: num ���� IP��ţ������飬Ĭ�ϵ�һ��
*
*   �� �� ֵ: pVerInfo ���� Э��汾�Ľṹָ��
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
*   �� �� ��: SetRTC
*
*   ����˵��: ����ʵʱʱ��
*
*   ��   ��: ʱ�ӽṹ
*
*   �� �� ֵ: none
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

    // ��ȡ����֡�е�ʱ����Ϣ��������RTC
    //system_parameter.clk = *((pRtcClk)rtn.appzone.userdata);
    HAL_RTC_SetTime(&hrtc, &sT, FORMAT_BCD);
    HAL_RTC_SetDate(&hrtc, &sD, FORMAT_BCD);
}

/*
*********************************************************************************************************
*   �� �� ��: GetRTC
*
*   ����˵��: ��ȡʵʱʱ��
*
*   ��   ��: ʱ�ӽṹ
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
void GetRTC(pRtcClk realtime)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    // ��ȡRTCʱ��
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
*   �� �� ��: GetCRC
*
*   ����˵��: ��ȡ����֡�ṹ�е�CRCУ���ֽ�
*
*   ��   ��: ֡���� 0 ���� ��֡
*                  >0 ���� ��֡��֡��Ϊ(frametype-1)
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
uint8_t GetCRC(uint8_t frametype)
{
    uint32_t i;
    uint8_t *ptr1,*ptr2;

    // ���¼���CRC
    if (frametype) // ��֡
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
    else    // ��֡
    {// ����������1����ַ��5���汾1��������
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
*   �� �� ��: RNG_Get_RandomRange
*
*   ����˵��: ��ȡ������Χ�ڵ��������
*
*   ��   ��: uint32_t xmin, uint32_t xmax ���� �������������
*
*   �� �� ֵ: �������
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
*   �� �� ��: rtn2frame
*
*   ����˵��: ��֡�ṹת��������
*
*   ��   ��:  pTX101 rptr ���� ֡�ṹָ��
*             uint8_t *pframe ���� ����
*
*   �� �� ֵ: ֡����
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
*   �� �� ��: rtn2frame
*
*   ����˵��: ��֡�ṹת��������
*
*   ��   ��:  pTX101 rptr ���� ֡�ṹָ��
*             uint8_t *pframe ���� ����
*
*   �� �� ֵ: ֡����
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
*   �� �� ��: FrmLnkLog
*
*   ����˵��: ��֯��·��½����֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmHrtDat
*
*   ����˵��: ��֯����������֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
*
*********************************************************************************************************
*/
uint16_t FrmHrtDat(uint8_t *buf)
{
    frame_link_chk(DATAFIELD_ONLINE);    // ���͵�¼֡
    return rtn2frame(&rtn, buf);
}

#ifdef TOWERBOX
/*
*********************************************************************************************************
*   �� �� ��: FrmDevLctSet
*
*   ����˵��: �����ն˵���λ��/��γ�Ȳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevLctSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevLct(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevLctQry
*
*   ����˵��: ��ѯ�ն˵���λ��/��γ�Ȳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevLctQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevLct(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTowRtDat
*
*   ����˵��: ��֯ʵʱ��ʱ���ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmTowWklpDat
*
*   ����˵��: ��֯����ѭ�����ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmTowWrnDat
*
*   ����˵��: ��֯ʵʱ�������ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmTowCaliDat
*
*   ����˵��: ��֯ʵʱ�궨����֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmFngrDat
*
*   ����˵��: ��ָ֯���ϱ�����֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmElvtRtDat
*
*   ����˵��: ��֯������ʵʱ��ʱ���ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmTowWklpDat
*
*   ����˵��: ��֯����ѭ�����ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmElvtCaliDat
*
*   ����˵��: ��֯ʵʱ�궨����֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmElvtWrnDat
*
*   ����˵��: ��֯������ʵʱ�������ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmDustRtDat
*
*   ����˵��: ��֯�ﳾ���ʵʱ��ʱ���ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmDustWrnDat
*
*   ����˵��: ��֯�ﳾ���ʵʱ�������ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmUPPlatRtDat
*
*   ����˵��: ��֯ж��ƽ̨ʵʱ��ʱ���ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmUPPlatWrnDat
*
*   ����˵��: ��֯ж��ƽ̨�������ݷ���֡
*
*   ��   ��: uint8_t
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmDevAddrDatSet
*
*   ����˵��: �����ն˵�ַ����֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevAddrDatSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetDevAddr(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevAddrDatQry
*
*   ����˵��: ��ѯ�ն˵�ַ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevAddrDatQry(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamQryDevaddr(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevRtcDatSet
*
*   ����˵��: �����ն�ʱ�Ӳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevRtcDatSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetDevRtc(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevRtcQry
*
*   ����˵��: ��ѯ�ն�ʱ�Ӳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevRtcQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRtc(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevWkModSet
*
*   ����˵��: �����ն˹���ģʽ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevWkModSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetWkMod(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevWkModQry
*
*   ����˵��: ��ѯ�ն˹���ģʽ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevWkModQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryWkMod(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevSnsrTypSet
*
*   ����˵��: �����ն˴��������Ͳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrTypSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetSnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevSnsrTypQry
*
*   ����˵��: ��ѯ�ն˴��������Ͳ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrTypQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQrySnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevSnsrCfgSet
*
*   ����˵��: �����ն˴�������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrCfgSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetSnsrCfg(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevSnsrCfgQry
*
*   ����˵��: ��ѯ�ն˴�������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSnsrCfgQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQrySnsrTyp(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevIpPortSet
*
*   ����˵��: �����ն˴洢������վIP��ַ�Ͷ˿ںŲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevIpPortSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevIpPort(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevIpPortQry
*
*   ����˵��: ��ѯ�ն˴洢������վIP��ַ�Ͷ˿ںŲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevIpPortQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevIpPort(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevHrtIntvlSet
*
*   ����˵��: �����ն������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevHrtIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetHrtIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevHrtIntvlQry
*
*   ����˵��: ��ѯ�ն������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevHrtIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryHrtIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevLnkReconIntvlSet
*
*   ����˵��: �����ն���·�����������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevLnkReconIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevLnkReconIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevLnkReconIntvlQry
*
*   ����˵��: ��ѯ�ն���·�����������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevLnkReconIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevLnkReconIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevLnkReconIntvlSet
*
*   ����˵��: �����ն���ʷ���ݴ��̼������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSavIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevRecIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevSavIntvlQry
*
*   ����˵��: ��ѯ�ն���ʷ���ݴ��̼������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevSavIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRecIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevRtdRptIntvlSet
*
*   ����˵��: �����ն�ʵʱ�����ϱ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevRtdRptIntvlSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevRTDReptIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevRtdRptIntvlQry
*
*   ����˵��: ��ѯ�ն�ʵʱ�����ϱ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevRtdRptIntvlQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevRTDReptIntvl(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevUpdSet
*
*   ����˵��: �����ն��������ݲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevUpdSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevUpd(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevVerInfoQry
*
*   ����˵��: ��ѯ�ն˰汾��Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevVerInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevVerInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevPwdSet
*
*   ����˵��: �����ն����벢��֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevPwdSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetDevPwd(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmDevPwdQry
*
*   ����˵��: ��ѯ�ն��ն����벢��֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmDevPwdQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryDevPwd(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmFngrDatSet
*
*   ����˵��: ����ָ�����ݱ������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmFngrDatSet(uint8_t *rf,uint8_t *sf)
{
    return mrtn2frame(ParamSetFngrDat(&mrtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmFngrDatDel
*
*   ����˵��: ɾ��ָ�����ݲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmFngrDatDel(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(Param_Del_FngrDat(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmRestart
*
*   ����˵��: ���������������ն������ѭ���ȴ����Ź���ʱ��λ
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: none
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
*   �� �� ��: FrmTwrInfoSet
*
*   ����˵��: ��������������Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrInfoSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrInfoQry
*
*   ����˵��: ��ѯ����������Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrPrtcZoneSet
*
*   ����˵��: ������������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrPrtcZoneSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetPrtcZone(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrPrtcZoneQry
*
*   ����˵��: ��ѯ��������������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrPrtcZoneQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryPrtcZone(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrLmtSet
*
*   ����˵��: ����������λ��Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrLmt(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrLmtQry
*
*   ����˵��: ��ѯ������λ��Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrLmtQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrLmt(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrTorqSet
*
*   ����˵��: ��������������Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrTorqSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrTorque(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrTorqQry
*
*   ����˵��: ��ѯ����������Ϣ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrTorqQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrTorque(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrCaliSet
*
*   ����˵��: ���������궨��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrCaliSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrCali(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrCaliQry
*
*   ����˵��: ��ѯ�����궨��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrCaliQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryTwrCali(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrLiftSet
*
*   ����˵��: ���������������ݲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmTwrLiftSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetTwrLift(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmTwrLiftQry
*
*   ����˵��: ��ѯ�����������ݲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
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
*   �� �� ��: FrmElvtInfoSet
*
*   ����˵��: ���������������ṹ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmElvtInfoSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetElvtInfo(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmElvtInfoQry
*
*   ����˵��: ��ѯ���������������ṹ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmElvtInfoQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryElvtInfo(&rtn), sf);
}


/*
*********************************************************************************************************
*   �� �� ��: FrmElvtFloorSet
*
*   ����˵��: ���������������ṹ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmElvtFloorSet(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetElvtFloor(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmElvtFloorQry
*
*   ����˵��: ��ѯ���������������ṹ��������֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
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
*   �� �� ��: FrmValveLmtSet
*
*   ����˵��: �����ﳾ����ŷ���ֵPM2.5����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetValveLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   �� �� ��: FrmValveLmtSet_Ext
*
*   ����˵��: �����ﳾ����ŷ�PM10��ֵ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtSet_Ext(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetValveLmt_Ext(&rtn), sf);
}

/*
*********************************************************************************************************
*   �� �� ��: FrmValveLmtQry
*
*   ����˵��: ��ѯ�ﳾ����ŷ�PM2.5��ֵ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtQry(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryValveLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   �� �� ��: FrmValveLmtQry_Ext
*
*   ����˵��: ��ѯ�ﳾ����ŷ�PM10��ֵ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmValveLmtQry_Ext(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamQryValveLmt_Ext(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmValveManual
*
*   ����˵��: �����ﳾ����ŷ��ֶ����ϲ���֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmValveManual(uint8_t *rf, uint8_t *sf)
{
    return rtn2frame(ParamSetValveMan(&rtn), sf);
}
/*
*********************************************************************************************************
*   �� �� ��: FrmNotice
*
*   ����˵��: �����ﳾ���OLED��Ϣ�·�����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
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
*   �� �� ��: FrmUPLmtSet
*
*   ����˵��: ����ж��ƽ̨��ֵ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
*
*********************************************************************************************************
*/
uint16_t FrmUPLmtSet(uint8_t *rf,uint8_t *sf)
{
    return rtn2frame(ParamSetUPLmt(&rtn), sf);
}

/*
*********************************************************************************************************
*   �� �� ��: FrmValveLmtQry
*
*   ����˵��: ��ж��ƽ̨��ֵ����֯Ӧ��֡
*
*   ��   ��: uint8_t *rf ���� �յ�������֡
*            uint8_t *sf ���� Ӧ�������֡
*
*   �� �� ֵ: Ӧ��֡��
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
