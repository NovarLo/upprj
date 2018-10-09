
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

#define AFN_DENY    0x00    // 否认/失败
#define AFN_CONFIRM 0x01    // 确认/成功
#define AFN_INVALID 0x02    // 无效请求
#define AFN_LINKCHK 0x03    // 链路接口检测

#define AFN_SET_ADDRESS             0x10    // 设置遥测终端地址
#define AFN_QRY_ADDRESS           0x50    // 查询遥测终端地址
#define AFN_SET_CLOCK               0x11    // 设置遥测终端时钟
#define AFN_QRY_CLOCK             0x51    // 查询遥测终端时钟
#define AFN_SET_WORKMODE            0x12    // 设置遥测终端工作模式
#define AFN_QRY_WORKMODE          0x52    // 查询遥测终端工作模式
#define AFN_SET_SENSORTYPE          0x13    // 设置遥测终端的传感器种类
#define AFN_QRY_SENSORTYPE        0x53    // 查询遥测终端的传感器种类
#define AFN_SET_SENSORPARAM         0x14    // 设置遥测终端的传感器参数
#define AFN_QRY_SENSORPARAM       0x54    // 查询遥测终端的传感器参数
#define AFN_SET_LOCATION            0x15    // 设置遥测终端地理位置/经纬度
#define AFN_QRY_LOCATION          0x55    // 查询遥测终端GPS位置
#define AFN_SET_TOWERPARAM          0x16    // 设置塔机静态结构参数
#define AFN_QRY_TOWERPARAM        0x56    // 查询塔机静态结构参数
#define AFN_SET_PROTECTIONZONE      0x17    // 设置塔机保护区
#define AFN_QRY_PROECTIONZONE     0x57    // 查询塔机保护区
#define AFN_SET_LIMIT               0x18    // 设置塔机限位信息
#define AFN_QRY_LIMIT             0x58    // 查询塔机限位信息
#define AFN_SET_MOMENTCURVE         0x19    // 设置塔机力矩曲线
#define AFN_QRY_MOMENTCURVE       0x59    // 查询塔机力矩曲线
#define AFN_SET_CALIBRATPARAM       0x1A    // 设置塔机标定参数
#define AFN_QRY_CALIBRATPARAM     0x5A    // 查询塔机标定参数
#define AFN_SET_DEVIPPORT           0x1F    // 设置遥测终端存储的中心站IP地址和端口号
#define AFN_QRY_DEVIPPORT         0x5F    // 查询遥测终端存储的中心站IP地址和端口号
#define AFN_SET_HEARTINTERVAL       0x20    // 设置遥测终端链路心跳间隔
#define AFN_QRY_HEARTINTERVAL     0x60    // 查询遥测终端链路心跳间隔
#define AFN_SET_RECONNECTINTERVAL   0x21    // 设置遥测终端链路重连间隔
#define AFN_QRY_RECONNECTINTERVAL 0x61    // 查询遥测终端链路重连间隔
#define AFN_SET_DATRECINTERVAL      0x22    // 设置遥测终端历史数据存盘间隔
#define AFN_QRY_DATRECINTERVAL    0x62    // 查询遥测终端历史数据存盘间隔
#define AFN_SET_DATUPLOADINTERVAL   0x23    // 设置遥测终端的实时数据上报间隔
#define AFN_QRY_DATUPLOADINTERVAL 0x63    // 查询遥测终端的实时数据上报间隔
#define AFN_SET_UPDATE      0x24    // 设置遥测终端的升级数据
#define AFN_QRY_VERINFO   0x64    // 查询遥测终端的版本信息
#define AFN_SET_PASSWORD    0x25    // 设置遥测终端的密码
#define AFN_QRY_PASSWORD  0x65    // 查询遥测终端的密码
#define AFN_SET_TOWERLIFT   0x26    // 设置塔机顶升数据
#define AFN_QRY_TOWERLIFT 0x66    // 查询塔机顶升数据
#define AFN_CHG_FINGER      0x28    // 指纹数据变更
#define AFN_DEL_FINGER      0x29    // 删除指纹数据
                                    //
#define AFN_SET_ELVTINFO    0x30    // 设置升降机基本结构参数
#define AFN_QRY_ELVTINFO  0x70    // 查询升降机基本结构参数
                                    //
#define AFN_SET_ELVTFLOOR   0x31    // 设置升降机楼层参数
#define AFN_QRY_ELVTFLOOR 0x71    // 查询升降机楼层参数
                                    //
#define AFN_SET_VALVELMT    0x40    // 设置扬尘在线监测终端电磁阀阈值
#define AFN_QRY_VALVELMT  0x80    // 查询扬尘在线监测终端电磁阀阈值
#define AFN_SET_MANVALVE    0x41    // 设置扬尘在线监测终端电磁阀手动开合
#define AFN_SET_VALVELMT_EXT    0x42    // 设置扬尘在线监测终端电磁阀阈值(仅仅使用于PM10)
#define AFN_QRY_VALVELMT_EXT  0x82    // 查询扬尘在线监测终端电磁阀阈值(仅仅使用于PM10)

#define AFN_SET_NOTICE      0x44    // 设置下发通知信息（OUTDOOR LED）

#define AFN_SET_UPLMT       0x45    // 设置卸料平台阈值参数
#define AFN_QRY_UPLMT       0x85    // 查询卸料平台阈值参数


#define AFN_RESTART 0xA0    // 遥测终端重启

