/*   */
/*   */

#ifdef  _LOCAL_MMI
#define _EXTERN
#else
#define _EXTERN extern
#endif



/* ***************************  ***************************/
/////////// type & variables

_EXTERN uint8_t mmi_rcvbyte;


#define ASC_LENGTH  10

_EXTERN uint8_t databuf[ASC_LENGTH];        // buffer of ASCii to display

#define STRING_DASH2()  {databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = 0;}

#define STRING_DASH3()  {databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = '-'; \
                            databuf[3] = 0;}

#define STRING_DASH4()  databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = '-'; \
                            databuf[3] = '-'; \
                            databuf[4] = 0;

#define STRING_DASH5()  databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = '-'; \
                            databuf[3] = '-'; \
                            databuf[4] = '-'; \
                            databuf[5] = 0;

#define STRING_DASH6()  databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = '-'; \
                            databuf[3] = '-'; \
                            databuf[4] = '-'; \
                            databuf[5] = '-'; \
                            databuf[6] = 0;

#define STRING_DASH7()  databuf[0] = '-'; \
                            databuf[1] = '-'; \
                            databuf[2] = '-'; \
                            databuf[3] = '-'; \
                            databuf[4] = '-'; \
                            databuf[5] = '-'; \
                            databuf[6] = '-'; \
                            databuf[7] = 0;

#define NORTH "北"
#define NNE   "东北偏北"
#define NE    "东北"
#define ENE   "东北偏东"
#define EAST  "东"
#define ESE   "东南偏东"
#define SE    "东南"
#define SSE   "东南偏南"
#define SOUTH "南"
#define SSW   "西南偏南"
#define SW    "西南"
#define WSW   "西南偏西"
#define WEST  "西"
#define WNW   "西北偏西"
#define NW    "西北"
#define NNW   "西北偏北"


#define CODE_ROW (UPSNR_NUM+2) // 卸料平台修正传感器内码值行=传感器个数+2
#define CODE_COL UPSNR_NUM  // 卸料平台修正传感器内码值列=传感器个数                                            
#define SCRDATA_SIZE    40
typedef struct
{
    BOOL refresh;
    uint32_t dat_ui[10];
    int32_t dat_i[SCRDATA_SIZE];
    float dat_f[SCRDATA_SIZE];
    float dat_f2[SCRDATA_SIZE];
    float dat_kb[CODE_ROW][CODE_COL];    // 卸料平台称重修正传感器内码值
}
APP_screendat_TypeDef;
_EXTERN APP_screendat_TypeDef scrdat;       // buffer of screen data



#define MMI_sendbuf_SIZE 1000
typedef struct
{
    BOOL    valid;              // true when data ready
    uint32_t    length;             // length of data to send
    uint8_t dat[MMI_sendbuf_SIZE];
}
APP_mmisendbuf_TypeDef;
_EXTERN APP_mmisendbuf_TypeDef mmi_sendbuf;


#define MMI_RCVBUFSIZE  40
#define MMI_DATBUFSIZE  40
typedef struct
{       // UART receiver buffer
    uint32_t    timestamp;
    uint32_t    rcvindex;           // index of received byte
    uint8_t rcvbyte[MMI_RCVBUFSIZE];

    // protocol analyse buffer
    uint8_t valid;
    uint8_t cmd;        //
    uint8_t msg;
    uint8_t ctrtype;
    uint16_t    scrid;      //
    uint16_t    ctrid;      //
    uint8_t rcvdat[MMI_DATBUFSIZE];
}
APP_mmircvbuf_TypeDef;
_EXTERN APP_mmircvbuf_TypeDef mmi_rcvbuf;


#define MMI_MAXGRADE 6      // max grade of frame
typedef struct
{
    BOOL    update;         // flag of refresh

    uint32_t    grade;          // current grade of frame
    uint32_t    scrid[MMI_MAXGRADE];    // screen ID for each grade

    uint32_t    last_scrid;     // screen showed last

    int32_t next_grade;     // grade change,
                            // -1:step-out; 0:no change; 1:step-in
}
APP_mmistat_TypeDef;
_EXTERN APP_mmistat_TypeDef mmi_stat;



typedef struct
{
    uint16_t scr_x;     // x,y in pix on screen
    uint16_t scr_y;
    uint16_t image_id;
    uint16_t image_x;       // x,y in pix in image
    uint16_t image_y;
    uint16_t image_w;   // width, hidth in pix in image
    uint16_t image_h;
    uint8_t mask;           // flag of color-mask
}
APP_imgattrib_TpyeDef;
_EXTERN APP_imgattrib_TpyeDef img_attrib;



#define MMI_DOTMAX  10
typedef struct
{
    uint16_t x;     // x,y in pix on screen
    uint16_t y;
}
APP_dotxy_TpyeDef;
typedef struct
{
    uint16_t size;      // size of dot
    APP_dotxy_TpyeDef dot[MMI_DOTMAX];
    //uint8_t mask;         // flag of color-mask
}
APP_dotattrib_TpyeDef;


typedef struct
{
    // screen info
    float collision_basex;      // center_x of collision zone, meter
    float collision_basey;      // center_y of collision zone, meter
    float scale;                // image scale, meter/pix

    uint16_t screen_basex;  // base_x of screen in pix
    uint16_t screen_basey;  // base_y of screen in pix

    // tower info
    float body_x;
    float body_y;

    float front;
    float rear;
    float margin;

    float rotat;
    float pitch;

    bool margin_valid;
    bool rotat_valid;
    bool pitch_valid;

    uint16_t r_body;        // radius of body, pix
    uint16_t r_car;     // radius of carriage, pix

}
APP_TOWERATTRIB_TypeDef;
_EXTERN APP_TOWERATTRIB_TypeDef tower_attrib;


