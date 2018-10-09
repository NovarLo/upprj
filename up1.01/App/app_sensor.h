/*   */
/*   */

#ifdef  _LOCAL_SENSOR
    #define _EXTERN
#else
    #define _EXTERN extern
#endif


/* *************************** sensor data sections ***************************/


/////////// type & variables

#define SENS_BUFSIZE    16      // maxim size of data buffer for all sensor
typedef struct
{
    uint32_t    bufsize;                // actual size of buffer
    uint32_t    bufindex;           // index of buffer, point to where new data to write
    uint32_t    dat[SENS_BUFSIZE];  // buffer of raw data from sensor
    uint32_t    result;             // result in uint32 format, filtered
    uint32_t    scale;              // scale of result, realvlaue = result / scale
    uint32_t    runindex;           // index of scan, increased each sample
    uint32_t    newdat;             // data of last scan
    uint32_t    timeout;                // timeout counter
    uint32_t    count;              // valid date counter
    bool    update;                     // flag of newdat, set by scanner, reset after process
    bool    valid;                      // flag of whether all data are valid

}
APP_SENSBUF_TypeDef;        // data buffer strcture of sensor

_EXTERN APP_SENSBUF_TypeDef sensor_dat[SENSOR_MAXCHANNEL];


/////////// macros

#define _APP_SET_SENSOR_DAT(CH, DAT)    {   \
                sensor_dat[CH].newdat = (uint32_t) (DAT * sensor_dat[CH].scale + 0.5f); \
                sensor_dat[CH].update = TRUE;   \
                sensor_dat[CH].runindex++; \
                sensor_dat[CH].timeout = 0;}


/////////// functions

void APP_common_average(APP_SENSBUF_TypeDef* buf);
void APP_common_average_moving(APP_SENSBUF_TypeDef* buf);
void APP_common_average_weighted(APP_SENSBUF_TypeDef* buf,  float weight);
void APP_common_sort_up(float* buf);
void APP_common_sort_down(float* buf);


/* **************************** sensor app sections ***************************/
/////////// type & variables


#define BIT_LOERR   0x01
#define BIT_LOALARM 0x02
#define BIT_LOWARN  0x04
#define BIT_HIWARN  0x20
#define BIT_HIALARM 0x40
#define BIT_HIERR   0x80

typedef enum
{
    SENSOR_NOERR=0,

    SENSOR_LOERR = BIT_LOWARN | BIT_LOALARM | BIT_LOERR,    //0x07, below lower limit
    SENSOR_LOALARM = BIT_LOWARN | BIT_LOALARM,          //0x06, reach lower limit
    SENSOR_LOWARN = BIT_LOWARN,                         //0x04, near lower limit
    SENSOR_HIWARN = BIT_HIWARN,                         //0x20, near upper limit
    SENSOR_HIALARM = BIT_HIWARN | BIT_HIALARM,              //0x60, reach upper limit
    SENSOR_HIERR = BIT_HIWARN | BIT_HIALARM | BIT_HIERR,    //0xE0, over upper limit

    SENSOR_ERR = 0xFF,          // sensor error
    SENSOR_ALARM_MAX
}
APP_SENSALARMID_TypeDef;

typedef struct
{
    bool    valid;  // flag whether data is valid
    APP_SENSALARMID_TypeDef alarm;
    float   value;      // real-time value
}
APP_SENSVALUE_TypeDef;

_EXTERN APP_SENSVALUE_TypeDef   sensor_value[SENSOR_MAXCHANNEL];


typedef enum
{
    MOVESTAT_STOP = 0,
    MOVESTAT_UP,
    MOVESTAT_DOWN,

    MOVESTAT_MAX
}
APP_MOVESTAT_TypeDef;