#define AFN_SELF_FINGER         0x94    // 指纹上报帧

#define AFN_SELF_REALTIMEDATA   0x90    // 塔机定时实时数据
#define AFN_SELF_WORKCYCLE      0x91    // 塔机工作循环数据
#define AFN_SELF_WARNING        0x92    // 塔机报警实时数据
#define AFN_SELF_CALIBRATION    0x98    // 塔机标定实时数据
                                        //
#define AFN_SELF_ELVTRT         0x9A    // 升降机定时实时数据
#define AFN_SELF_ELVTWKLP       0x9B    // 升降机工作循环数据
#define AFN_SELF_ELVTWARN       0x9C    // 升降机实时报警数据
#define AFN_SELF_ELVTCALI       0x9D    // 升降机标定实时数据

#define AFN_SELF_DUSTRT         0x96    // 扬尘监测实时数据
#define AFN_SELF_DUSTWARN       0x97    // 扬尘监测报警数据

#define AFN_SELF_UPPLATRT       0xA1    // 卸料平台实时数据
#define AFN_SELF_UPPLATWARN     0xA2    // 卸料平台报警数据

// 控制域中的链路层功能码定义

// 下行
#define LFN_DIR0_CONFIRM    0x01    // 下发命令
#define LFN_DIR0_NORESPON   0x02    // 用户数据
#define LFN_DIR0_LNKRESPON     0x03    // 链路测试
#define LFN_DIR0_PARAMRESPON   0x04    // 被测参数
#define LFN_DIR0_WARNRESPON    0x13    // 报警或状态参数

// 上行
#define LFN_DIR1_FAIL       0x00    // 否认/失败 命令响应帧
#define LFN_DIR1_OK         0x01    // 确认/成功 命令响应帧
#define LFN_DIR1_DENYRESPON    0x02    // 响应帧 否认/无所召唤的数据
#define LFN_DIR1_QUERYRESPON   0x03    // 查询/响应帧 链路状态
#define LFN_DIR1_PARAMRESPON   0x04    // 响应帧 相应参数
#define LFN_DIR1_TIMINGSTATUS  0x05    // 定时自报帧 相应状态
#define LFN_DIR1_RANDOMWORK    0x06    // 随机自报帧 工作循环
#define LFN_DIR1_RANDOMWARN    0x07    // 随机自报帧 报警
#define LFN_DIR1_RANDOMFAULT   0x08    // 随机自报帧 故障
#define LFN_DIR1_RANDOMILLEGAL 0x09    // 随机自报帧 违章

// 控制域中的链路层功能码定义结束

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
#define RESEND_INTERVAL 5000    // 重发间隔默认值：5000ms
#define RESEND_TIMES    3       // 失败重发次数为3次

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

// 常用基本结构
typedef struct
{
    uint8_t STAT_PRINT_BUSY:1;  //
    uint8_t STAT_GPRSSEND_BUSY:1;   // 0,enable sending frame；1,disable sending frame
    uint8_t STAT_FNGRSEND_BUSY:1;
    uint8_t STAT_NO_INIT:1;     // 0,tip_init run ok,1,tip_init run fault
    uint8_t STAT_GPRS_ZPPP:1;   // 0,GPRS connected;1,GPRS disconnected
    uint8_t STAT_LINK_LOG:1;    // 0,GRRS login;1,GPRS logout
    uint8_t STAT_LINK_OK:1;     // 0, Link logout;1,Link login success
    uint8_t STAT_HEART_OV:1;    // 0,disable send heart frame;1,enable send heart frame;
    uint8_t STAT_CSQ_CHK:1;     // 0,disable send csq frame;1,enable send check signal quarlity
    uint8_t rev:4;          // 保留
    uint32_t resend_start;  // 起始节拍
    uint32_t resend_interval;// 超时间隔(ms)
    uint8_t resend_times;   // 失败重发次数
    uint32_t mframecnt;     // 多帧计数器
    uint8_t RTDATA_FLAG;    // 定时实时数据上传标志 0 —— 无 1 —— 有
    uint8_t WORKCYCLE_FLAG; // 工作循环数据上传标志 0 —— 无 1 —— 有
    uint8_t WARNING_FLAG;   // 告警实时数据上传标志 0 —— 无 1 —— 有
    uint8_t CALIBRATE_FLAG; // 标定实时数据上传标志 0 —— 无 1 —— 有
    uint8_t STAT_WAIT_ACK;  // 发送完主动帧后置位，等待应答标志
}StatFlag,*pStatFlag;
_EXTERN StatFlag StatusFlag;

// 0~99
typedef struct
{
    uint8_t single:4;   // BCD码个位
    uint8_t digit:4;    // BCD码十位
}BCD_dig,*pBCD_dig;

// -9999.9~+9999.9
//带符号为的千位BCD码，±xxxx.x,有无符号位都用该结构，为符号位是忽略符号位值
typedef struct
{
    uint8_t thousand:4;     // 千位
    uint8_t symbol:4;       // 符号位
    uint8_t digit:4;        // 十位
    uint8_t handred:4;  // 百位
    uint8_t dot:4;          // 小数位
    uint8_t single:4;       // 个位
}BCD_th,*pBCD_th;

// -999.9 ~ +999.9
//带符号为的千位BCD码，±xxx.x
typedef struct
{
    uint8_t digit:4;        // 十位
    uint8_t handred:4;  // 百位
    uint8_t dot:4;          // 小数位
    uint8_t single:4;       // 个位
}BCD_hun,*pBCD_hun;

