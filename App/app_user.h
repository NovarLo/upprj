/*   */
/*   */


#ifdef _LOCAL_MAIN
#define _EXTERN
#else
#define _EXTERN extern
#endif

//#define TOWERBOX
//#define ELIVATOR
//#define DUSTMON
#define UPPLAT

//---------- switch of behavior -----------//

#define WDOG_ON

//#define RPT_WARN      // report warning to sever if actived
#define COLLISION_ON
//#define PITCH_ON      // pitch for movable-arm  tower


#define PSWD_USER   "4006029012"        // 02968581898
#define PSWD_DBG    "798925"
#define PSWD_ADMIN  "097538361"
#define PSWD_OPT    "071112"    // version option configuration

//#define QRC_MFR   "www.htachina.com"      // off: hide, string: info to show

/*************************** system information ******************************/

#define DEV_VERSOFT     0x53        // software ver5.02
#define DEV_VERPRTCL    0x11        // protocol ver1.1
#define DEV_VERMMI      0x25        // MMI ver2.5

#define IPV4_01_BYTE1   122         // default: 122.114.22.87:8086
#define IPV4_01_BYTE2   114
#define IPV4_01_BYTE3   22
#define IPV4_01_BYTE4   87
#define IPV4_01_PORT    8086

/**************************************  
  0 —— 陕西泰新博坤智能科技有限公司
  1 —— 青岛一开电气科技有限公司
  2 —— 郑州蓝宇环保科技有限公司
  3 —— 陕西荣炜建筑科技有限公司
  4 —— 西安明路环保科技有限公司
  5 —— 四川中桴美达科技有限公司
  6 —— 厦门日升建机信息科技有限公司
  7 —— 山东微控科技发展集团有限公司
  8 —— 渝仁蓉达科技有限公司
  9 —— 广联达(西安)筑材网络科技有限公司
  10—— 兰州新区智能信息技术发展有限公司
  11—— 咸阳盛林鑫环保科技有限公司
***************************************/
#define Default_manufacturer 0

// 陕西泰新博坤智能科技有限公司
#define TXBK_DEV_MFR         0x0001      // 0x00 01: xi'an TaiXin
#define TXBK_DEV_MODEL       0x0003      // 0x00 02: SPS20A
#define TXBK_DEV_REGION      29          // 029: shaanxi (default)

#define TXBK_ICONIDX_LOGO    ICONIDX_LOGOTX
#define TXBK_ICONIDX_QRC     ICONIDX_QRCTXBK

#define TXBK_TXTLOGO1    "扬尘在线监测系统"
#define TXBK_TXTLOGO2    "DCS-II"

// 青岛一开电气科技有限公司
#define EKYJ_DEV_MFR         0x0006
#define EKYJ_DEV_MODEL       3
#define EKYJ_DEV_REGION      29

#define EKYJ_ICONIDX_LOGO    ICONIDX_LOGOQDYK
#define EKYJ_ICONIDX_QRC     ICONIDX_QRCNULL

#define EKYJ_TXTLOGO1    "扬尘智能在线监测控制系统"
#define EKYJ_TXTLOGO2    "EKYJ-07"

//郑州蓝宇环保科技有限公司
#define ZZLY_DEV_MFR         0x000B
#define ZZLY_DEV_MODEL       3
#define ZZLY_DEV_REGION      38

#define ZZLY_ICONIDX_LOGO    ICONIDX_LOGOLYHB
#define ZZLY_ICONIDX_QRC     ICONIDX_QRCNULL

#define ZZLY_TXTLOGO1    "扬尘智能在线监测控制系统"
#define ZZLY_TXTLOGO2    "LY-02"

//西安明路环保科技有限公司
#define XAML_DEV_MFR         0x0009
#define XAML_DEV_MODEL       3
#define XAML_DEV_REGION      29

#define XAML_ICONIDX_LOGO    ICONIDX_LOGOMLHB
#define XAML_ICONIDX_QRC     ICONIDX_QRCNULL

#define XAML_TXTLOGO1    "扬尘智能在线监测控制系统"
#define XAML_TXTLOGO2    "ML-Z1000"