/////////// constants
/* 

*/

// 定义RGB色值
#define   COLOR_BLACK       0x0000      /*  黑色：    0,   0,   0 */
#define   COLOR_NAVY            0x000F      /*  深蓝色：  0,   0, 128 */
#define   COLOR_DGREEN      0x03E0      /*  深绿色：  0, 128,   0 */
#define   COLOR_DCYAN       0x03EF      /*  深青色：  0, 128, 128 */
#define   COLOR_MAROON      0x7800      /*  深红色：128,   0,   0 */
#define   COLOR_PURPLE      0x780F      /*  紫色：  128,   0, 128 */
#define   COLOR_OLIVE       0x7BE0      /*  橄榄绿：128, 128,   0 */
#define   COLOR_LGRAY       0xC618      /*  灰白色：192, 192, 192 */
#define   COLOR_DGRAY       0x7BEF      /*  深灰色：128, 128, 128 */
#define   COLOR_BLUE            0x001F      /*  蓝色：    0,   0, 255 */
#define   COLOR_GREEN       0x07E0      /*  绿色：    0, 255,   0 */
#define   COLOR_CYAN            0x07FF      /*  青色：    0, 255, 255 */
#define   COLOR_RED         0xF800      /*  红色：  255,   0,   0 */
#define   COLOR_MAGENTA     0xF81F      /*  品红：  255,   0, 255 */
#define   COLOR_YELLOW      0xFFE0      /*  黄色：  255, 255, 0   */
#define   COLOR_WHITE       0xFFFF      /*  白色：  255, 255, 255 */

// transceiver command
#define LCM_CMDSTART        0xEE            // start of frame send to LCM
#define LCM_CMDEND      0xFFFCFFFF      // end of frame send to LCM

#define LCM_RCVSTART        0xEE            // start of frame received from LCM
#define LCM_RCVEND          0xFFFCFFFF      // end of frame received from LCM

// status of LCM
#define LCMSTAT_RELEASE     0x00        // control release (up)
#define LCMSTAT_PRESS           0x01        // control presse (down)

// command type of LCM
#define LCMCMD_TPPRESS      0X01        // touchpad pressed
#define LCMCMD_TPRELEASE        0X03        // touchpad releaseed

#define LCMCMD_HANDSHAKE        0x04        // hand shake
#define LCMCMD_HANDSHAKEACK 0x55        // hand shake ACK

#define LCMCMD_CLEARLAYER   0x05        // clear lcm layer

#define LCMCMD_SHOWIMGFULL  0x31        // show image full-screen
#define LCMCMD_SHOWIMGPART  0x32        // show image inside screen
#define LCMCMD_SHOWIMGCUT   0x33        // show image cut-off

#define LCMCMD_SETFORGROND  0x41        // set forground color
#define LCMCMD_SETBACKGROND 0x42        // set background color

#define LCMCMD_DRAWLINE     0x51        // draw line

#define LCMCMD_DRAWCIRCLE   0x52        // draw circle outline
#define LCMCMD_FILLCIRCLE       0x53        // draw circle & fill

#define LCMCMD_DRAWRETANGLE 0x54        // draw retangle outline
#define LCMCMD_FILLRETANGLE 0x55        // draw retangle & fill

#define LCMCMD_DRAWELLIPSE  0x56        // draw ellipse outline
#define LCMCMD_FILLELLIPSE  0x57        // draw ellipse & fill

#define LCMCMD_SETRTC           0x81        // set rtc value
#define LCMCMD_GETRTC           0x82        // get rtc value

#define LCMCMD_SETCTR           0xB1        // set control data
#define LCMCMD_REFRESH      0xB3        // set screen refresh enable

#define LCMCMD_GET_REV      0xFE        // get version ?


// control type
#define LCMCTR_NULL     0x00
#define LCMCTR_BTN          0x10            // button control
#define LCMCTR_TXT          0x11            // text control
#define LCMCTR_PRGS     0x12            // progress control
#define LCMCTR_SLD          0x13            // slide control
#define LCMCTR_MTR          0x14            // meter control
#define LCMCTR_GIF          0x26            // animation control
#define LCMCTR_ICON     0x26            // icon control



// message type
#define LCMMSG_NULL     0xFF        //

#define LCMMSG_SCRSET       0x00        // set screen
#define LCMMSG_SCRGET       0x01        // get screen
#define LCMMSG_CURSET       0x02        // set cursor command
#define LCMMSG_CTRSHOW  0x03        // set control show/hide


#define LCMMSG_DATSET       0x10        // set control data
#define LCMMSG_DATGET       0x11        // get control data
#define LCMMSG_SETBATCH 0x12        // set data in batch

#define LCMMSG_SETBACKGROND 0x18    // set background color for control
#define LCMMSG_SETFORGROND  0x19        // set forground color for control

#define LCMMSG_GIFRUN       0x20        // start gif
#define LCMMSG_GIFSTOP  0x21        // stop gif
#define LCMMSG_GIFPAUS  0x22        // pause gif
#define LCMMSG_GIFFRM       0x23        // set gif frame
#define LCMMSG_GIFPRE       0x24        // show previous frame
#define LCMMSG_GIFNEXT  0x25        // show next frame
#define LCMMSG_GIFGET       0x26        // get gif data

