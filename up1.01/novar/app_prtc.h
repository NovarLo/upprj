
#ifndef _APP_PRTC_H
#define _APP_PRTC_H

#ifdef _LOCAL_PRTC
    #define _EXTERN
#else
    #define _EXTERN extern
#endif

// test macro
//#define NOVAR_TEST
//#define PRINT_UART1

// application layer function code

#define AFN_DENY    0x00    // ����/ʧ��
#define AFN_CONFIRM 0x01    // ȷ��/�ɹ�
#define AFN_INVALID 0x02    // ��Ч����
#define AFN_LINKCHK 0x03    // ��·�ӿڼ��

#define AFN_SET_ADDRESS             0x10    // ����ң���ն˵�ַ
#define AFN_QRY_ADDRESS           0x50    // ��ѯң���ն˵�ַ
#define AFN_SET_CLOCK               0x11    // ����ң���ն�ʱ��
#define AFN_QRY_CLOCK             0x51    // ��ѯң���ն�ʱ��
#define AFN_SET_WORKMODE            0x12    // ����ң���ն˹���ģʽ
#define AFN_QRY_WORKMODE          0x52    // ��ѯң���ն˹���ģʽ
#define AFN_SET_SENSORTYPE          0x13    // ����ң���ն˵Ĵ���������
#define AFN_QRY_SENSORTYPE        0x53    // ��ѯң���ն˵Ĵ���������
#define AFN_SET_SENSORPARAM         0x14    // ����ң���ն˵Ĵ���������
#define AFN_QRY_SENSORPARAM       0x54    // ��ѯң���ն˵Ĵ���������
#define AFN_SET_LOCATION            0x15    // ����ң���ն˵���λ��/��γ��
#define AFN_QRY_LOCATION          0x55    // ��ѯң���ն�GPSλ��
#define AFN_SET_TOWERPARAM          0x16    // ����������̬�ṹ����
#define AFN_QRY_TOWERPARAM        0x56    // ��ѯ������̬�ṹ����
#define AFN_SET_PROTECTIONZONE      0x17    // ��������������
#define AFN_QRY_PROECTIONZONE     0x57    // ��ѯ����������
#define AFN_SET_LIMIT               0x18    // ����������λ��Ϣ
#define AFN_QRY_LIMIT             0x58    // ��ѯ������λ��Ϣ
#define AFN_SET_MOMENTCURVE         0x19    // ����������������
#define AFN_QRY_MOMENTCURVE       0x59    // ��ѯ������������
#define AFN_SET_CALIBRATPARAM       0x1A    // ���������궨����
#define AFN_QRY_CALIBRATPARAM     0x5A    // ��ѯ�����궨����
#define AFN_SET_DEVIPPORT           0x1F    // ����ң���ն˴洢������վIP��ַ�Ͷ˿ں�
#define AFN_QRY_DEVIPPORT         0x5F    // ��ѯң���ն˴洢������վIP��ַ�Ͷ˿ں�
#define AFN_SET_HEARTINTERVAL       0x20    // ����ң���ն���·�������
#define AFN_QRY_HEARTINTERVAL     0x60    // ��ѯң���ն���·�������
#define AFN_SET_RECONNECTINTERVAL   0x21    // ����ң���ն���·�������
#define AFN_QRY_RECONNECTINTERVAL 0x61    // ��ѯң���ն���·�������
#define AFN_SET_DATRECINTERVAL      0x22    // ����ң���ն���ʷ���ݴ��̼��
#define AFN_QRY_DATRECINTERVAL    0x62    // ��ѯң���ն���ʷ���ݴ��̼��
#define AFN_SET_DATUPLOADINTERVAL   0x23    // ����ң���ն˵�ʵʱ�����ϱ����
#define AFN_QRY_DATUPLOADINTERVAL 0x63    // ��ѯң���ն˵�ʵʱ�����ϱ����
#define AFN_SET_UPDATE      0x24    // ����ң���ն˵���������
#define AFN_QRY_VERINFO   0x64    // ��ѯң���ն˵İ汾��Ϣ
#define AFN_SET_PASSWORD    0x25    // ����ң���ն˵�����
#define AFN_QRY_PASSWORD  0x65    // ��ѯң���ն˵�����
#define AFN_SET_TOWERLIFT   0x26    // ����������������
#define AFN_QRY_TOWERLIFT 0x66    // ��ѯ������������
#define AFN_CHG_FINGER      0x28    // ָ�����ݱ��
#define AFN_DEL_FINGER      0x29    // ɾ��ָ������
                                    //
#define AFN_SET_ELVTINFO    0x30    // ���������������ṹ����
#define AFN_QRY_ELVTINFO  0x70    // ��ѯ�����������ṹ����
                                    //
#define AFN_SET_ELVTFLOOR   0x31    // ����������¥�����
#define AFN_QRY_ELVTFLOOR 0x71    // ��ѯ������¥�����
                                    //
#define AFN_SET_VALVELMT    0x40    // �����ﳾ���߼���ն˵�ŷ���ֵ
#define AFN_QRY_VALVELMT  0x80    // ��ѯ�ﳾ���߼���ն˵�ŷ���ֵ
#define AFN_SET_MANVALVE    0x41    // �����ﳾ���߼���ն˵�ŷ��ֶ�����
#define AFN_SET_VALVELMT_EXT    0x42    // �����ﳾ���߼���ն˵�ŷ���ֵ(����ʹ����PM10)
#define AFN_QRY_VALVELMT_EXT  0x82    // ��ѯ�ﳾ���߼���ն˵�ŷ���ֵ(����ʹ����PM10)

#define AFN_SET_NOTICE      0x44    // �����·�֪ͨ��Ϣ��OUTDOOR LED��

#define AFN_SET_UPLMT       0x45    // ����ж��ƽ̨��ֵ����
#define AFN_QRY_UPLMT       0x85    // ��ѯж��ƽ̨��ֵ����


#define AFN_RESTART 0xA0    // ң���ն�����

#define AFN_SELF_FINGER         0x94    // ָ���ϱ�֡

#define AFN_SELF_REALTIMEDATA   0x90    // ������ʱʵʱ����
#define AFN_SELF_WORKCYCLE      0x91    // ��������ѭ������
#define AFN_SELF_WARNING        0x92    // ��������ʵʱ����
#define AFN_SELF_CALIBRATION    0x98    // �����궨ʵʱ����
                                        //
#define AFN_SELF_ELVTRT         0x9A    // ��������ʱʵʱ����
#define AFN_SELF_ELVTWKLP       0x9B    // ����������ѭ������
#define AFN_SELF_ELVTWARN       0x9C    // ������ʵʱ��������
#define AFN_SELF_ELVTCALI       0x9D    // �������궨ʵʱ����

#define AFN_SELF_DUSTRT         0x96    // �ﳾ���ʵʱ����
#define AFN_SELF_DUSTWARN       0x97    // �ﳾ��ⱨ������

#define AFN_SELF_UPPLATRT       0xA1    // ж��ƽ̨ʵʱ����
#define AFN_SELF_UPPLATWARN     0xA2    // ж��ƽ̨��������

// �������е���·�㹦���붨��

// ����
#define LFN_DIR0_CONFIRM    0x01    // �·�����
#define LFN_DIR0_NORESPON   0x02    // �û�����
#define LFN_DIR0_LNKRESPON     0x03    // ��·����
#define LFN_DIR0_PARAMRESPON   0x04    // �������
#define LFN_DIR0_WARNRESPON    0x13    // ������״̬����

