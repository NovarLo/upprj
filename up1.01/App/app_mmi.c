
#define _LOCAL_MMI

#include "stm32f4xx_hal.h"
#include "arm_math.h"
#include "string.h"
#include "app_user.h"
#include "app_sensor.h"
#include "app_mmi.h"
#include "app_linear.h"


//#define   _BODY_ROUND         // shape of tower body, round/square


#define CALI_MODE_ENABLE        // if actived, work mode of tower will be
// changed according with local calibration

static uint8_t work_mode_shadow = WKMOD_MIX;

#ifdef CALI_MODE_ENABLE
#define CALI_BACKUP(old)        old = device_info.work_mod
#define CALI_ENTER()            device_info.work_mod = WKMOD_CALI
#define CALI_RESTORE(old)   device_info.work_mod = old
#else
#define CALI_BACKUP(old)
#define CALI_ENTER()
#define CALI_RESTORE(old)
#endif


#ifdef MMI_DEBUG
#define _STATIC static
#else
#define _STATIC
#endif

/* ***************************   functions  **********************************/

/* **************************************************
 fucntion:      APP_mmi_reciveisr
 input:     byte: data received form uart
 output:
 describe:  uart receive isr of MMI
***************************************************/

void APP_mmi_rcvisr(void)
{
	enum
	{
		MMI_RCVSTEP_IDLE = 0,
//      MMI_RCVSTEP_HEAD,
		MMI_RCVSTEP_DAT,            // receiving data
		MMI_RCVSTEP_END1,           // find 1st byte of end data
		MMI_RCVSTEP_END2,           // find 2nd byte of end data
		MMI_RCVSTEP_END3,           // find 3rd byte of end data
		MMI_RCVSTEP_END4,           // find 4th byte of end data

		MMI_RCVSTEP_MAX
	};
	static uint8_t mmi_rcvstep = MMI_RCVSTEP_IDLE;

	uint32_t tick;
	uint32_t index;
	uint8_t byte = mmi_rcvbyte;

	HAL_UART_Receive_IT(HDL_UART_TFT, &mmi_rcvbyte, 1);

	tick = HAL_GetTick();
	if ((tick - mmi_rcvbuf.timestamp) > 200)
	{
		mmi_rcvstep = MMI_RCVSTEP_IDLE;
		mmi_rcvbuf.rcvindex = 0;
	}
	mmi_rcvbuf.timestamp = tick;

	switch (mmi_rcvstep)
	{
	case MMI_RCVSTEP_IDLE:
		mmi_rcvbuf.rcvindex = 0;
		if (byte == LCM_RCVSTART)
		{       // find start byte
			mmi_rcvstep = MMI_RCVSTEP_DAT;
			mmi_rcvbuf.rcvbyte[mmi_rcvbuf.rcvindex++] = byte;
		}
		break;

	case MMI_RCVSTEP_DAT:
		if (mmi_rcvbuf.rcvindex >= MMI_RCVBUFSIZE)
		{
			mmi_rcvstep = MMI_RCVSTEP_IDLE;
			mmi_rcvbuf.rcvindex = 0;
		}
		else
		{
			mmi_rcvbuf.rcvbyte[mmi_rcvbuf.rcvindex++] = byte;
			if (byte == ((LCM_RCVEND >> 24) & 0xFF))
			{       // find 1st byte of end
				mmi_rcvstep = MMI_RCVSTEP_END1;
			}
		}
		break;

	case MMI_RCVSTEP_END1:
		mmi_rcvbuf.rcvbyte[mmi_rcvbuf.rcvindex++] = byte;
		if (byte == ((LCM_RCVEND >> 16) & 0xFF))
		{       // find 2nd byte of end
			mmi_rcvstep = MMI_RCVSTEP_END2;
		}
		else
		{       // back to data-receive
			mmi_rcvstep = MMI_RCVSTEP_DAT;
		}
		break;

	case MMI_RCVSTEP_END2:
		mmi_rcvbuf.rcvbyte[mmi_rcvbuf.rcvindex++] = byte;
		if (byte == ((LCM_RCVEND >> 8) & 0xFF))
		{       // find 3rd byte of end
			mmi_rcvstep = MMI_RCVSTEP_END3;
		}
		else
		{       // back to data-receive
			mmi_rcvstep = MMI_RCVSTEP_DAT;
		}
		break;

	case MMI_RCVSTEP_END3:
		mmi_rcvbuf.rcvbyte[mmi_rcvbuf.rcvindex++] = byte;
		if (byte == (LCM_RCVEND & 0xFF))
		{       // find 4th byte of end, copy to rcvdat
			for (index = 0; index < mmi_rcvbuf.rcvindex; index++)
			{
				mmi_rcvbuf.rcvdat[index] = mmi_rcvbuf.rcvbyte[index];
			}
			mmi_rcvbuf.valid = TRUE;

			mmi_rcvstep = MMI_RCVSTEP_IDLE;
			mmi_rcvbuf.rcvindex = 0;
		}
		else if (byte == ((LCM_RCVEND >> 16) & 0xFF))
		{       // find 2nd byte of end
			mmi_rcvstep = MMI_RCVSTEP_END2;
		}
		else
		{       // back to data-receive
			mmi_rcvstep = MMI_RCVSTEP_DAT;
		}
		break;

	default :
		mmi_rcvstep = MMI_RCVSTEP_IDLE;
		mmi_rcvbuf.rcvindex = 0;
		break;

	}
}


/* **************************************************
 fucntion:      APP_mmi_input
 input:
 output:
 describe:  uart receive process of MMI
***************************************************/

void APP_mmi_input(void)
{
	// reset sections
	mmi_rcvbuf.msg = 0;
	mmi_rcvbuf.scrid = 0;
	mmi_rcvbuf.ctrid = 0;
	mmi_rcvbuf.ctrtype = LCMCTR_NULL;

	// get command type from data buffer
	mmi_rcvbuf.cmd = mmi_rcvbuf.rcvdat[1];

	switch (mmi_rcvbuf.cmd)
	{
	case LCMCMD_TPPRESS:        // touchpad pressed
	case LCMCMD_TPRELEASE:      // touchpad released
		;
		break;

	case LCMCMD_SETCTR:     // control update

		mmi_rcvbuf.msg = mmi_rcvbuf.rcvdat[2];

		mmi_rcvbuf.scrid = mmi_rcvbuf.rcvdat[3] << 8;
		mmi_rcvbuf.scrid |= mmi_rcvbuf.rcvdat[4];

		switch (mmi_rcvbuf.msg)      // check message type
		{
		case LCMMSG_SCRGET:     // get scrid
			;   // set scrid in mmi_stat, reserved
			break;

		case LCMMSG_DATGET:     // control updata
								//case LCMMSG_GIFGET:
		case LCMMSG_ICONGET:
			mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
			mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

			mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];
			break;

		default:
			break;
		}
		break;

		//case LCMCMDR_:        // non_control cammand, etc
		//  break;

	default:
		mmi_rcvbuf.msg = LCMMSG_NULL;
		mmi_rcvbuf.ctrtype = LCMCTR_NULL;
		mmi_rcvbuf.scrid = 0;
		mmi_rcvbuf.ctrid = 0;

		break;
	}

	switch (mmi_rcvbuf.scrid)
	{
	case SCRID_SCRMAIN:
		APP_mmi_ctrmain();
		break;

	case SCRID_SCRRTC:
		APP_mmi_ctrrtc();
		break;

	case SCRID_SCRABOUT:
		APP_mmi_ctrabout();
		break;

	case SCRID_SCRUPLOADINGINFO:
		APP_mmi_ctruploadinginfo();
		break;

	case SCRID_SCRCOMCFG:
		APP_mmi_ctrcomcfg();
		break;

	case SCRID_SCRLMTCFG:
		APP_mmi_ctrlmtcfg();
		break;

	case SCRID_SCRMENUADMIN:
		APP_mmi_ctrmenuadmin();
		break;

	case SCRID_SCRMENUUSER:
		APP_mmi_ctrmenuuser();
		break;

	case SCRID_SCRMENUADJ:
		APP_mmi_ctradj();
		break;

	case SCRID_SCROEMOPT:
		APP_mmi_ctroemopt();
		break;

	case SCRID_SCRCALIWEIGHT:
		APP_mmi_ctrupweightadj();
		break;

	case SCRID_SCRCALIWIRE1:
		APP_mmi_ctrupwire1adj();
		break;

	case SCRID_SCRCALIWIRE2:
		APP_mmi_ctrupwire2adj();
		break;
	case SCRID_SCRCALIKB:
		APP_mmi_ctrupcalikbadj();
		break;
/* 
		case SCRID_SCRSENSORADJ:
			APP_mmi_ctrsensoradj();
			break;
 */


	default:
		mmi_stat.grade = 0;
		mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMAIN;
		break;
	}
	scrdat.refresh = TRUE;

//  mmi_rcvbuf.valid = FALSE;
}


/* **************************************************
 fucntion:      APP_mmi_display
 input:
 output:
 describe:  output routine of MMI
***************************************************/

void APP_mmi_display(void)
{
	if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRMAIN)
	{
		APP_mmi_scrmain();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRRTC)
	{
		APP_mmi_scrrtc();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRABOUT)
	{
		APP_mmi_scrabout();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRUPLOADINGINFO)
	{
		APP_mmi_scruploadinginfo();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRCOMCFG)
	{
		APP_mmi_scrcomcfg();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRLMTCFG)
	{
		APP_mmi_scrlmtcfg();
		}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRMENUADMIN)
	{
		APP_mmi_scrmenuadmin();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRMENUUSER)
	{
		APP_mmi_scrmenuuser();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCROEMOPT)
	{
		APP_mmi_scroemopt();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRMENUADJ)
	{
		APP_mmi_scradj();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRCALIWEIGHT)
	{
		APP_mmi_scrupweightadj();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRCALIWIRE1)
	{
		APP_mmi_scrupwire1adj();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRCALIWIRE2)
	{
		APP_mmi_scrupwire2adj();
	}
	else if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRCALIKB)
	{
		APP_mmi_scrupcalikbadj();
	}
	else
	// if (mmi_stat.scrid[mmi_stat.grade] == SCRID_SCRLOGO)
	{
//      static uint8_t step = 0;

		if (HAL_GetTick() - tick_powerup > 9000)        // && step++ > 5)
		{
//          step = 0;
//uint8_t* buf = mmi_sendbuf.dat;

			//APP_mmi_showscreen(&buf, SCRID_SCRMAIN);
			//mmi_sendbuf.length = buf - mmi_sendbuf.dat;
			//HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);

			mmi_stat.grade = 0;
			mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMAIN;
		}
	}

	mmi_stat.last_scrid = mmi_stat.scrid[mmi_stat.grade];
}



/* **************************************************
 fucntion:      APP_mmi_ini
 input:
 output:
 describe:  MMI initial & show logo screen
***************************************************/

void APP_mmi_ini(void)
{
	HAL_UART_Receive_IT(HDL_UART_TFT, &mmi_rcvbyte, 1);

	mmi_stat.update = FALSE;
	mmi_stat.grade = 0;
	mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRLOGO;
	mmi_sendbuf.valid = FALSE;

	APP_mmi_scrlogo();
}


/* **************************************************
 fucntion:      APP_mmi_scrlogo
 input:
 output:
 describe:  show logo screen
***************************************************/