// 控制域
typedef struct
{
    uint8_t func:4; // 功能码定义
    uint8_t fcb:2;  // 帧计数位—— 防止信息传输的丢失和重复
    uint8_t div:1;  // 拆分标志位—— 1表示此报文已经被拆分成若干帧,
                    // 此时控制域后增加4字节作为拆分计数帧。
                    // 前两个字节为拆分帧总数,后两字节标示当前帧数
                    // 采用BIN倒计数(65535~1),1表示最后一帧
    uint8_t dir:1;  // 传输方向位—— 0下行 1上行
}CtrlZone,*pCtrlZone;

// 地址域
typedef struct
{
    uint8_t a1_low;  // 行政区划码 A1 BCD —— 2Bytes
    uint8_t a1_high;
    uint8_t a2_low;  // 终端、中继地址 A2 BIN —— 3Bytes
    uint8_t a2_middle;
    uint8_t a2_high;
}AddrZone,*pAddrZone;

// 协议版本
typedef struct
{
    uint8_t firstver:4;    // 次版本
    uint8_t secondver:4;     // 主版本
}VerNo,*pVerNo;

// 密码区
typedef struct
{
    uint8_t key_low;      //12bit密钥,BCD编码,取值范围0~999
    uint8_t key_high:4;
    uint8_t key_alg:4;    // 密钥算法,BCD编码,取值0~9
}KeyZone,*pKeyZone;

// 时间标签
typedef struct     // BCD 时钟标签
{
    uint8_t sec_single:4;    // 秒 个位数
    uint8_t sec_digit:4;    // 秒 十位数
    uint8_t min_single:4;    // 分 个位数
    uint8_t min_digit:4;    // 分 十位数
    uint8_t clk_single:4;    // 时 个位数
    uint8_t clk_digit:4;    // 时 十位数
    uint8_t dat_single:4;    // 日 个位数
    uint8_t dat_digit:4;    // 日 十位数
    uint8_t delay;          // 允许发送传输延时时间
}TimeLabel,*pTimeLabel;

// 附加信息域
typedef struct
{
    KeyZone key_zone;       // 密码区，2bytes
    TimeLabel time_label;   // 时间标签区，5bytes
}AuxInfo,*pAuxInfo;

// 应用区
typedef struct
{
    uint8_t functioncode;   // 应用层功能码 1byte
    void *userdata;         // 应用层用户数据
    //AuxInfo auxinfo;      // 应用层附加信息 7byte 内容附在userdata数组末尾
}AppZone,*pAppZone;

// 泰新塔吊完整协议帧结构 企业标准
#define MAX_SIZE 248
typedef struct
{
    uint8_t startb1;    // 起始字符0x68
    uint8_t length;     // 长度：从控制域到校验之前的所有字节
    uint8_t startb2;    // 起始字符0x68
    CtrlZone ctrlzone;  // 控制域 1byte
    AddrZone addrzone;  // 地址域 5bytes
    VerNo version;      // 版本：0.0~15.15 1byte
    AppZone appzone;    // 应用区域 (8+?)Bytes
    uint8_t cs;         // CRC校验区 1byte
    uint8_t endbyte;    // 结束字符0x16 1byte
}TX101,*pTX101;
// 定义一个全局结构和缓冲区作为返回帧存放处
// 该结构开始用于存放下行帧，经处理后作为上行帧发送
_EXTERN TX101 rtn;
_EXTERN uint8_t rtnbuf[MAX_SIZE];    //用户数据区除去AFN最长248Bytes

// 多帧结构
typedef struct
{
    uint8_t startb1;    // 起始字符0x68
    uint8_t length;     // 长度：从控制域到校验之前的所有字节
    uint8_t startb2;    // 起始字符0x68
    CtrlZone ctrlzone;  // 控制域 1byte
    uint16_t framenum;  // 总帧数
    uint16_t framecnt;  // 多帧计数器
    AddrZone addrzone;  // 地址域 5bytes
    VerNo version;      // 版本：0.0~15.15 1byte
    AppZone appzone;    // 应用区域 (8+?)Bytes
    uint8_t cs;         // CRC校验区 1byte
    uint8_t endbyte;    // 结束字符0x16 1byte
}MultiTX101,*pMultiTX101;

#define MAX_MFRAME 16
typedef struct
{
    uint8_t mf_flag;    // 多帧标志，0，无多帧1，有多帧
    uint16_t mframe_num;    // 总帧数
    uint16_t mframe_cnt;    // 当前收到的帧号
    uint16_t mframe_st; // 多帧状态位（初始化为零），没接收到一帧多帧信息，对应该位信息置一
                        // eg：多帧共5包，只有判断mframe_st低五位全为高才表示多帧收完
    MultiTX101 frame[MAX_MFRAME];   // 预留十六帧
    uint8_t mlen[MAX_MFRAME];   // 每帧应用程序数据区占用缓冲区长度
}MF_TX101,*pMF_TX101;

_EXTERN MF_TX101 mrtn;
_EXTERN uint8_t mrtnbuf[MAX_MFRAME][MAX_SIZE];  // 多帧应用层数据缓冲区，最多16帧
_EXTERN uint8_t mrtnbuffer[MAX_MFRAME*MAX_SIZE];    // 多帧合并后应用数据区

typedef union taixin_protocol_union
{
    TX101 tx101;
}TX101UNION;