// ����
#define LFN_DIR1_FAIL       0x00    // ����/ʧ�� ������Ӧ֡
#define LFN_DIR1_OK         0x01    // ȷ��/�ɹ� ������Ӧ֡
#define LFN_DIR1_DENYRESPON    0x02    // ��Ӧ֡ ����/�����ٻ�������
#define LFN_DIR1_QUERYRESPON   0x03    // ��ѯ/��Ӧ֡ ��·״̬
#define LFN_DIR1_PARAMRESPON   0x04    // ��Ӧ֡ ��Ӧ����
#define LFN_DIR1_TIMINGSTATUS  0x05    // ��ʱ�Ա�֡ ��Ӧ״̬
#define LFN_DIR1_RANDOMWORK    0x06    // ����Ա�֡ ����ѭ��
#define LFN_DIR1_RANDOMWARN    0x07    // ����Ա�֡ ����
#define LFN_DIR1_RANDOMFAULT   0x08    // ����Ա�֡ ����
#define LFN_DIR1_RANDOMILLEGAL 0x09    // ����Ա�֡ Υ��

// �������е���·�㹦���붨�����

//
#define STARTCHAR   0x68
#define ENDCHAR     0x16
#define VERSION     0x01

// data field define
#define DATAFIELD_LOGIN 0xF0
#define DATAFIELD_LOGOUT 0xF1
#define DATAFIELD_ONLINE 0xF2

// return status
#define TIP_FAIL    0
#define TIP_OK      1
#define RESEND_INTERVAL 5000    // �ط����Ĭ��ֵ��5000ms
#define RESEND_TIMES    3       // ʧ���ط�����Ϊ3��

// protocol sensor type
#define PRO_SENS_WIND       1       // wind on frequence
#define PRO_SENS_ROTAT      2       // rotation of tower
#define PRO_SENS_MARGIN     3       // margin on PWM
#define PRO_SENS_HEIGHT     4       // height on PWM
#define PRO_SENS_WEIGHT     5       // weight  on PWM
#define PRO_SENS_TORQUE     6       // torque of tower
#define PRO_SENS_TILT       8       // tilt of tower
#define PRO_SENS_WALK       9       // walk on PWM
#define PRO_SENS_SPEED      10      // speed of elivator
#define PRO_SENS_PEOPLE     13      // people number of elivator
#define PRO_SENS_MOTOR1     14      // motor 1
#define PRO_SENS_MOTOR2     15      // motor 2
#define PRO_SENS_MOTOR3     16      // motor 3
#define PRO_SENS_FINGER     17      // finger
#define PRO_SENS_PM25       18      // PM2.5
#define PRO_SENS_PM10       19      // PM10
#define PRO_SENS_TEMP       20      // temperature
#define PRO_SENS_HMDT       21      // humidity
#define PRO_SENS_NOISE      22      // noise
#define PRO_SENS_UPWEIGHT   23      // uploading platform weight
#define PRO_SENS_UPLEFT     24      // uploading platform left force
#define PRO_SENS_UPRIGHT    25      // uploading platform right force
// struct define

// ���û����ṹ
typedef struct
{
    uint8_t STAT_PRINT_BUSY:1;  //
    uint8_t STAT_GPRSSEND_BUSY:1;   // 0,enable sending frame��1,disable sending frame
    uint8_t STAT_FNGRSEND_BUSY:1;
    uint8_t STAT_NO_INIT:1;     // 0,tip_init run ok,1,tip_init run fault
    uint8_t STAT_GPRS_ZPPP:1;   // 0,GPRS connected;1,GPRS disconnected
    uint8_t STAT_LINK_LOG:1;    // 0,GRRS login;1,GPRS logout
    uint8_t STAT_LINK_OK:1;     // 0, Link logout;1,Link login success
    uint8_t STAT_HEART_OV:1;    // 0,disable send heart frame;1,enable send heart frame;
    uint8_t STAT_CSQ_CHK:1;     // 0,disable send csq frame;1,enable send check signal quarlity
    uint8_t rev:4;          // ����
    uint32_t resend_start;  // ��ʼ����
    uint32_t resend_interval;// ��ʱ���(ms)
    uint8_t resend_times;   // ʧ���ط�����
    uint32_t mframecnt;     // ��֡������
    uint8_t RTDATA_FLAG;    // ��ʱʵʱ�����ϴ���־ 0 ���� �� 1 ���� ��
    uint8_t WORKCYCLE_FLAG; // ����ѭ�������ϴ���־ 0 ���� �� 1 ���� ��
    uint8_t WARNING_FLAG;   // �澯ʵʱ�����ϴ���־ 0 ���� �� 1 ���� ��
    uint8_t CALIBRATE_FLAG; // �궨ʵʱ�����ϴ���־ 0 ���� �� 1 ���� ��
    uint8_t STAT_WAIT_ACK;  // ����������֡����λ���ȴ�Ӧ���־
}StatFlag,*pStatFlag;
_EXTERN StatFlag StatusFlag;

// 0~99
typedef struct
{
    uint8_t single:4;   // BCD���λ
    uint8_t digit:4;    // BCD��ʮλ
}BCD_dig,*pBCD_dig;

// -9999.9~+9999.9
//������Ϊ��ǧλBCD�룬��xxxx.x,���޷���λ���øýṹ��Ϊ����λ�Ǻ��Է���λֵ
typedef struct
{
    uint8_t thousand:4;     // ǧλ
    uint8_t symbol:4;       // ����λ
    uint8_t digit:4;        // ʮλ
    uint8_t handred:4;  // ��λ
    uint8_t dot:4;          // С��λ
    uint8_t single:4;       // ��λ
}BCD_th,*pBCD_th;

// -999.9 ~ +999.9
//������Ϊ��ǧλBCD�룬��xxx.x
typedef struct
{
    uint8_t digit:4;        // ʮλ
    uint8_t handred:4;  // ��λ
    uint8_t dot:4;          // С��λ
    uint8_t single:4;       // ��λ
}BCD_hun,*pBCD_hun;

// ������
typedef struct
{
    uint8_t func:4; // �����붨��
    uint8_t fcb:2;  // ֡����λ���� ��ֹ��Ϣ����Ķ�ʧ���ظ�
    uint8_t div:1;  // ��ֱ�־λ���� 1��ʾ�˱����Ѿ�����ֳ�����֡,
                    // ��ʱ�����������4�ֽ���Ϊ��ּ���֡��
                    // ǰ�����ֽ�Ϊ���֡����,�����ֽڱ�ʾ��ǰ֡��
                    // ����BIN������(65535~1),1��ʾ���һ֡
    uint8_t dir:1;  // ���䷽��λ���� 0���� 1����
}CtrlZone,*pCtrlZone;

// ��ַ��
typedef struct
{
    uint8_t a1_low;  // ���������� A1 BCD ���� 2Bytes
    uint8_t a1_high;
    uint8_t a2_low;  // �նˡ��м̵�ַ A2 BIN ���� 3Bytes
    uint8_t a2_middle;
    uint8_t a2_high;
}AddrZone,*pAddrZone;

// Э��汾
typedef struct
{
    uint8_t firstver:4;    // �ΰ汾
    uint8_t secondver:4;     // ���汾
}VerNo,*pVerNo;

// ������
typedef struct
{
    uint8_t key_low;      //12bit��Կ,BCD����,ȡֵ��Χ0~999
    uint8_t key_high:4;
    uint8_t key_alg:4;    // ��Կ�㷨,BCD����,ȡֵ0~9
}KeyZone,*pKeyZone;