void APP_mmi_scrlogo(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint8_t txt_buf[21], tmph, tmpl;

	HAL_UART_Transmit(HDL_UART_TFT, "A55A", 2, 50);

	APP_mmi_showrtc(&buf, TRUE);

	APP_mmi_screenupdate(&buf, FALSE);

	APP_mmi_showscreen(&buf, SCRID_SCRLOGO);

//-------------- start of batch update ---------------
	FILLCMD_START(&buf);
	FILLCMD_1B(&buf, LCMCMD_SETCTR);
	FILLCMD_1B(&buf, LCMMSG_SETBATCH);
	FILLSCRID(&buf, SCRID_SCRLOGO);

	switch (dustmon_info.company_no)
	{
	case 1:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, EKYJ_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, EKYJ_TXTLOGO2);
		break;
	case 2:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, ZZLY_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, ZZLY_TXTLOGO2);
		break;
	case 3:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, SXRW_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, SXRW_TXTLOGO2);
		break;
	case 4:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, XAML_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, XAML_TXTLOGO2);
		break;
	case 5:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, ZFMD_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, ZFMD_TXTLOGO2);
		break;
	case 6:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, XMRS_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, XMRS_TXTLOGO2);
		break;
	case 7:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, SDWK_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, SDWK_TXTLOGO2);
		break;
	case 8:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, YRRD_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, YRRD_TXTLOGO2);
		break;
	case 9:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, GLD_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, GLD_TXTLOGO2);
		break;
	case 10:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, LZXQ_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, LZXQ_TXTLOGO2);
		break;
	case 11:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, SLX_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, SLX_TXTLOGO2);
		break;
	default:
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO1, TXBK_TXTLOGO1);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO2, TXBK_TXTLOGO2);
		break;
	}

	tmpl = device_info.addr[0];
	tmph = tmpl / 100;
	txt_buf[0] = tmph + '0';
	tmpl %= 100;
	tmph = tmpl / 10;
	txt_buf[1] = tmph + '0';
	tmpl %= 10;
	txt_buf[2] = tmpl + '0';
	txt_buf[3] = '-';

	tmpl = device_info.addr[1];
	tmph = tmpl / 100;
	txt_buf[4] = tmph + '0';
	tmpl %= 100;
	tmph = tmpl / 10;
	txt_buf[5] = tmph + '0';
	tmpl %= 10;
	txt_buf[6] = tmpl + '0';
	txt_buf[7] = '-';

	tmpl = device_info.addr[2];
	tmph = tmpl / 100;
	txt_buf[8] = tmph + '0';
	tmpl %= 100;
	tmph = tmpl / 10;
	txt_buf[9] = tmph + '0';
	tmpl %= 10;
	txt_buf[10] = tmpl + '0';
	txt_buf[11] = '-';

	tmpl = device_info.addr[3];
	tmph = tmpl / 100;
	txt_buf[12] = tmph + '0';
	tmpl %= 100;
	tmph = tmpl / 10;
	txt_buf[13] = tmph + '0';
	tmpl %= 10;
	txt_buf[14] = tmpl + '0';
	txt_buf[15] = '-';

	tmpl = device_info.addr[4];
	tmph = tmpl / 100;
	txt_buf[16] = tmph + '0';
	tmpl %= 100;
	tmph = tmpl / 10;
	txt_buf[17] = tmph + '0';
	tmpl %= 10;
	txt_buf[18] = tmpl + '0';
	txt_buf[19] = '\0';

	APP_mmi_showtext_batch(&buf, CTRID_SCRLOGO_TXTLOGO3, txt_buf);

	FILLCMD_END(&buf);
//-------------- end of batch update ---------------

	// show logo
	switch (dustmon_info.company_no)
	{
	case 1:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, EKYJ_ICONIDX_LOGO);
		break;
	case 2:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, ZZLY_ICONIDX_LOGO);
		break;
	case 3:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, SXRW_ICONIDX_LOGO);
		break;
	case 4:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, XAML_ICONIDX_LOGO);
		break;
	case 5:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, ZFMD_ICONIDX_LOGO);
		break;
	case 6:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, XMRS_ICONIDX_LOGO);
		break;
	case 7:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, SDWK_ICONIDX_LOGO);
		break;
	case 8:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, YRRD_ICONIDX_LOGO);
		break;
	case 9:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, GLD_ICONIDX_LOGO);
		break;
	case 10:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, LZXQ_ICONIDX_LOGO);
		break;
	case 11:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, SLX_ICONIDX_LOGO);
		break;
	default:
		APP_mmi_showicon(&buf, SCRID_SCRLOGO, CTRID_SCRLOGO_ICONLOGO, TXBK_ICONIDX_LOGO);
		break;
	}


	APP_mmi_screenupdate(&buf, TRUE);


	// get version of MMI
	APP_mmi_gettext(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_TXTMMIVER);

	// set ready flag to send.
	mmi_sendbuf.length = buf - mmi_sendbuf.dat;
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length, 50);
	mmi_sendbuf.valid = FALSE;
}


/* **************************************************
 fucntion:      APP_mmi_ctrmain
 input:
 output:
 describe:  control input process of main screen
***************************************************/

void APP_mmi_ctrmain(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
//  float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRMAIN_BTNABOUT:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRABOUT;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid == CTRID_SCRMAIN_TXTPSWD)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				for (index = 0; index < 8; index++)
				{
					if (strcmp((const char *)src, (const char *)device_info.pswd[index]) == 0) break;
				}
				if (index == 0)
				{
					mmi_stat.grade++;
					mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMENUUSER;
				}
				else if (index == 1)
				{
					mmi_stat.grade++;
					mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMENUADMIN;
				}
				else if (index == 2)
				{
					mmi_stat.grade++;
					mmi_stat.scrid[mmi_stat.grade] = SCRID_SCROEMOPT;
				}
#ifdef PSWD_ADMIN
				else if (strcmp((const char *)src, PSWD_ADMIN) == 0)
				{
					mmi_stat.grade++;
					mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMENUADMIN;
				}
#endif
#ifdef PSWD_OPT
				else if (strcmp((const char *)src, PSWD_OPT) == 0)
				{
					mmi_stat.grade++;
					mmi_stat.scrid[mmi_stat.grade] = SCRID_SCROEMOPT;
				}
#endif

			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrmain
 input:
 output:
 describe:  set data for main screen
***************************************************/

void APP_mmi_scrmain(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t icon_index;
	uint8_t i;

	APP_mmi_screenupdate(&buf, FALSE);

//-------------- start of batch update ---------------

	FILLCMD_START(&buf);
	FILLCMD_1B(&buf, LCMCMD_SETCTR);
	FILLCMD_1B(&buf, LCMMSG_SETBATCH);
	FILLSCRID(&buf, SCRID_SCRMAIN);

	// clear password txt
	APP_mmi_showtext_batch(&buf, CTRID_SCRMAIN_TXTPSWD, (uint8_t *)"");

	// show line 1~16
	for (i = 0; i < 16; i++)
	{
		if (sensor_value[SENS_UPSNR1 + i].valid)
		{
			APP_mmi_f2a(sensor_value[SENS_UPSNR1 + i].value, 0, databuf);
		}
		else
		{
			STRING_DASH5();
		}
		APP_mmi_showtext_batch(&buf, CTRID_SCRMAIN_TXTAIN1 + i, databuf);
	}
	for (i = 0; i < 16; i++)
	{
		if (sensor_value[SENS_UPSNR1 + i].valid)
		{
			APP_mmi_f2a(sensor_value[SENS_UPSNR1 + i].value * 3300 / 4096, 0, databuf);
		}
		else
		{
			STRING_DASH5();
		}
		APP_mmi_showtext_batch(&buf, CTRID_SCRMAIN_TXTKG1 + i, databuf);
	}

	// show weight
	if (sensor_value[SENS_UPSNR1].valid | sensor_value[SENS_UPSNR2].valid | sensor_value[SENS_UPSNR3].valid | sensor_value[SENS_UPSNR4].valid |
		sensor_value[SENS_UPSNR5].valid | sensor_value[SENS_UPSNR6].valid | sensor_value[SENS_UPSNR7].valid | sensor_value[SENS_UPSNR8].valid |
		sensor_value[SENS_UPSNR9].valid | sensor_value[SENS_UPSNR10].valid | sensor_value[SENS_UPSNR11].valid | sensor_value[SENS_UPSNR12].valid |
		sensor_value[SENS_UPSNR13].valid | sensor_value[SENS_UPSNR14].valid | sensor_value[SENS_UPSNR15].valid | sensor_value[SENS_UPSNR16].valid)
	{
		APP_mmi_f2a(sensor_value[SENS_UPWEIGHT].value, 0, databuf);
	}
	else
	{
		STRING_DASH5();
	}
	APP_mmi_showtext_batch(&buf, CTRID_SCRMAIN_TXTWEIGHT, databuf);
	// show weight KG
	if (sensor_value[SENS_UPSNR1].valid | sensor_value[SENS_UPSNR2].valid | sensor_value[SENS_UPSNR3].valid | sensor_value[SENS_UPSNR4].valid |
		sensor_value[SENS_UPSNR5].valid | sensor_value[SENS_UPSNR6].valid | sensor_value[SENS_UPSNR7].valid | sensor_value[SENS_UPSNR8].valid |
		sensor_value[SENS_UPSNR9].valid | sensor_value[SENS_UPSNR10].valid | sensor_value[SENS_UPSNR11].valid | sensor_value[SENS_UPSNR12].valid |
		sensor_value[SENS_UPSNR13].valid | sensor_value[SENS_UPSNR14].valid | sensor_value[SENS_UPSNR15].valid | sensor_value[SENS_UPSNR16].valid)
	{
		APP_mmi_f2a(sensor_value[SENS_UPWEIGHT].value * 3300 / 4096, 0, databuf);
	}
	else
	{
		STRING_DASH5();
	}
	APP_mmi_showtext_batch(&buf, CTRID_SCRMAIN_TXTKG, databuf);

	FILLCMD_END(&buf);

//-------------- end of batch update ---------------


//-------------- start of independent update ---------------
//  show ADC input line icon
	for (i = 0; i < 16; i++)
	{
		if (sensor_value[SENS_UPSNR1 + i].valid)
		{
			icon_index = ICONIDX_CHECKED;
		}
		else icon_index = ICONIDX_UNCHECKED;
		APP_mmi_showicon(&buf, SCRID_SCRMAIN, CTRID_SCRMAIN_ICONAIN1 + i, icon_index);
	}

	// show GPRS stat
	if (gprs_rssi == 3)	// UDP正常通信
	{
		if (DTU_rssi == 99)
		{
			icon_index = ICONIDX_GPRSLV0;
		}
		else if ((DTU_rssi < 2) || (DTU_rssi > 100 && DTU_rssi <= 105))
		{
			icon_index = ICONIDX_GPRSLV1;
		}
		else if ((DTU_rssi < 10) || (DTU_rssi > 105 && DTU_rssi <= 140))
		{
			icon_index = ICONIDX_GPRSLV2;
		}
		else if ((DTU_rssi < 20) || (DTU_rssi > 140 && DTU_rssi <= 160))
		{
			icon_index = ICONIDX_GPRSLV3;
		}
		else
		{
			icon_index = ICONIDX_GPRSLV4;
		}
	}
	else if (gprs_rssi == 2)	// 打开网络成功
	{
		icon_index = ICONIDX_GPRSLV0;
	}
	else if (gprs_rssi == 1)	// AT命令正常应答
	{
		icon_index = ICONIDX_GPRSNULL;
	}
	else	// 模块未工作
	{
		icon_index = ICONIDX_GPRSNULL;
	}
	APP_mmi_showicon(&buf, SCRID_SCRMAIN, CTRID_SCRMAIN_ICONGPRS, icon_index);



	// show time
	APP_mmi_showrtc(&buf, FALSE);


	// could add other routines infront this line.

//-------------- end of independent update ---------------

	APP_mmi_showscreen(&buf, SCRID_SCRMAIN);

	// show debug data
#ifdef  MMI_DEBUG
	length = buf - mmi_sendbuf.dat;
	length += (11 + 4);     // max length of this frame
	APP_mmi_i2a((int32_t)length, 4, databuf);
	APP_mmi_showtext(&buf, SCRID_SCRMAIN,\
						 CTRID_SCRMAIN_TXTDEBUG, databuf);
#else
	APP_mmi_showtext(&buf, SCRID_SCRMAIN,\
						 CTRID_SCRMAIN_TXTDEBUG, "");
#endif

	APP_mmi_screenupdate(&buf, TRUE);

	// set ready flag to send.
	mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);

}