// 系统参数结构，保存下行报文中需要设置的结构变量
// 临时定义，替换时注释


// 时间标签
typedef struct     // BCD 时钟标签
{
    uint8_t sec;    // 秒
    uint8_t min;    // 分
    uint8_t hour;    // 时
    uint8_t date;    // 日
    uint8_t mon_single:5;   // 月
    uint8_t week_single:3;  // 星期
    uint8_t year;           // 年 BCD码个位
}RtcClk,*pRtcClk;

// 传感器类型
typedef struct
{
    uint8_t sensornum;      // 传感器数量
    uint8_t sensortype[255];// 传感器配置数据：有多少数量就有多少个
}SensorType,pSensorType;

// 传感器配置
typedef struct
{
    uint8_t sensortype;     // 传感器类型
    uint8_t sensorpara[2];  // 传感器参数：因为各种传感器参数最大2Byte，取最大
}SensorCfg,pSensorCfg;

// 风速传感器
typedef struct
{
    uint8_t pulse_high;     // 脉冲常数高字节
    uint8_t pulse_low;      // 脉冲常数低字节
}SensorWind,pSensorWind;

// 回转传感器
typedef struct
{
    uint8_t resolution:3;   // 分辨率
    uint8_t rotationdir:1;  // 转向
    uint8_t rev:1;          // 保留
    uint8_t factor:1;       // 特性
    uint8_t interfacemode:2;// 接口方式

}SensorRotary,*pSensorRotary;

// 称重传感器
typedef struct
{
    uint8_t rev:6;  // 保留
    uint8_t interfacemode:2;    // 接口方式
}SensorWeigh,*pSensorWeigh;

// 终端地理位置/经纬度
typedef struct
{
    uint8_t rev1;
    uint8_t lo_degree_single:4; // "度" 个位
    uint8_t lo_degree_digit:4;  // "度" 十位
    uint8_t lo_min_single:4;    // "分" 个位
    uint8_t lo_min_digit:4; // "分" 十位
    uint8_t lo_min_hundredth:4;// "分" 百分位
    uint8_t lo_min_digith:4;    // "分" 十分位
    uint8_t lo_min_miriade:4;   // "分" 万分位
    uint8_t lo_min_thousandth:4;    // "分" 千分位
    uint8_t longitude;          // 经度 "N" or "S"
    uint8_t la_degree_hundred:4;    // "度"
    uint8_t la_degree_single:4; // "度"  个位
    uint8_t la_degree_digit:4;  // "度"  十位
    uint8_t la_min_single:4;    // "分"  个位
    uint8_t la_min_digit:4; // "分"  十位
    uint8_t la_min_hundredth:4;// "分"  百分位
    uint8_t la_min_digith:4;    // "分"  十分位
    uint8_t la_min_miriade:4;   // "分"  万分位
    uint8_t la_min_thousandth:4;    // "分" 千分位
    uint8_t latitude;       // 纬度 "E" or "W"
    uint8_t altitude_hundred:4;     // 海拔 百位
    uint8_t altitude_thousand:4;    // 海拔 千位
    uint8_t altitude_single:4;      // 海拔 个位
    uint8_t altitude_digit:4;       // 海拔 十位
}Location,*pLocation;

// 塔机基本机构参数
typedef struct
{
    uint8_t name[16];   // 塔机名称，16个字节，ASCII字符串，如果不足，后续字节以\0填充
    uint8_t ID;         // 塔机ID，1个字节，塔机所在塔群的编号，范围0-63
    uint8_t IDs;        // 塔群ID，1个字节，塔机在当前塔群中的编号，范围0-62，63用于地面
    BCD_th x;           // 坐标X，3个字节，以0.1米为单位，范围-999.9-9999.9米
    BCD_th y;           // 坐标Y，3个字节，以0.1米为单位，范围-999.9-9999.9米
    uint8_t typecode[2];    // 塔机型号代码，2个字节
    uint8_t ratedload;      // 额定载重，1个字节，以0.1吨为单位，范围0.0－25.5吨
    uint8_t type_scales;    // 塔机类型/倍率，1个字节

    BCD_th length_forearm;  // 前臂长度，3个字节，0.1米为单位，范围0米-9999.9米
    BCD_th length_backarm;  // 后臂长度，3个字节，0.1米为单位，范围0米-9999.9米
    BCD_th frontbar_location1;  // 前拉杆1位置（远端拉杆），3个字节，0.1米为单位，范围0米-9999.9米
    BCD_th frontbar_location2;  // 前拉杆2位置（近端拉杆），3个字节，0.1米为单位，范围0米-9999.9米
    BCD_th backbar_location;    // 后拉杆位置，3个字节，0.1米为单位，范围0米-9999.9米
    BCD_th towerbar_loweredge_heigh;    // 塔臂下沿高度，3个字节，0.1米为单位，范围0米-9999.9米

    BCD_hun towerbar_heigh[2];      // 塔臂自身高度，2个字节，0.1米为单位，范围0米-99.9米
    BCD_hun towerbar_heigh_ltot[2]; // 塔臂上沿到塔尖高度，2个字节，0.1米为单位，范围0米-99.9米
    uint8_t rotary_inertia_coef;    // 回转惯性系数，1个字节，范围0 - 99
    BCD_hun walk_angle;         // 行走角度，2个字节，0.1度为单位，范围0.0-359.9度
}TowPara,*pTowPara;

// 保护区各种类型元素信息