#define LCMMSG_ICONSET  0x23        // set icon frame
#define LCMMSG_ICONGET  0x26        // get icon data

#define LCMMSG_GRPCHADD 0x30        // add graph channel
#define LCMMSG_GRPCHDEL 0x31        // del graph channel
#define LCMMSG_GRPDATADD    0x32        // add channel data
#define LCMMSG_GRPDATCLR    0x33        // clear channel data
#define LCMMSG_GRPVIEW  0x34        // set graph view port
#define LCMMSG_GRPDATINS    0x35        // insert graph channel



/////////// macros

#define FILLCMD_START(pbuf) {(*(*pbuf)++) = LCM_CMDSTART;}
#define FILLCMD_END(pbuf)       {(*(*pbuf)++) = ((LCM_CMDEND >> 24) & 0xFF);    \
                                  (*(*pbuf)++) = ((LCM_CMDEND >> 16) & 0xFF);   \
                                  (*(*pbuf)++) = ((LCM_CMDEND >>8) & 0xFF); \
                                  (*(*pbuf)++) = (LCM_CMDEND & 0xFF); }

#define FILLCMD_1B(pbuf, cmd)   {(*(*pbuf)++) = (cmd & 0xFF);}
#define FILLCMD_2B(pbuf, cmd)   {(*(*pbuf)++) = ((cmd >> 8) & 0xFF); \
                                  (*(*pbuf)++) = (cmd & 0xFF);}

#define FILLSCRID(pbuf, scrid)  {(*(*pbuf)++) = ((scrid >> 8) & 0xFF); \
                                  (*(*pbuf)++) = (scrid & 0xFF);}
#define FILLCTRID(pbuf, ctrid)  {(*(*pbuf)++) = ((ctrid >> 8) & 0xFF); \
                                  (*(*pbuf)++) = (ctrid & 0xFF);}




typedef enum
{
    SCRID_SCRLOGO = 0,          // logo screen
    SCRID_SCRMAIN,              // main screen
    SCRID_SCRRTC,               // RTC screen
    SCRID_SCRABOUT,         // about screen

    SCRID_SCRUPLOADINGINFO,   // information screen
                            //SCRID_SCRFLOOR,           // floor screen
    SCRID_SCRCOMCFG,            // communication config screen
    SCRID_SCRLMTCFG,        //SCRID_SCRLIMIT,           // limit screen
    
    SCRID_SCRMENUUSER,      // user-menu screen
    SCRID_SCRMENUADMIN,     // admin_menu screen
    SCRID_SCROEMOPT,       // version option configuration
    SCRID_SCRMENUADJ,     // sensor adjust
    SCRID_SCRCALIWEIGHT,   // weight adjust
    SCRID_SCRCALIWIRE1,   // wire pulling force adjust
    SCRID_SCRCALIWIRE2,   // wire pulling force adjust
    SCRID_SCRCALIKB,      // K,B coef calibration


    SCRID_MAXSCR
}
APP_SCRID_TypeDef;