typedef struct
{
        // value
    uint32_t floor;             // current floor, TBD
    BOOL aligned;               // flag of aligned to floor

    APP_MOVESTAT_TypeDef move;  // moving stat of elivator
    BOOL goingdown;             // last direction

    BOOL finger_alarm;

    BOOL alarm_doorin;          // alarm flag of door_in
    BOOL alarm_doorout;         // alarm flag of door_out

        // input
    BOOL doorin_open;
    BOOL doorout_open;
    BOOL alarm_top;
    BOOL alarm_bottom;
    BOOL gpio5;

        // output
    BOOL relay1_on;
}
APP_IOVALUE_TypeDef;

_EXTERN APP_IOVALUE_TypeDef elivator_stat;


typedef struct
{
    BOOL alarm_open2close[4];           // alarm flag of door_in
    BOOL alarm_close2open[4];

    // output
    BOOL valve1_en;
    BOOL valve2_en;
    BOOL valve3_en;
    BOOL valve4_en;
}
APP_VALVE_TypeDef;

_EXTERN APP_VALVE_TypeDef valve_stat;


/////////////// shadow of alarm, aginst & error

// bit-field of alarm-status
#define RPT_STATUSBIT_ALARM     0x01
#define RPT_STATUSBIT_AGAINST   0x02
#define RPT_STATUSBIT_ERROR     0x04

// common bit-field
#define RPT_ALARM_NULL      0
#define RPT_ALARM_WARN      1
#define RPT_ALARM_ALARM     2
#define RPT_ALARM_RSVD      3

// limit bit-field
#define RPT_ALARM_X_NULL        0
#define RPT_ALARM_X_WARN        1
#define RPT_ALARM_X_ALARM   2
#define RPT_ALARM_X_RSVD        3

// byte1 of limit-alarm
#define RPT_ALARM_LOW_NULL      (RPT_ALARM_X_NULL << 0)
#define RPT_ALARM_LOW_WARN      (RPT_ALARM_X_WARN << 0)
#define RPT_ALARM_LOW_ALARM     (RPT_ALARM_X_ALARM << 0)

#define RPT_ALARM_HIGH_NULL     (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_HIGH_WARN     (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_HIGH_ALARM        (RPT_ALARM_X_ALARM << 2)

#define RPT_ALARM_RIGHT_NULL        (RPT_ALARM_X_NULL << 4)
#define RPT_ALARM_RIGHT_WARN    (RPT_ALARM_X_WARN << 4)
#define RPT_ALARM_RIGHT_ALARM   (RPT_ALARM_X_ALARM << 4)

#define RPT_ALARM_LEFT_NULL     (RPT_ALARM_X_NULL << 6)
#define RPT_ALARM_LEFT_WARN     (RPT_ALARM_X_WARN << 6)
#define RPT_ALARM_LEFT_ALARM        (RPT_ALARM_X_ALARM << 6)

// byte2 of limit-alalm
 #define    RPT_ALARM_TORQUE_NULL   (RPT_ALARM_X_NULL << 0)
#define RPT_ALARM_TORQUE_WARN   (RPT_ALARM_X_WARN << 0)
#define RPT_ALARM_TORQUE_ALARM  (RPT_ALARM_X_ALARM << 0)

#define RPT_ALARM_WEIGHT_NULL   (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_WEIGHT_WARN   (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_WEIGHT_ALARM  (RPT_ALARM_X_ALARM << 2)

#define RPT_ALARM_PM25_NULL (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_PM25_WARN (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_PM25_ALARM    (RPT_ALARM_X_ALARM << 2)

#define RPT_ALARM_NEAR_NULL     (RPT_ALARM_X_NULL << 4)
#define RPT_ALARM_NEAR_WARN     (RPT_ALARM_X_WARN << 4)
#define RPT_ALARM_NEAR_ALARM    (RPT_ALARM_X_ALARM << 4)

#define RPT_ALARM_FAR_NULL      (RPT_ALARM_X_NULL << 6)
#define RPT_ALARM_FAR_WARN      (RPT_ALARM_X_WARN << 6)
#define RPT_ALARM_FAR_ALARM     (RPT_ALARM_X_ALARM << 6)

// byte3 of limit-alalm
 #define    RPT_ALARM_BACK_NULL     (RPT_ALARM_X_NULL << 0)