// 点
typedef struct
{
    uint8_t etype;  // 保护区元素类型
    //X坐标 BCD 码 unit:0.1m -9999.9~9999.9m
    BCD_th x;   // X坐标值
                            //
    //Y坐标 BCD 码 unit:0.1m -9999.9~9999.9m
    BCD_th y;   // Y坐标值
}EPoint,*pEPoint;

// 圆弧
typedef struct
{
    // 圆心坐标
    // 圆心坐标X，3个字节，以0.1米为单位，范围-9999.9-9999.9米
    BCD_th centrey_x;

    // 圆心坐标Y，3个字节，以0.1米为单位，范围-9999.9-9999.9米
    BCD_th centrey_y;

    // 圆半径，3个字节，以0.1米为单位，范围0-9999.9米
    BCD_th radius;

}EArc,*pEArc;

// 保护区信息，长度不定，格式如下
typedef struct
{
    uint8_t p_type; // 保护区类型，1个字节：0－禁行区，1－障碍物
    uint8_t p_id;   // 保护区编号，1个字节。同一工地，每个保护区有唯一编号
    uint8_t p_building; // 保护区建筑类型，1个字节：0－其他，1－医院，2－学校，3－广场，4－道路，5－居民区，6－办公区，7－高压线；其余类型待定（只对禁行区有效）
    BCD_th p_high;  // 保护区高度，3个字节BCD码。只对障碍物有效
    uint8_t p_K;    // 保护区元素个数K，1个字节
    uint8_t etype;  // 元素类型，1个字节：0x00－点；0x01－圆弧（保留）
    // 保护区元素数据
    uint8_t *edata; // 元素数据区
}ProtectInfo,*pProtectInfo;

// 保护区
typedef struct
{
    uint8_t pz_num; // 保护区个数

    //pz_num个 保护区信息
    //ProtectInfo protect_info;
    // 不定长的保护区元素数据
    /*元素数据区 */
    uint8_t ptr_pz; // 因为不知道长度，暂时定义一个指针代替
}ProtectZone,*pProtectZone;

// 塔机限位信息
typedef struct
{
    BCD_th left;        // 左限位，3个字节BCD码，，范围-9999.9-9999.9度
    BCD_th right;       // 右限位，3个字节BCD码，，范围-9999.9-9999.9度
    BCD_hun rotaryprewarn1; // 回转限位预警值，2字节BCD码，范围0.0-999.9度
    BCD_th top;         // 高限位，3个字节BCD码，范围-9999.9-9999.9米
    BCD_th bottom;      // 低限位，3个字节BCD码，范围-9999.9-9999.9米
    BCD_hun topprewarn1;    // 高度限位预警值，2字节BCD码，范围0.0-999.9米
    BCD_hun far;        // 远限位，2个字节BCD码，范围0米-999.9米
    BCD_hun near;       // 近限位，2个字节BCD码，范围0米-999.9米
    BCD_hun scopeprewarn1;  // 幅度限位预警值，2个字节BCD码，范围0米-999.9米
    BCD_th forward;     // 前限位，3个字节BCD码，范围-9999.9-9999.9米
    BCD_th backward;    // 后限位，3个字节BCD码，范围-9999.9-9999.9米
    BCD_hun walkprewarn1;// 行走限位预警值，2字节BCD码，范围0.0-999.9米
    BCD_hun dipprewarn; // 倾角预警值，2字节BCD码，范围0.0-99.9度
    BCD_hun dipwarn;        // 倾角报警值，2字节BCD码，范围0.0-99.9度
    BCD_hun windprewarn;// 风速预警值，2字节BCD码，范围0.0-999.9m/s
    BCD_hun windwarn;// 风速报警值，2字节BCD码，范围0.0-999.9m/s
    uint8_t rev1[12];       // 预留12Bytes
    BCD_hun rotaryprotect;  // 回转保护值，2个字节BCD码，范围0米-999.9度
    BCD_hun rotaryprewarn2; // 回转预警值，2个字节BCD码，范围0米-999.9度
    BCD_hun topprotect;     // 高度保护值，2个字节BCD码，范围0米-999.9米
    BCD_hun topprewarn2;        // 高度预警值，2个字节BCD码，范围0米-999.9米
    BCD_hun scopeprotect;       // 高度保护值，2个字节BCD码，范围0米-999.9米
    BCD_hun scopeprewarn2;      // 高度预警值，2个字节BCD码，范围0米-999.9米
    BCD_hun walkprotect;        // 高度保护值，2个字节BCD码，范围0米-999.9米
    BCD_hun walkprewarn2;       // 高度预警值，2个字节BCD码，范围0米-999.9米
    BCD_hun towerprotect;       // 高度保护值，2个字节BCD码，范围0米-999.9米
    BCD_hun towerprewarn;       // 高度预警值，2个字节BCD码，范围0米-999.9米
    uint8_t rev2[12];       // 预留12Bytes
}TowLimit,*pTowLimit;

// 塔机力矩曲线
typedef struct
{
    uint8_t rate;   // 倍率
    uint8_t n;      // 力矩曲线点数N

    /*第一点到第N点的幅度和重量值 */
    // 幅度为2字节BCD，重量为3字节BCD，每个点五个字节
    // 幅度格式如下，范围0.0-999.9米
    // 重量格式如下，范围0.0000-99.9999吨
    uint8_t *ptr;
}TowMoment,*pTowMoment;

