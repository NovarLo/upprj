/*   */
/*   */

#ifdef	_LOCAL_AUDIO
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

/////////// type & variables

#define AUDIOLIST_MAX 16

typedef enum
{
	AUDIOSTAT_IDLE = 0,	// idle
	AUDIOSTAT_READY,		// list ready to play
	AUDIOSTAT_PLAYING,	// audio palying
	AUDIOSTAT_PAUSE,		// audio paused
	AUDIOSTAT_STOP,		// audio stopped
	AUDIOSTAT_FINISHED,	// audio finished
	AUDIOSTAT_RESTART,	// audio restart

	AUDIOSTAT_MAX,
}
APP_AUDIOSTAT_TypeDef;

typedef struct
{
	uint16_t* wave;		// pointer of wave
	uint32_t samples;		// samples of wave
}
APP_AUDIOLIST_TypeDef;


typedef struct
{
	uint8_t flag;		// flag of audio, see APP_AUDIOSTAT_TypeDef
	uint8_t sn;		// serial number
	//uint8_t sn2;		// shadow of serial number
	uint8_t size;		// number of wave to play
	uint8_t idx;		// index of wave in playing
	APP_AUDIOLIST_TypeDef list[AUDIOLIST_MAX];
}
APP_AUDIOPLAY_TypeDef;

_EXTERN APP_AUDIOPLAY_TypeDef audio_play;

/////////// functions
void APP_audio_ini(void);
void APP_audio_list_ini(void);
void APP_audio_list_add(uint16_t* wav, uint32_t samples);
void APP_audio_play(void);
void APP_audio_stop(void);
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef * hi2s);

////////// only for test

#define TEST_AUDIO_MAXBUF		3
#define TEST_AUDIO_MAXSAMPLE	8192
//_EXTERN uint16_t wave[TEST_AUDIO_MAXBUF][TEST_AUDIO_MAXSAMPLE <<1];	// wave buffer for test
void APP_audio_fill_buffer(uint16_t* buf, float frq, uint32_t samples);


#undef	_EXTERN

/* ******************************  END OF FILE  *******************************/