#define RPT_ALARM_BACK_WARN     (RPT_ALARM_X_WARN << 0)
#define RPT_ALARM_BACK_ALARM        (RPT_ALARM_X_ALARM << 0)

#define RPT_ALARM_FRONT_NULL        (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_FRONT_WARN    (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_FRONT_ALARM   (RPT_ALARM_X_ALARM << 2)

// byte3 of door-alalm
#define    RPT_ALARM_DOOROUT_NULL  (RPT_ALARM_X_NULL << 0)
#define RPT_ALARM_DOOROUT_WARN  (RPT_ALARM_X_WARN << 0)
#define RPT_ALARM_DOOROUT_ALARM (RPT_ALARM_X_ALARM << 0)

#define RPT_ALARM_DOORIN_NULL   (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_DOORIN_WARN   (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_DOORIN_ALARM  (RPT_ALARM_X_ALARM << 2)

#define RPT_ALARM_UPWEIGHT_NULL (RPT_ALARM_X_NULL << 4)
#define RPT_ALARM_UPWEIGHT_WARN (RPT_ALARM_X_WARN << 4)
#define RPT_ALARM_UPWEIGHT_ALARM    (RPT_ALARM_X_ALARM << 4)

#define RPT_ALARM_CABL_NULL (RPT_ALARM_X_NULL << 0)
#define RPT_ALARM_CABL_WARN (RPT_ALARM_X_WARN << 0)
#define RPT_ALARM_CABL_ALARM    (RPT_ALARM_X_ALARM << 0)

#define RPT_ALARM_CABR_NULL (RPT_ALARM_X_NULL << 2)
#define RPT_ALARM_CABR_WARN (RPT_ALARM_X_WARN << 2)
#define RPT_ALARM_CABR_ALARM    (RPT_ALARM_X_ALARM << 2)


typedef enum
{
    RPT_ALARMID_COLLISION = 1,
    RPT_ALARMID_FORBIDDEN,
    RPT_ALARMID_OBSTACLE,
    RPT_ALARMID_LIMIT,
    RPT_ALARMID_WEIGHT,
    RPT_ALARMID_TORQUE,
    RPT_ALARMID_WIND,
    RPT_ALARMID_TILT,
    RPT_ALARMID_WALK,       // TBD

    RPT_ALARMID_DOOR,
    RPT_ALARMID_PEOPLE,
    RPT_ALARMID_VALVE = 32,
    RPT_ALARMID_UPPLAT,

    RPT_ALARMID_MAX
}
RPT_ALARMID_TypeDef;
#define RPT_ALARMID_SIZE    3   // size in byte of alarm attrib


typedef enum
{
    RPT_AGAINSTID_COLLISION = 1,
    RPT_AGAINSTID_FORBIDDEN,
    RPT_AGAINSTID_OBSTACLE,
    RPT_AGAINSTID_LIMIT,
    RPT_AGAINSTID_WEIGHT,
    RPT_AGAINSTID_TORQUE,
    RPT_AGAINSTID_WIND,
    RPT_AGAINSTID_TILT,
    RPT_AGAINSTID_IDENTITY,

    RPT_AGAINSTID_MAX
}
RPT_AGAINSTID_TypeDef;
#define RPT_AGAINSTID_SIZE  2   // size in byte of against attrib