/* **************************************************
 fucntion:      APP_mmi_ctrrtc
 input:
 output:
 describe:  control input process of rtc screen
***************************************************/

void APP_mmi_ctrrtc(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRRTC_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRRTC_BTNSAVE:
					if (status == LCMSTAT_RELEASE)
					{
						// save time into rtc
						RTC_DateTypeDef sDate;
						RTC_TimeTypeDef sTime;

						sDate.Year = scrdat.dat_i[0];
						sDate.Month = scrdat.dat_i[1];
						sDate.Date = scrdat.dat_i[2];
						sDate.WeekDay = scrdat.dat_i[3];

						sTime.Hours = scrdat.dat_i[4];
						sTime.Minutes = scrdat.dat_i[5];
						sTime.Seconds = scrdat.dat_i[6];
						sTime.TimeFormat = RTC_HOURFORMAT12_AM;
						sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
						sTime.StoreOperation = RTC_STOREOPERATION_RESET;

						HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);
						HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN);

						scrdat.dat_ui[0] = TRUE;    // rtc update flag
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			src = &mmi_rcvbuf.rcvdat[8];
			if (APP_mmi_a2f(src, &dat_f))
			{
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRRTC_TXTYEAR:
					scrdat.dat_i[0] = dat_f;
					break;

				case CTRID_SCRRTC_TXTMONTH:
					scrdat.dat_i[1] = dat_f;
					break;

				case CTRID_SCRRTC_TXTDAY:
					scrdat.dat_i[2] = dat_f;
					break;

				case CTRID_SCRRTC_TXTWEEK:
					scrdat.dat_i[3] = dat_f;
					break;

				case CTRID_SCRRTC_TXTHOUR:
					scrdat.dat_i[4] = dat_f;
					break;

				case CTRID_SCRRTC_TXTMINUTE:
					scrdat.dat_i[5] = dat_f;
					break;

				case CTRID_SCRRTC_TXTSECOND:
					scrdat.dat_i[6] = dat_f;
					break;

				default:
					break;
				}

			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrrtc
 input:
 output:
 describe:  set data for rtc screen
***************************************************/

void APP_mmi_scrrtc(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;

	// test
	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read date & time from rtc.
		RTC_DateTypeDef sDate;
		RTC_TimeTypeDef sTime;

		HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);

		scrdat.dat_i[0] = sDate.Year;
		scrdat.dat_i[1] = sDate.Month;
		scrdat.dat_i[2] = sDate.Date;
		scrdat.dat_i[3] = sDate.WeekDay;

		scrdat.dat_i[4] = sTime.Hours;
		scrdat.dat_i[5] = sTime.Minutes;
		scrdat.dat_i[6] = sTime.Seconds;


		scrdat.dat_ui[0] = FALSE;   // rtc update flag

		scrdat.refresh = TRUE;

	}

//  if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRRTC);

		// show year
		APP_mmi_i2a(scrdat.dat_i[0], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTYEAR, databuf);

		// show month
		APP_mmi_i2a(scrdat.dat_i[1], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTMONTH, databuf);

		// show date
		APP_mmi_i2a(scrdat.dat_i[2], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTDAY, databuf);

		// show weekday
		if (scrdat.dat_i[3] > 6)
		{
			APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTWEEK, "日");
		}
		else
		{
			APP_mmi_i2a(scrdat.dat_i[3], 1, databuf);
			APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTWEEK, databuf);
		}


		// show hour
		APP_mmi_i2a(scrdat.dat_i[4], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTHOUR, databuf);

		// show minute
		APP_mmi_i2a(scrdat.dat_i[5], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTMINUTE, databuf);

		// show sencond
		APP_mmi_i2a(scrdat.dat_i[6], 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRRTC_TXTSECOND, databuf);


		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------

		// update rtc time after setup
		//if (scrdat.dat_ui[0])
		{
			scrdat.dat_ui[0] = FALSE;
			APP_mmi_showrtc(&buf, FALSE);
		}

		APP_mmi_showscreen(&buf, SCRID_SCRRTC);

		// show debug data
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef  MMI_DEBUG
		APP_mmi_showtext(&buf, SCRID_SCRRTC,\
							 CTRID_SCRRTC_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRRTC,\
							 CTRID_SCRRTC_TXTDEBUG, databuf);
#endif
		APP_mmi_showtext(&buf, SCRID_SCRRTC,\
							 CTRID_SCRRTC_TXTDEBUG, "    ");

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}


/* **************************************************
 fucntion:      APP_mmi_ctrabout
 input:
 output:
 describe:  control input process of about screen
***************************************************/

void APP_mmi_ctrabout(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
	uint8_t *src;
	uint32_t head, tail;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRABOUT_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				default:
					break;
				}
			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			src = &mmi_rcvbuf.rcvdat[8];
			if (mmi_rcvbuf.ctrid == CTRID_SCRABOUT_TXTMMIVER)
			{
				head = 0;
				tail = 0;
				while (*src != '\0' && *src != '.')
				{
					if (*src >= '0' && *src <= '9')
					{
						head *= 10;
						head += *src - 0x30;
						src++;
					}
					else
					{
						head = 0;
					}
				}
				if (*src == '.') src++;
				while (*src != '\0')
				{
					if (*src >= '0' && *src <= '9')
					{
						tail *= 10;
						tail += *src - 0x30;
						src++;
					}
					else
					{
						tail = 0;
					}
				}
				head <<= 4;
				head |= tail & 0x0F;
				device_ver.ver_mmi = (uint8_t)head;

				device_ver.delay = 470;
				device_ver.flag = CFGSTAT_SAVE;
			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrabout
 input:
 output:
 describe:  set data for about screen
***************************************************/

void APP_mmi_scrabout(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint8_t ver;
	float head, tail;

	uint8_t txt_buf[21], tmph, tmpl;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, show date & get MMI version.
/* 
		scrdat.refresh = TRUE;
	}

	if (scrdat.refresh)
	{
*/
		scrdat.refresh = FALSE;

		APP_mmi_screenupdate(&buf, FALSE);

		APP_mmi_showscreen(&buf, SCRID_SCRABOUT);

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRABOUT);

		switch (dustmon_info.company_no)
		{
		case 1:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, EKYJ_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, EKYJ_TXTLOGO2);
			break;
		case 2:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, ZZLY_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, ZZLY_TXTLOGO2);
			break;
		case 3:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, SXRW_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, SXRW_TXTLOGO2);
			break;
		case 4:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, XAML_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, XAML_TXTLOGO2);
			break;
		case 5:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, ZFMD_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, ZFMD_TXTLOGO2);
			break;
		case 6:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, XMRS_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, XMRS_TXTLOGO2);
			break;
		case 7:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, SDWK_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, SDWK_TXTLOGO2);
			break;
		case 8:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, YRRD_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, YRRD_TXTLOGO2);
			break;
		case 9:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, GLD_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, GLD_TXTLOGO2);
			break;
		case 10:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, LZXQ_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, LZXQ_TXTLOGO2);
			break;
		case 11:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, SLX_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, SLX_TXTLOGO2);
			break;
		default:
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO1, TXBK_TXTLOGO1);
			APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO2, TXBK_TXTLOGO2);
			break;
		}

		tmpl = device_info.addr[0];
		tmph = tmpl / 100;
		txt_buf[0] = tmph + '0';
		tmpl %= 100;
		tmph = tmpl / 10;
		txt_buf[1] = tmph + '0';
		tmpl %= 10;
		txt_buf[2] = tmpl + '0';
		txt_buf[3] = '-';

		tmpl = device_info.addr[1];
		tmph = tmpl / 100;
		txt_buf[4] = tmph + '0';
		tmpl %= 100;
		tmph = tmpl / 10;
		txt_buf[5] = tmph + '0';
		tmpl %= 10;
		txt_buf[6] = tmpl + '0';
		txt_buf[7] = '-';

		tmpl = device_info.addr[2];
		tmph = tmpl / 100;
		txt_buf[8] = tmph + '0';
		tmpl %= 100;
		tmph = tmpl / 10;
		txt_buf[9] = tmph + '0';
		tmpl %= 10;
		txt_buf[10] = tmpl + '0';
		txt_buf[11] = '-';

		tmpl = device_info.addr[3];
		tmph = tmpl / 100;
		txt_buf[12] = tmph + '0';
		tmpl %= 100;
		tmph = tmpl / 10;
		txt_buf[13] = tmph + '0';
		tmpl %= 10;
		txt_buf[14] = tmpl + '0';
		txt_buf[15] = '-';

		tmpl = device_info.addr[4];
		tmph = tmpl / 100;
		txt_buf[16] = tmph + '0';
		tmpl %= 100;
		tmph = tmpl / 10;
		txt_buf[17] = tmph + '0';
		tmpl %= 10;
		txt_buf[18] = tmpl + '0';
		txt_buf[19] = '\0';

		APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTLOGO3, txt_buf);

		// show software version
		ver = device_ver.ver_soft;
		head = (float)(ver >> 4);
		tail = (float)(ver & 0x0F);
		head += tail / 100.0f + 0.005f;
		APP_mmi_f2a(head, 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTSOFTVER, databuf);

		// show protocol version
		ver = device_ver.ver_prtcl;
		head = (float)(ver >> 4);
		tail = (float)(ver & 0x0F);
		head += tail / 100.0f + 0.005f;
		APP_mmi_f2a(head, 2, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRABOUT_TXTPROTOCOLVER, databuf);

		FILLCMD_END(&buf);

		//-------------- end of batch update ---------------

/* 
*/
		switch (dustmon_info.company_no)
		{
		case 1:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, EKYJ_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, EKYJ_ICONIDX_QRC);
			break;
		case 2:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, ZZLY_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, ZZLY_ICONIDX_QRC);
			break;
		case 3:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, SXRW_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, SXRW_ICONIDX_QRC);
			break;
		case 4:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, XAML_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, XAML_ICONIDX_QRC);
			break;
		case 5:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, ZFMD_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, ZFMD_ICONIDX_QRC);
			break;
		case 6:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, XMRS_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, XMRS_ICONIDX_QRC);
			break;
		case 7:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, SDWK_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, SDWK_ICONIDX_QRC);
			break;
		case 8:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, YRRD_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, YRRD_ICONIDX_QRC);
			break;
		case 9:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, GLD_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, GLD_ICONIDX_QRC);
			break;
		case 10:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, LZXQ_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, LZXQ_ICONIDX_QRC);
			break;
		case 11:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, SLX_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, SLX_ICONIDX_QRC);
			break;
		default:
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONLOGO, TXBK_ICONIDX_LOGO);
			APP_mmi_showicon(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_ICONMFR, TXBK_ICONIDX_QRC);
			break;
		}

		// get version of MMI
		APP_mmi_gettext(&buf, SCRID_SCRABOUT, CTRID_SCRABOUT_TXTMMIVER);


/* 
#ifdef QRC_MFR
			// show OEM info
		APP_mmi_showtext(&buf, SCRID_SCRABOUT, \
							CTRID_SCRABOUT_QRCMFR, QRC_MFR);
//      APP_mmi_ctrvisiable(&buf, SCRID_SCRABOUT, \
//                          CTRID_SCRABOUT_QRCMFR, TRUE);
#else
			// hide QR-code
		#define QRC_LEFT    383
		#define QRC_UP      318
		#define QRC_WIDTH   110
		#define QRC_HIDTH   110
		#define QRC_RIGHT   (QRC_LEFT + QRC_WIDTH)
		#define QRC_DOWN    (QRC_UP + QRC_HIDTH)

		APP_mmi_setforground(&buf, COLOR_BLACK);
		APP_mmi_showretangle(&buf, QRC_LEFT, QRC_UP, \
									QRC_RIGHT, QRC_DOWN, TRUE);
//      APP_mmi_ctrvisiable(&buf, SCRID_SCRABOUT, \
//                          CTRID_SCRABOUT_QRCMFR, FALSE);
#endif
*/

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRABOUT,\
							 CTRID_SCRABOUT_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRABOUT,\
							 CTRID_SCRABOUT_TXTDEBUG, "    ");