// 塔机标定参数
typedef struct
{
    SensorType sensortype;  // 传感器类型
    uint8_t n;              // 标定数据点数N

    /*
    BCD_th original1;       // 第一点原始值
    BCD_th field1;          // 第一点工程量
    */
    // 此处定义N个点的原始值和工程量
    uint8_t *ptr;
}TowCalibratParam,*pTowCalibratParam;

// 遥测终端存储的中心站IP地址和端口号
typedef struct
{
    uint8_t port1[2];   // 端口号,低位在前
    uint8_t ip1[4];     // IPv4地址,低位在前

    uint8_t port2[2];   // 端口号,低位在前
    uint8_t ip2[4];     // IPv4地址,低位在前

    uint8_t port3[2];   // 端口号,低位在前
    uint8_t ip3[4];     // IPv4地址,低位在前

    uint8_t port4[2];   // 端口号,低位在前
    uint8_t ip4[4];     // IPv4地址,低位在前

}IPandPort,*pIPandPort;

// 版本信息
typedef struct
{
    VerNo fireware; // 软件版本
    VerNo protocol; // 协议版本
    VerNo MMI;      // MMI版本
}VerInfo,*pVerInfo;

// 密码
typedef struct
{
    uint8_t pwtype; // 密码类型 密码类型：1字节。
                    //0x00－用户查询密码；
                    //0x01－用户操作密码；
                    //0x08－调试密码
                    //0x09－超级用户密
                    //码其它－保留
    uint8_t pwlen;  // 密码长宽N：1B，描述密码的长度（包含结束符），范围0－255

    // 密码字节：ASCII码字符串，以“/0”结尾
    uint8_t *ptr;
}PW,*pPW;

// 需要保存的参数
typedef struct
{
    AddrZone Taddr;     // 终端地址
    RtcClk  clk;            // 时钟
    uint8_t workmode;   // 工作模式
    SensorType sensortype;  // 传感器类型
    SensorCfg sensorcfg;    // 传感器参数
    Location location;  // 位置/经纬度
    TowPara towerparam; // 塔机基本结构参数
    ProtectZone pz;     // 保护区信息
    TowLimit towerlimit;    // 塔机限位信息
    TowMoment towermoment;  // 塔机力矩曲线
    TowCalibratParam towercalibpara;    //塔机标定参数
    IPandPort ipandport;    // 中心站IP地址和端口号
    BCD_dig heartinterval;  // 1字节（压缩的BCD码）为心跳间隔时间： 取值范围为：0—99；单位：分钟。程序默认值为：10分钟
    BCD_hun reconnectinterval;  // 2字节压缩的BCD码，链路重链接间隔时间， 取值范围为0—9999；单位：分钟。程序默认值为：120分钟
    BCD_hun saveinterval;        // 数据域，2字节（压缩的BCD码）为历史数据存盘间隔时间： 取值范围为：0—9999；单位：秒。程序默认值为：10秒
    BCD_hun updatainterval;      // 2字节（压缩的BCD码）为实时数据上报间隔时间： 取值范围为：0—9999；单位：秒。程序默认值为：30秒
    VerInfo versioninfo;    // 版本信息
    PW password[4];         // 目前有四种密码
    uint8_t towertoplift;       // 数据域：1字节，表示本次顶升高度，范围0.0－25.5米
}SysPara,*pSysPara;

// 传感器数据
typedef struct
{
    // 传感器状态
    uint8_t sensor_OK:1;    // OK：传感器工作状态，0－无效，1－正常；
    uint8_t sensor_EN:1;    // EN：传感器使能状态，0－禁能，1－使能；
    uint8_t rev1:6;         // 保留
    uint8_t warning;        // 报警状态：　　
                            //0x00：无报警　　　　　　　　　
                            //0x20：接近额定起重量；　　　　　　　　　
                            //0x60：到达额定起重量； 　　　　　　　　　
                            //0xE0:吊重超载

    BCD_th data;            // 带符号3字节BCD码，0.0-9999.9吨
}SensorDat,*pSensorDat;

// 报警代码结构（碰撞、障碍物、禁行区）
typedef struct
{
    uint8_t ID; // 对方塔机ID，0表示无报警
    uint8_t low:2;      // 低碰撞
    uint8_t rev1:2;     // 保留
    uint8_t right:2;    // 右碰撞
    uint8_t left:2;     // 左碰撞

    uint8_t Inttype:2;  // 干涉类型。01H：干涉本塔机塔臂,02H：干涉本塔机钢丝绳
    uint8_t rev2:2;     // 保留
    uint8_t near:2;     // 近碰撞
    uint8_t far:2;      // 远碰撞
}WarningCode,*pWarningCode;

// 定时上传实时数据域
typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint8_t packtype[2];    // 包类型待定
    SensorDat gravitylift;  // 吊重力
    SensorDat height;       // 高度
    SensorDat scope;        // 幅度
    SensorDat rotary;       // 回转角度
    SensorDat wind;         // 风速
    SensorDat dipangle;     // 倾角
    SensorDat moment;       // 力矩比率
    SensorDat walk;         // 行走
    SensorDat rev1[4];      // 预留

    // 报警状态
    WarningCode collision;  //碰撞代码（3B）
    WarningCode obstacle;   // 障碍物
    WarningCode forbidden;  //禁行区3B
    WarningCode rev2[4];    // 预留3BX4

    uint8_t rev3[6];        // 预留6B
}DatRealT,*pDatRealT;

// 定义自动上传报文结构
_EXTERN DatRealT realtimedata;  // 定时实时数据