// 陕西荣炜建筑科技有限公司
#define SXRW_DEV_MFR         7
#define SXRW_DEV_MODEL       3
#define SXRW_DEV_REGION      29

#define SXRW_ICONIDX_LOGO    ICONIDX_LOGOSXRW
#define SXRW_ICONIDX_QRC     ICONIDX_QRCNULL

#define SXRW_TXTLOGO1    "扬尘在线监测系统"
#define SXRW_TXTLOGO2    "RYC-II"

// 四川南充
#define NCSC_DEV_MFR         1
#define NCSC_DEV_MODEL       3
#define NCSC_DEV_REGION      29

#define NCSC_ICONIDX_LOGO    ICONIDX_LOGONULL
#define NCSC_ICONIDX_QRC     ICONIDX_QRCNULL

#define NCSC_TXTLOGO1    "南充市扬尘在线监测系统"
#define NCSC_TXTLOGO2    "DCS-II"


// 四川中桴美达科技有限公司
#define ZFMD_DEV_MFR         2
#define ZFMD_DEV_MODEL       3
#define ZFMD_DEV_REGION      28

#define ZFMD_ICONIDX_LOGO    ICONIDX_LOGOZFMD
#define ZFMD_ICONIDX_QRC     ICONIDX_QRCZFMD_CHS

#define ZFMD_TXTLOGO1    "扬尘在线监测系统"
#define ZFMD_TXTLOGO2    "ZDOM-01"


// 厦门日升建机信息科技有限公司
#define XMRS_DEV_MFR         3
#define XMRS_DEV_MODEL       3
#define XMRS_DEV_REGION      29

#define XMRS_ICONIDX_LOGO    ICONIDX_LOGOXMRISHENG
#define XMRS_ICONIDX_QRC     ICONIDX_QRCXMRISHENG

#define XMRS_TXTLOGO1    "扬尘在线监测系统"
#define XMRS_TXTLOGO2    "DCS-II"

// 山东微控科技发展集团有限公司
#define SDWK_DEV_MFR         4
#define SDWK_DEV_MODEL       3
#define SDWK_DEV_REGION      29

#define SDWK_ICONIDX_LOGO    ICONIDX_LOGOSDWEIKONG
#define SDWK_ICONIDX_QRC     ICONIDX_QRCNULL

#define SDWK_TXTLOGO1    "扬尘在线监测系统"
#define SDWK_TXTLOGO2    "DCS-II"

// 渝仁蓉达科技有限公司
#define YRRD_DEV_MFR         6
#define YRRD_DEV_MODEL       3
#define YRRD_DEV_REGION      23

#define YRRD_ICONIDX_LOGO    ICONIDX_LOGOCQYRRD
#define YRRD_ICONIDX_QRC     ICONIDX_QRCNULL

#define YRRD_TXTLOGO1    "扬尘在线监测系统"
#define YRRD_TXTLOGO2    "YRDM-01"

// 广联达(西安)筑材网络科技有限公司
#define GLD_DEV_MFR         1           // 0x00 01: xi'an TaiXin
#define GLD_DEV_MODEL       1           // 0x00 01: SPS32+
#define GLD_DEV_REGION      29          // 029: shaanxi (default)

#define GLD_ICONIDX_LOGO    ICONIDX_LOGOGLD
#define GLD_ICONIDX_QRC     ICONIDX_QRCNULL

#define GLD_TXTLOGO1    "扬尘在线监测系统"
#define GLD_TXTLOGO2    "GTJ02"

//咸阳盛林鑫环保科技有限公司
#define SLX_DEV_MFR         1           // 0x00 01: xi'an TaiXin
#define SLX_DEV_MODEL       1           // 0x00 01: SPS32+
#define SLX_DEV_REGION      29          // 029: shaanxi (default)

#define SLX_ICONIDX_LOGO    ICONIDX_LOGOSLX
#define SLX_ICONIDX_QRC     ICONIDX_QRCNULL

#define SLX_TXTLOGO1    "扬尘在线监测系统"
#define SLX_TXTLOGO2    "YCJK2018"
 