// ʱ���ǩ
typedef struct     // BCD ʱ�ӱ�ǩ
{
    uint8_t sec_single:4;    // �� ��λ��
    uint8_t sec_digit:4;    // �� ʮλ��
    uint8_t min_single:4;    // �� ��λ��
    uint8_t min_digit:4;    // �� ʮλ��
    uint8_t clk_single:4;    // ʱ ��λ��
    uint8_t clk_digit:4;    // ʱ ʮλ��
    uint8_t dat_single:4;    // �� ��λ��
    uint8_t dat_digit:4;    // �� ʮλ��
    uint8_t delay;          // �����ʹ�����ʱʱ��
}TimeLabel,*pTimeLabel;

// ������Ϣ��
typedef struct
{
    KeyZone key_zone;       // ��������2bytes
    TimeLabel time_label;   // ʱ���ǩ����5bytes
}AuxInfo,*pAuxInfo;

// Ӧ����
typedef struct
{
    uint8_t functioncode;   // Ӧ�ò㹦���� 1byte
    void *userdata;         // Ӧ�ò��û�����
    //AuxInfo auxinfo;      // Ӧ�ò㸽����Ϣ 7byte ���ݸ���userdata����ĩβ
}AppZone,*pAppZone;

// ̩����������Э��֡�ṹ ��ҵ��׼
#define MAX_SIZE 248
typedef struct
{
    uint8_t startb1;    // ��ʼ�ַ�0x68
    uint8_t length;     // ���ȣ��ӿ�����У��֮ǰ�������ֽ�
    uint8_t startb2;    // ��ʼ�ַ�0x68
    CtrlZone ctrlzone;  // ������ 1byte
    AddrZone addrzone;  // ��ַ�� 5bytes
    VerNo version;      // �汾��0.0~15.15 1byte
    AppZone appzone;    // Ӧ������ (8+?)Bytes
    uint8_t cs;         // CRCУ���� 1byte
    uint8_t endbyte;    // �����ַ�0x16 1byte
}TX101,*pTX101;
// ����һ��ȫ�ֽṹ�ͻ�������Ϊ����֡��Ŵ�
// �ýṹ��ʼ���ڴ������֡�����������Ϊ����֡����
_EXTERN TX101 rtn;
_EXTERN uint8_t rtnbuf[MAX_SIZE];    //�û���������ȥAFN�248Bytes

// ��֡�ṹ
typedef struct
{
    uint8_t startb1;    // ��ʼ�ַ�0x68
    uint8_t length;     // ���ȣ��ӿ�����У��֮ǰ�������ֽ�
    uint8_t startb2;    // ��ʼ�ַ�0x68
    CtrlZone ctrlzone;  // ������ 1byte
    uint16_t framenum;  // ��֡��
    uint16_t framecnt;  // ��֡������
    AddrZone addrzone;  // ��ַ�� 5bytes
    VerNo version;      // �汾��0.0~15.15 1byte
    AppZone appzone;    // Ӧ������ (8+?)Bytes
    uint8_t cs;         // CRCУ���� 1byte
    uint8_t endbyte;    // �����ַ�0x16 1byte
}MultiTX101,*pMultiTX101;

#define MAX_MFRAME 16
typedef struct
{
    uint8_t mf_flag;    // ��֡��־��0���޶�֡1���ж�֡
    uint16_t mframe_num;    // ��֡��
    uint16_t mframe_cnt;    // ��ǰ�յ���֡��
    uint16_t mframe_st; // ��֡״̬λ����ʼ��Ϊ�㣩��û���յ�һ֡��֡��Ϣ����Ӧ��λ��Ϣ��һ
                        // eg����֡��5����ֻ���ж�mframe_st����λȫΪ�߲ű�ʾ��֡����
    MultiTX101 frame[MAX_MFRAME];   // Ԥ��ʮ��֡
    uint8_t mlen[MAX_MFRAME];   // ÿ֡Ӧ�ó���������ռ�û���������
}MF_TX101,*pMF_TX101;

_EXTERN MF_TX101 mrtn;
_EXTERN uint8_t mrtnbuf[MAX_MFRAME][MAX_SIZE];  // ��֡Ӧ�ò����ݻ����������16֡
_EXTERN uint8_t mrtnbuffer[MAX_MFRAME*MAX_SIZE];    // ��֡�ϲ���Ӧ��������

typedef union taixin_protocol_union
{
    TX101 tx101;
}TX101UNION;

// ϵͳ�����ṹ���������б�������Ҫ���õĽṹ����
// ��ʱ���壬�滻ʱע��


// ʱ���ǩ
typedef struct     // BCD ʱ�ӱ�ǩ
{
    uint8_t sec;    // ��
    uint8_t min;    // ��
    uint8_t hour;    // ʱ
    uint8_t date;    // ��
    uint8_t mon_single:5;   // ��
    uint8_t week_single:3;  // ����
    uint8_t year;           // �� BCD���λ
}RtcClk,*pRtcClk;

// ����������
typedef struct
{
    uint8_t sensornum;      // ����������
    uint8_t sensortype[255];// �������������ݣ��ж����������ж��ٸ�
}SensorType,pSensorType;

// ����������
typedef struct
{
    uint8_t sensortype;     // ����������
    uint8_t sensorpara[2];  // ��������������Ϊ���ִ������������2Byte��ȡ���
}SensorCfg,pSensorCfg;

// ���ٴ�����
typedef struct
{
    uint8_t pulse_high;     // ���峣�����ֽ�
    uint8_t pulse_low;      // ���峣�����ֽ�
}SensorWind,pSensorWind;

// ��ת������
typedef struct
{
    uint8_t resolution:3;   // �ֱ���
    uint8_t rotationdir:1;  // ת��
    uint8_t rev:1;          // ����
    uint8_t factor:1;       // ����
    uint8_t interfacemode:2;// �ӿڷ�ʽ

}SensorRotary,*pSensorRotary;

// ���ش�����
typedef struct
{
    uint8_t rev:6;  // ����
    uint8_t interfacemode:2;    // �ӿڷ�ʽ
}SensorWeigh,*pSensorWeigh;

// �ն˵���λ��/��γ��
typedef struct
{
    uint8_t rev1;
    uint8_t lo_degree_single:4; // "��" ��λ
    uint8_t lo_degree_digit:4;  // "��" ʮλ
    uint8_t lo_min_single:4;    // "��" ��λ
    uint8_t lo_min_digit:4; // "��" ʮλ
    uint8_t lo_min_hundredth:4;// "��" �ٷ�λ
    uint8_t lo_min_digith:4;    // "��" ʮ��λ
    uint8_t lo_min_miriade:4;   // "��" ���λ
    uint8_t lo_min_thousandth:4;    // "��" ǧ��λ
    uint8_t longitude;          // ���� "N" or "S"
    uint8_t la_degree_hundred:4;    // "��"
    uint8_t la_degree_single:4; // "��"  ��λ
    uint8_t la_degree_digit:4;  // "��"  ʮλ
    uint8_t la_min_single:4;    // "��"  ��λ
    uint8_t la_min_digit:4; // "��"  ʮλ
    uint8_t la_min_hundredth:4;// "��"  �ٷ�λ
    uint8_t la_min_digith:4;    // "��"  ʮ��λ
    uint8_t la_min_miriade:4;   // "��"  ���λ
    uint8_t la_min_thousandth:4;    // "��" ǧ��λ
    uint8_t latitude;       // γ�� "E" or "W"
    uint8_t altitude_hundred:4;     // ���� ��λ
    uint8_t altitude_thousand:4;    // ���� ǧλ
    uint8_t altitude_single:4;      // ���� ��λ
    uint8_t altitude_digit:4;       // ���� ʮλ
}Location,*pLocation;