#endif

		APP_mmi_screenupdate(&buf, TRUE);

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

/* **************************************************
 fucntion:      APP_mmi_ctruploadinginfo
 input:
 output:
 describe:  control input process of elivator info screen
***************************************************/

void APP_mmi_ctruploadinginfo(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRDUSTMONINFO_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRDUSTMONINFO_BTNENTER:
					if (status == LCMSTAT_RELEASE)
					{
						// save data

						if (scrdat.dat_i[15] == TRUE && scrdat.dat_i[16] == FALSE)
						{
							device_ver.ver_dtu = DTU_MG2639;
						}
						else
						{
							device_ver.ver_dtu = DTU_SIM7600CEL;
						}

						device_ver.delay = 500;
						device_ver.flag = CFGSTAT_SAVE;

						mmi_stat.grade--;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			;
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		status = mmi_rcvbuf.rcvdat[7];
		switch (mmi_rcvbuf.ctrid)
		{

		case CTRID_SCRHWVER_ICONA21:
			if (status == LCMSTAT_RELEASE)
			{
				scrdat.dat_i[15] = TRUE;
				scrdat.dat_i[16] = FALSE;
			}
			break;
		case CTRID_SCRHWVER_ICONB00:
			if (status == LCMSTAT_RELEASE)
			{
				scrdat.dat_i[16] = TRUE;
				scrdat.dat_i[15] = FALSE;
			}
			break;
		default:
			break;
		}

		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scruploadinginfo
 input:
 output:
 describe:  set data for elivator info screen
***************************************************/

void APP_mmi_scruploadinginfo(void)
{
	uint8_t *buf = mmi_sendbuf.dat;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		switch (device_ver.ver_dtu)
		{
		case DTU_MG2639:
			scrdat.dat_i[15] = TRUE;
			scrdat.dat_i[16] = FALSE;
			break;
		case DTU_SIM7600CEL:
			scrdat.dat_i[15] = FALSE;
			scrdat.dat_i[16] = TRUE;
			break;
		}


		scrdat.refresh = TRUE;
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRUPLOADINGINFO);

		FILLCMD_END(&buf);

		// show hardware sps32 a21 enable ICON
		APP_mmi_showicon(&buf, SCRID_SCRUPLOADINGINFO,
						 CTRID_SCRHWVER_ICONA21,
						 scrdat.dat_i[15]);

		// show hardware sps32 b00 enable ICON
		APP_mmi_showicon(&buf, SCRID_SCRUPLOADINGINFO,
						 CTRID_SCRHWVER_ICONB00,
						 scrdat.dat_i[16]);

		//-------------- end of batch update ---------------

		APP_mmi_showscreen(&buf, SCRID_SCRUPLOADINGINFO);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRUPLOADINGINFO,\
							 CTRID_SCRDUSTMONINFO_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRUPLOADINGINFO,\
							 CTRID_SCRDUSTMONINFO_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}


/* **************************************************
 fucntion:      APP_mmi_ctrcomcfg
 input:
 output:
 describe:  control input process of com cfg screen
***************************************************/

void APP_mmi_ctrcomcfg(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRCOMCFG_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRCOMCFG_BTNENTER:
					if (status == LCMSTAT_RELEASE)
					{
						uint32_t index2;
						int32_t *src2;

						// save data

						src2 = scrdat.dat_i;

						// address in dat_i[0:4]
						for (index = 0; index < 5; index++)
						{
							device_info.addr[index] = (uint8_t)*src2++;
						}

						// ip_port in dat_i[5:24]
						for (index2 = 0; index2 < 4; index2++)
						{
							for (index = 0; index < 4; index++)
							{
								device_info.ip_port[index2][index] = (uint8_t)*src2++;
							}
							device_info.ip_port[index2][index++] = (uint8_t)(*src2 >> 8);
							device_info.ip_port[index2][index++] = (uint8_t)*src2++;
						}

						// address in dat_i[25:29]
						device_info.beat_time = scrdat.dat_i[25];
						device_info.recon_time = scrdat.dat_i[26];
						device_info.datrpt_time = scrdat.dat_i[27];
						device_info.datsave_time = scrdat.dat_i[28];
						device_info.link_timeout = scrdat.dat_i[29];

						device_info.delay = 500;
						device_info.flag = CFGSTAT_SAVE;

						mmi_stat.grade--;
					}
					break;

				default:
					break;
				}
			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRCOMCFG_TXTADDR_0 &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRCOMCFG_TXTTMOUT)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_i[mmi_rcvbuf.ctrid - CTRID_SCRCOMCFG_TXTADDR_0] = dat_f;
				}
			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		status = mmi_rcvbuf.rcvdat[7];
		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrcomcfg
 input:
 output:
 describe:  set data for com config screen
***************************************************/

void APP_mmi_scrcomcfg(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t index, index2;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		int32_t *dest;

		// first incoming, read com config data.

		dest = scrdat.dat_i;

		// address in dat_i[0:4]
		for (index = 0; index < 5; index++)
		{
			*dest++ = device_info.addr[index];
		}

		// ip_port in dat_i[5:24]
		for (index2 = 0; index2 < 4; index2++)
		{
			for (index = 0; index < 4; index++)
			{
				*dest++ = device_info.ip_port[index2][index];
			}
			*dest = device_info.ip_port[index2][index++];
			*dest <<= 8;
			*dest += device_info.ip_port[index2][index++];
			dest++;
		}

		// address in dat_i[25:29]
		*dest++ = device_info.beat_time;
		*dest++ = device_info.recon_time;
		*dest++ = device_info.datrpt_time;
		*dest++ = device_info.datsave_time;
		*dest++ = device_info.link_timeout;

		scrdat.refresh = TRUE;

	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRCOMCFG);

		index = 0;
		index2 = CTRID_SCRCOMCFG_TXTADDR_0;
		while (index2 < CTRID_SCRCOMCFG_TXTMAX)
		{
			APP_mmi_f2a(scrdat.dat_i[index++], 0, databuf);
			APP_mmi_showtext_batch(&buf, index2++, databuf);
		}

		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------


		//--------------- end of independent update ----------------

		APP_mmi_showscreen(&buf, SCRID_SCRCOMCFG);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRCOMCFG,\
							 CTRID_SCRCOMCFG_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRCOMCFG,\
							 CTRID_SCRCOMCFG_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

/* **************************************************
 fucntion:      APP_mmi_ctrlmtcfg
 input:
 output:
 describe:  control input process of limit config screen
***************************************************/

void APP_mmi_ctrlmtcfg(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRLMTCFG_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRLMTCFG_BTNENTER:
					if (status == LCMSTAT_RELEASE)
					{
						// save data
						limit_tbl.limit[SENS_UPWEIGHT].hilimit = scrdat.dat_f[0];
						limit_tbl.limit[SENS_UPWEIGHT].lolimit = scrdat.dat_f[1];
						limit_tbl.limit[SENS_UPWEIGHT].hiwarn = scrdat.dat_f[2];

						limit_tbl.limit[SENS_CABR].hilimit = limit_tbl.limit[SENS_CABL].hilimit = scrdat.dat_f[3];
						limit_tbl.limit[SENS_CABR].lolimit = limit_tbl.limit[SENS_CABL].lolimit = scrdat.dat_f[4];
						limit_tbl.limit[SENS_CABR].hiwarn = limit_tbl.limit[SENS_CABL].hiwarn = scrdat.dat_f[5];

						limit_tbl.delay = 500;
						limit_tbl.flag = CFGSTAT_SAVE;

						mmi_stat.grade--;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRLMTCFG_TXTWTHIGH &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRLMTCFG_TXTWRPRE)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f[mmi_rcvbuf.ctrid - CTRID_SCRLMTCFG_TXTWTHIGH] = dat_f;
				}
			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:  

		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrlmtcfg
 input:
 output:
 describe:  set data for limit config screen
***************************************************/

void APP_mmi_scrlmtcfg(void)
{
	uint8_t *buf = mmi_sendbuf.dat;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		scrdat.dat_f[0] = limit_tbl.limit[SENS_UPWEIGHT].hilimit;
		scrdat.dat_f[1] = limit_tbl.limit[SENS_UPWEIGHT].lolimit;
		scrdat.dat_f[2] = limit_tbl.limit[SENS_UPWEIGHT].hiwarn;  

		scrdat.dat_f[3] = limit_tbl.limit[SENS_CABL].hilimit;
		scrdat.dat_f[4] = limit_tbl.limit[SENS_CABL].lolimit;
		scrdat.dat_f[5] = limit_tbl.limit[SENS_CABL].hiwarn;

		scrdat.refresh = TRUE;
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRLMTCFG);

		// show valve 1 data
		APP_mmi_f2a(scrdat.dat_f[0], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWTHIGH, databuf);

		// show valve 2 data
		APP_mmi_f2a(scrdat.dat_f[1], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWTLOW, databuf);

		// show valve 3 data
		APP_mmi_f2a(scrdat.dat_f[2], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWTPRE, databuf);

		// show valve 4 data
		APP_mmi_f2a(scrdat.dat_f[3], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWRHIGH, databuf);

		// show valve 5 data
		APP_mmi_f2a(scrdat.dat_f[4], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWRLOW, databuf);

		// show valve 6 data
		APP_mmi_f2a(scrdat.dat_f[5], 1, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRLMTCFG_TXTWRPRE, databuf);

		FILLCMD_END(&buf);

		//-------------- end of batch update ---------------

		APP_mmi_showscreen(&buf, SCRID_SCRLMTCFG);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRLMTCFG,\
							 CTRID_SCRLMTCFG_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRLMTCFG,\
							 CTRID_SCRLMTCFG_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}


/* **************************************************
 fucntion:      APP_mmi_ctrmenuadmin
 input:
 output:
 describe:  control input process of admin menu
***************************************************/

void APP_mmi_ctrmenuadmin(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
	uint8_t *src;
//  float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRMENUADMIN_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRMENUADMIN_BTNDUSTMONINFO:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRUPLOADINGINFO;
					}
					break;

				case CTRID_SCRMENUADMIN_BTNSNRADJ:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRMENUADJ;
					}
					break;

				case CTRID_SCRMENUADMIN_BTNCOMCFG:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRCOMCFG;
					}
					break;
				case CTRID_SCRMENUADMIN_BTNLMT:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRLMTCFG;
					}

					break;
				case CTRID_SCRMENUADMIN_BTNRTC:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRRTC;
					}
					break;

				case CTRID_SCRMENUADMIN_BTNABOUT:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRABOUT;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid == CTRID_SCRMENUADMIN_TXTDEBUG)
			{       // reset all configurations to default
				src = &mmi_rcvbuf.rcvdat[8];
				if (strcmp((const char *)src, (const char *)device_info.pswd[1]) == 0 ||\
						strcmp((const char *)src, PSWD_ADMIN) == 0)
				{
					APP_devinfo_default();
					device_info.delay = 50;
					device_info.flag = CFGSTAT_SAVE;

					APP_devver_default();
					device_ver.delay = 50;
					device_ver.flag = CFGSTAT_SAVE;

					APP_dustmoninfo_default();
					elivator_info.delay = 50;
					elivator_info.flag = CFGSTAT_SAVE;

					APP_notice_default();
					elivator_info.delay = 50;
					elivator_info.flag = CFGSTAT_SAVE;

					APP_limittbl_default();
					limit_tbl.delay = 50;
					limit_tbl.flag = CFGSTAT_SAVE;

					APP_floortbl_default();
					floor_tbl.delay = 50;
					floor_tbl.flag = CFGSTAT_SAVE;

					APP_calitbl_default();
					cali_tbl.delay = 50;
					cali_tbl.flag = CFGSTAT_SAVE;
				}
			}
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}