typedef enum
{
    // logo frmae
    CTRID_SCRLOGO_TXTLOGO1 = 5,
    CTRID_SCRLOGO_TXTLOGO2,
    CTRID_SCRLOGO_TXTLOGO3,

    CTRID_SCRLOGO_ICONLOGO = 4,

    // main frame controls
    CTRID_SCRMAIN_BTNABOUT = 5,

    CTRID_SCRMAIN_TXTPM25 = 2,  // PM2.5
    CTRID_SCRMAIN_ICONPM25 = 9,
    CTRID_SCRMAIN_PRGSPM25 = 47,


    CTRID_SCRMAIN_ICONAIN1 = 60,
    CTRID_SCRMAIN_ICONAIN2 = 61,
    CTRID_SCRMAIN_ICONAIN3 = 62,
    CTRID_SCRMAIN_ICONAIN4 = 63,
    CTRID_SCRMAIN_ICONAIN5 = 64,
    CTRID_SCRMAIN_ICONAIN6 = 65,
    CTRID_SCRMAIN_ICONAIN7 = 66,
    CTRID_SCRMAIN_ICONAIN8 = 67,
    CTRID_SCRMAIN_ICONAIN9 = 68,
    CTRID_SCRMAIN_ICONAIN10 = 69,
    CTRID_SCRMAIN_ICONAIN11 = 70,
    CTRID_SCRMAIN_ICONAIN12 = 71,
    CTRID_SCRMAIN_ICONAIN13 = 72,
    CTRID_SCRMAIN_ICONAIN14 = 73,
    CTRID_SCRMAIN_ICONAIN15 = 74,
    CTRID_SCRMAIN_ICONAIN16 = 75,

    CTRID_SCRMAIN_TXTAIN1 = 80,
    CTRID_SCRMAIN_TXTAIN2 = 81,
    CTRID_SCRMAIN_TXTAIN3 = 82,
    CTRID_SCRMAIN_TXTAIN4 = 83,
    CTRID_SCRMAIN_TXTAIN5 = 84,
    CTRID_SCRMAIN_TXTAIN6 = 85,
    CTRID_SCRMAIN_TXTAIN7 = 86,
    CTRID_SCRMAIN_TXTAIN8 = 87,
    CTRID_SCRMAIN_TXTAIN9 = 88,
    CTRID_SCRMAIN_TXTAIN10 = 89,
    CTRID_SCRMAIN_TXTAIN11 = 90,
    CTRID_SCRMAIN_TXTAIN12 = 91,
    CTRID_SCRMAIN_TXTAIN13 = 92,
    CTRID_SCRMAIN_TXTAIN14 = 93,
    CTRID_SCRMAIN_TXTAIN15 = 94,
    CTRID_SCRMAIN_TXTAIN16 = 95,

    CTRID_SCRMAIN_TXTKG1 = 100,
    CTRID_SCRMAIN_TXTKG2 = 101,
    CTRID_SCRMAIN_TXTKG3 = 102,
    CTRID_SCRMAIN_TXTKG4 = 103,
    CTRID_SCRMAIN_TXTKG5 = 104,
    CTRID_SCRMAIN_TXTKG6 = 105,
    CTRID_SCRMAIN_TXTKG7 = 106,
    CTRID_SCRMAIN_TXTKG8 = 107,
    CTRID_SCRMAIN_TXTKG9 = 108,
    CTRID_SCRMAIN_TXTKG10 = 109,
    CTRID_SCRMAIN_TXTKG11 = 110,
    CTRID_SCRMAIN_TXTKG12 = 111,
    CTRID_SCRMAIN_TXTKG13 = 112,
    CTRID_SCRMAIN_TXTKG14 = 113,
    CTRID_SCRMAIN_TXTKG15 = 114,
    CTRID_SCRMAIN_TXTKG16 = 115,

    CTRID_SCRMAIN_TXTWEIGHT = 96,
    CTRID_SCRMAIN_TXTKG = 97,
    

    CTRID_SCRMAIN_ICONGPRS = 39,
    CTRID_SCRMAIN_ICONZIGBEE = 40,
    CTRID_SCRMAIN_ICONGPS = 41,

    CTRID_SCRMAIN_TXTPSWD = 36,

    CTRID_SCRMAIN_TXTDEBUG = 35,           // debug data

    // rtc frame
    CTRID_SCRRTC_BTNSAVE  = 9,
    CTRID_SCRRTC_BTNESC  = 10,

    CTRID_SCRRTC_TXTYEAR  = 16,
    CTRID_SCRRTC_TXTMONTH = 15,
    CTRID_SCRRTC_TXTDAY = 14,
    CTRID_SCRRTC_TXTWEEK = 24,
    CTRID_SCRRTC_TXTHOUR = 13,
    CTRID_SCRRTC_TXTMINUTE = 17,
    CTRID_SCRRTC_TXTSECOND = 3,

    CTRID_SCRRTC_TXTDEBUG = 19,            // debug data


    // about frame
    CTRID_SCRABOUT_BTNESC = 9,

    CTRID_SCRABOUT_TXTLOGO1 = 16,
    CTRID_SCRABOUT_TXTLOGO2 = 17,
    CTRID_SCRABOUT_TXTLOGO3 = 18,

    CTRID_SCRABOUT_TXTSOFTVER = 2,
    CTRID_SCRABOUT_TXTPROTOCOLVER = 5,
    CTRID_SCRABOUT_TXTMMIVER = 7,

    CTRID_SCRABOUT_ICONLOGO = 10,           // LOGO of manufacture
    CTRID_SCRABOUT_ICONMFR = 19,            // QR-code of manufacture
    CTRID_SCRABOUT_QRCMFR = 13,         // QR-code of manufacture

    CTRID_SCRABOUT_TXTDEBUG = 12,      // debug data


    // uploading-info frame
    CTRID_SCRDUSTMONINFO_BTNENTER = 26,
    CTRID_SCRDUSTMONINFO_BTNESC = 37,

    CTRID_SCRDUSTMONINFO_TXTVALVE1 = 101,       // PM2.5 valve 1
    CTRID_SCRDUSTMONINFO_TXTVALVE2 = 102,       // PM2.5 valve 2
    CTRID_SCRDUSTMONINFO_TXTVALVE3 = 103,       // PM2.5 valve 3
    CTRID_SCRDUSTMONINFO_TXTVALVE4 = 104,       // PM2.5 valve 4
    CTRID_SCRDUSTMONINFO_TXTVALVE5 = 105,       // PM10 valve 1
    CTRID_SCRDUSTMONINFO_TXTVALVE6 = 106,       // PM10 valve 2
    CTRID_SCRDUSTMONINFO_TXTVALVE7 = 107,       // PM10 valve 3
    CTRID_SCRDUSTMONINFO_TXTVALVE8 = 108,       // PM10 valve 4

    CTRID_SCRVALVECFG_ICONV1EN_PM25 = 11,   // valve 1 auto checkbox
    CTRID_SCRVALVECFG_ICONV2EN_PM25 = 14,   // valve 2 auto checkbox
    CTRID_SCRVALVECFG_ICONV3EN_PM25 = 17,   // valve 3 auto checkbox
    CTRID_SCRVALVECFG_ICONV4EN_PM25 = 18,   // valve 4 auto checkbox
    CTRID_SCRVALVECFG_ICONV1EN_PM10 = 45,   // valve 1 auto checkbox
    CTRID_SCRVALVECFG_ICONV2EN_PM10 = 46,   // valve 2 auto checkbox
    CTRID_SCRVALVECFG_ICONV3EN_PM10 = 47,   // valve 3 auto checkbox
    CTRID_SCRVALVECFG_ICONV4EN_PM10 = 48,   // valve 4 auto checkbox

    CTRID_SCRVALVECFG_ICONVALVE1MA = 20,    // valve 1 manual checkbox
    CTRID_SCRVALVECFG_ICONVALVE2MA = 12,    // valve 2 manual checkbox
    CTRID_SCRVALVECFG_ICONVALVE3MA = 15,    // valve 3 manual checkbox
    CTRID_SCRVALVECFG_ICONVALVE4MA = 19,    // valve 4 manual checkbox

    CTRID_SCRLEDOPT_ICON2LINE = 50,         // outdoor led 2 lines option
    CTRID_SCRLEDOPT_ICON3LINE = 52,         // outdoor led 3 lines option
    CTRID_SCRLEDOPT_ICON4LINE = 53,         // outdoor led 4 lines option

    CTRID_SCRHWVER_ICONA21 = 80,            // SPS32 A21 & pre
    CTRID_SCRHWVER_ICONB00 = 81,            // SPS32 B00

    CTRID_SCRPMTYP_ICONSDS = 90,            // SDS011
    CTRID_SCRPMTYP_ICONYT  = 91,            // YT-PM2510

    CTRID_SCRDUSTMONINFO_TXTDEBUG = 41,        // debug data

    // com config frame
    CTRID_SCRCOMCFG_BTNENTER = 26,
    CTRID_SCRCOMCFG_BTNESC = 37,

    CTRID_SCRCOMCFG_TXTADDR_0 = 100,    // base of device address
    CTRID_SCRCOMCFG_TXTADDR_1,
    CTRID_SCRCOMCFG_TXTADDR_2,
    CTRID_SCRCOMCFG_TXTADDR_3,
    CTRID_SCRCOMCFG_TXTADDR_4,      // end of device address

    CTRID_SCRCOMCFG_TXTIP1_0,       // base of IP1
    CTRID_SCRCOMCFG_TXTIP1_1,
    CTRID_SCRCOMCFG_TXTIP1_2,
    CTRID_SCRCOMCFG_TXTIP1_3,
    CTRID_SCRCOMCFG_TXTIP1_PORT,        // end of IP1

    CTRID_SCRCOMCFG_TXTIP2_0,       // base of IP2
    CTRID_SCRCOMCFG_TXTIP2_1,
    CTRID_SCRCOMCFG_TXTIP2_2,
    CTRID_SCRCOMCFG_TXTIP2_3,
    CTRID_SCRCOMCFG_TXTIP2_PORT,        // end of IP2

    CTRID_SCRCOMCFG_TXTIP3_0,       // base of IP3
    CTRID_SCRCOMCFG_TXTIP3_1,
    CTRID_SCRCOMCFG_TXTIP3_2,
    CTRID_SCRCOMCFG_TXTIP3_3,
    CTRID_SCRCOMCFG_TXTIP3_PORT,        // end of IP3

    CTRID_SCRCOMCFG_TXTIP4_0,       // base of IP4
    CTRID_SCRCOMCFG_TXTIP4_1,
    CTRID_SCRCOMCFG_TXTIP4_2,
    CTRID_SCRCOMCFG_TXTIP4_3,
    CTRID_SCRCOMCFG_TXTIP4_PORT,        // end of IP4

    CTRID_SCRCOMCFG_TXTBEAT,            // heartbeat
    CTRID_SCRCOMCFG_TXTREC,         // reconeect
    CTRID_SCRCOMCFG_TXTRPRT,            // data-report
    CTRID_SCRCOMCFG_TXTSAVE,            // data-save
    CTRID_SCRCOMCFG_TXTTMOUT,       // link timeout

    CTRID_SCRCOMCFG_TXTMAX,

    CTRID_SCRCOMCFG_TXTDEBUG = 58, // debug data

    // limit config frame
    CTRID_SCRLMTCFG_BTNENTER = 26,
    CTRID_SCRLMTCFG_BTNESC = 37,

    CTRID_SCRLMTCFG_TXTWTHIGH = 101,       // 
    CTRID_SCRLMTCFG_TXTWTLOW,
    CTRID_SCRLMTCFG_TXTWTPRE,

    CTRID_SCRLMTCFG_TXTWRHIGH,
    CTRID_SCRLMTCFG_TXTWRLOW,
    CTRID_SCRLMTCFG_TXTWRPRE,

    CTRID_SCRLMTCFG_TXTDEBUG = 41, // debug data

    // admin-menu frame
    CTRID_SCRMENUADMIN_BTNESC = 102,

    CTRID_SCRMENUADMIN_BTNDUSTMONINFO = 1,      // dustmon info
    CTRID_SCRMENUADMIN_BTNSNRADJ = 2,           // sensor adjust
    CTRID_SCRMENUADMIN_BTNCOMCFG = 3,           // com config
    CTRID_SCRMENUADMIN_BTNLMT = 5,              // limit parameter


    CTRID_SCRMENUADMIN_BTNRTC = 9,              // RTC
    CTRID_SCRMENUADMIN_BTNABOUT = 11,           // about

    CTRID_SCRMENUADMIN_TXTDEBUG = 12,          // debug data


    // user-menu frame
    CTRID_SCRMENUUSER_BTNESC = 102,

    CTRID_SCRMENUUSER_BTNDUSTMONINFO = 1,       // dustmon info

    CTRID_SCRMENUUSER_BTNRTC = 6,               // RTC
    CTRID_SCRMENUUSER_BTNABOUT = 14,            // about

    CTRID_SCRMENUUSER_TXTDEBUG = 12,           // debug data

    // option menu frame

    CTRID_SCROEMOPT_BTNENTER = 3,
    CTRID_SCROEMOPT_BTNESC = 4,

    CTRID_SCROEMOPT_ICONCOMPANY0_EN = 20,
    CTRID_SCROEMOPT_ICONCOMPANY1_EN = 21, 
    CTRID_SCROEMOPT_ICONCOMPANY2_EN = 22, 
    CTRID_SCROEMOPT_ICONCOMPANY3_EN = 23, 
    CTRID_SCROEMOPT_ICONCOMPANY4_EN = 24, 
    CTRID_SCROEMOPT_ICONCOMPANY5_EN = 25, 
    CTRID_SCROEMOPT_ICONCOMPANY6_EN = 26, 
    CTRID_SCROEMOPT_ICONCOMPANY7_EN = 27, 
    CTRID_SCROEMOPT_ICONCOMPANY8_EN = 28, 
    CTRID_SCROEMOPT_ICONCOMPANY9_EN = 29, 
    CTRID_SCROEMOPT_ICONCOMPANY10_EN = 30,
    CTRID_SCROEMOPT_ICONCOMPANY11_EN = 31,
    CTRID_SCROEMOPT_ICONCOMPANY12_EN = 32, 
    CTRID_SCROEMOPT_ICONCOMPMAX,

    CTRID_SCROEMOPT_TXTDEBUG = 41,

    // sensor adjust menu frame
    CTRID_SCRMENUSNRADJ_BTNADJWEIGHT = 1,
    CTRID_SCRMENUSNRADJ_BTNADJWIRE1,
    CTRID_SCRMENUSNRADJ_BTNADJWIRE2,
    CTRID_SCRMENUSNRADJ_BTNADJKB,

    CTRID_SCRMENUSNRADJ_BTNESC = 102,
    CTRID_SCRMENUSNRADJ_TXTDEBUG = 12,

    // weight adjust menu frame
    CTRID_SCRCALIWEIGHT_BTNSORTUP = 20,
    CTRID_SCRCALIWEIGHT_BTNENTER = 26,
    CTRID_SCRCALIWEIGHT_BTNESC = 37,
                                    
    CTRID_SCRCALIWEIGHT_TXTRAW = 71,				// raw data
    CTRID_SCRCALIWEIGHT_TXTPTS = 157,				// points of cali-table
    
    CTRID_SCRCALIWEIGHT_BTNREAD_01 = 201,		// base of read
    CTRID_SCRCALIWEIGHT_BTNREAD_10 = 210,		// end of read
    
    CTRID_SCRCALIWEIGHT_TXTWEIGHT_01 = 1,		// base of WEIGHT
    CTRID_SCRCALIWEIGHT_TXTWEIGHT_10 = 10,		// end of WEIGHT
    
    CTRID_SCRCALIWEIGHT_TXTRAW_01 = 101,		// base of raw
    CTRID_SCRCALIWEIGHT_TXTRAW_10 = 110,		// end of raw

    CTRID_SCRCALIWEIGHT_TXTDEBUG = 18 ,			// debug data

    // wire1 adjust menu frame
    CTRID_SCRCALIWIRE1_BTNSORTUP = 20,
    CTRID_SCRCALIWIRE1_BTNENTER = 26,
    CTRID_SCRCALIWIRE1_BTNESC = 37,
                                    
    CTRID_SCRCALIWIRE1_TXTRAW = 71,				// raw data
    CTRID_SCRCALIWIRE1_TXTPTS = 157,				// points of cali-table
    
    CTRID_SCRCALIWIRE1_BTNREAD_01 = 201,		// base of read
    CTRID_SCRCALIWIRE1_BTNREAD_10 = 210,		// end of read
    
    CTRID_SCRCALIWIRE1_TXTWIRE1_01 = 1,		// base of WIRE1
    CTRID_SCRCALIWIRE1_TXTWIRE1_10 = 10,		// end of WIRE1
    
    CTRID_SCRCALIWIRE1_TXTRAW_01 = 101,		// base of raw
    CTRID_SCRCALIWIRE1_TXTRAW_10 = 110,		// end of raw

    CTRID_SCRCALIWIRE1_TXTDEBUG = 18 ,			// debug data

    // wire2 adjust menu frame
    CTRID_SCRCALIWIRE2_BTNSORTUP = 20,
    CTRID_SCRCALIWIRE2_BTNENTER = 26,
    CTRID_SCRCALIWIRE2_BTNESC = 37,
                                    
    CTRID_SCRCALIWIRE2_TXTRAW = 71,				// raw data
    CTRID_SCRCALIWIRE2_TXTPTS = 157,				// points of cali-table
    
    CTRID_SCRCALIWIRE2_BTNREAD_01 = 201,		// base of read
    CTRID_SCRCALIWIRE2_BTNREAD_10 = 210,		// end of read
    
    CTRID_SCRCALIWIRE2_TXTWIRE2_01 = 1,		// base of WIRE2
    CTRID_SCRCALIWIRE2_TXTWIRE2_10 = 10,		// end of WIRE2
    
    CTRID_SCRCALIWIRE2_TXTRAW_01 = 101,		// base of raw
    CTRID_SCRCALIWIRE2_TXTRAW_10 = 110,		// end of raw

    CTRID_SCRCALIWIRE2_TXTDEBUG = 18 ,			// debug data

    // Loadometer correction
    CTRID_SCRCALIKB_BTNENTER = 112, // enter
    CTRID_SCRCALIKB_BTNESC = 113,      // ESC

    CTRID_SCRCALIKB_EMPTY_S1 = 0,    //
    CTRID_SCRCALIKB_EMPTY_S7 = 7,    //

    CTRID_SCRCALIKB_ANGLE1_S1 = 8,    //
    CTRID_SCRCALIKB_ANGLE1_S7 = 15,    //

    CTRID_SCRCALIKB_ANGLE2_S1 = 16,    //
    CTRID_SCRCALIKB_ANGLE2_S7 = 23,    //

    CTRID_SCRCALIKB_ANGLE3_S1 = 24,    //
    CTRID_SCRCALIKB_ANGLE3_S7 = 31,    //

    CTRID_SCRCALIKB_ANGLE4_S1 = 32,    //
    CTRID_SCRCALIKB_ANGLE4_S7 = 39,    //

    CTRID_SCRCALIKB_ANGLE5_S1 = 40,    //
    CTRID_SCRCALIKB_ANGLE5_S7 = 47,    //

    CTRID_SCRCALIKB_ANGLE6_S1 = 48,    //
    CTRID_SCRCALIKB_ANGLE6_S7 = 55,    //

    CTRID_SCRCALIKB_ANGLE7_S1 = 56,    //
    CTRID_SCRCALIKB_ANGLE7_S7 = 63,    //

    CTRID_SCRCALIKB_ANGLE8_S1 = 64,    //
    CTRID_SCRCALIKB_ANGLE8_S7 = 71,    //

    CTRID_SCRCALIKB_DISPERSED_S1 = 72,    //
    CTRID_SCRCALIKB_DISPERSED_S7 = 79,    //

    CTRID_SCRCALIKB_WOC_S = 119,    // 压角砝码重量
    CTRID_SCRCALIKB_WOC_D = 120,    // 分散砝码总重量
    CTRID_SCRCALIKB_ICONV1EN = 130,   // algorithm enable auto checkbox

    CTRID_SCRCALIKB_BTNREAD_01 = 201,
    CTRID_SCRCALIKB_BTNREAD_10 = 210,



    CTRID_SCRCALIKB_TXTDEBUG = 116, // debug data
/* 
    CTRID_SCRxx_BTNxx,
    CTRID_SCRxx_TXTxx,
    CTRID_SCRxx_ICONxx,
    CTRID_SCRxx_MTRxx,
*/

    CTRID_MAXCTR
}
APP_CTRID_TypeDef;