// ����������������
typedef struct
{
    uint8_t name[16];   // �������ƣ�16���ֽڣ�ASCII�ַ�����������㣬�����ֽ���\0���
    uint8_t ID;         // ����ID��1���ֽڣ�����������Ⱥ�ı�ţ���Χ0-63
    uint8_t IDs;        // ��ȺID��1���ֽڣ������ڵ�ǰ��Ⱥ�еı�ţ���Χ0-62��63���ڵ���
    BCD_th x;           // ����X��3���ֽڣ���0.1��Ϊ��λ����Χ-999.9-9999.9��
    BCD_th y;           // ����Y��3���ֽڣ���0.1��Ϊ��λ����Χ-999.9-9999.9��
    uint8_t typecode[2];    // �����ͺŴ��룬2���ֽ�
    uint8_t ratedload;      // ����أ�1���ֽڣ���0.1��Ϊ��λ����Χ0.0��25.5��
    uint8_t type_scales;    // ��������/���ʣ�1���ֽ�

    BCD_th length_forearm;  // ǰ�۳��ȣ�3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��
    BCD_th length_backarm;  // ��۳��ȣ�3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��
    BCD_th frontbar_location1;  // ǰ����1λ�ã�Զ�����ˣ���3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��
    BCD_th frontbar_location2;  // ǰ����2λ�ã��������ˣ���3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��
    BCD_th backbar_location;    // ������λ�ã�3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��
    BCD_th towerbar_loweredge_heigh;    // �������ظ߶ȣ�3���ֽڣ�0.1��Ϊ��λ����Χ0��-9999.9��

    BCD_hun towerbar_heigh[2];      // ��������߶ȣ�2���ֽڣ�0.1��Ϊ��λ����Χ0��-99.9��
    BCD_hun towerbar_heigh_ltot[2]; // �������ص�����߶ȣ�2���ֽڣ�0.1��Ϊ��λ����Χ0��-99.9��
    uint8_t rotary_inertia_coef;    // ��ת����ϵ����1���ֽڣ���Χ0 - 99
    BCD_hun walk_angle;         // ���߽Ƕȣ�2���ֽڣ�0.1��Ϊ��λ����Χ0.0-359.9��
}TowPara,*pTowPara;

// ��������������Ԫ����Ϣ

// ��
typedef struct
{
    uint8_t etype;  // ������Ԫ������
    //X���� BCD �� unit:0.1m -9999.9~9999.9m
    BCD_th x;   // X����ֵ
                            //
    //Y���� BCD �� unit:0.1m -9999.9~9999.9m
    BCD_th y;   // Y����ֵ
}EPoint,*pEPoint;

// Բ��
typedef struct
{
    // Բ������
    // Բ������X��3���ֽڣ���0.1��Ϊ��λ����Χ-9999.9-9999.9��
    BCD_th centrey_x;

    // Բ������Y��3���ֽڣ���0.1��Ϊ��λ����Χ-9999.9-9999.9��
    BCD_th centrey_y;

    // Բ�뾶��3���ֽڣ���0.1��Ϊ��λ����Χ0-9999.9��
    BCD_th radius;

}EArc,*pEArc;

// ��������Ϣ�����Ȳ�������ʽ����
typedef struct
{
    uint8_t p_type; // ���������ͣ�1���ֽڣ�0����������1���ϰ���
    uint8_t p_id;   // ��������ţ�1���ֽڡ�ͬһ���أ�ÿ����������Ψһ���
    uint8_t p_building; // �������������ͣ�1���ֽڣ�0��������1��ҽԺ��2��ѧУ��3���㳡��4����·��5����������6���칫����7����ѹ�ߣ��������ʹ�����ֻ�Խ�������Ч��
    BCD_th p_high;  // �������߶ȣ�3���ֽ�BCD�롣ֻ���ϰ�����Ч
    uint8_t p_K;    // ������Ԫ�ظ���K��1���ֽ�
    uint8_t etype;  // Ԫ�����ͣ�1���ֽڣ�0x00���㣻0x01��Բ����������
    // ������Ԫ������
    uint8_t *edata; // Ԫ��������
}ProtectInfo,*pProtectInfo;

// ������
typedef struct
{
    uint8_t pz_num; // ����������

    //pz_num�� ��������Ϣ
    //ProtectInfo protect_info;
    // �������ı�����Ԫ������
    /*Ԫ�������� */
    uint8_t ptr_pz; // ��Ϊ��֪�����ȣ���ʱ����һ��ָ�����
}ProtectZone,*pProtectZone;

// ������λ��Ϣ
typedef struct
{
    BCD_th left;        // ����λ��3���ֽ�BCD�룬����Χ-9999.9-9999.9��
    BCD_th right;       // ����λ��3���ֽ�BCD�룬����Χ-9999.9-9999.9��
    BCD_hun rotaryprewarn1; // ��ת��λԤ��ֵ��2�ֽ�BCD�룬��Χ0.0-999.9��
    BCD_th top;         // ����λ��3���ֽ�BCD�룬��Χ-9999.9-9999.9��
    BCD_th bottom;      // ����λ��3���ֽ�BCD�룬��Χ-9999.9-9999.9��
    BCD_hun topprewarn1;    // �߶���λԤ��ֵ��2�ֽ�BCD�룬��Χ0.0-999.9��
    BCD_hun far;        // Զ��λ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun near;       // ����λ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun scopeprewarn1;  // ������λԤ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_th forward;     // ǰ��λ��3���ֽ�BCD�룬��Χ-9999.9-9999.9��
    BCD_th backward;    // ����λ��3���ֽ�BCD�룬��Χ-9999.9-9999.9��
    BCD_hun walkprewarn1;// ������λԤ��ֵ��2�ֽ�BCD�룬��Χ0.0-999.9��
    BCD_hun dipprewarn; // ���Ԥ��ֵ��2�ֽ�BCD�룬��Χ0.0-99.9��
    BCD_hun dipwarn;        // ��Ǳ���ֵ��2�ֽ�BCD�룬��Χ0.0-99.9��
    BCD_hun windprewarn;// ����Ԥ��ֵ��2�ֽ�BCD�룬��Χ0.0-999.9m/s
    BCD_hun windwarn;// ���ٱ���ֵ��2�ֽ�BCD�룬��Χ0.0-999.9m/s
    uint8_t rev1[12];       // Ԥ��12Bytes
    BCD_hun rotaryprotect;  // ��ת����ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun rotaryprewarn2; // ��תԤ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun topprotect;     // �߶ȱ���ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun topprewarn2;        // �߶�Ԥ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun scopeprotect;       // �߶ȱ���ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun scopeprewarn2;      // �߶�Ԥ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun walkprotect;        // �߶ȱ���ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun walkprewarn2;       // �߶�Ԥ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun towerprotect;       // �߶ȱ���ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    BCD_hun towerprewarn;       // �߶�Ԥ��ֵ��2���ֽ�BCD�룬��Χ0��-999.9��
    uint8_t rev2[12];       // Ԥ��12Bytes
}TowLimit,*pTowLimit;