// 升降机上传实时数据域
typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // 包类型待定
    SensorDat gravitylift;  // 吊重力
    SensorDat height;       // 高度
    SensorDat speed;        // 速度
    SensorDat wind;         // 风速
    SensorDat dipangle;     // 倾角
    SensorDat motor[3];     // 电机一、二、三

    uint8_t people_flag;
    uint8_t people_alarm;
    uint8_t people_value;

    uint8_t floor_flag;
    uint8_t floor_aligned;
    uint8_t floor_value;

    uint8_t door_limit;// 门及限位状态

    uint8_t rev[19];    // 保留

    uint8_t rev1[6];    // 保留

}RTDatElvt,*pRTDatElvt;

// 定义自动上传报文结构
_EXTERN RTDatElvt elvtdata; // 升降机定时实时数据


// 扬尘在线监测上传实时数据域
typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // 包类型待定
    SensorDat pm25;     // pm25
    SensorDat pm10;     // pm10
    SensorDat temperature;  // 温度
    SensorDat humidity;     // 湿度
    SensorDat wind;         // 风速
    SensorDat valve[4];     // 电磁阀一、二、三、四
    SensorDat noise;        // 噪声
    SensorDat vane;     // 风向
    uint8_t rev[10];    // 保留

    uint8_t rev1[6];    // 保留

}RTDatDust,*pRTDatDust;
_EXTERN RTDatDust dustdata; // 扬尘在线监测定时实时数据

// 卸料平台称重监测上传实时数据域
typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // 包类型待定
    SensorDat cableleft;     // 斜拉索一
    SensorDat cableright;    // 斜拉索二
    SensorDat weightsensor[16];  // 称重传感器1~16
    SensorDat weight;     // 卸料平台总称重

    uint8_t rev[20];    // 保留

    uint8_t rev1[6];    // 保留

}RTDatUPPlat,*pRTDatUPPlat;
_EXTERN RTDatUPPlat upplatdata; //卸料平台称重在线监测定时实时数据

// 塔机工作循环数据
typedef struct
{
    uint8_t workcyclenum[2];    // 工作循环编号：2个字节，低字节在前，高字节在后
    uint32_t name_id;
    RtcClk starttime;           // 开始时间
    RtcClk endtime;             // 结束时间
    BCD_th maxlift;             // 最大起重，3个字节，0-9999.9吨
    BCD_th maxheight;           // 最大高度，3个字节，-9999.9 - 9999.9米
    #ifdef TOWERBOX
    BCD_th maxmoment;           // 最大力矩，3个字节，0-9999.9吨*米
    BCD_th minheight;           // 最小高度，3个字节，-9999.9 - 9999.9米
    BCD_th maxscope;            // 最大幅度，3个字节，-9999.9 - 9999.9米
    BCD_th minscope;            // 最小幅度，3个字节，-9999.9 - 9999.9米
    BCD_th maxrotary;           // 最大回转角度，3个字节，-9999.9 - 9999.9度
    BCD_th minrotary;           // 最小回转角度，3个字节，-9999.9 - 9999.9度
    BCD_th maxwalk;             // 最大行走距离，3个字节，-9999.9 - 9999.9米
    BCD_th minwalk;             // 最小行走距离，3个字节，-9999.9 - 9999.9米
    BCD_th liftpointangle;      // 起吊点角度，3个字节，-9999.9-9999.9度
    BCD_th liftpointscope;      // 起吊点幅度，3个字节，0-9999.9米
    BCD_th liftpointheight;     // 起吊点高度，3个字节，-9999.9-9999.9度
    BCD_th unloadpointangle;    // 卸吊点角度，3个字节，-9999.9-9999.9度
    BCD_th unloadpointscope;    // 卸吊点幅度，3个字节，0-9999.9米
    BCD_th unloadpointheight;   // 卸吊点高度，3个字节，-9999.9-9999.9度
    #endif
    #ifdef ELIVATOR
    uint8_t maxpeople;
    uint8_t maxfloor;
    #endif
}DatWork,*pDatWork;

_EXTERN DatWork TowWklpDat;     // 塔机工作循环数据
_EXTERN DatWork ElvtWklpDat;    // 升降机工作循环数据

// 报警实时数据域
#define WARN_MAX_SIZE 32
#define ILGL_MAX_SIZE 32
#define FLT_MAX_SIZE 62

typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint8_t packtype[2];    // 包类型待定
    SensorDat gravitylift;  // 吊重力
    SensorDat height;       // 高度
    SensorDat scope;        // 幅度
    SensorDat rotary;       // 回转角度
    SensorDat wind;         // 风速
    SensorDat dipangle;     // 倾角
    SensorDat moment;       // 力矩比率
    SensorDat walk;         // 行走
    SensorDat rev1[4];      // 预留4bytes
    uint8_t status;         // 状态字
    // 报警状态
    uint8_t warning[WARN_MAX_SIZE]; // 报警信息不定
    uint8_t illegal[ILGL_MAX_SIZE]; // 违章信息不定
    uint8_t fault[FLT_MAX_SIZE ];       // 故障信息不定
    uint8_t rev2[6];    // 预留6bytes
}DatWarn,*pDatWarn;

_EXTERN DatWarn warningdata;    // 报警实时数据


typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // 包类型待定
    SensorDat pm25;     // pm25
    SensorDat pm10;     // pm10
    SensorDat temperature;  // 温度
    SensorDat humidity;     // 湿度
    SensorDat wind;         // 风速
    SensorDat valve[4];     // 电磁阀一、二、三、四
    SensorDat noise;        // 噪声
    SensorDat vane;     // 风向

    uint8_t rev[10];    // 保留

    uint8_t rev1[6];    // 保留
    uint8_t status;         // 状态字
    // 报警状态
    uint8_t warning[WARN_MAX_SIZE]; // 报警信息不定
    uint8_t illegal[ILGL_MAX_SIZE]; // 违章信息不定
    uint8_t fault[FLT_MAX_SIZE ];       // 故障信息不定
    uint8_t rev2[6];    // 预留6bytes
}DatDustWarn,*pDatDustWarn;

_EXTERN DatDustWarn DustWrnData;    // 扬尘报警实时数据

typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;
    uint8_t packtype[2];    // 包类型待定
    SensorDat weight;     // 卸料平台总称重
    SensorDat cableleft;     // 斜拉索一
    SensorDat cableright;    // 斜拉索二
    SensorDat weightsensor[16];  // 称重传感器1~16

    uint8_t rev[20];    // 保留

    uint8_t rev1[6];    // 保留
    uint8_t status;         // 状态字
    // 报警状态
    uint8_t warning[WARN_MAX_SIZE]; // 报警信息不定
    uint8_t illegal[ILGL_MAX_SIZE]; // 违章信息不定
    uint8_t fault[FLT_MAX_SIZE ];   // 故障信息不定
    uint8_t rev2[6];    // 预留6bytes
}DatUPPlatWarn,*pDatUPPlatWarn;

_EXTERN DatUPPlatWarn UPPlatWrnData;    // 卸料平台实时数据

typedef struct
{
    RtcClk clock;   // 时间BCD，6bytes
    uint32_t name_id;       // 人员标识码
    uint8_t packtype[2];    // 包类型待定
    SensorDat gravitylift;  // 吊重力
    SensorDat height;       // 高度
    SensorDat speed;        // 速度
    SensorDat wind;         // 风速
    SensorDat dipangle;     // 倾角
    SensorDat motor[3];     // 电机一、二、三

    uint8_t people_flag;
    uint8_t people_alarm;
    uint8_t people_value;

    uint8_t floor_flag;
    uint8_t floor_aligned;
    uint8_t floor_value;

    uint8_t door_limit;// 门及限位状态

    uint8_t rev[19];    // 保留

    uint8_t rev1[6];    // 保留
    uint8_t status;         // 状态字
    // 报警状态
    uint8_t warning[WARN_MAX_SIZE]; // 报警信息不定
    uint8_t illegal[ILGL_MAX_SIZE]; // 违章信息不定
    uint8_t fault[FLT_MAX_SIZE ];       // 故障信息不定
    uint8_t rev2[6];    // 预留6bytes
}DatElvtWarn,*pDatElvtWarn;

_EXTERN DatElvtWarn ElvtWrnData;    // 升降机报警实时数据

// 标定实时数据
typedef struct
{
    RtcClk clock;           // 时间BCD，6bytes
    uint8_t packtype[2];    // 包类型待定
    SensorDat gravitylift;  // 吊重力
    SensorDat height;       // 高度
    SensorDat scope;        // 幅度
    SensorDat rotary;       // 回转角度
    SensorDat wind;         // 风速
    SensorDat dipangle;     // 倾角
    //SensorDat moment;     // 力矩比率
    SensorDat walk;         // 行走
    SensorDat rev1[4];      // 预留4bytes
}DatCalib,*pDatCalib;

_EXTERN DatCalib TowCaliDat;    // 塔机黑匣子标定实时数据
_EXTERN DatCalib ElvtCaliDat;   // 升降机标定实时数据

//====== elvt information

#define ELVT_MAXDAT 2       // maxim size of elevator table
typedef struct
{
    float height;       // Floor height
    uint16_t floor;         // floor number
}
APP_ELVTDAT_TypeDef;    // data buffer strcture of elevator
                            // 4B@20150923
// 指纹上报数据结构
typedef struct
{
    uint8_t flag;       //
    uint32_t staffid;   // 指纹所属人员标识码4bytes
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

    uint8_t people_count;   // 人数

    int8_t floor;   // 楼层

    uint8_t stat_low;   // 下限位状态
    uint8_t stat_high;  // 上限位状态
    uint8_t stat_outdoor;   // 出料门状态
    uint8_t stat_indoor;    // 进料门状态
    uint8_t warn_outdoor;   // 出料门报警
    uint8_t warn_indoor;    // 进了门报警

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

    uint8_t people_count;   // 人数

    int8_t floor;   // 楼层

    uint8_t stat_low;   // 下限位状态
    uint8_t stat_high;  // 上限位状态
    uint8_t stat_outdoor;   // 出料门状态
    uint8_t stat_indoor;    // 进料门状态
    uint8_t warn_outdoor;   // 出料门报警
    uint8_t warn_indoor;    // 进了门报警

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
// 全局状态标志位，用于判断依据

// 定义一个最长帧存放数组
//uint8_t extparam.buf[MAX_SIZE+12];
_EXTERN uint8_t RFULL_frame[MAX_SIZE+12];

_EXTERN uint8_t crcbuf[255];    // 该缓冲区用来存储用户数据用于计算CRC校验位

// 上位机下发设定参数，需要在终端保存，临时定义，需要时可替换
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