//兰州新区智能信息技术发展有限公司
#define LZXQ_DEV_MFR         1           // 0x00 01: xi'an TaiXin
#define LZXQ_DEV_MODEL       1           // 0x00 01: SPS32+
#define LZXQ_DEV_REGION      29          // 029: shaanxi (default)

#define LZXQ_ICONIDX_LOGO    ICONIDX_LOGOLZXQ
#define LZXQ_ICONIDX_QRC     ICONIDX_QRCNULL

#define LZXQ_TXTLOGO1    "扬尘在线监测系统"
#define LZXQ_TXTLOGO2    "DCS-II"

////---------- switch of test ------------//

//#define NVRAM_INI         // NVRAM ini
//#define NVRAM_CNT         // NVRAM size count

//#define MMI_DEBUG         // mmi show debug dat


#define AUDIO_LEVEL 7

#define _AUDIO_ROTATERROR
#define _AUDIO_ROTATALARM
#define _AUDIO_ROTATWARN

#if AUDIO_LEVEL == 0
#define _AUDIO_TIPS     1
//  #define _AUDIO_WARN 1
//  #define _AUDIO_ALARM    1
//  #define _AUDIO_ERROR    1
//  #define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 1
//  #define _AUDIO_TIPS     1
//  #define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
//  #define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 2
//  #define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
//  #define _AUDIO_ALARM    1
//  #define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 3
//  #define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
//  #define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 4
//  #define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
#define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 5
#define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
#define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
//  #define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 6
#define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
#define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
//  #define _AUDIO_FAULT    1
#define _AUDIO_COLLISION    1
#elif AUDIO_LEVEL == 7
#define _AUDIO_TIPS     1
#define _AUDIO_WARN 1
#define _AUDIO_ALARM    1
#define _AUDIO_ERROR    1
#define _AUDIO_FINGER   1
#define _AUDIO_FAULT    1
#define _AUDIO_COLLISION    1
#endif

/* ***************************** common sections ******************************/

/////////// type & variables
typedef enum
{
	FALSE = 0,
//  false=0,
	TRUE = 1,
//  true = 1
}
bool;
#define BOOL    bool

#define BIT0        (1 << 0)
#define BIT1        (1 << 1)
#define BIT2        (1 << 2)
#define BIT3        (1 << 3)
#define BIT4        (1 << 4)
#define BIT5        (1 << 5)
#define BIT6        (1 << 6)
#define BIT7        (1 << 7)

#define BIT8        (1 << 8)
#define BIT9        (1 << 9)
#define BIT10   (1 << 10)
#define BIT11   (1 << 11)
#define BIT12   (1 << 12)
#define BIT13   (1 << 13)
#define BIT14   (1 << 14)
#define BIT15   (1 << 15)


////// globle resources

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern I2S_HandleTypeDef hi2s2;
extern DMA_HandleTypeDef hdma_spi2_tx;

extern IWDG_HandleTypeDef hiwdg;

extern RNG_HandleTypeDef hrng;

extern RTC_HandleTypeDef hrtc;

extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_spi3_rx;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim14;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

extern NAND_HandleTypeDef hnand1;



/////////// variables

_EXTERN uint32_t tick_powerup;  // system-tick value of powerup
_EXTERN uint8_t gprs_rssi;          // gprs Received Signal Strength Indicator
_EXTERN uint8_t DTU_rssi;       // 
_EXTERN uint8_t zigbee_rssi;        // Zigbee Received Signal Strength Indicator
_EXTERN uint8_t gps_fixed;      // stat of GPS,~0-invalid, 0-unfixed, 1-fixed
_EXTERN uint32_t NAND_WPTR;


_EXTERN bool flag_wklp_check;       // flag of work-loop check
_EXTERN bool flag_alarm_find;           // flag of alarm find

#define DRIVER_NAMESIZE 13
typedef struct
{
	BOOL sensor_valid;
	BOOL finger_valid;                  // TRUE / FLASE, flag of ID check
	uint8_t id;                         // id of check, 1-no finger, 2-refused
	uint8_t name[DRIVER_NAMESIZE];      // name of driver
	uint32_t name_id;                       // id of driver
}
APP_DRIVER_TypeDef;

_EXTERN APP_DRIVER_TypeDef driver;


/////////// constant