// ������������
typedef struct
{
    uint8_t rate;   // ����
    uint8_t n;      // �������ߵ���N

    /*��һ�㵽��N��ķ��Ⱥ�����ֵ */
    // ����Ϊ2�ֽ�BCD������Ϊ3�ֽ�BCD��ÿ��������ֽ�
    // ���ȸ�ʽ���£���Χ0.0-999.9��
    // ������ʽ���£���Χ0.0000-99.9999��
    uint8_t *ptr;
}TowMoment,*pTowMoment;

// �����궨����
typedef struct
{
    SensorType sensortype;  // ����������
    uint8_t n;              // �궨���ݵ���N

    /*
    BCD_th original1;       // ��һ��ԭʼֵ
    BCD_th field1;          // ��һ�㹤����
    */
    // �˴�����N�����ԭʼֵ�͹�����
    uint8_t *ptr;
}TowCalibratParam,*pTowCalibratParam;

// ң���ն˴洢������վIP��ַ�Ͷ˿ں�
typedef struct
{
    uint8_t port1[2];   // �˿ں�,��λ��ǰ
    uint8_t ip1[4];     // IPv4��ַ,��λ��ǰ

    uint8_t port2[2];   // �˿ں�,��λ��ǰ
    uint8_t ip2[4];     // IPv4��ַ,��λ��ǰ

    uint8_t port3[2];   // �˿ں�,��λ��ǰ
    uint8_t ip3[4];     // IPv4��ַ,��λ��ǰ

    uint8_t port4[2];   // �˿ں�,��λ��ǰ
    uint8_t ip4[4];     // IPv4��ַ,��λ��ǰ

}IPandPort,*pIPandPort;

// �汾��Ϣ
typedef struct
{
    VerNo fireware; // ����汾
    VerNo protocol; // Э��汾
    VerNo MMI;      // MMI�汾
}VerInfo,*pVerInfo;

// ����
typedef struct
{
    uint8_t pwtype; // �������� �������ͣ�1�ֽڡ�
                    //0x00���û���ѯ���룻
                    //0x01���û��������룻
                    //0x08����������
                    //0x09�������û���
                    //������������
    uint8_t pwlen;  // ���볤��N��1B����������ĳ��ȣ�����������������Χ0��255

    // �����ֽڣ�ASCII���ַ������ԡ�/0����β
    uint8_t *ptr;
}PW,*pPW;

// ��Ҫ����Ĳ���
typedef struct
{
    AddrZone Taddr;     // �ն˵�ַ
    RtcClk  clk;            // ʱ��
    uint8_t workmode;   // ����ģʽ
    SensorType sensortype;  // ����������
    SensorCfg sensorcfg;    // ����������
    Location location;  // λ��/��γ��
    TowPara towerparam; // ���������ṹ����
    ProtectZone pz;     // ��������Ϣ
    TowLimit towerlimit;    // ������λ��Ϣ
    TowMoment towermoment;  // ������������
    TowCalibratParam towercalibpara;    //�����궨����
    IPandPort ipandport;    // ����վIP��ַ�Ͷ˿ں�
    BCD_dig heartinterval;  // 1�ֽڣ�ѹ����BCD�룩Ϊ�������ʱ�䣺 ȡֵ��ΧΪ��0��99����λ�����ӡ�����Ĭ��ֵΪ��10����
    BCD_hun reconnectinterval;  // 2�ֽ�ѹ����BCD�룬��·�����Ӽ��ʱ�䣬 ȡֵ��ΧΪ0��9999����λ�����ӡ�����Ĭ��ֵΪ��120����
    BCD_hun saveinterval;        // ������2�ֽڣ�ѹ����BCD�룩Ϊ��ʷ���ݴ��̼��ʱ�䣺 ȡֵ��ΧΪ��0��9999����λ���롣����Ĭ��ֵΪ��10��
    BCD_hun updatainterval;      // 2�ֽڣ�ѹ����BCD�룩Ϊʵʱ�����ϱ����ʱ�䣺 ȡֵ��ΧΪ��0��9999����λ���롣����Ĭ��ֵΪ��30��
    VerInfo versioninfo;    // �汾��Ϣ
    PW password[4];         // Ŀǰ����������
    uint8_t towertoplift;       // ������1�ֽڣ���ʾ���ζ����߶ȣ���Χ0.0��25.5��
}SysPara,*pSysPara;

// ����������
typedef struct
{
    // ������״̬
    uint8_t sensor_OK:1;    // OK������������״̬��0����Ч��1��������
    uint8_t sensor_EN:1;    // EN��������ʹ��״̬��0�����ܣ�1��ʹ�ܣ�
    uint8_t rev1:6;         // ����
    uint8_t warning;        // ����״̬������
                            //0x00���ޱ���������������������
                            //0x20���ӽ����������������������������
                            //0x60�������������� ������������������
                            //0xE0:���س���

    BCD_th data;            // ������3�ֽ�BCD�룬0.0-9999.9��
}SensorDat,*pSensorDat;

// ��������ṹ����ײ���ϰ����������
typedef struct
{
    uint8_t ID; // �Է�����ID��0��ʾ�ޱ���
    uint8_t low:2;      // ����ײ
    uint8_t rev1:2;     // ����
    uint8_t right:2;    // ����ײ
    uint8_t left:2;     // ����ײ

    uint8_t Inttype:2;  // �������͡�01H�����汾��������,02H�����汾������˿��
    uint8_t rev2:2;     // ����
    uint8_t near:2;     // ����ײ
    uint8_t far:2;      // Զ��ײ
}WarningCode,*pWarningCode;

// ��ʱ�ϴ�ʵʱ������
typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint8_t packtype[2];    // �����ʹ���
    SensorDat gravitylift;  // ������
    SensorDat height;       // �߶�
    SensorDat scope;        // ����
    SensorDat rotary;       // ��ת�Ƕ�
    SensorDat wind;         // ����
    SensorDat dipangle;     // ���
    SensorDat moment;       // ���ر���
    SensorDat walk;         // ����
    SensorDat rev1[4];      // Ԥ��

    // ����״̬
    WarningCode collision;  //��ײ���루3B��
    WarningCode obstacle;   // �ϰ���
    WarningCode forbidden;  //������3B
    WarningCode rev2[4];    // Ԥ��3BX4

    uint8_t rev3[6];        // Ԥ��6B
}DatRealT,*pDatRealT;

// �����Զ��ϴ����Ľṹ
_EXTERN DatRealT realtimedata;  // ��ʱʵʱ����

// �������ϴ�ʵʱ������
typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // �����ʹ���
    SensorDat gravitylift;  // ������
    SensorDat height;       // �߶�
    SensorDat speed;        // �ٶ�
    SensorDat wind;         // ����
    SensorDat dipangle;     // ���
    SensorDat motor[3];     // ���һ��������

    uint8_t people_flag;
    uint8_t people_alarm;
    uint8_t people_value;

    uint8_t floor_flag;
    uint8_t floor_aligned;
    uint8_t floor_value;

    uint8_t door_limit;// �ż���λ״̬

    uint8_t rev[19];    // ����

    uint8_t rev1[6];    // ����

}RTDatElvt,*pRTDatElvt;

// �����Զ��ϴ����Ľṹ
_EXTERN RTDatElvt elvtdata; // ��������ʱʵʱ����


// �ﳾ���߼���ϴ�ʵʱ������
typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // �����ʹ���
    SensorDat pm25;     // pm25
    SensorDat pm10;     // pm10
    SensorDat temperature;  // �¶�
    SensorDat humidity;     // ʪ��
    SensorDat wind;         // ����
    SensorDat valve[4];     // ��ŷ�һ������������
    SensorDat noise;        // ����
    SensorDat vane;     // ����
    uint8_t rev[10];    // ����

    uint8_t rev1[6];    // ����

}RTDatDust,*pRTDatDust;
_EXTERN RTDatDust dustdata; // �ﳾ���߼�ⶨʱʵʱ����

// ж��ƽ̨���ؼ���ϴ�ʵʱ������
typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // �����ʹ���
    SensorDat cableleft;     // б����һ
    SensorDat cableright;    // б������
    SensorDat weightsensor[16];  // ���ش�����1~16
    SensorDat weight;     // ж��ƽ̨�ܳ���

    uint8_t rev[20];    // ����

    uint8_t rev1[6];    // ����

}RTDatUPPlat,*pRTDatUPPlat;
_EXTERN RTDatUPPlat upplatdata; //ж��ƽ̨�������߼�ⶨʱʵʱ����

// ��������ѭ������
typedef struct
{
    uint8_t workcyclenum[2];    // ����ѭ����ţ�2���ֽڣ����ֽ���ǰ�����ֽ��ں�
    uint32_t name_id;
    RtcClk starttime;           // ��ʼʱ��
    RtcClk endtime;             // ����ʱ��
    BCD_th maxlift;             // ������أ�3���ֽڣ�0-9999.9��
    BCD_th maxheight;           // ���߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    #ifdef TOWERBOX
    BCD_th maxmoment;           // ������أ�3���ֽڣ�0-9999.9��*��
    BCD_th minheight;           // ��С�߶ȣ�3���ֽڣ�-9999.9 - 9999.9��
    BCD_th maxscope;            // �����ȣ�3���ֽڣ�-9999.9 - 9999.9��
    BCD_th minscope;            // ��С���ȣ�3���ֽڣ�-9999.9 - 9999.9��
    BCD_th maxrotary;           // ����ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    BCD_th minrotary;           // ��С��ת�Ƕȣ�3���ֽڣ�-9999.9 - 9999.9��
    BCD_th maxwalk;             // ������߾��룬3���ֽڣ�-9999.9 - 9999.9��
    BCD_th minwalk;             // ��С���߾��룬3���ֽڣ�-9999.9 - 9999.9��
    BCD_th liftpointangle;      // �����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    BCD_th liftpointscope;      // �������ȣ�3���ֽڣ�0-9999.9��
    BCD_th liftpointheight;     // �����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    BCD_th unloadpointangle;    // ж����Ƕȣ�3���ֽڣ�-9999.9-9999.9��
    BCD_th unloadpointscope;    // ж������ȣ�3���ֽڣ�0-9999.9��
    BCD_th unloadpointheight;   // ж����߶ȣ�3���ֽڣ�-9999.9-9999.9��
    #endif
    #ifdef ELIVATOR
    uint8_t maxpeople;
    uint8_t maxfloor;
    #endif
}DatWork,*pDatWork;

_EXTERN DatWork TowWklpDat;     // ��������ѭ������
_EXTERN DatWork ElvtWklpDat;    // ����������ѭ������

// ����ʵʱ������
#define WARN_MAX_SIZE 32
#define ILGL_MAX_SIZE 32
#define FLT_MAX_SIZE 62

typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint8_t packtype[2];    // �����ʹ���
    SensorDat gravitylift;  // ������
    SensorDat height;       // �߶�
    SensorDat scope;        // ����
    SensorDat rotary;       // ��ת�Ƕ�
    SensorDat wind;         // ����
    SensorDat dipangle;     // ���
    SensorDat moment;       // ���ر���
    SensorDat walk;         // ����
    SensorDat rev1[4];      // Ԥ��4bytes
    uint8_t status;         // ״̬��
    // ����״̬
    uint8_t warning[WARN_MAX_SIZE]; // ������Ϣ����
    uint8_t illegal[ILGL_MAX_SIZE]; // Υ����Ϣ����
    uint8_t fault[FLT_MAX_SIZE ];       // ������Ϣ����
    uint8_t rev2[6];    // Ԥ��6bytes
}DatWarn,*pDatWarn;

_EXTERN DatWarn warningdata;    // ����ʵʱ����


typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // �����ʹ���
    SensorDat pm25;     // pm25
    SensorDat pm10;     // pm10
    SensorDat temperature;  // �¶�
    SensorDat humidity;     // ʪ��
    SensorDat wind;         // ����
    SensorDat valve[4];     // ��ŷ�һ������������
    SensorDat noise;        // ����
    SensorDat vane;     // ����

    uint8_t rev[10];    // ����

    uint8_t rev1[6];    // ����
    uint8_t status;         // ״̬��
    // ����״̬
    uint8_t warning[WARN_MAX_SIZE]; // ������Ϣ����
    uint8_t illegal[ILGL_MAX_SIZE]; // Υ����Ϣ����
    uint8_t fault[FLT_MAX_SIZE ];       // ������Ϣ����
    uint8_t rev2[6];    // Ԥ��6bytes
}DatDustWarn,*pDatDustWarn;

_EXTERN DatDustWarn DustWrnData;    // �ﳾ����ʵʱ����

typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // �����ʹ���
    SensorDat weight;     // ж��ƽ̨�ܳ���
    SensorDat cableleft;     // б����һ
    SensorDat cableright;    // б������
    SensorDat weightsensor[16];  // ���ش�����1~16

    uint8_t rev[20];    // ����

    uint8_t rev1[6];    // ����
    uint8_t status;         // ״̬��
    // ����״̬
    uint8_t warning[WARN_MAX_SIZE]; // ������Ϣ����
    uint8_t illegal[ILGL_MAX_SIZE]; // Υ����Ϣ����
    uint8_t fault[FLT_MAX_SIZE ];   // ������Ϣ����
    uint8_t rev2[6];    // Ԥ��6bytes
}DatUPPlatWarn,*pDatUPPlatWarn;

_EXTERN DatUPPlatWarn UPPlatWrnData;    // ж��ƽ̨ʵʱ����

typedef struct
{
    RtcClk clock;   // ʱ��BCD��6bytes
    uint32_t name_id;       // ��Ա��ʶ��
    uint8_t packtype[2];    // �����ʹ���
    SensorDat gravitylift;  // ������
    SensorDat height;       // �߶�
    SensorDat speed;        // �ٶ�
    SensorDat wind;         // ����
    SensorDat dipangle;     // ���
    SensorDat motor[3];     // ���һ��������

    uint8_t people_flag;
    uint8_t people_alarm;
    uint8_t people_value;

    uint8_t floor_flag;
    uint8_t floor_aligned;
    uint8_t floor_value;

    uint8_t door_limit;// �ż���λ״̬

    uint8_t rev[19];    // ����

    uint8_t rev1[6];    // ����
    uint8_t status;         // ״̬��
    // ����״̬
    uint8_t warning[WARN_MAX_SIZE]; // ������Ϣ����
    uint8_t illegal[ILGL_MAX_SIZE]; // Υ����Ϣ����
    uint8_t fault[FLT_MAX_SIZE ];       // ������Ϣ����
    uint8_t rev2[6];    // Ԥ��6bytes
}DatElvtWarn,*pDatElvtWarn;

_EXTERN DatElvtWarn ElvtWrnData;    // ����������ʵʱ����

// �궨ʵʱ����
typedef struct
{
    RtcClk clock;           // ʱ��BCD��6bytes
    uint8_t packtype[2];    // �����ʹ���
    SensorDat gravitylift;  // ������
    SensorDat height;       // �߶�
    SensorDat scope;        // ����
    SensorDat rotary;       // ��ת�Ƕ�
    SensorDat wind;         // ����
    SensorDat dipangle;     // ���
    //SensorDat moment;     // ���ر���
    SensorDat walk;         // ����
    SensorDat rev1[4];      // Ԥ��4bytes
}DatCalib,*pDatCalib;

_EXTERN DatCalib TowCaliDat;    // ������ϻ�ӱ궨ʵʱ����
_EXTERN DatCalib ElvtCaliDat;   // �������궨ʵʱ����

//====== elvt information

#define ELVT_MAXDAT 2       // maxim size of elevator table
typedef struct
{
    float height;       // Floor height
    uint16_t floor;         // floor number
}
APP_ELVTDAT_TypeDef;    // data buffer strcture of elevator
                            // 4B@20150923
// ָ���ϱ����ݽṹ
typedef struct
{
    uint8_t flag;       //
    uint32_t staffid;   // ָ��������Ա��ʶ��4bytes
}FngrDat,*pFngrDat;

_EXTERN FngrDat fingerdata;


// elevator data
typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of buffer, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------

    uint16_t sn;            // serial number

    uint8_t sec;            // time in BCD
    uint8_t min;
    uint8_t hour;
    uint8_t date;
    uint8_t month;      // weekday 1-7 in bit[7:5]
    uint8_t year;           // 20xx

    uint16_t attrib;        // reserved

    uint8_t weight_flag;
    uint8_t weight_alarm;
    float weight_value;

    uint8_t height_flag;
    uint8_t height_alarm;
    float height_value;

    uint8_t speed_flag;
    uint8_t speed_alarm;
    float speed_value;

    uint8_t wind_flag;
    uint8_t wind_alarm;
    float wind_value;

    uint8_t tilt_flag;
    uint8_t tilt_alarm;
    float tilt_value;

    uint8_t motor1_flag;
    uint8_t motor1_alarm;

    uint8_t motor2_flag;
    uint8_t motor2_alarm;

    uint8_t motor3_flag;
    uint8_t motor3_alarm;

    uint8_t people_count;   // ����

    int8_t floor;   // ¥��

    uint8_t stat_low;   // ����λ״̬
    uint8_t stat_high;  // ����λ״̬
    uint8_t stat_outdoor;   // ������״̬
    uint8_t stat_indoor;    // ������״̬
    uint8_t warn_outdoor;   // �����ű���
    uint8_t warn_indoor;    // �����ű���

    //-------------- end flag section --------------
    float end;          // last object of structure
}
APP_ELVTVALUE_TypeDef;


_EXTERN APP_ELVTVALUE_TypeDef elvt_value;   // elevator send to server
_EXTERN APP_ELVTVALUE_TypeDef elvtsave_value;   // perdical save into NAND FLASH

#define RPT_ELVTALARM_BUFSIZE   6   // size of alarm buffer
typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of buffer, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------

    uint16_t sn;            // serial number

    uint8_t sec;            // time in BCD
    uint8_t min;
    uint8_t hour;
    uint8_t date;
    uint8_t month;      // weekday 1-7 in bit[7:5]
    uint8_t year;           // 20xx

    uint16_t attrib;        // reserved

    uint8_t weight_flag;
    uint8_t weight_alarm;
    float weight_value;

    uint8_t height_flag;
    uint8_t height_alarm;
    float height_value;

    uint8_t speed_flag;
    uint8_t speed_alarm;
    float speed_value;

    uint8_t wind_flag;
    uint8_t wind_alarm;
    float wind_value;

    uint8_t tilt_flag;
    uint8_t tilt_alarm;
    float tilt_value;

    uint8_t motor1_flag;
    uint8_t motor1_alarm;

    uint8_t motor2_flag;
    uint8_t motor2_alarm;

    uint8_t motor3_flag;
    uint8_t motor3_alarm;

    uint8_t people_count;   // ����

    int8_t floor;   // ¥��

    uint8_t stat_low;   // ����λ״̬
    uint8_t stat_high;  // ����λ״̬
    uint8_t stat_outdoor;   // ������״̬
    uint8_t stat_indoor;    // ������״̬
    uint8_t warn_outdoor;   // �����ű���
    uint8_t warn_indoor;    // �����ű���

    // alarm
    uint8_t alarm_stat; // stat of alarm
                        // bit0: 0-no alarm, 1- alarm exist
                        // bit1: 0-no against, 1- against exist
                        // bit2: 0-no error, 1- error exist

    uint8_t alarm_num;
    APP_RPTALARM_TypeDef alarm[RPT_ALARMID_MAX];

    uint8_t against_num;
    APP_RPTAGAINST_TypeDef against[RPT_AGAINSTID_MAX];

    uint8_t error_num;
    APP_RPTERROR_TypeDef error[RPT_ERRORID_MAX];

    uint8_t spare_other[6];

    //-------------- end flag section --------------
    float end;          // last object of structure
}
APP_ELVTALARMDAT_TypeDef;

_EXTERN APP_ELVTALARMDAT_TypeDef elvt_alarm_dat[RPT_ELVTALARM_BUFSIZE]; // alarm send to server

//////////////// variable //////////////////
// ȫ��״̬��־λ�������ж�����

// ����һ���֡�������
//uint8_t extparam.buf[MAX_SIZE+12];
_EXTERN uint8_t RFULL_frame[MAX_SIZE+12];

_EXTERN uint8_t crcbuf[255];    // �û����������洢�û��������ڼ���CRCУ��λ

// ��λ���·��趨��������Ҫ���ն˱��棬��ʱ���壬��Ҫʱ���滻
_EXTERN SysPara system_parameter;
_EXTERN AddrZone tempA;
_EXTERN VerNo   tempV;

////////////////////////// function/////////////////////////
///
void TIP_init(void);
void TIP_login(void);
uint8_t TIP_frame_get(pRingBuf ringbuf, uint8_t *buffer);
pTX101 frame_invalid(pTX101 frame);
pTX101 frame_link_chk(uint8_t datazone);
pTX101 ParamSetDevAddr(pTX101 frame);
pTX101 ParamQryDevaddr(pTX101 frame);
pTX101 ParamSetDevRtc(pTX101 frame);
pTX101 ParamQryDevRtc(pTX101 frame);
pTX101 ParamSetWkMod(pTX101 frame);
pTX101 ParamQryWkMod(pTX101 frame);
pTX101 ParamSetSnsrTyp(pTX101 frame);
pTX101 ParamQrySnsrTyp(pTX101 frame);
pTX101 ParamSetSnsrCfg(pTX101 frame);
pTX101 ParamQrySnsrCfg(pTX101 frame);
pTX101 ParamSetDevIpPort(pTX101 frame);
pTX101 ParamQryDevIpPort(pTX101 frame);
pTX101 ParamSetHrtIntvl(pTX101 frame);
pTX101 ParamQryHrtIntvl(pTX101 frame);
pTX101 ParamSetDevLnkReconIntvl(pTX101 frame);
pTX101 ParamQryDevLnkReconIntvl(pTX101 frame);
pTX101 ParamSetDevRecIntvl(pTX101 frame);
pTX101 ParamQryDevRecIntvl(pTX101 frame);
pTX101 ParamSetDevRTDReptIntvl(pTX101 frame);
pTX101 ParamQryDevRTDReptIntvl(pTX101 frame);
pTX101 ParamSetDevUpd(pTX101 frame);
pTX101 ParamQryDevVerInfo(pTX101 frame);
pTX101 ParamSetDevPwd(pTX101 frame);
pTX101 ParamQryDevPwd(pTX101 frame);
pMultiTX101 ParamSetFngrDat(pMF_TX101 frame);
pTX101 Param_Del_FngrDat(pTX101 frame);
pTX101 Param_Restart(pTX101 frame);