// index of icon used in frames
typedef enum
{
    // alarm icon index
    ICONIDX_INVALID = 0,
    ICONIDX_OK,

    ICONIDX_LOERR,
    ICONIDX_LOALARM,
    ICONIDX_LOWARN,
    ICONIDX_HIWARN,
    ICONIDX_HIALARM,
    ICONIDX_HIERR,

    ICONIDX_LEFTERR,
    ICONIDX_LEFTALARM,
    ICONIDX_LEFTWARN,
    ICONIDX_RIGHTWARN,
    ICONIDX_RIGHTALARM,
    ICONIDX_RIGHTERR,

    ICONIDX_DISABLE,
    ICONIDX_NULL,

    // GPRS icon index
    ICONIDX_GPRSNULL = 0,
    ICONIDX_GPRSLV0,
    ICONIDX_GPRSLV1,
    ICONIDX_GPRSLV2,
    ICONIDX_GPRSLV3,
    ICONIDX_GPRSLV4,
    ICONIDX_GPRSBLANK,

    // Zigbee icon index
    ICONIDX_ZIGBEENULL = 0,
    ICONIDX_ZIGBEELV0,
    ICONIDX_ZIGBEELV1,
    ICONIDX_ZIGBEELV2,
    ICONIDX_ZIGBEELV3,
    ICONIDX_ZIGBEEBLANK,

    // uploading platform icon index
    ICONIDX_UPNULL = 0,
    ICONIDX_UPFIXED,
    ICONIDX_UPBLANK,

    // check-box icon index
    ICONIDX_UNCHECKED = 0,
    ICONIDX_CHECKED,

    // moving icon index
    ICONIDX_UP = 0,
    ICONIDX_DOWN,
    ICONIDX_STOP,

    // door icon index
    ICONIDX_DOOR00 = 0,
    ICONIDX_DOOR01,
    ICONIDX_DOOR10,
    ICONIDX_DOOR11,

    // LOGO icon index
    ICONIDX_LOGONULL = 0,			// 默认开机图标，“智慧工地”文字
	ICONIDX_LOGONULL_TOWER,		// 中性图标，塔机图
	ICONIDX_LOGONULL_TOWER_LIFT,	// 中性图标，塔机+升降机图
	
	ICONIDX_LOGOTX,				// 陕西泰新博坤智能科技有限公司
	ICONIDX_LOGOZFMD,				// 四川中桴美达科技有限公司
	ICONIDX_LOGOXMRISHENG,		// 厦门日升建机信息科技有限公司
	ICONIDX_LOGOSDWEIKONG,		// 山东微控科技发展有限公司
	ICONIDX_LOGOZZBRT,			// 郑州博睿特科技有限公司
	
	ICONIDX_LOGOCQYRRD,			// 渝仁蓉达科技有限公司
	ICONIDX_LOGOSXRW,				// 陕西荣炜建筑科技有限公司
	ICONIDX_LOGOGLD,				// 陕西广联达华筑科技有限公司
	ICONIDX_LOGOTST,				// 贵州途顺通科技有限公司
	ICONIDX_LOGOQDYK,				// 青岛一开电气科技有限公司
	
	ICONIDX_LOGOMLHB,				// 西安明路环保科技有限公司
	ICONIDX_LOGOCCCE,				// 中煤地建设工程有限公司
	ICONIDX_LOGOLYHB,				// 郑州蓝宇环保科技有限公司
	ICONIDX_LOGOLZXQ,				// 兰州新区智能信息技术发展有限公司
	ICONIDX_LOGOSLX,				// 咸阳盛林鑫环保科技有限公司


    // QRC icon index
    ICONIDX_QRCHTACHINA = 0,        // www.htachina.com
    ICONIDX_QRCTXBK,            // 陕西泰新博坤智能科技有限公司
    ICONIDX_QRCZFMD_CHS,            // 四川中桴美达科技有限公司
    ICONIDX_QRCXMRISHENG,           // 厦门日升建机信息科技有限公司

    ICONIDX_QRCNULL,
    ICONIDX_END
}
APP_ICONIDX_TypeDef;