/* **************************************************
 fucntion:      APP_mmi_scrmenuadmin
 input:
 output:
 describe:  show admin menu
***************************************************/

void APP_mmi_scrmenuadmin(void)
{
	uint8_t *buf = mmi_sendbuf.dat;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, set data
		;

		scrdat.refresh = TRUE;      // screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------

		//--------------- end of independent update ----------------

		APP_mmi_showscreen(&buf, SCRID_SCRMENUADMIN);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRMENUADMIN,\
							 CTRID_SCRMENUADMIN_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRMENUADMIN,\
							 CTRID_SCRMENUADMIN_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

/***************************************************
 fucntion:      APP_mmi_ctrmenuuser
 input:
 output:
 describe:  control input process of user menu
***************************************************/

void APP_mmi_ctrmenuuser(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
//  uint8_t *src;
//  float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRMENUUSER_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRMENUUSER_BTNDUSTMONINFO:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRUPLOADINGINFO;
					}
					break;

				case CTRID_SCRMENUUSER_BTNRTC:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRRTC;
					}
					break;

				case CTRID_SCRMENUUSER_BTNABOUT:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRABOUT;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}

/***************************************************
 fucntion:      APP_mmi_scrmenuuser
 input:
 output:
 describe:  show user menu
***************************************************/

void APP_mmi_scrmenuuser(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, set data
		;

		scrdat.refresh = TRUE;      // screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------

		//--------------- end of independent update ----------------

		APP_mmi_showscreen(&buf, SCRID_SCRMENUUSER);

		// show debug data
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef  MMI_DEBUG
		APP_mmi_showtext(&buf, SCRID_SCRMENUUSER,\
							 CTRID_SCRMENUUSER_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRMENUUSER,\
							 CTRID_SCRMENUUSER_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

/* **************************************************
 fucntion:      APP_mmi_ctroemopt
 input:
 output:
 describe:  control input process of option menu
***************************************************/
void APP_mmi_ctroemopt(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
	//uint8_t *src;
	//float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCROEMOPT_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCROEMOPT_BTNENTER:
					if (status == LCMSTAT_RELEASE)
					{
						// save data
						dustmon_info.company_no = 0;

						for (uint8_t i = 1; i <= (CTRID_SCROEMOPT_ICONCOMPANY12_EN - CTRID_SCROEMOPT_ICONCOMPANY0_EN); i++)
						{
							if (scrdat.dat_i[i]) dustmon_info.company_no = i - 1;
						}

						dustmon_info.delay = 500;
						dustmon_info.flag = CFGSTAT_SAVE;

						mmi_stat.grade--;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			;
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		status = mmi_rcvbuf.rcvdat[7];

		if (mmi_rcvbuf.ctrid >= CTRID_SCROEMOPT_ICONCOMPANY0_EN &&\
				mmi_rcvbuf.ctrid <= CTRID_SCROEMOPT_ICONCOMPANY12_EN)
		{
			if (status == LCMSTAT_RELEASE)
			{
				for (uint8_t i = 1; i <= (CTRID_SCROEMOPT_ICONCOMPANY12_EN - CTRID_SCROEMOPT_ICONCOMPANY0_EN); i++) scrdat.dat_i[i] = FALSE;
				scrdat.dat_i[mmi_rcvbuf.ctrid - CTRID_SCROEMOPT_ICONCOMPANY0_EN] = TRUE;
			}
		}

		break;

	default:
		break;
	}
}
/***************************************************
 fucntion:      APP_mmi_scroemopt
 input:
 output:
 describe:  show option menu
***************************************************/
void APP_mmi_scroemopt(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	//uint32_t length;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read elivator info data.
		for (uint8_t i = 0; i < (CTRID_SCROEMOPT_ICONCOMPMAX - CTRID_SCROEMOPT_ICONCOMPANY0_EN); i++)
		{
			scrdat.dat_i[i] = 0;
		}
		scrdat.dat_i[dustmon_info.company_no + 1] = 1;

		scrdat.refresh = TRUE;
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCROEMOPT);

		// show company_no 1~10 enable ICON
		for (uint8_t i = 0; i < (CTRID_SCROEMOPT_ICONCOMPMAX - CTRID_SCROEMOPT_ICONCOMPANY0_EN); i++)
		{
			APP_mmi_showicon(&buf, SCRID_SCROEMOPT,
							 CTRID_SCROEMOPT_ICONCOMPANY0_EN + i,
							 scrdat.dat_i[i]);
		}


		//-------------- end of batch update ---------------

		APP_mmi_showscreen(&buf, SCRID_SCROEMOPT);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCROEMOPT,\
							 CTRID_SCROEMOPT_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCROEMOPT,\
							 CTRID_SCROEMOPT_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

void APP_mmi_ctradj(void)
{
	uint8_t status;
	uint8_t subtype;
//  uint32_t index;
//  float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRMENUSNRADJ_BTNESC:
					if (status == LCMSTAT_RELEASE) mmi_stat.grade--;
					break;

				case CTRID_SCRMENUSNRADJ_BTNADJWEIGHT:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRCALIWEIGHT;
					}
					break;

				case CTRID_SCRMENUSNRADJ_BTNADJWIRE1:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRCALIWIRE1;
					}
					break;

				case CTRID_SCRMENUSNRADJ_BTNADJWIRE2:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRCALIWIRE2;
					}
					break;

				case CTRID_SCRMENUSNRADJ_BTNADJKB:
					if (status == LCMSTAT_RELEASE)
					{
						mmi_stat.grade++;
						mmi_stat.scrid[mmi_stat.grade] = SCRID_SCRCALIKB;
					}
					break;


				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			;
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}
void APP_mmi_scradj(void)
{
	uint8_t *buf = mmi_sendbuf.dat;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, set data
		;

		scrdat.refresh = TRUE;      // screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------

		//--------------- end of independent update ----------------

		APP_mmi_showscreen(&buf, SCRID_SCRMENUADJ);

		// show debug data
#ifdef  MMI_DEBUG
		length = buf - mmi_sendbuf.dat;
		length += (11 + 4);     // max length of this frame
		APP_mmi_i2a((int32_t)length, 4, databuf);
		APP_mmi_showtext(&buf, SCRID_SCRMENUADJ,\
							 CTRID_SCRMENUSNRADJ_TXTDEBUG, databuf);
#else
		APP_mmi_showtext(&buf, SCRID_SCRMENUADJ,\
							 CTRID_SCRMENUSNRADJ_TXTDEBUG, "    ");
#endif

		// set ready flag to send.
		mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
		mmi_sendbuf.valid = TRUE;

		HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
	}
}

void APP_mmi_ctrupweightadj(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWEIGHT_BTNREAD_01 &&\
						mmi_rcvbuf.ctrid <= CTRID_SCRCALIWEIGHT_BTNREAD_10 &&\
						status == LCMSTAT_RELEASE)
				{
					index = mmi_rcvbuf.ctrid - CTRID_SCRCALIWEIGHT_BTNREAD_01;
					scrdat.dat_f2[index] = sensor_dat[SENS_UPWEIGHT].result;

					CALI_ENTER();
					break;
				}

				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRCALIWEIGHT_BTNESC:
					//if (status == LCMSTAT_RELEASE)
					{
						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWEIGHT_BTNENTER:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);

						// save data
						cali_tbl.chdat[SENS_UPWEIGHT].tblsize = scrdat.dat_ui[0];

						index = 0;
						while (index < scrdat.dat_ui[0])
						{
							cali_tbl.chdat[SENS_UPWEIGHT].dat[index].y = scrdat.dat_f[index];
							cali_tbl.chdat[SENS_UPWEIGHT].dat[index].x = scrdat.dat_f2[index];
							index++;
						}
						while (index < CALI_MAXDAT)
						{
							cali_tbl.chdat[SENS_UPWEIGHT].dat[index].y = 0.0f;
							cali_tbl.chdat[SENS_UPWEIGHT].dat[index].x = 0.0f;
							index++;
						}

						cali_tbl.delay = 500;
						cali_tbl.flag = CFGSTAT_SAVE;

						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWEIGHT_BTNSORTUP:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWEIGHT_TXTWEIGHT_01 &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRCALIWEIGHT_TXTWEIGHT_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f[mmi_rcvbuf.ctrid - CTRID_SCRCALIWEIGHT_TXTWEIGHT_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWEIGHT_TXTRAW_01 &&\
						 mmi_rcvbuf.ctrid <= CTRID_SCRCALIWEIGHT_TXTRAW_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f2[mmi_rcvbuf.ctrid - CTRID_SCRCALIWEIGHT_TXTRAW_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid == CTRID_SCRCALIWEIGHT_TXTPTS)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_ui[0] = dat_f;
				}
			}

			CALI_ENTER();
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}