#define COLLISION_MAXTOWERS     32      // max number of tower in a group


// timer counter & limit
#define TIM6_STEP   5               // step of timer6 in mili-seconds
#define TIM_FINDALARM       95      // interval of find-alarm

#define SENSOR_TMOUT    3000    // timeout of sensor in mili-seconds

//
_EXTERN BOOL flag_nandwrsave;       // flag to save nand pointer

_EXTERN uint32_t timer_beat;        // timer of heart-beat
_EXTERN BOOL flag_beat;         // flag to send heart-beat frame

_EXTERN uint32_t timer_recon;       // timer of reconnect
_EXTERN BOOL flag_recon;            // flag to reconnect

_EXTERN uint32_t timer_datsave; // timer of real-time data save
_EXTERN BOOL flag_datsave;      // flag of real-time data save

_EXTERN uint32_t timer_datrpt;      // timer of real-time data report
_EXTERN BOOL flag_datrpt;       // flag of real-time data report

_EXTERN BOOL flag_framrst;      // flag of fram reset



//// IO port define

#define MAX_PWM_DUTY    1.0f            // 100%, max duty of PWM, valid range 1% ~ 99%
#define MAX_UPSNR_DUTY  1.0f
#define MAX_FRQ_DAT 100         // max frequnce, 100 means 45m/s for wind speed.
#define MAX_ADC_DAT (1 << 12)   //full scale of 12-bit ADC, valid range 0 ~ 4095.

// full scale of SSI port, valid range 0 ~ x-1.
#define MAX_SSI_DAT (1 << ((device_info.rotat_cfg & 0x07) + 8))


// SENSOR BONDING
typedef enum
{
	SENSOR_MINCHANNEL = 0,

	// ----- fixed sonsor channel ------
	CH_PWM1 = 0,            // pwm channels
	CH_PWM2,
	CH_PWM3,
	CH_PWM4,
	CH_PWM5,

	CH_FRQ1,                // frequence channels
	CH_FRQ2,

	CH_ANA1,                // analog channels
	CH_ANA2,

	CH_SSI,                 // SSI channels

	// ----- flexible sonsor channel ------

	CH_SPEED = 10,          // speed channel
	CH_WEIGHT,              // weight
	CH_PEOPLE,              // people channel

	CH_FINGER = 15,         // finger channel

	CH_TEMP = 16,           // temprature channel
	CH_HUMIDITY = 17,       // humidity channel
	CH_UPSNR1,            // uploading platform weight sensor
	CH_UPSNR2,            // uploading platform weight sensor
	CH_UPSNR3,            // uploading platform weight sensor
	CH_UPSNR4,            // uploading platform weight sensor
	CH_UPSNR5,            // uploading platform weight sensor
	CH_UPSNR6,            // uploading platform weight sensor
	CH_UPSNR7,            // uploading platform weight sensor
	CH_UPSNR8,            // uploading platform weight sensor
	CH_UPSNR9,            // uploading platform weight sensor
	CH_UPSNR10,            // uploading platform weight sensor
	CH_UPSNR11,            // uploading platform weight sensor
	CH_UPSNR12,            // uploading platform weight sensor
	CH_UPSNR13,            // uploading platform weight sensor
	CH_UPSNR14,            // uploading platform weight sensor
	CH_UPSNR15,            // uploading platform weight sensor
	CH_UPSNR16,            // uploading platform weight sensor

	CH_UPWEIGHT,           // uploading platform weight channel CH_UPSNR1+CH_UPSNR2+...+CH_UPSNR8

	SENSOR_MAXCHANNEL  // maxim channel of sensor
}
SENSOR_CHANNELS;


//// normal sensor bonding

//#define   WEIGHT_PWM      // enabled when weight sensor is on PWM channel
#define PM_PWM  // enabled when PM2.5&PM10 sensor is on pwm channel


#ifdef  PM_PWM
#define SENS_PM25       CH_PWM1     // pm2.5 on PWM1
#define SENS_PM10       CH_PWM2     // pm10 on PWM2
#define MAX_PM_DAT  (MAX_PWM_DUTY * sensor_dat[SENS_PM25].scale)
#else
#define SENS_PM25       CH_ANA1         // pm2.5 on ANA1
#define SENS_PM10       CH_ANA2         // pm10 on ANA2
#define MAX_PM_DAT  ((MAX_ADC_DAT-1) * sensor_dat[SENS_PM25].scale)
#endif