pTX101 AUFFngrDat(void);

uint8_t link_layer_unpack(pTX101 pframe);
void link_layer_pack(pTX101 pframe, uint8_t dir, uint8_t div, uint8_t fcb, uint8_t linkfuncode);
uint8_t app_layer_unpack(pTX101 pframe);
void app_layer_pack(pTX101 pframe);
pAddrZone GetTAddr(void);
pVerNo GetProtocolVersion(void);
void GetIPHost(uint8_t num, uint8_t *str);
void GetRTC(pRtcClk realtime);
void SetRTC(uint8_t *rtc);
uint8_t GetCRC(uint8_t frametype);

uint32_t RNG_Get_RandomRange(uint32_t xmin, uint32_t xmax);

uint16_t FrmLnkLog(uint8_t *buf);
uint16_t FrmHrtDat(uint8_t *buf);
uint16_t FrmFngrDat(uint8_t *buf);
uint16_t FrmDevAddrDatSet(uint8_t *rf, uint8_t *sf);
uint16_t FrmDevAddrDatQry(uint8_t *rf, uint8_t *sf);
uint16_t FrmDevRtcDatSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevRtcQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevWkModSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevWkModQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSnsrTypSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSnsrTypQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSnsrCfgSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSnsrCfgQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevIpPortSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevIpPortQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevHrtIntvlSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevHrtIntvlQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevLnkReconIntvlSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevLnkReconIntvlQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSavIntvlSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevSavIntvlQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevRtdRptIntvlSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevRtdRptIntvlQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevUpdSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevVerInfoQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevPwdSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevPwdQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmFngrDatSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmFngrDatDel(uint8_t *rf,uint8_t *sf);
uint16_t FrmRestart(uint8_t *rf,uint8_t *sf);
#ifdef TOWERBOX
pTX101 ParamSetDevLct(pTX101 frame);
pTX101 ParamQryDevLct(pTX101 frame);
pTX101 ParamSetTwrInfo(pTX101 frame);
pTX101 ParamQryTwrInfo(pTX101 frame);
pTX101 ParamSetPrtcZone(pTX101 frame);
pTX101 ParamQryPrtcZone(pTX101 frame);
pTX101 ParamSetTwrLmt(pTX101 frame);
pTX101 ParamQryTwrLmt(pTX101 frame);
pTX101 ParamSetTwrTorque(pTX101 frame);
pTX101 ParamQryTwrTorque(pTX101 frame);
pTX101 ParamSetTwrCali(pTX101 frame);
pTX101 ParamQryTwrCali(pTX101 frame);
pTX101 ParamSetTwrLift(pTX101 frame);
pTX101 ParamQryTwrLift(pTX101 frame);
pTX101 AUFTowRTDat(void);
pTX101 AUFTowWklpDat(void);
pTX101 AUFTowWrnDat(void);
pTX101 AUFTowCaliDat(void);
void GetTowRTDat(void);
void GetTowWklpDat(void);
void GetTowWrnDat(uint8_t num);
void GetTowCaliDat(void);
uint16_t FrmDevLctSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmDevLctQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrInfoSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrInfoQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrPrtcZoneSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrPrtcZoneQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrLmtSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrLmtQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrTorqSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrTorqQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrCaliSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrCaliQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrLiftSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmTwrLiftQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmTowRtDat(uint8_t *buf);
uint16_t FrmTowWklpDat(uint8_t *buf);
uint16_t FrmTowWrnDat(uint8_t *buf);
uint16_t FrmTowCaliDat(uint8_t *buf);
#endif
#ifdef ELIVATOR
pTX101 ParamSetElvtInfo(pTX101 frame);
pTX101 ParamQryElvtInfo(pTX101 frame);
pTX101 ParamSetElvtFloor(pTX101 frame);
pTX101 ParamQryElvtFloor(pTX101 frame);
pTX101 AUFElvtRTDat(void);
pTX101 AUFElvtWklpDat(void);
pTX101 AUFElvtWrnDat(void);
pTX101 AUFElvtCaliDat(void);
void GetElvtRTData(void);
void GetElvtWklpDat(void);
void GetElvtWrnDat(uint8_t num);
void GetElvtCaliData(void);
uint16_t FrmElvtInfoSet(uint8_t *rf,uint8_t *sf);
uint16_t FrmElvtInfoQry(uint8_t *rf,uint8_t *sf);
uint16_t FrmElvtFloorSet(uint8_t *rf, uint8_t *sf);
uint16_t FrmElvtFloorQry(uint8_t *rf, uint8_t *sf);
uint16_t FrmElvtRtDat(uint8_t *buf);
uint16_t FrmElvtWklpDat(uint8_t *buf);
uint16_t FrmElvtWrnDat(uint8_t *buf);
uint16_t FrmElvtCaliDat(uint8_t *buf);
#endif
#ifdef DUSTMON
pTX101 ParamSetValveLmt(pTX101 frame);
pTX101 ParamSetValveMan(pTX101 frame);
pTX101 ParamQryValveLmt(pTX101 frame);
pTX101 ParamSetValveLmt_Ext(pTX101 frame);
pTX101 ParamQryValveLmt_Ext(pTX101 frame);
pTX101 ParamSetNotice(pTX101 frame);
void GetDustRTData(void);
void GetDustWrnDat(uint8_t num);
pTX101 AUFDustRTDat(void);
pTX101 AUFDustWrnDat(void);
uint16_t FrmDustRtDat(uint8_t *buf);
uint16_t FrmDustWrnDat(uint8_t *buf);
uint16_t FrmValveLmtSet(uint8_t *rf, uint8_t *sf);
uint16_t FrmValveLmtSet_Ext(uint8_t *rf, uint8_t *sf);
uint16_t FrmValveLmtQry(uint8_t *rf, uint8_t *sf);
uint16_t FrmValveLmtQry_Ext(uint8_t *rf, uint8_t *sf);
uint16_t FrmValveManual(uint8_t *rf, uint8_t *sf);
uint16_t FrmNotice(uint8_t *rf, uint8_t *sf);
#endif
#ifdef UPPLAT
pTX101 ParamSetUPLmt(pTX101 frame);
pTX101 ParamQryUPLmt(pTX101 frame);
void GetUPPlatRTData(void);
pTX101 AUFUPPlatRTDat(void);
void GetUPPlatWrnDat(uint8_t num);
pTX101 AUFUPPlatWrnDat(void);
uint16_t FrmUPPlatRtDat(uint8_t *buf);
uint16_t FrmUPPlatWrnDat(uint8_t *buf);
uint16_t FrmUPLmtSet(uint8_t *rf, uint8_t *sf);
uint16_t FrmUPLmtQry(uint8_t *rf, uint8_t *sf);

#endif 


#undef _EXTERN
#endif