void APP_mmi_scrupweightadj(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;
	uint32_t index, index2, index3;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read uploading platform weight cali data

		scrdat.dat_ui[0] = cali_tbl.chdat[SENS_UPWEIGHT].tblsize;

		index = 0;
		while (index < scrdat.dat_ui[0])
		{
			scrdat.dat_f[index] = cali_tbl.chdat[SENS_UPWEIGHT].dat[index].y;
			scrdat.dat_f2[index] = cali_tbl.chdat[SENS_UPWEIGHT].dat[index].x;
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			scrdat.dat_f[index] = 0.0f;
			scrdat.dat_f2[index] = MAX_UPWT_DAT;
			index++;
		}

		//scrdat.refresh = TRUE;		// screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRCALIWEIGHT);

		index = 0;
		index2 = CTRID_SCRCALIWEIGHT_TXTWEIGHT_01;
		index3 = CTRID_SCRCALIWEIGHT_TXTRAW_01;

		// show PM25 vs sample data
		while (index < scrdat.dat_ui[0])
		{
			APP_mmi_f2a(scrdat.dat_f[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index2++, databuf);

			APP_mmi_f2a(scrdat.dat_f2[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index3++, databuf);
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			APP_mmi_showtext_batch(&buf, index2++, " ");
			APP_mmi_showtext_batch(&buf, index3++, " ");
			index++;
		}

		// show number of cali-data
		APP_mmi_f2a(scrdat.dat_ui[0], 0, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRCALIWEIGHT_TXTPTS, databuf);

		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------


		//--------------- end of independent update ----------------
	}

	// show current sample data
	APP_mmi_f2a(sensor_dat[SENS_UPWEIGHT].result, 0, databuf);
	APP_mmi_showtext(&buf, SCRID_SCRCALIWEIGHT,\
						 CTRID_SCRCALIWEIGHT_TXTRAW, databuf);

	APP_mmi_showscreen(&buf, SCRID_SCRCALIWEIGHT);

	// show debug data
	length = buf - mmi_sendbuf.dat;
	length += (11 + 4);     // max length of this frame
	APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef	MMI_DEBUG
	APP_mmi_showtext(&buf, SCRID_SCRCALIWEIGHT,\
						 CTRID_SCRCALIWEIGHT_TXTDEBUG, databuf);
#else
	APP_mmi_showtext(&buf, SCRID_SCRCALIWEIGHT,\
						 CTRID_SCRCALIWEIGHT_TXTDEBUG, "    ");
#endif

	// set ready flag to send.
	mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
}

void APP_mmi_ctrupwire1adj(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE1_BTNREAD_01 &&\
						mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE1_BTNREAD_10 &&\
						status == LCMSTAT_RELEASE)
				{
					index = mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE1_BTNREAD_01;
					scrdat.dat_f2[index] = sensor_dat[SENS_CABL].result;

					CALI_ENTER();
					break;
				}

				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRCALIWIRE1_BTNESC:
					//if (status == LCMSTAT_RELEASE)
					{
						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWIRE1_BTNENTER:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);

						// save data
						cali_tbl.chdat[SENS_CABL].tblsize = scrdat.dat_ui[0];

						index = 0;
						while (index < scrdat.dat_ui[0])
						{
							cali_tbl.chdat[SENS_CABL].dat[index].y = scrdat.dat_f[index];
							cali_tbl.chdat[SENS_CABL].dat[index].x = scrdat.dat_f2[index];
							index++;
						}
						while (index < CALI_MAXDAT)
						{
							cali_tbl.chdat[SENS_CABL].dat[index].y = 0.0f;
							cali_tbl.chdat[SENS_CABL].dat[index].x = 0.0f;
							index++;
						}

						cali_tbl.delay = 500;
						cali_tbl.flag = CFGSTAT_SAVE;

						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWIRE1_BTNSORTUP:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE1_TXTWIRE1_01 &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE1_TXTWIRE1_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f[mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE1_TXTWIRE1_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE1_TXTRAW_01 &&\
						 mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE1_TXTRAW_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f2[mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE1_TXTRAW_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid == CTRID_SCRCALIWIRE1_TXTPTS)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_ui[0] = dat_f;
				}
			}

			CALI_ENTER();
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}
void APP_mmi_scrupwire1adj(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;
	uint32_t index, index2, index3;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read WIRE1 cali data

		scrdat.dat_ui[0] = cali_tbl.chdat[SENS_CABL].tblsize;

		index = 0;
		while (index < scrdat.dat_ui[0])
		{
			scrdat.dat_f[index] = cali_tbl.chdat[SENS_CABL].dat[index].y;
			scrdat.dat_f2[index] = cali_tbl.chdat[SENS_CABL].dat[index].x;
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			scrdat.dat_f[index] = 0.0f;
			scrdat.dat_f2[index] = MAX_UPWIRE_DAT;
			index++;
		}

		//scrdat.refresh = TRUE;		// screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRCALIWIRE1);

		index = 0;
		index2 = CTRID_SCRCALIWIRE1_TXTWIRE1_01;
		index3 = CTRID_SCRCALIWIRE1_TXTRAW_01;

		// show WIRE1 vs sample data
		while (index < scrdat.dat_ui[0])
		{
			APP_mmi_f2a(scrdat.dat_f[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index2++, databuf);

			APP_mmi_f2a(scrdat.dat_f2[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index3++, databuf);
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			APP_mmi_showtext_batch(&buf, index2++, " ");
			APP_mmi_showtext_batch(&buf, index3++, " ");
			index++;
		}

		// show number of cali-data
		APP_mmi_f2a(scrdat.dat_ui[0], 0, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRCALIWIRE1_TXTPTS, databuf);

		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------


		//--------------- end of independent update ----------------
	}

	// show current sample data
	APP_mmi_f2a(sensor_dat[SENS_CABL].result, 0, databuf);
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE1,\
						 CTRID_SCRCALIWIRE1_TXTRAW, databuf);

	APP_mmi_showscreen(&buf, SCRID_SCRCALIWIRE1);

	// show debug data
	length = buf - mmi_sendbuf.dat;
	length += (11 + 4);     // max length of this frame
	APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef	MMI_DEBUG
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE1,\
						 CTRID_SCRCALIWIRE1_TXTDEBUG, databuf);
#else
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE1,\
						 CTRID_SCRCALIWIRE1_TXTDEBUG, "    ");
#endif

	// set ready flag to send.
	mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
}

void APP_mmi_ctrupwire2adj(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE2_BTNREAD_01 &&\
						mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE2_BTNREAD_10 &&\
						status == LCMSTAT_RELEASE)
				{
					index = mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE2_BTNREAD_01;
					scrdat.dat_f2[index] = sensor_dat[SENS_CABR].result;

					CALI_ENTER();
					break;
				}

				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRCALIWIRE2_BTNESC:
					//if (status == LCMSTAT_RELEASE)
					{
						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWIRE2_BTNENTER:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);

						// save data
						cali_tbl.chdat[SENS_CABR].tblsize = scrdat.dat_ui[0];

						index = 0;
						while (index < scrdat.dat_ui[0])
						{
							cali_tbl.chdat[SENS_CABR].dat[index].y = scrdat.dat_f[index];
							cali_tbl.chdat[SENS_CABR].dat[index].x = scrdat.dat_f2[index];
							index++;
						}
						while (index < CALI_MAXDAT)
						{
							cali_tbl.chdat[SENS_CABR].dat[index].y = 0.0f;
							cali_tbl.chdat[SENS_CABR].dat[index].x = 0.0f;
							index++;
						}

						cali_tbl.delay = 500;
						cali_tbl.flag = CFGSTAT_SAVE;

						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIWIRE2_BTNSORTUP:
					//if (status == LCMSTAT_RELEASE)
					{
						// sort-up table
						APP_mmi_sortupbyx(scrdat.dat_f2, scrdat.dat_f, scrdat.dat_ui[0]);
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE2_TXTWIRE2_01 &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE2_TXTWIRE2_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f[mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE2_TXTWIRE2_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIWIRE2_TXTRAW_01 &&\
						 mmi_rcvbuf.ctrid <= CTRID_SCRCALIWIRE2_TXTRAW_10)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f2[mmi_rcvbuf.ctrid - CTRID_SCRCALIWIRE2_TXTRAW_01] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid == CTRID_SCRCALIWIRE2_TXTPTS)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_ui[0] = dat_f;
				}
			}

			CALI_ENTER();
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		break;

	default:
		break;
	}
}
void APP_mmi_scrupwire2adj(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;
	uint32_t index, index2, index3;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read WIRE2 cali data

		scrdat.dat_ui[0] = cali_tbl.chdat[SENS_CABR].tblsize;

		index = 0;
		while (index < scrdat.dat_ui[0])
		{
			scrdat.dat_f[index] = cali_tbl.chdat[SENS_CABR].dat[index].y;
			scrdat.dat_f2[index] = cali_tbl.chdat[SENS_CABR].dat[index].x;
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			scrdat.dat_f[index] = 0.0f;
			scrdat.dat_f2[index] = MAX_PM_DAT;
			index++;
		}

		//scrdat.refresh = TRUE;		// screen update enable
	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRCALIWIRE2);

		index = 0;
		index2 = CTRID_SCRCALIWIRE2_TXTWIRE2_01;
		index3 = CTRID_SCRCALIWIRE2_TXTRAW_01;

		// show WIRE2 vs sample data
		while (index < scrdat.dat_ui[0])
		{
			APP_mmi_f2a(scrdat.dat_f[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index2++, databuf);

			APP_mmi_f2a(scrdat.dat_f2[index], 0, databuf);
			APP_mmi_showtext_batch(&buf, index3++, databuf);
			index++;
		}
		while (index < CALI_MAXDAT)
		{
			APP_mmi_showtext_batch(&buf, index2++, " ");
			APP_mmi_showtext_batch(&buf, index3++, " ");
			index++;
		}

		// show number of cali-data
		APP_mmi_f2a(scrdat.dat_ui[0], 0, databuf);
		APP_mmi_showtext_batch(&buf, CTRID_SCRCALIWIRE2_TXTPTS, databuf);

		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------

		//-------------- start of independent update ---------------


		//--------------- end of independent update ----------------
	}

	// show current sample data
	APP_mmi_f2a(sensor_dat[SENS_CABR].result, 0, databuf);
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE2,\
						 CTRID_SCRCALIWIRE2_TXTRAW, databuf);

	APP_mmi_showscreen(&buf, SCRID_SCRCALIWIRE2);

	// show debug data
	length = buf - mmi_sendbuf.dat;
	length += (11 + 4);     // max length of this frame
	APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef	MMI_DEBUG
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE2,\
						 CTRID_SCRCALIWIRE2_TXTDEBUG, databuf);
#else
	APP_mmi_showtext(&buf, SCRID_SCRCALIWIRE2,\
						 CTRID_SCRCALIWIRE2_TXTDEBUG, "    ");
#endif

	// set ready flag to send.
	mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
}

void APP_mmi_ctrupcalikbadj(void)
{
	uint8_t status;
	uint8_t subtype;
	uint32_t index;
	uint8_t *src;
	float dat_f;

	switch (mmi_rcvbuf.msg)      // check message type
	{
	case LCMMSG_DATGET:     // control updata
		mmi_rcvbuf.ctrid = mmi_rcvbuf.rcvdat[5] << 8;   // get ctrid
		mmi_rcvbuf.ctrid |= mmi_rcvbuf.rcvdat[6];

		mmi_rcvbuf.ctrtype = mmi_rcvbuf.rcvdat[7];

		switch (mmi_rcvbuf.ctrtype)
		{
		case LCMCTR_BTN:    // button
			subtype = mmi_rcvbuf.rcvdat[8];
			status = mmi_rcvbuf.rcvdat[9];

			if (subtype < 2)
			{               // check crtid & status(up/down)
				if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIKB_BTNREAD_01 &&\
						mmi_rcvbuf.ctrid <= CTRID_SCRCALIKB_BTNREAD_10 &&\
						status == LCMSTAT_RELEASE)
				{
					index = mmi_rcvbuf.ctrid - CTRID_SCRCALIKB_BTNREAD_01;

					for (uint8_t i = 0; i < UPSNR_NUM; i++)
					{
						scrdat.dat_kb[index][i] = sensor_dat[SENS_UPSNR1 + i].result;
					}

					CALI_ENTER();
					break;
				}

				switch (mmi_rcvbuf.ctrid)
				{
				case CTRID_SCRCALIKB_BTNESC:
					//if (status == LCMSTAT_RELEASE)
					{
						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				case CTRID_SCRCALIKB_BTNENTER:
					//if (status == LCMSTAT_RELEASE)
					{
						// save data
						for (uint8_t i = 0; i < CODE_ROW; i++)
						{
							for (uint8_t j = 0; j < CODE_COL; j++)
							{
								cali_tbl.upsnr_code[i][j] = scrdat.dat_kb[i][j];
							}
						}
						cali_tbl.upsnr_cornweight = scrdat.dat_f[0];
						cali_tbl.upsnr_dispweight = scrdat.dat_f[1];

						cali_tbl.algorithm_enable = (BOOL)scrdat.dat_i[0];

						get_correctionfactor(cali_tbl.alpha);

						cali_tbl.delay = 500;
						cali_tbl.flag = CFGSTAT_SAVE;

						CALI_RESTORE(work_mode_shadow);
						mmi_stat.grade--;
					}
					break;

				default:
					break;
				}

			}
			else
			{               // check ctrid & status(keycode)
				;
			}
			break;

		case LCMCTR_TXT:    // data input process
			if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIKB_EMPTY_S1 &&\
					mmi_rcvbuf.ctrid <= CTRID_SCRCALIKB_DISPERSED_S7)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_kb[mmi_rcvbuf.ctrid / 8][mmi_rcvbuf.ctrid % 8] = dat_f;
				}
			}
			else if (mmi_rcvbuf.ctrid >= CTRID_SCRCALIKB_WOC_S &&\
						 mmi_rcvbuf.ctrid <= CTRID_SCRCALIKB_WOC_D)
			{
				src = &mmi_rcvbuf.rcvdat[8];
				if (APP_mmi_a2f(src, &dat_f))
				{
					scrdat.dat_f[mmi_rcvbuf.ctrid - CTRID_SCRCALIKB_WOC_S] = dat_f;
				}
			}

			CALI_ENTER();
			break;

		case LCMCTR_PRGS:
			;
			break;

		case LCMCTR_SLD:
			;
			break;

		case LCMCTR_MTR:
			;
			break;

		default:
			break;
		}
		break;

	case LCMMSG_ICONGET:        // icon/animation update
		status = mmi_rcvbuf.rcvdat[7];
		switch (mmi_rcvbuf.ctrid)
		{
		case CTRID_SCRCALIKB_ICONV1EN:
			if (status == LCMSTAT_RELEASE)
			{
				scrdat.dat_i[0] = (scrdat.dat_i[0]) ? FALSE : TRUE;
			}
			break;

		default:
			break;
		}
	}
}