#define SENS_HEIGHT     CH_PWM3     // height on PWM3
#define MAX_HEIGHT_DAT  (MAX_PWM_DUTY* sensor_dat[SENS_HEIGHT].scale)

#define SENS_WIND       CH_FRQ2     // wind on FRQ2
#define SENS_VANE       CH_ANA2     // wind vane
#define SENS_TILT       CH_PWM5 // tilt on PWM5

// other sensor bonding
#define SENS_SPEED      CH_SPEED        // speed
//#define   SENS_HEIGHT_REAL    CH_HEIGHT_REAL  // height include offset
#define SENS_WEIGHT     CH_WEIGHT       // number of people
#define SENS_PEOPLE     CH_PEOPLE       // number of people
#define SENS_FINGER     CH_FINGER       // finger

#define SENS_TEMP       CH_TEMP
#define SENS_HMDT       CH_HUMIDITY

// noise
#define SENS_NOISE      CH_ANA1

// uploading platform
#define SENS_UPSNR1     CH_UPSNR1
#define SENS_UPSNR2     CH_UPSNR2
#define SENS_UPSNR3     CH_UPSNR3
#define SENS_UPSNR4     CH_UPSNR4
#define SENS_UPSNR5     CH_UPSNR5
#define SENS_UPSNR6     CH_UPSNR6
#define SENS_UPSNR7     CH_UPSNR7
#define SENS_UPSNR8     CH_UPSNR8

#define SENS_UPWEIGHT   CH_UPWEIGHT
#define MAX_UPWT_DAT  (MAX_ADC_DAT * 8 * sensor_dat[SENS_UPWEIGHT].scale)

#define SENS_UPSNR9     CH_UPSNR9
#define SENS_UPSNR10     CH_UPSNR10
#define SENS_UPSNR11     CH_UPSNR11
#define SENS_UPSNR12     CH_UPSNR12
#define SENS_UPSNR13     CH_UPSNR13
#define SENS_UPSNR14     CH_UPSNR14
#define SENS_UPSNR15     CH_UPSNR15
#define SENS_UPSNR16     CH_UPSNR16

#define SENS_CABL       CH_UPSNR9
#define SENS_CABR       CH_UPSNR10

#define MAX_UPWIRE_DAT  (MAX_ADC_DAT * sensor_dat[SENS_UPWEIGHT].scale)
// UART
#define UART_232        USART1
#define UART_LED        UART_232
#define UART_TFT        USART3
#define UART_ZIGBEE UART4
#define UART_GPRS       UART5
#define UART_UP        USART6

#define HDL_UART_232    &huart1     // handle of UART1
#define HDL_UART_LED    HDL_UART_232        // handle of UART1
#define HDL_UART_TFT    &huart3     // handle of USART3
#define HDL_UART_ZIGBEE &huart4     // handle of UART4
#define HDL_UART_GPRS   &huart5     // handle of UART5
#define HDL_UART_UP    &huart6     // handle of USART6


/////////// macros

#define _WDOG_ON()  __HAL_IWDG_START(&hiwdg)
#define _WDOG_KICK()    __HAL_IWDG_RELOAD_COUNTER(&hiwdg)


// LEDs

#define _LED_TST2_ON()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET)
#define _LED_TST2_OFF() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET)
#define _LED_TST2_TOGGLE()  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0)

#define _LED_TST3_ON()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET)
#define _LED_TST3_OFF() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET)
#define _LED_TST3_TOGGLE()  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1)

#define _LED_TST4_ON()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
#define _LED_TST4_OFF() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
#define _LED_TST4_TOGGLE()  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2)

#define _LED_RUN_ON()   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_RESET)
#define _LED_RUN_OFF()  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_SET)
#define _LED_RUN_TOGGLE()   HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_8)

#define _LED_COM_ON()   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET)
#define _LED_COM_OFF()  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET)
#define _LED_COM_TOGGLE()   HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_7)