typedef enum
{
    RPT_ERRORID_NOERROR = 0,

            // error code of limiter
    RPT_ERRORID_LIMITLEFT = 0x11,
    RPT_ERRORID_LIMITRIGHT,
    RPT_ERRORID_LIMITFAR,
    RPT_ERRORID_LIMITNEAR,
    RPT_ERRORID_LIMITHIGH,
    RPT_ERRORID_LIMITLOW,
    RPT_ERRORID_LIMITPM25,
    RPT_ERRORID_LIMITWEIGHT,
    RPT_ERRORID_LIMITTORQUE,
    RPT_ERRORID_LIMITFRONT,
    RPT_ERRORID_LIMITBACK,

            // error code of titl
    RPT_ERRORID_TILTBODY = 0x21,

            // error code of device & sensor
    RPT_ERRORID_MAINBOARD = 0x31,
    RPT_ERRORID_SCREEN,
    RPT_ERRORID_FINGER,
    RPT_ERRORID_ZIGBEE,
    RPT_ERRORID_ROTAT,
    RPT_ERRORID_MARGIN,
    RPT_ERRORID_HEIGHT,
    RPT_ERRORID_PM25,
    RPT_ERRORID_WEIGHT,
    RPT_ERRORID_TILT,
    RPT_ERRORID_WIND,
    RPT_ERRORID_WALK,
    RPT_ERRORID_CONFIG,
    RPT_ERRORID_UPW,
    RPT_ERRORID_UPCABL,   
    RPT_ERRORID_UPCABR,   

    RPT_ERRORID_MAX
}
RPT_ERRORID_TypeDef;
#define RPT_ERRORID_SIZE    1   // size in byte of error attrib


typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of buffer, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------

    uint16_t sn;            // serial number

    BOOL pm25_valid;
    APP_SENSALARMID_TypeDef pm25_alarm;

    BOOL weight_valid;
    APP_SENSALARMID_TypeDef weight_alarm;

    BOOL height_valid;
    APP_SENSALARMID_TypeDef height_alarm;

    BOOL speed_valid;
    APP_SENSALARMID_TypeDef speed_alarm;

    BOOL wind_valid;
    APP_SENSALARMID_TypeDef wind_alarm;

    BOOL tilt_valid;
    APP_SENSALARMID_TypeDef tilt_alarm;

    BOOL motor1_valid;
    APP_SENSALARMID_TypeDef motor1_alarm;

    BOOL motor2_valid;
    APP_SENSALARMID_TypeDef motor2_alarm;

    BOOL motor3_valid;
    APP_SENSALARMID_TypeDef motor3_alarm;

    BOOL people_valid;
    APP_SENSALARMID_TypeDef people_alarm;

    uint32_t floor;             // current floor, TBD
    BOOL aligned;               // flag of aligned to floor

    APP_MOVESTAT_TypeDef move;  // moving stat of elivator
    BOOL goingdown;             // last direction

    BOOL fingersensor_valid;
    //BOOL finger_valid;
    BOOL finger_alarm;
    //APP_FINGERSTEP_TypeDef finger_step;
    
    BOOL alarm_doorin;          // alarm flag of door_in
    BOOL alarm_doorout;         // alarm flag of door_out
    BOOL alarm_top;
    BOOL alarm_bottom;
    //BOOL gpio5;

        // output
    //BOOL relay1_on;
    BOOL upweight_valid;
    APP_SENSALARMID_TypeDef upweight_alarm;

    BOOL cabl_valid;
    APP_SENSALARMID_TypeDef cabl_alarm;

    BOOL cabr_valid;
    APP_SENSALARMID_TypeDef cabr_alarm;



    uint8_t spare1_alarm;
    uint8_t spare2_alarm;
    uint8_t spare3_alarm;
    uint8_t spare4_alarm;

    uint8_t status;                         // status of next 3 types
    uint8_t alarm[RPT_ALARMID_MAX][RPT_ALARMID_SIZE];       // alarm
    uint8_t against[RPT_AGAINSTID_MAX][RPT_AGAINSTID_SIZE]; // against
    uint8_t error[RPT_ERRORID_MAX];         // error

    uint8_t spare_flag[8];      // reserved

    uint8_t spare_other[6];

    //-------------- end flag section --------------
    float end;          // last object of structure
}
APP_ALARMSHADOW_TypeDef;

_EXTERN APP_ALARMSHADOW_TypeDef alarm_shadow;       // shadow of alarm, against & error



/////// buffers of periodic data, work-loop data, alarm data, cali data ///////