void APP_mmi_scrupcalikbadj(void)
{
	uint8_t *buf = mmi_sendbuf.dat;
	uint32_t length;
	uint32_t index, index2;

	if (mmi_stat.scrid[mmi_stat.grade] != mmi_stat.last_scrid)
	{
		// first incoming, read uploading platform weight cali data
		scrdat.dat_ui[0] = CODE_ROW * CODE_COL;

		for (uint8_t i = 0; i < CODE_ROW; i++)
		{
			for (uint8_t j = 0; j < CODE_COL; j++)
			{
				scrdat.dat_kb[i][j] = cali_tbl.upsnr_code[i][j];
			}
		}

		scrdat.dat_f[0] = cali_tbl.upsnr_cornweight;
		scrdat.dat_f[1] = cali_tbl.upsnr_dispweight;

		scrdat.dat_i[0] = cali_tbl.algorithm_enable;

	}

	if (scrdat.refresh)
	{
		scrdat.refresh = FALSE;

		//-------------- start of batch update ---------------

		FILLCMD_START(&buf);
		FILLCMD_1B(&buf, LCMCMD_SETCTR);
		FILLCMD_1B(&buf, LCMMSG_SETBATCH);
		FILLSCRID(&buf, SCRID_SCRCALIKB);

		index = 0;
		index2 = CTRID_SCRCALIKB_WOC_S;

		// show uploading platform weight sensor code data

		for (uint8_t i = 0; i < CODE_ROW; i++)
		{
			for (uint8_t j = 0; j < CODE_COL; j++)
			{
				APP_mmi_f2a(scrdat.dat_kb[i][j], 0, databuf);
				APP_mmi_showtext_batch(&buf, index++, databuf);
			}
		}

		// corner weight & Dispersed weight
		for (uint8_t i = 0; i < 2; i++)
		{
			APP_mmi_f2a(scrdat.dat_f[i], 0, databuf);
			APP_mmi_showtext_batch(&buf, index2++, databuf);
		}


		FILLCMD_END(&buf);
		//-------------- end of batch update ---------------
		// show threshold 1 enable ICON
		APP_mmi_showicon(&buf, SCRID_SCRCALIKB,
						 CTRID_SCRCALIKB_ICONV1EN,
						 scrdat.dat_i[0]);

		//-------------- start of independent update ---------------


		//--------------- end of independent update ----------------
	}

	APP_mmi_showscreen(&buf, SCRID_SCRCALIKB);

	// show debug data
	length = buf - mmi_sendbuf.dat;
	length += (11 + 4);     // max length of this frame
	APP_mmi_i2a((int32_t)length, 4, databuf);
#ifdef	MMI_DEBUG
	APP_mmi_showtext(&buf, SCRID_SCRCALIKB,\
						 CTRID_SCRCALIKB_TXTDEBUG, databuf);
#else
	APP_mmi_showtext(&buf, SCRID_SCRCALIKB,\
						 CTRID_SCRCALIKB_TXTDEBUG, "    ");
#endif

	// set ready flag to send.
	mmi_sendbuf.length = (buf - mmi_sendbuf.dat);
	mmi_sendbuf.valid = TRUE;

	HAL_UART_Transmit_IT(HDL_UART_TFT, mmi_sendbuf.dat, mmi_sendbuf.length);
}

/* **************************************************
 fucntion:      APP_mmi_screenupdate
 input:     pbuf: where data to put
			enable: true: start refresh
					false: stop refresh
 describe:  set screen updata enable/disable
***************************************************/

void APP_mmi_screenupdate(uint8_t **pbuf, BOOL enable)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_REFRESH);
	FILLCMD_1B(pbuf, enable);
	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showscreen
 input:
 output:
 describe:  show specified screen
***************************************************/

void APP_mmi_showscreen(uint8_t **pbuf, uint32_t scrid)
{
//  uint32_t length = 9;

	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_SCRSET);
	FILLSCRID(pbuf, scrid);
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = length;
//  mmi_sendbuf.valid = TRUE;
}


/* **************************************************
 fucntion:      APP_mmi_showtext
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
			string: what to show
 output:
 describe:  show text
***************************************************/

void APP_mmi_showtext(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint8_t *string)
{
//  uint8_t* buf;
//  uint32_t length = 11;

//  buf = mmi_sendbuf.dat;
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_DATSET);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	while (*string)
	{
		*((*pbuf)++) = *string++;
//      length++;
	}
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = length;
//  mmi_sendbuf.valid = TRUE;
}


/* **************************************************
 fucntion:      APP_mmi_showtext_batch
 input:     pbuf: where data to put
			crtid: control ID
			string: what to show
 output:
 describe:  show text in batch mode
***************************************************/

void APP_mmi_showtext_batch(uint8_t **pbuf, uint32_t ctrid, uint8_t *string)
{
	uint32_t length = 0;
	uint8_t *tmp = string;

	FILLCTRID(pbuf, ctrid);

	while (*tmp++) length++;
	FILLCMD_2B(pbuf, length);

	while (*string)
	{
		*((*pbuf)++) = *string++;
	}
}


/* **************************************************
 fucntion:      APP_mmi_gettext
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
			string: what to show
 output:
 describe:  show text
***************************************************/

void APP_mmi_gettext(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_DATGET);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	FILLCMD_END(pbuf);

}


/* **************************************************
 fucntion:      APP_mmi_showicon
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
			string: index of icon
 output:
 describe:  show icon
***************************************************/

void APP_mmi_showicon(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t index)
{
//  uint8_t* buf;

//  buf = mmi_sendbuf.dat;
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_ICONSET);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	(*(*pbuf)++) = index;
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = 12;
//  mmi_sendbuf.valid = TRUE;
}


/* **************************************************
 fucntion:      APP_mmi_startgif
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
 output:
 describe:  start gif
***************************************************/

void APP_mmi_startgif(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid)
{
//  uint8_t* buf;

//  buf = mmi_sendbuf.dat;
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_GIFRUN);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = 12;
//  mmi_sendbuf.valid = TRUE;
}


/* **************************************************
 fucntion:      APP_mmi_stopgif
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
 output:
 describe:  start gif
***************************************************/

void APP_mmi_stopgif(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid)
{
//  uint8_t* buf;

//  buf = mmi_sendbuf.dat;
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_GIFSTOP);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = 12;
//  mmi_sendbuf.valid = TRUE;
}

/* **************************************************
 fucntion:      APP_mmi_showmeter
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
			string: value of meter
 output:
 describe:  show meter
***************************************************/

void APP_mmi_showmeter(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t dat)
{
//  uint8_t* buf;
//  uint32_t length = 15;

//  buf = mmi_sendbuf.dat;
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_DATSET);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	*((*pbuf)++) = (dat >> 24) & 0xFF;
	*((*pbuf)++) = (dat >> 16) & 0xFF;
	*((*pbuf)++) = (dat >> 8) & 0xFF;
	*((*pbuf)++) = dat & 0xFF;
	FILLCMD_END(pbuf);

//  mmi_sendbuf.length = length;
//  mmi_sendbuf.valid = TRUE;
}


/* **************************************************
 fucntion:      APP_mmi_showmeter_batch
 input:     pbuf: where data to put
			crtid: control ID
			string: value of meter
 output:
 describe:  show meter in batch mode
***************************************************/

void APP_mmi_showmeter_batch(uint8_t **pbuf, uint32_t ctrid, uint32_t dat)
{
	FILLCTRID(pbuf, ctrid);
	FILLCMD_2B(pbuf, 4);
	*((*pbuf)++) = (dat >> 24) & 0xFF;
	*((*pbuf)++) = (dat >> 16) & 0xFF;
	*((*pbuf)++) = (dat >> 8) & 0xFF;
	*((*pbuf)++) = dat & 0xFF;
}


/* **************************************************
 fucntion:      APP_mmi_showprogress
 input:     pbuf: where data to put
			scrid: screen ID
			crtid: control ID
			string: value of progress
 output:
 describe:  show progress
***************************************************/

void APP_mmi_showprogress(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint32_t dat)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_DATSET);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);
	*((*pbuf)++) = (dat >> 24) & 0xFF;
	*((*pbuf)++) = (dat >> 16) & 0xFF;
	*((*pbuf)++) = (dat >> 8) & 0xFF;
	*((*pbuf)++) = dat & 0xFF;
	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showprogress_batch
 input:     pbuf: where data to put
			crtid: control ID
			string: value of progress
 output:
 describe:  show progress in batch mode
***************************************************/

void APP_mmi_showprogress_batch(uint8_t **pbuf, uint32_t ctrid, uint32_t dat)
{
	FILLCTRID(pbuf, ctrid);
	FILLCMD_2B(pbuf, 4);
	*((*pbuf)++) = (dat >> 24) & 0xFF;
	*((*pbuf)++) = (dat >> 16) & 0xFF;
	*((*pbuf)++) = (dat >> 8) & 0xFF;
	*((*pbuf)++) = dat & 0xFF;
}


/* **************************************************
 fucntion:      APP_mmi_showrtc
 input:     pbuf: where data to put
			instant: flag to instant update
 output:
 describe:  show rtc time periodcally or on request
***************************************************/

void APP_mmi_showrtc(uint8_t **pbuf, BOOL instant)
{
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;

	uint8_t temp;
	static BOOL update;
	static uint8_t last_second;

	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

	// period setup
	if (!update && sTime.Seconds != last_second)
	{
		if (sTime.Seconds & 0x01)   // update on odd seconds
		{
			update = TRUE;
			last_second = sTime.Seconds;
		}
	}
	if (update || instant)
	{
		update = FALSE;

		FILLCMD_START(pbuf);
		FILLCMD_1B(pbuf, LCMCMD_SETRTC);

		*(*pbuf)++ = sTime.Seconds;
		*(*pbuf)++ = sTime.Minutes;
		*(*pbuf)++ = sTime.Hours;
		*(*pbuf)++ = sDate.Date;

		temp = sDate.WeekDay;
		if (temp > 6) temp = 0;     // sunday=0 in tft
		*(*pbuf)++ = temp;

		*(*pbuf)++ = sDate.Month;
		*(*pbuf)++ = sDate.Year;

		FILLCMD_END(pbuf);
	}

}



/* **************************************************
 fucntion:      APP_mmi_clearlayer
 input:     pbuf: where data to put
			layer: which layer to clear
 output:
 describe:  clear specified layer
***************************************************/

void APP_mmi_clearlayer(uint8_t **pbuf, uint16_t layer)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_CLEARLAYER);

	*(*pbuf)++ = layer;

	FILLCMD_END(pbuf);
}

/* **************************************************
 fucntion:      APP_mmi_ctrvisiable
 input:     pbuf: where data to put
			enable: true: control visible
					false: control invisible
 describe:  set control visible/invisible
***************************************************/

void APP_mmi_ctrvisiable(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, BOOL enable)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_CTRSHOW);
	FILLCMD_2B(pbuf, scrid);
	FILLCMD_2B(pbuf, ctrid);
	FILLCMD_1B(pbuf, enable);
	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_setforground_ctr
 input:     pbuf: where data to put
			color: forground color in RGB565
 output:
 describe:  set forground color for control
***************************************************/

void APP_mmi_setforground_ctr(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint16_t RGB565)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_SETFORGROND);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);

	*(*pbuf)++ = RGB565 >> 8;
	*(*pbuf)++ = RGB565 & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_setbackground_ctr
 input:     pbuf: where data to put
			color: background color in RGB565
 output:
 describe:  set background color for control
***************************************************/