#define _LED_ERR_ON()   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_RESET)
#define _LED_ERR_OFF()  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_SET)
#define _LED_ERR_TOGGLE()   HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_5)


// Zigbee config
#define _ZIGBEE_CFG_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET)
#define _ZIGBEE_CFG_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET)


// GPRS
#define _VGPRS_ON() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET)
#define _VGPRS_OFF()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET)

#define _GPRS_PWKEY_PUSH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET)
#define _GPRS_PWKEY_RELEASE()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET)

#define _IS_GPRS_ON()   (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_SET)
#define _IS_SIM_EXSIT() (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET)

// IIS
#define _AMP_ON()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define _AMP_OFF()      //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)

#define IIS_SPK         I2S2ext // handle of speaker
#define HDL_IIS_SPK &hi2s2  // handle of speaker

// GPIO PORTS
#define _IS_DOORIN_OPEN()       (BOOL)(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == GPIO_PIN_SET)
#define _IS_DOOROUT_OPEN()  (BOOL)(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET)
#define _IS_LIMITTOP_ON()       (BOOL)(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_SET)
#define _IS_LIMITBOT_ON()       (BOOL)(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_4) == GPIO_PIN_SET)
#define _IS_GPIO5_ON()          (BOOL)(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_SET)

#define _RLY1_OFF()     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)
#define _RLY1_ON()      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)
#define _RLY2_OFF()     HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET)
#define _RLY2_ON()      HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET)
#define _RLY3_OFF()     HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET)
#define _RLY3_ON()      HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET)
#define _RLY4_OFF()     HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET)
#define _RLY4_ON()      HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET)


// FRAM
#define _FM_NCS_ENABLE()        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, GPIO_PIN_RESET)
#define _FM_NCS_DISABLE()       HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, GPIO_PIN_SET)

#define HDL_SPI_FRAM    &hspi3  // handle of FRAM

// NVRAM address for configuration data
#define NVADDR_DUSTMONINFO  0x0000      // base-addr of elivator info
#define NVADDR_DEVINFO      0x00B0      // base-addr of device info
#define NVADDR_DEVVER       0x01D0      // base-addr of device version

//#define   NVADDR_ROTAT            0x01E0      // base-addr of rotat_dat
#define NVADDR_NANDWR       0x01E4      // base-addr of NAND write-addr

//#define   NVADDR_ZONETBL      0x0400      // base-addr of zone-table
//#define   NVADDR_PROTECTTBL   0x1300      // base-addr of protect-table
#define NVADDR_LIMITTBL     0x1400      // base-addr of limit-table
#define NVADDR_FLOORTBL     0x1600      // base-addr of floor-table
#define NVADDR_CALITBL      0x1800      // base-addr of cali-table
#define NVADDR_NOTICE       0x1E00      // base-addr of notice

#define MVADDR_MAX          0x1FFF      // last byte of NVRAM





/* ***************************** config sections ******************************/
/////////// type & variables

typedef enum
{
	BUFSTAT_NULL = 0,       // buffer empty
	BUFSTAT_READY,      // data ready to use

	BUFSTAT_MAX
}
APP_BUFSTAT_TypeDef;



typedef enum
{
	CFGSTAT_INVALID = 0,  // means data buffer invalid
	CFGSTAT_VALID,          // neans data buffer valid

	// CAUTION: temporary request below, should be
	// replaced with "CFGSTAT_VALID" in NVRAM-processing

	CFGSTAT_SAVE = 8,       // means data buffer need to be saved
	CFGSTAT_LOAD,           // means data buffer need to be load

	CFGSTAT_MAX
}
APP_CFGSTAT_TypeDef;


//====== elivator information

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	uint8_t mfr_ID;     // ID of manufacture
	uint8_t model_ID;       // ID of model

	float rated_load;       // rated load of tower, kg
	uint8_t people;     // number of people
	uint8_t midweight;  // middle-weight, kg

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_TOWERINFO_TypeDef;      // 92B@20150707

_EXTERN APP_TOWERINFO_TypeDef elivator_info;


typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	uint8_t mfr_ID;     // ID of manufacture
	uint8_t model_ID;       // ID of model

	float value1;       // valve 1 limit value
	float value2;       // valve 2 limit value
	float value3;       // valve 3 limit value
	float value4;       // valve 4 limit value

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_DUSTMONVALVE_TypeDef;       // 92B@20150707

_EXTERN APP_DUSTMONVALVE_TypeDef valve_lmt;

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;
	//----------- configuration section ------------
	uint8_t mfr_ID;     // ID of manufacture
	uint8_t model_ID;       // ID of model

	float threshold[8];   // Solenoid valve 1/2/3/4 threshold for pm2.5&pm10
	BOOL thrshdflag[8]; // threshold 1/2/3/4 enable flag
	BOOL manualflag[4];

	BOOL soundsensor_en;
	BOOL outdoorled_4lines_en;
	uint8_t company_no;

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_VALVEINFO_Typedef;

_EXTERN APP_VALVEINFO_Typedef dustmon_info;


// information for outdoor led notice
typedef struct
{
	uint16_t year_start;
	uint8_t month_start;
	uint8_t day_start;
	uint16_t year_end;
	uint8_t month_end;
	uint8_t day_end;
	uint8_t week;
	uint8_t timeslot_hour_start;
	uint8_t timeslot_min_start;
	uint8_t timeslot_hour_end;
	uint8_t timeslot_min_end;
	uint8_t display_char[234];
}APP_DUSTOLED_TypeDef;
typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;
	//----------- configuration section ------------
	BOOL oled_update;
	APP_DUSTOLED_TypeDef oled_dispchar;
	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_NOTCIE_Typedef;

_EXTERN APP_NOTCIE_Typedef dustmon_notice;

_EXTERN float temperature;
_EXTERN float humidity;





//====== device version
typedef enum
{
	DTU_MG2639 = 0,
	DTU_SIM7600CEL,

	DTU_TYPE_MAX
}
APP_DTUTYPE_typedef;

typedef enum
{
    PMSENSOR_SDS011 = 0,
    PMSENSOR_YT2510,

    PMSEN_TYPE_MAX
}
APP_pmsensor_typedef;

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	uint8_t mfr[2];     // "TX" : "xi'an TaiXin";
	uint8_t model[2];       //  0x00 00 : SPS32A
	uint8_t ver_soft;       // version of software
	uint8_t ver_prtcl;      // version of protocol
	uint8_t ver_mmi;        // version of MMI
	uint8_t ver_dtu;		// version of hardware
    uint8_t ver_pmsen;      // version of pm sensor

	uint8_t spare;

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_DEVVER_TypeDef;     // 12B@20150706

_EXTERN APP_DEVVER_TypeDef device_ver;



//====== device infomation

typedef enum
{
	WKMOD_MIX = 0,      // mixture mode
	WKMOD_SELF,     // self-report mode
	WKMOD_ASK,      // ask-anwser mode
	WKMOD_DEBUG,        // debug mode
	WKMOD_CALI,     // calibration mode

	WKMOD_MAX
}
APP_WKMOD_TypeDef;

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----- configuration section 1, mustn't modify ------
	uint8_t pswd[8][16];        // password of device

	uint8_t addr[5];            // address of device
	uint8_t ip_port[4][6];      // IP & port for GPRS

	uint8_t work_mod;       // work mode, see APP_WKMOD_TypeDef
	uint8_t sensor_en[SENSOR_MAXCHANNEL];   // TRUE/FALSE, sensor enable flag

	uint16_t beat_time;     // interval of heartbeat in minutes
	uint16_t recon_time;        // interval of reconnect in minutes
	uint16_t datrpt_time;       // interval of dat-report in seconds
	uint16_t datsave_time;  // interval of dat-save in seconds
	uint16_t link_timeout;      // timeout of link in seconds

	//----------- ---------------------------
	float height_offset;        // offset of height
	float weight_tare;

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_DEVINFO_TypeDef;    // 213B@20150706

_EXTERN APP_DEVINFO_TypeDef device_info;



//====== floor information

#define FLOOR_MAXTYPE   10      // maxim data of floor buffer