// period data
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

    uint32_t name_id;

    uint16_t attrib;        // reserved

    uint8_t pm25_flag;
    uint8_t pm25_alarm;
    float pm25_value;

    uint8_t pm10_flag;
    uint8_t pm10_alarm;
    float pm10_value;

    uint8_t wind_flag;
    uint8_t wind_alarm;
    float wind_value;

    uint8_t temperature_flag;
    uint8_t temperature_alarm;
    float temperature_value;

    uint8_t humidity_flag;
    uint8_t humidity_alarm;
    float humidity_value;

    uint8_t valve1_flag;
    uint8_t valve1_alarm;

    uint8_t valve2_flag;
    uint8_t valve2_alarm;

    uint8_t valve3_flag;
    uint8_t valve3_alarm;

    uint8_t valve4_flag;
    uint8_t valve4_alarm;

    uint8_t noise_flag;
    uint8_t noise_alarm;
    float noise_value;

    uint8_t vane_flag;
    uint8_t vane_alarm;
    float vane_value;

    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    float spare1_value;

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    float spare2_value;

    //uint8_t spare4_flag;
/* 
    uint8_t collision_alarm[3];
    uint8_t obstacle_alarm[3];
    uint8_t forbid_alarm[3];

    uint8_t spare_flag[12];     // reserved
*/
    uint8_t spare_other[6];

    //-------------- end flag section --------------
    float end;          // last object of structure
}
APP_PRDVALUE_TypeDef;

_EXTERN APP_PRDVALUE_TypeDef period_value;  // perdical send to server
_EXTERN APP_PRDVALUE_TypeDef save_value;    // perdical save into NAND FLASH

#define SENSORMAX 16
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

    uint32_t name_id;

    uint16_t attrib;        // reserved

    uint8_t weight_flag;
    uint8_t weight_alarm;
    float weight_value;

    uint8_t cableleft_flag;
    uint8_t cableleft_alarm;
    float cableleft_value;

    uint8_t cableright_flag;
    uint8_t cableright_alarm;
    float cableright_value;

    uint8_t weightsensor_flag[SENSORMAX];
    uint8_t weightsensor_alarm[SENSORMAX];
    float weightsensor_value[SENSORMAX];


    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    float spare1_value;

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    float spare2_value;

    uint8_t spare3_flag;
    uint8_t spare3_alarm;
    float spare3_value;

    uint8_t spare4_flag;
    uint8_t spare4_alarm;
    float spare4_value;
    
    uint8_t spare_other[6];

    //-------------- end flag section --------------
    float end;          // last object of structure
}
APP_UPPRDVALUE_TypeDef;

_EXTERN APP_UPPRDVALUE_TypeDef upperiod_value;  // perdical send to server
_EXTERN APP_UPPRDVALUE_TypeDef upsave_value;    // perdical save into NAND FLASH


// work-loop data
typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of work-loop check, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------

    uint16_t sn;            //  serial number of work-loop

    uint32_t name_id;

    uint8_t sec_begin;  // begin time, BCD
    uint8_t min_begin;
    uint8_t hour_begin;
    uint8_t date_begin;
    uint8_t month_begin;    // weekday 1-7 in bit[7:5]
    uint8_t year_begin; // 20xx

    uint8_t sec_end;        // end time, BCD
    uint8_t min_end;
    uint8_t hour_end;
    uint8_t date_end;
    uint8_t month_end;  // weekday 1-7 in bit[7:5]
    uint8_t year_end;       // 20xx

    float weight_max;   // max load of work-loop
    uint32_t people_max;
    float height_max;
    uint32_t floor_max;
}
APP_WKLPDAT_TypeDef;

_EXTERN APP_WKLPDAT_TypeDef workloop;



/////// alarm, against & error report data
typedef enum
{
    FINGERSTAT_NULL = 0,

    FINGERSTAT_UNCHKED,     // finger not checked
    FINGERSTAT_REFUSED,     // finger refused
    FINGERSTAT_PASS,            // finger pass

    FINGERSTAT_MAX
}
APP_FINGERSTAT_TypeDef;

typedef struct
{
    uint8_t alarm_code;
    uint8_t alarm_byte[3];
}
APP_RPTALARM_TypeDef;