void APP_mmi_setbackground_ctr(uint8_t **pbuf, uint32_t scrid, uint32_t ctrid, uint16_t RGB565)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETCTR);
	FILLCMD_1B(pbuf, LCMMSG_SETBACKGROND);
	FILLSCRID(pbuf, scrid);
	FILLCTRID(pbuf, ctrid);

	*(*pbuf)++ = RGB565 >> 8;
	*(*pbuf)++ = RGB565 & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showimgfull
 input:     pbuf: where data to put
			img_attrib: img attribute
 output:
 describe:  show image in full-screen
***************************************************/

void APP_mmi_showimgfull(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SHOWIMGFULL);

	*(*pbuf)++ = img_attrib->image_id >> 8;
	*(*pbuf)++ = img_attrib->image_id & 0xFF;

	*(*pbuf)++ = img_attrib->mask;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showimgpart
 input:     pbuf: where data to put
			img_attrib: img attribute
 output:
 describe:  show image on part of screen
***************************************************/

void APP_mmi_showimgpart(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SHOWIMGPART);

	*(*pbuf)++ = img_attrib->scr_x >> 8;
	*(*pbuf)++ = img_attrib->scr_x & 0xFF;

	*(*pbuf)++ = img_attrib->scr_y >> 8;
	*(*pbuf)++ = img_attrib->scr_y & 0xFF;

	*(*pbuf)++ = img_attrib->image_id >> 8;
	*(*pbuf)++ = img_attrib->image_id & 0xFF;

	*(*pbuf)++ = img_attrib->mask;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showimgcut
 input:     pbuf: where data to put
			img_attrib: strcture of attribute
 output:
 describe:  show image cut from specified image
***************************************************/

void APP_mmi_showimgcut(uint8_t **pbuf, APP_imgattrib_TpyeDef *img_attrib)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SHOWIMGCUT);

	*(*pbuf)++ = img_attrib->scr_x >> 8;
	*(*pbuf)++ = img_attrib->scr_x & 0xFF;

	*(*pbuf)++ = img_attrib->scr_y >> 8;
	*(*pbuf)++ = img_attrib->scr_y & 0xFF;

	*(*pbuf)++ = img_attrib->image_id >> 8;
	*(*pbuf)++ = img_attrib->image_id & 0xFF;

	*(*pbuf)++ = img_attrib->image_x >> 8;
	*(*pbuf)++ = img_attrib->image_x & 0xFF;

	*(*pbuf)++ = img_attrib->image_y >> 8;
	*(*pbuf)++ = img_attrib->image_y & 0xFF;

	*(*pbuf)++ = img_attrib->image_w >> 8;
	*(*pbuf)++ = img_attrib->image_w & 0xFF;

	*(*pbuf)++ = img_attrib->image_h >> 8;
	*(*pbuf)++ = img_attrib->image_h & 0xFF;

	*(*pbuf)++ = img_attrib->mask;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_setforground
 input:     pbuf: where data to put
			color: forground color in RGB565
 output:
 describe:  set forground color, globle
***************************************************/

void APP_mmi_setforground(uint8_t **pbuf, uint16_t RGB565)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETFORGROND);

	*(*pbuf)++ = RGB565 >> 8;
	*(*pbuf)++ = RGB565 & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_setbackground
 input:     pbuf: where data to put
			color: background color in RGB565
 output:
 describe:  set background color, globle
***************************************************/

void APP_mmi_setbackground(uint8_t **pbuf, uint16_t RGB565)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_SETBACKGROND);

	*(*pbuf)++ = RGB565 >> 8;
	*(*pbuf)++ = RGB565 & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showline
 input:     pbuf: where data to put
			dot_attrib : attribute of dot start/end
 output:
 describe:  show circle
***************************************************/

void APP_mmi_showline(uint8_t **pbuf, APP_dotattrib_TpyeDef *dot_attrib)
{
	FILLCMD_START(pbuf);
	FILLCMD_1B(pbuf, LCMCMD_DRAWLINE);

	*(*pbuf)++ = dot_attrib->dot[0].x >> 8;
	*(*pbuf)++ = dot_attrib->dot[0].x & 0xFF;
	*(*pbuf)++ = dot_attrib->dot[0].y >> 8;
	*(*pbuf)++ = dot_attrib->dot[0].y & 0xFF;

	*(*pbuf)++ = dot_attrib->dot[1].x >> 8;
	*(*pbuf)++ = dot_attrib->dot[1].x & 0xFF;
	*(*pbuf)++ = dot_attrib->dot[1].y >> 8;
	*(*pbuf)++ = dot_attrib->dot[1].y & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showcircle
 input:     pbuf: where data to put
			x,y : position of center
			r : radius of circle
			fill : fill flag
 output:
 describe:  show circle
***************************************************/

void APP_mmi_showcircle(uint8_t **pbuf, uint16_t x, uint16_t y, uint16_t r, BOOL fill)
{
	FILLCMD_START(pbuf);
	if (fill)
	{
		FILLCMD_1B(pbuf, LCMCMD_FILLCIRCLE);

	}
	else
	{
		FILLCMD_1B(pbuf, LCMCMD_DRAWCIRCLE);
	}

	*(*pbuf)++ = x >> 8;
	*(*pbuf)++ = x & 0xFF;
	*(*pbuf)++ = y >> 8;
	*(*pbuf)++ = y & 0xFF;
	*(*pbuf)++ = r >> 8;
	*(*pbuf)++ = r & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showretangle
 input:     pbuf: where data to put
			x0,y0 : position of left-top corner
			x1,y1 : position of right-down corner
			fill : fill flag
 output:
 describe:  show retangle
***************************************************/

void APP_mmi_showretangle(uint8_t **pbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, BOOL fill)
{
	FILLCMD_START(pbuf);
	if (fill)
	{
		FILLCMD_1B(pbuf, LCMCMD_FILLRETANGLE);
	}
	else
	{
		FILLCMD_1B(pbuf, LCMCMD_DRAWRETANGLE);
	}

	*(*pbuf)++ = x0 >> 8;
	*(*pbuf)++ = x0 & 0xFF;
	*(*pbuf)++ = y0 >> 8;
	*(*pbuf)++ = y0 & 0xFF;
	*(*pbuf)++ = x1 >> 8;
	*(*pbuf)++ = x1 & 0xFF;
	*(*pbuf)++ = y1 >> 8;
	*(*pbuf)++ = y1 & 0xFF;

	FILLCMD_END(pbuf);
}


/* **************************************************
 fucntion:      APP_mmi_showellipse
 input:     pbuf: where data to put
			x0,y0 : position of left-top corner
			x1,y1 : position of right-down corner
			fill : fill flag
 output:
 describe:  show ellipse
***************************************************/

void APP_mmi_showellipse(uint8_t **pbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, BOOL fill)
{
	FILLCMD_START(pbuf);
	if (fill)
	{
		FILLCMD_1B(pbuf, LCMCMD_FILLELLIPSE);}
	else
	{
		FILLCMD_1B(pbuf, LCMCMD_DRAWELLIPSE);
	}

	*(*pbuf)++ = x0 >> 8;
	*(*pbuf)++ = x0 & 0xFF;
	*(*pbuf)++ = y0 >> 8;
	*(*pbuf)++ = y0 & 0xFF;
	*(*pbuf)++ = x1 >> 8;
	*(*pbuf)++ = x1 & 0xFF;
	*(*pbuf)++ = y1 >> 8;
	*(*pbuf)++ = y1 & 0xFF;

	FILLCMD_END(pbuf);
}





/* **************************************************
 fucntion:      APP_mmi_f2a
 input:
 output:
 describe:  translate float to ascii
***************************************************/

void APP_mmi_f2a(float dat, uint32_t mantissa, uint8_t *buf)
{
	uint8_t asc[10];
	uint32_t index = mantissa;
	uint32_t temp;

	while (index--) dat *= 10.0f;
	if (dat < 0.0f)
	{
		*buf++ = '-';
		dat = -dat;
	}
	temp = (dat > 999999) ? 999999 : dat;

	asc[0] =  (temp / 100000);
	asc[1] =  (temp / 10000 % 10);
	asc[2] =  (temp / 1000 % 10);
	asc[3] =  (temp / 100 % 10);
	asc[4] =  (temp / 10 % 10);
	asc[5] =  (temp % 10);

	index = 0;
	while (asc[index] == 0 && index < (5 - mantissa)) index++;
	for (; index < 6; index++)
	{
		*buf++ = asc[index] + 0x30;
		if ((mantissa) && (index == (5 - mantissa))) *buf++ = '.';
	}
	*buf = 0;
}


/* **************************************************
 fucntion:      APP_mmi_a2f
 input:     buf: where asc-string to get
 output:    output: float
			return: status
 describe:  translate ascii-string to float
***************************************************/

BOOL APP_mmi_a2f(uint8_t *buf, float *output)
{
#define A2F_INPUTERR
	float   head, tail;
	float scale;
	float dat;
	BOOL neg = FALSE;

	if (*buf == NULL)
	{
		*output = 0.0f;
		return (TRUE);
	}

	if (*buf == '-')    // neg
	{
		neg = TRUE;
		buf++;
	}
	else
	{
		neg = FALSE;
	}

	head = 0.0f;

	while ((*buf != '.') && (*buf != '\0'))
	{
		if (*buf < '0' || *buf > '9')
		{
			//* output = 0.0f;
			return (FALSE);
		}


		head = head * 10.0f + (float)(*buf - 0x30);
		buf++;
	}
	if ((*buf == '\0'))
	{
		*output = head;
		return (TRUE);
	}
	else        // if (*buf == '.')
	{
		buf++;
	}

	tail = 0.0f;
	scale = 10.0f;

	while ((*buf != '\0'))
	{
		if (*buf < '0' || *buf > '9')
		{
			//* output = 0.0f;
			return (FALSE);
		}

		tail +=  (float)(*buf - 0x30) / scale;
		buf++;
		scale *= 10.0f;
	}

	dat = head + tail;
	if (neg) dat = -dat;

	*output = dat;
	return (TRUE);
}


/* **************************************************
 fucntion:      APP_mmi_i2a
 input:     dat: singed int
			digit: digits to output
			buf: where data to put
 output:
 describe:  translate int to ascii
***************************************************/

void APP_mmi_i2a(int32_t dat, uint32_t digit, uint8_t *buf)
{
	uint8_t asc[10];
	uint32_t index;
//  int32_t temp = 1;

	if (dat < 0)
	{
		*buf++ = '-';
		dat = -dat;
	}
	dat = (dat > 999999) ? 999999 : dat;

	asc[0] =  (dat / 100000);
	asc[1] =  (dat / 10000 % 10);
	asc[2] =  (dat / 1000 % 10);
	asc[3] =  (dat / 100 % 10);
	asc[4] =  (dat / 10 % 10);
	asc[5] =  (dat % 10);


	if (digit <= 6)
	{
		index = 6 - digit;
//      while (asc[index] == 0)
//      {
//          *buf++ = ' ';
//          index++;
//      }
		for (; index < 6; index++)
		{
			*buf++ = asc[index] + 0x30;
		}
		*buf = 0;
	}
	else
	{
		index = 0;
		while (asc[index] == 0) index++;
		for (; index < 6; index++)
		{
			*buf++ = asc[index] + 0x30;
		}
	}
	*buf = 0;
}


/* **************************************************
 fucntion:      APP_mmi_sortupbyx
 input:     x,y: pointer of input array
			size: size of array
 output:
 describe:  sort-up array by x
***************************************************/

void APP_mmi_sortupbyx(float *x, float *y, uint32_t size)
{
	float tempx, tempy;
	uint32_t i, j;

	for (i = 0; i < size - 1; i++)
	{
		for (j = i + 1; j < size; j++)
		{
			if (x[i] > x[j])
			{
				tempx = x[i];
				tempy = y[i];
				x[i] = x[j];
				y[i] = y[j];
				x[j] = tempx;
				y[j] = tempy;

			}
		}
	}
}


#undef _STATIC


#undef  _LOCAL_MMI

/* ****************************  END OF FILE  *********************************/