typedef struct
{
	float   height;             // height of floor
	uint32_t number;            // number of floors
}
APP_FLOORDAT_TypeDef;
// 8B@20150707

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	uint32_t    tblsize;            // actual size of table
	uint32_t    floor;          // total floor of building
	float    height;                // total height of building
	APP_FLOORDAT_TypeDef type[FLOOR_MAXTYPE];

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_FLOORTBL_TypeDef;

_EXTERN APP_FLOORTBL_TypeDef floor_tbl;



//====== limit information

typedef struct
{
	float lolimit;        // low limit
	float lowarn;       // low warning offset
	float hiwarn;       // high warning offset
	float hilimit;      // high limit
}
APP_LIMITDAT_TypeDef;   // 16B@20150707

typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	APP_LIMITDAT_TypeDef limit[SENSOR_MAXCHANNEL];

	//-------------- end flag section --------------
	float end;          // last object of structure
}
APP_LIMITTBL_TypeDef;       // 264B@20150707

_EXTERN APP_LIMITTBL_TypeDef limit_tbl;



//====== calibration information

#define CALI_MAXDAT 10      // maxim data of calibration buffer for each sensor

typedef struct
{
	uint32_t x;         // raw data of sensor, include scale
	float   y;              // actual value
}
APP_CALIDAT_TypeDef;
// 8B@20150707

typedef struct
{
	uint32_t    tblsize;            // actual size of table
	APP_CALIDAT_TypeDef dat[CALI_MAXDAT];
}
APP_CALICHDAT_TypeDef;
// 84B@20150707

#define UPSNR_NUM   8   // 卸料平台称重传感器数量（不含斜拉索）
#define UPSNR_WOC   1000    // weight of counterpoise (unit:kg)
typedef struct
{
	//-------------- control section ---------------
	uint16_t delay;     // delay in mili-seconds befor NVRAM-write process
	uint8_t flag;           // state of configuration data
							// see APP_CFGSTAT_TypeDef
	uint8_t spare1;

	//----------- configuration section ------------
	APP_CALICHDAT_TypeDef chdat[SENSOR_MAXCHANNEL];
	//----------- uploading platform weight sensor correct ------------
	uint16_t upsnr_code[UPSNR_NUM+2][UPSNR_NUM];
	uint16_t upsnr_cornweight;  // 压角重量
	uint16_t upsnr_dispweight;  // 分散总重量
	double	alpha[UPSNR_NUM];	// 修正系数
	bool algorithm_enable;		// 是否使能算法
	//-------------- end flag section --------------
	float end;              // last object of structure
}
APP_CALITBL_TypeDef;    // data table strcture of calibration
						// 1352B@20150707

_EXTERN APP_CALITBL_TypeDef cali_tbl;

//
#define CNT_MEASURE 10
#define CNT_SENSOR  8
typedef struct
{
	uint16_t load_code[CNT_MEASURE][CNT_SENSOR];
	float coef_K[CNT_SENSOR];
	float coef_B[CNT_SENSOR];
}APP_CALIKB_TypeDef;

_EXTERN APP_CALIKB_TypeDef cali_kb;

/////////// macros


/////////// functions


void delay(uint32_t clk);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

void APP_ADC1_ini(void);
void APP_ADC1_convert(void);

void APP_devinfo_default(void);
void APP_devinfo_ini(void);
void APP_devinfo_manage(void);

void APP_devver_default(void);
void APP_devver_ini(void);
void APP_devver_manage(void);

void APP_dustmoninfo_default(void);
void APP_dustmoninfo_ini(void);
void APP_dustmoninfo_manage(void);

void APP_notice_default(void);
void APP_notice_ini(void);
void APP_notice_manage(void);

void APP_limittbl_default(void);
void APP_limittbl_ini(void);
void APP_limittbl_manage(void);

void APP_floortbl_default(void);
void APP_floortbl_ini(void);
void APP_floortbl_manage(void);

void APP_calitbl_default(void);
void APP_calitbl_ini(void);
void APP_calitbl_manage(void);

void APP_setcali_rotate(uint32_t x, float y);
void APP_setcali_height(float dat);
void APP_setcali_wind(float dat);



#undef _EXTERN

/* ******************************  END OF FILE  *******************************/