typedef struct
{
    uint8_t against_code;
    uint8_t against_byte[2];
}
APP_RPTAGAINST_TypeDef;

typedef struct
{
    uint8_t error_code;
}
APP_RPTERROR_TypeDef;


#define RPT_ALARM_BUFSIZE   6   // size of alarm buffer

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

    uint32_t name_id;

    uint16_t attrib;        // reserved

    uint8_t pm25_flag;
    uint8_t pm25_alarm;
    float pm25_value;

    uint8_t pm10_flag;
    uint8_t pm10_alarm;
    float pm10_value;

    uint8_t wind_flag;
    uint8_t wind_alarm;
    float wind_value;

    uint8_t temperature_flag;
    uint8_t temperature_alarm;
    float temperature_value;

    uint8_t humidity_flag;
    uint8_t humidity_alarm;
    float humidity_value;

    uint8_t valve1_flag;
    uint8_t valve1_alarm;
    float valve1_value;

    uint8_t valve2_flag;
    uint8_t valve2_alarm;
    float valve2_value;

    uint8_t valve3_flag;
    uint8_t valve3_alarm;
    float valve3_value;

    uint8_t valve4_flag;
    uint8_t valve4_alarm;
    float valve4_value;

    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    float spare1_value;

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    float spare2_value;

    uint8_t spare3_flag;
    uint8_t spare3_alarm;
    float spare3_value;

    uint8_t spare4_flag;
    uint8_t spare4_alarm;
    float spare4_value;

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
APP_ALARMDAT_TypeDef;

_EXTERN APP_ALARMDAT_TypeDef alarm_dat[RPT_ALARM_BUFSIZE];  // alarm send to server

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

    uint32_t name_id;

    uint16_t attrib;        // reserved

    uint8_t weight_flag;
    uint8_t weight_alarm;
    float weight_value;

    uint8_t cableleft_flag;
    uint8_t cableleft_alarm;
    float cableleft_value;

    uint8_t cableright_flag;
    uint8_t cableright_alarm;
    float cableright_value;

    uint8_t weightsensor_flag[SENSORMAX];
    uint8_t weightsensor_alarm[SENSORMAX];
    float weightsensor_value[SENSORMAX];


    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    float spare1_value;

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    float spare2_value;

    uint8_t spare3_flag;
    uint8_t spare3_alarm;
    float spare3_value;

    uint8_t spare4_flag;
    uint8_t spare4_alarm;
    float spare4_value;

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
APP_UPALARMDAT_TypeDef;

_EXTERN APP_UPALARMDAT_TypeDef upalarm_dat[RPT_ALARM_BUFSIZE];  // alarm send to server


typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of buffer, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------


    uint8_t sec;            // time in BCD
    uint8_t min;
    uint8_t hour;
    uint8_t date;
    uint8_t month;      // weekday 1-7 in bit[7:5]
    uint8_t year;           // 20xx

    uint16_t attrib;        // reserved

    uint8_t weight_flag;
    uint8_t weight_alarm;
    uint32_t weight;

    uint8_t height_flag;
    uint8_t height_alarm;
    uint32_t heigh;

    uint8_t wind_flag;
    uint8_t wind_alarm;
    uint32_t wind;

    uint8_t tilt_flag;
    uint8_t tilt_alarm;
    uint32_t tilt;

    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    uint32_t spare1_dat;        // reserved

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    uint32_t spare2_dat;        // reserved

    uint8_t spare3_flag;
    uint8_t spare3_alarm;
    uint32_t spare3_dat;        // reserved

    uint8_t spare4_flag;
    uint8_t spare4_alarm;
    uint32_t spare4_dat;        // reserved

}
APP_CALIRAWDAT_TypeDef;

_EXTERN APP_CALIRAWDAT_TypeDef cali_dat;

