
#define	_LOCAL_AUDIO

#include "stm32f4xx_hal.h"
#include "arm_math.h"

#include "app_user.h"
#include "app_audio.h"


//#define IIS_SPK_DMA		// DMA mode when enable, IT mode otherwise

/* **************************************************
 fucntion:		APP_audio_ini
 input:
 output:
 describe:	audio initialize
***************************************************/
void APP_audio_ini(void)
{
	_AMP_ON();
	//APP_audio_list_ini();
	audio_play.flag = AUDIOSTAT_IDLE;
	audio_play.sn = 0;
	audio_play.size = 0;
	audio_play.idx = 0;
}


/* **************************************************
 fucntion:		APP_audio_list_ini
 input:
 output:
 describe:	initialize playlist
***************************************************/
void APP_audio_list_ini(void)
{
	audio_play.flag = AUDIOSTAT_IDLE;
	audio_play.sn ++;
	audio_play.size = 0;
	audio_play.idx = 0;
}

 
/* **************************************************
 fucntion:		APP_audio_list_add
 input:		*wav: pointer of wave to play
 output:	samples: samples of wave
 describe:	add wave into playlist
***************************************************/
void APP_audio_list_add(uint16_t* wav, uint32_t samples)
{
	audio_play.list[audio_play.size].wave = wav;
	audio_play.list[audio_play.size].samples = samples;
	audio_play.size++;
}


/* **************************************************
 fucntion:		APP_audio_play
 input:
 output:
 describe:	start playing whole playlist
***************************************************/
void APP_audio_play(void)
{
	_AMP_ON();
	
	audio_play.flag = AUDIOSTAT_PLAYING;
	//audio_play.sn2 = audio_play.sn;
	audio_play.idx = 0;
	if (audio_play.size == 0) return;

#ifdef IIS_SPK_DMA
	HAL_I2S_Transmit_DMA(HDL_IIS_SPK, \
						audio_play.list[audio_play.idx].wave, \
						audio_play.list[audio_play.idx].samples);
#else
	HAL_I2S_Transmit_IT(HDL_IIS_SPK, \
						audio_play.list[audio_play.idx].wave, \
						audio_play.list[audio_play.idx].samples);
#endif
}

 
/* **************************************************
 fucntion:		APP_audio_stop
 input:
 output:
 describe:	stop play
***************************************************/
void APP_audio_stop(void)
{
#ifdef IIS_SPK_DMA
	HAL_I2S_DMAStop(HDL_IIS_SPK);
#endif
      /*  Disable TXE and ERR interrupt */
	__HAL_I2S_DISABLE_IT(HDL_IIS_SPK, (I2S_IT_TXE | I2S_IT_ERR));
	__HAL_I2S_DISABLE(HDL_IIS_SPK);
      
      ((I2S_HandleTypeDef *)HDL_IIS_SPK)->State = HAL_I2S_STATE_READY;

      /*  Process Unlocked */
      __HAL_UNLOCK((I2S_HandleTypeDef *)HDL_IIS_SPK);

	//_AMP_OFF();
	audio_play.flag = AUDIOSTAT_IDLE;
	audio_play.size = 0;
	audio_play.idx = 0;
}


/* **************************************************
 fucntion:		APP_audio_fill_buffer
 input:		buf: data buffer to fill
 			frq: wave freqence in Hz
 			samples: number of 16-bit data to fill
 output:
 describe:	fill test buffer for test with specified frqence
***************************************************/
void APP_audio_fill_buffer(uint16_t* buf, float frq, uint32_t samples)
{
#define	_TEST_
#ifdef	_TEST_
	#define _STATIC static
#else
	#define _STATIC
#endif

	#define IIS_RATE	8000.0f	// IIS 8k samples-per-sencond
	_STATIC float32_t step, x,dat;
	_STATIC uint32_t idx;
	_STATIC int16_t tmp;

	step = frq / IIS_RATE * 2 * PI;
	x = 0;
	idx = (samples & ~1);		// samples should be even
	idx = (idx > TEST_AUDIO_MAXSAMPLE) ? TEST_AUDIO_MAXSAMPLE : idx;

	while (idx)
	{
	//	dat = arm_sin_f32(x);
		dat = sin(x);
		dat *= 16000;	//32767;
		tmp = (int16_t) dat;
		*buf++ = (uint16_t) tmp;
		*buf++ = (uint16_t) tmp;
		x += step;
		if (x >= (2 * PI)) x -= (2 * PI);
		idx -= 2;
	}
#undef	_STATIC
#undef	_TEST_
}

#undef	_LOCAL_AUDIO

/* ****************************  END OF FILE  *********************************/