// index of image
typedef enum
{
    IMGIDX_CASE = 14,

    IMGIDX_END
}
APP_IMGIDX_TypeDef;


/////////// functions
void APP_mmi_rcvisr(void);
void APP_mmi_input(void);
void APP_mmi_display(void);

void APP_mmi_ini(void);
void APP_mmi_scrlogo(void);

void APP_mmi_scrmain(void);
void APP_mmi_ctrmain(void);

void APP_mmi_scrrtc(void);
void APP_mmi_ctrrtc(void);

void APP_mmi_scrabout(void);
void APP_mmi_ctrabout(void);

void APP_mmi_scruploadinginfo(void);
void APP_mmi_ctruploadinginfo(void);

void APP_mmi_scrcomcfg(void);
void APP_mmi_ctrcomcfg(void);

void APP_mmi_ctrlmtcfg(void);
void APP_mmi_scrlmtcfg(void);

void APP_mmi_scrmenuadmin(void);
void APP_mmi_ctrmenuadmin(void);

void APP_mmi_scrmenuuser(void);
void APP_mmi_ctrmenuuser(void);
void APP_mmi_scroemopt(void);
void APP_mmi_ctroemopt(void);

void APP_mmi_ctradj(void);
void APP_mmi_scradj(void);
void APP_mmi_ctrupweightadj(void);
void APP_mmi_scrupweightadj(void);
void APP_mmi_ctrupwire1adj(void);
void APP_mmi_scrupwire1adj(void);
void APP_mmi_ctrupwire2adj(void);
void APP_mmi_scrupwire2adj(void);
void APP_mmi_ctrupcalikbadj(void);
void APP_mmi_scrupcalikbadj(void);