typedef struct
{
    //-------------------- flag section -----------------------

    uint8_t flag;       // flag of buffer, see APP_BUFSTAT_TypeDef

    //-------------------- data section -----------------------


    uint8_t sec;            // time in BCD
    uint8_t min;
    uint8_t hour;
    uint8_t date;
    uint8_t month;      // weekday 1-7 in bit[7:5]
    uint8_t year;           // 20xx

    uint16_t attrib;        // reserved

    uint8_t upweight_flag;
    uint8_t upweight_alarm;
    uint32_t upweight;

    uint8_t cabl_flag;
    uint8_t cabl_alarm;
    uint32_t cabl;

    uint8_t cabr_flag;
    uint8_t cabr_alarm;
    uint32_t cabr;

    uint8_t spare1_flag;
    uint8_t spare1_alarm;
    uint32_t spare1_dat;        // reserved

    uint8_t spare2_flag;
    uint8_t spare2_alarm;
    uint32_t spare2_dat;        // reserved

    uint8_t spare3_flag;
    uint8_t spare3_alarm;
    uint32_t spare3_dat;        // reserved

    uint8_t spare4_flag;
    uint8_t spare4_alarm;
    uint32_t spare4_dat;        // reserved

}
APP_UPCALIRAWDAT_TypeDef;

_EXTERN APP_UPCALIRAWDAT_TypeDef upcali_dat;

////////////   buffers of zone, torque, collision   ///////////////

typedef enum
{
    ALARM_NULL = 0,     // empty
    ALARM_WARN,     // warnning
    ALARM_ALARM,        // alarmming

    ALARM_ERROR,

    ALARM_MAX
}
APP_ALARMID_TypeDef;

typedef enum
{
    ZONETYPE_FBD = 0,   // zone forbidding
    ZONETYPE_OBS,       // zone obstacle

    ZONETYPE_MAX
}
APP_ZONETYPEID_TypeDef;

typedef struct
{
    uint8_t zone_index; // 0: no alarm; other: zone (index-1) in alarm
    APP_ZONETYPEID_TypeDef zone_type;
    APP_ALARMID_TypeDef zone_alarmid;
}
APP_ZONEALM_TypeDef;
_EXTERN APP_ZONEALM_TypeDef zone_alarm;

_EXTERN bool senswarn_torque, sensalarm_torque;     // warning & alarm flag of torque
_EXTERN uint8_t senswarn_zonefbd,sensalarm_zonefbd;     // warning & alarm flag of zone forbidding
_EXTERN uint8_t senswarn_zoneobs,sensalarm_zoneobs;     // warning & alarm flag of zone obstacle
_EXTERN uint8_t senswarn_collision,sensalarm_collision;     // warning & alarm flag of collision




////////////     ///////////////

//====== dot and line in zone check

typedef struct
{
    float x0;       // point x
    float y0;       // point y
    float x1;       // line-head x
    float y1;       // line-head y
    float x2;       // line-tail x
    float y2;       // line-tail y
}
APP_ZONEXY3_TypeDef;

_EXTERN APP_ZONEXY3_TypeDef zone_xy;


///////////////// data frame  ////////////////

typedef enum
{
    FRMID_RLTM=0,   // real-time data frame
    FRMID_ZONE, // zone alarm frame
    FRMID_ALARM,    // alarm data frame

    FRMID_MAX       // max frame
}
APP_FRMID_TypeDef;

typedef enum
{
    FRMATT_EMPTY = 0,       // empty frame
    FRMATT_RLTM,            // real-time data frame
    FRMATT_ALARM,           // alarm frame.

    FRMATT_MAX          // max attribute
}
APP_FRMATT_TypeDef;

typedef enum
{
    FRMALMID_WEIGHT = 0,        //alarm frame of weight.
    FRMALMID_HEIGHT,            //alarm frame of height.
    FRMALMID_MARGIN,            //alarm frame of margin.
    FRMALMID_ROTAT,     //alarm frame of rotation.
    FRMALMID_WIND,          //alarm frame of wind.
    FRMALMID_TILT,              //alarm frame of tilt.

    FRMALMID_TORQUE = 0x10, //alarm frame of torque.
    FRMALMID_ZONEFBD,           //alarm frame of fobidding area.
    FRMALMID_ZONEOBS,           //alarm frame of odstacle area.
    FRMALMID_COLLISION,     //alarm frame of collision.

    FRMALMID_NULL,
    FRMALMID_MAX                // max alarm
}
APP_FRMALMID_TypeDef;

