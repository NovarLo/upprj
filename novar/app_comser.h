/************************************************************************
		ComSer.h
************************************************************************/
#ifndef _APP_COMSER_H
#define _APP_COMSER_H

#ifdef	_LOCAL_COMSER
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

//���г���
#define TXSERARRAYNUM   100     // ���Ͷ��г���
#define RXSERARRAYNUM   100     // ���ն��г���
//��ʱ����
#define ATOVERTM        250     // AT����س�ʱ 250 * 20ms = 5S
#define RXOVERTM        3000    // Э��֡���ؽ���Ӧ��ʱʱ�� 3000 * 20ms = 60s                            
#define REPTXTIMS       3       // �ط�����

// �շ������������(���ֽ�Ϊ������,��5��; ���ֽ�Ϊ������,���255��)
#define COMTYPE_AT      0x0000  //����ģ����
#define COMTYPE_TICK    0x0100  //��¼/��������
#define COMTYPE_REPORT  0x0200  //�Ա���
#define COMTYPE_SET     0x0300  //������
#define COMTYPE_QUERY   0x0400  //��ѯ��
#define COMTYPE_ERR     0x0500  //������

//�շ�����������ݽṹ 
typedef struct
{
    uint16_t  SerType;    // �շ���������
                          // 1�ν����У��շ���ֵӦ��ͬ����Ӧ������=�������ͣ��ɴ���ԡ�
    uint16_t  overtime;   // �ȴ�Ӧ��ʱ��������ֵ,��������������������,������븳0 
    uint32_t  par1;       // ����1
    uint32_t  par2;       // ����2
    void      (*par3)(uint32_t par1,uint32_t par2); //ִ�к���
}COMSERARRAY;

//-----������GPRSͨѶ�������ݽṹ----------------------
typedef struct
{
    COMSERARRAY  TxArray[TXSERARRAYNUM];
    COMSERARRAY *TxRpoint;      // ��
    COMSERARRAY *TxWpoint;      // д
    COMSERARRAY *TxOpoint;      // ������
                                
    COMSERARRAY  RxArray[RXSERARRAYNUM];
    COMSERARRAY *RxRpoint;      // ��
    COMSERARRAY *RxWpoint;      // д
    COMSERARRAY *RxOpoint;      // ������
                                 
	uint8_t     TxRepTimsEn;    // �ط�������־
    uint8_t     TxRepTims;      // �ط�����������
    uint8_t     TxOverDlyFlag;  // ���ͳ�ʱ������־
    uint16_t    TxOverDlyCnt;   // ���ͳ�ʱ������
                                 
}COMSERCTR;
_EXTERN COMSERCTR  ComSerCtr;	

typedef struct
{
	uint16_t SerType;
	uint8_t len;
	uint32_t timeout;
	uint8_t buf[260];
	uint8_t lock;   // =1����ʾ������=0������д��
}param_ext;

_EXTERN param_ext extparam;		// �������еĲ���par1��par2��������������Ϣʱ��Ϊ������������ָ�븳��par2

// extern variable

//-------����--------------------------------------
void ComSer_Main(void);
void ComSeverInit(void);  // �շ�������г�ʼ��
int16_t Add2TxSerArray(COMSERARRAY *point,uint8_t priority); // ���Ͷ��м���1�������͵�����
void ComTxSeverExc(void); // ����������õķ��ͷ���

int16_t Add2RxSerArray(COMSERARRAY *point); // �ɽ����жϳ�����õ�����ն��м���1�������͵�����
void ComRxSeverExc(void); // ����������õĽ��շ���
void RxOverDly(void);     // �ɶ�ʱ�жϳ�����õĽ���Ӧ��ʱ
void Fnull(uint32_t par1, uint32_t par2);
void FProtocolAck(uint32_t par1, uint32_t par2);
void StartTimeoutCnt(void);// ������ʱ��ʱ��

#undef _EXTERN
#endif

/***************** end line ********************/