void APP_mmi_screenupdate(uint8_t **pbuf, BOOL enable);
void APP_mmi_showscreen(uint8_t **pbuf, uint32_t scrid);

void APP_mmi_showtext(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint8_t *string);
void APP_mmi_showtext_batch(uint8_t **pbuf, uint32_t ctrid, uint8_t *string);
void APP_mmi_gettext(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid);

void APP_mmi_showicon(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t index);

void APP_mmi_startgif(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid);
void APP_mmi_stopgif(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid);

void APP_mmi_showmeter(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t dat);
void APP_mmi_showmeter_batch(uint8_t **pbuf, uint32_t ctrid, uint32_t dat);

void APP_mmi_showprogress(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t dat);
void APP_mmi_showprogress_batch(uint8_t **pbuf, uint32_t ctrid, uint32_t dat);

void APP_mmi_showrtc(uint8_t **pbuf, BOOL instant);

void APP_mmi_clearlayer(uint8_t **pbuf, uint16_t layer);

void APP_mmi_ctrvisiable(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, BOOL enable);

void APP_mmi_setforground_ctr(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint16_t RGB565);
void APP_mmi_setbackground_ctr(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint16_t RGB565);

void APP_mmi_showimgfull(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib);
void APP_mmi_showimgpart(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib);
void APP_mmi_showimgcut(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib);

void APP_mmi_setforground(uint8_t **pbuf, uint16_t RGB565);
void APP_mmi_setbackground(uint8_t **pbuf, uint16_t RGB565);

void APP_mmi_showline(uint8_t **pbuf, APP_dotattrib_TpyeDef *dot_attrib);

void APP_mmi_showcircle(uint8_t **pbuf, uint16_t x, uint16_t y, uint16_t r, BOOL fill);
void APP_mmi_showretangle(uint8_t **pbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, BOOL fill);
void APP_mmi_showellipse(uint8_t **pbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, BOOL fill);

void APP_mmi_f2a(float dat, uint32_t mantissa, uint8_t *buf);
BOOL APP_mmi_a2f(uint8_t *buf, float *output);
void APP_mmi_i2a(int32_t dat, uint32_t digit, uint8_t *buf);
void APP_mmi_sortupbyx(float *x, float *y, uint32_t size);



#undef  _EXTERN
/* ******************************  END OF FILE  *******************************/