typedef enum
{
    FRMDATCH_WEIGHT = 0,    //
    FRMDATCH_HEIGHT,        //
    FRMDATCH_MARGIN,
    FRMDATCH_ROTAT,
    FRMDATCH_WIND,
    FRMDATCH_TILT,


    FRMDATCH_MAX = 11
}
APP_FRMDATCH_TypeDef;

typedef enum
{
    FRMALMCH_TORQUE = 0,    //
    FRMALMCH_ZONEFBD,   //
    FRMALMCH_ZONEOBS,   //
    FRMALMCH_COLLISION, //

    FRMALMCH_MAX = 10
}
APP_FRMALMCH_TypeDef;


typedef struct              // size of this struct must be less than 128 Byte.
{
    uint8_t devid[6];
    uint8_t prtclver[4];

    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t attrib; // attribute of frame.
    uint8_t alarmid;

    APP_SENSVALUE_TypeDef value[FRMDATCH_MAX];

    uint8_t spare[6];

    uint8_t alarm[FRMALMCH_MAX];

    uint8_t spare2[6];

}
APP_DATFRM_TypeDef;

_EXTERN APP_DATFRM_TypeDef dat_frame[FRMID_MAX];


/////////// macros

#define PWM1_START()    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1); \
                        HAL_TIM_IC_Start(&htim5, TIM_CHANNEL_2)

#define PWM2_START()    HAL_TIM_IC_Start_IT(&htim9, TIM_CHANNEL_1); \
                        HAL_TIM_IC_Start(&htim9, TIM_CHANNEL_2)
#define PWM3_START()    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2); \
                        HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1)

#define PWM4_START()    HAL_TIM_IC_Start_IT(&htim12, TIM_CHANNEL_1);    \
                        HAL_TIM_IC_Start(&htim12, TIM_CHANNEL_2)

#define PWM5_START()    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1); \
                        HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_2)

#define PWM_START_ALL() {PWM1_START();PWM2_START(); \
                            PWM3_START();PWM4_START();  \
                            PWM5_START();}


#define FRQ1_START()    HAL_TIM_Base_Start_IT(&htim2);  \
                        HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2)

#define FRQ2_START()    HAL_TIM_Base_Start_IT(&htim2);  \
                        HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);

#define FRQ_START_ALL() {FRQ1_START();FRQ2_START();}


//#define   ANA_START_ALL() (HAL_ADC_Start_DMA(&hadc1, ADC1_DMAbuf, ADC1_LENGTH);)


/////////// functions
void APP_sensor_gpio_ini(void);
void APP_sensor_gpio_update(uint32_t step);

void APP_sensor_speedupdate(float height);
void APP_sensor_floorupdate(float height);

void APP_sensor_stateupdate(void);

void APP_sensor_raw2value(void);
void APP_sensor_alarmcheck(SENSOR_CHANNELS channel);

void APP_sensor_dat_ini(void);
void APP_sensor_value_ini(void);
void APP_sensor_valvestat_ini(void);

void APP_sensor_sendprdvalue(void);
void APP_sensor_savprdvalue(void);
void APP_sensor_wklpchk(void);

void APP_sensor_alarm_ini(void);
void APP_sensor_findalarm(void);
void APP_sensor_clearalarmbuf(APP_UPALARMDAT_TypeDef *buf);

void APP_sensor_sendrawdat(void);

float APP_sensor_linear_interpolation(uint32_t dat, APP_CALICHDAT_TypeDef* buf);
float APP_sensor_torquetbl_seek(float distance);
void APP_sensor_torquechk(void);
float APP_sensor_getdistance(APP_ZONEXY3_TypeDef zone_xy);
void APP_sensor_zonechk(void);

uint32_t APP_wind_rank(float speed);
#undef  _EXTERN

/* ******************************  END OF FILE  *******************************/


