
/*   */
/*   */

#ifdef	_LOCAL_LED
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif



/* ***************************  ***************************/
/////////// type & variables
_EXTERN uint8_t led_rcvbyte;

// SRT_PM  PM2.5&PM10  384bytes(96*32/8) 
/* *****************
*   PM2.5:----ug  *
*   PM1 0:----ug  *
******************/ 
_EXTERN const uint8_t SRT_PM[];

// SRT_TH Temperature&Humidity 384Bytes(96*32/8)

/* *****************
*   温 度:--.-℃  *
*   湿 度:--.-％  *
******************/ 
_EXTERN const uint8_t SRT_TH[];

// STR_WS wind speed & company name 384Bytes(96*32/8)

/* *****************
*   风 速: -.-m/s *
*   陕西泰新博坤  *
******************/ 
_EXTERN const uint8_t SRT_WIND[];

/* *****************
*   风 速: -.-m/s *
*   噪 声: ---.-dB*
******************/ 
_EXTERN const uint8_t SRT_WIND_NOISE[];


/////////// macros
#define	FILL_1B(pbuf, cmd)	{(*(*pbuf)++) = (cmd & 0xFF);}
#define	FILL_2B(pbuf, cmd)	{(*(*pbuf)++) = (cmd & 0xFF); \
							   (*(*pbuf)++) = ((cmd >> 8) & 0xFF);}

#define	FILL_HEAD(pbuf)	{(*(*pbuf)++) = EX30_SYN & 0xFF;	\
						   (*(*pbuf)++) = (EX30_SYN >>8) & 0xFF;	\
						   (*(*pbuf)++) = EX30_ADDR & 0xFF;	\
						   (*(*pbuf)++) = (EX30_ADDR >> 8) & 0xFF;}

#define FILL_LEN(pbuf,length) FILL_2B(pbuf,length)
#define FILL_SERIAL(pbuf,num) FILL_2B(pbuf,num)
#define FILL_TYPE(pbuf,type) FILL_1B(pbuf,type)
#define FILL_CMD(pbuf,cmd) FILL_1B(pbuf,cmd)
#define FILL_TAG(pbuf,tag) FILL_1B(pbuf,tag)
#define	FILL_CHKSUM(pbuf,chksum)	FILL_2B(pbuf,chksum)


#define	LED_SENDBUFSIZE	2000
typedef struct
{
	BOOL	valid;				// true when data ready
	BOOL	busy;				// true when 
	uint32_t	length;			// length of data to send
	uint8_t	dat[LED_SENDBUFSIZE];
}
APP_ledsendbuf_TypeDef;
_EXTERN APP_ledsendbuf_TypeDef led_sendbuf;


#define	LED_RCVBUFSIZE	40
#define	LED_DATBUFSIZE	40
typedef struct
{		// UART receiver buffer
	uint32_t	timestamp;
	uint32_t	rcvindex;			// index of received byte
	uint8_t	rcvbyte[LED_RCVBUFSIZE];

		// protocol analyse buffer
	uint8_t	valid;
	uint16_t len;
	uint16_t serial;
	uint8_t type;
	uint8_t	cmd;		//
	uint8_t	tlv[LED_RCVBUFSIZE-12];
	uint16_t chksum;
	uint8_t	rcvdat[LED_DATBUFSIZE];
}
APP_ledrcvbuf_TypeDef;
_EXTERN APP_ledrcvbuf_TypeDef led_rcvbuf;

#define TLV_VALUE_MAXSIZE 1024
typedef struct
{
	uint8_t tag;
	uint16_t len;
	uint8_t value[TLV_VALUE_MAXSIZE];
}
APP_TLV_TypeDef;

#define TLV_TAG_MAXSIZE 32
typedef struct
{
	uint16_t syn;
	uint16_t addr;
	uint16_t len;
	uint16_t serial;
	uint8_t type;
	uint8_t cmd;
	uint8_t tlvnum;	// tlv number
	APP_TLV_TypeDef tlv_buf[TLV_TAG_MAXSIZE];
	uint16_t chksum;
}
APP_EX30_TypeDef;
_EXTERN APP_EX30_TypeDef tlv_frame;

#define LED_MAXGRADE 6		// max grade of frame
typedef struct
{
	BOOL	update;			// flag of refresh

	uint16_t packnum;		// serial number
}
APP_ledstat_TypeDef;
_EXTERN APP_ledstat_TypeDef led_stat;

typedef struct
{
	uint8_t programid;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
}
APP_ProgramOption_TypeDef;
_EXTERN APP_ProgramOption_TypeDef prgm_opt;

//////////// constants 
#define EX30_SYN 0x55AA
#define EX30_ADDR 0xFFFF
#define EX30_TYPE 0xC1
#define EX30_CMD_WR 0x02
#define EX30_ACK_WR 0x82
#define EX30_CMD_RD 0x03
#define EX30_ACK_RD 0x83

// TAG define
#define EX30_TAG_REV1 0x01
#define EX30_TAG_REV2 0x02
#define EX30_TAG_REV3 0x03
#define EX30_TAG_SWITCH 0x04	// open/close led
#define EX30_TAG_RTC 0x05		// RTC
#define EX30_TAG_LIGHT 0x06		// brightness
#define EX30_TAG_ADDR 0x07		// protocol address
#define EX30_TAG_DEL 0x08		// delete cmd
#define EX30_TAG_SAV 0x09		// save mode
#define EX30_TAG_DEVINFO 0x0a	// device information
#define EX30_TAG_SCRPRAM 0x0b	// screen parameter
#define EX30_TAG_PRGRMID 0x0c	// program ID
#define EX30_TAG_PARTID 0x0d	// partition ID
#define EX30_TAG_PLAYITEMID 0x0e// play item ID
#define EX30_TAG_PRGRMATTR 0x0f	// program attribiton
#define EX30_TAG_PARTATTR 0x10	// partition attribiton
#define EX30_TAG_PLAYITEMATTR 0x11 // play item attribiton
#define EX30_TAG_PACKETATTR 0x12// data packet attribition
#define EX30_TAG_PACKET 0x13	// data packet
#define EX30_TAG_PLAYITEMSPEC 0x14	// play item special effects
#define EX30_TAG_REV4 0x15
#define EX30_TAG_TIMCTRL 0x16	// time interval control
#define EX30_TAG_IOBIND 0x17	// IO binding
#define EX30_TAG_SWCMD	0x18	// switch command
#define EX30_TAG_UPDAT 0x19		// real time update
#define EX30_TAG_ASSIGN 0x1A 	// program resource distribute

// delete option
#define TAG_DEL_PRGM 0	// delete program
#define TAG_DEL_PART 1	// delete partition
#define TAG_DEL_ITEM 2	// delete play item

// program ID for send
#define PRGRM_PM 0	// PM2.5&PM10 srt program id
#define PRGRM_TH 1	// Humidity&Temperature srt program id
#define PRGRM_WN 2	// wind speed & noise srt program id
#define PRGRM_MFR 3	// company logo srt program id

#define PRGRM_ALL 0xff	

#define PART_ALL 0xff	

#define ITEM_ALL 0xff	

#define PRGRM_WL 2	// wind speed & company logo srt program id

// txt axias for src

// pm2.5 & pm10
#define TXT_PM_X 0x0030
#define TXT_PM_Y 0x0000
#define TXT_PM_W 0x0020
#define TXT_PM_H 0x0020
// temperature & humidity
#define TXT_TH_X 0x0027
#define TXT_TH_Y 0x0000
#define TXT_TH_W 0x0022
#define TXT_TH_H 0x0020
// wind speed & noise
#define TXT_WN_X 0x0027
#define TXT_WN_Y 0x0000
#define TXT_WN_W 0x0022
#define TXT_WN_H 0x0020
// company name
#define TXT_MFR_X 0x0020
#define TXT_MFR_Y 0x0000
#define TXT_MFR_W 0x0020
#define TXT_MFR_H 0x0010

// wind speed & company name
#define TXT_WL_X 0x0020
#define TXT_WL_Y 0x0000
#define TXT_WL_W 0x0020
#define TXT_WL_H 0x0010

/////////// functions
uint16_t get_chksum16(uint8_t *data, uint16_t num);
void APP_led_rcvisr(void);
void APP_led_full(APP_EX30_TypeDef *tlv);
void APP_led_display(void);
void APP_led_ini(void);
void APP_led_logo(void);
void APP_led_connect(void);
void APP_led_setsrccfg(void);
void APP_led_setlight(void);
void APP_led_del(uint8_t del_type, uint8_t prgm_id, uint8_t part_id, uint8_t item_id);
void APP_led_delallprgm(void);
void APP_led_pmdel(void);
void APP_led_pmtxtdel(void);
void APP_led_thdel(void);
void APP_led_thtxtdel(void);
void APP_led_wndel(void);
void APP_led_wntxtdel(void);
void APP_led_mfrdel(void);
void APP_led_mfrtxtdel(void);
void APP_led_opt(APP_ProgramOption_TypeDef *ptr);
void APP_led_pmopt(void);
void APP_led_thopt(void);
void APP_led_wnopt(void);
void APP_led_srt(uint8_t prgm_id, uint8_t *pdata);
void APP_led_pmsrt(void);
void APP_led_thsrt(void);
void APP_led_wnsrt(void);
void APP_led_txt(uint8_t prgm_id, uint8_t *str);
void APP_led_pmtxt(void);
void APP_led_thtxt(void);
void APP_led_wntxt(void);
void APP_led_task(void);
void APP_led_task_noise(void);
void APP_led_pmf2a(float pm25_dat, float pm10_dat, uint8_t *buf);
void APP_led_thf2a(float temperature, float humidity, uint8_t *buf);
void APP_led_wnf2a(float windspeed, float noise, uint8_t *buf);

void APP_led_wldel(void);
void APP_led_wltxtdel(void);
void APP_led_wlopt(void);
void APP_led_wlsrt(void);
void APP_led_wltxt(void);
void APP_led_wlf2a(float windspeed, uint8_t *buf);

#undef	_EXTERN
/* ******************************  END OF FILE  *******************************/
