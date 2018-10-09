
#define _LOCAL_SENSOR

#include "stm32f4xx_hal.h"
#include "arm_math.h"

#include "app_user.h"
#include "app_fram.h"
#include "app_audio.h"
#include "app_wave.h"
#include "app_sensor.h"
#include "app_upsnr.h"

/* *************************** common functions *******************************/

/* **************************************************
 fucntion:      average
 input:
 output:
 describe:  average and put result
***************************************************/
void APP_common_average(APP_SENSBUF_TypeDef *buf)
{
	uint32_t    index, number;
	uint64_t    sum;

	number = buf->bufsize;
	for (sum = 0, index = 0; index < number; index++)
	{
		sum += buf->dat[index];
	}
	sum /= number;
	buf->result = sum;

}


/* **************************************************
 fucntion:      moving-average
 input:     *buf: pointer of data structure
 output:
 describe:  1. insert new data into buffer (fifo mode)
			2. average and put result
***************************************************/
void APP_common_average_moving(APP_SENSBUF_TypeDef *buf)
{
	uint32_t    index, number;
	uint32_t    dat, max, min;
	uint64_t    sum;

	buf->dat[buf->bufindex] = buf->newdat;
	if (++buf->bufindex >= buf->bufsize) buf->bufindex = 0;

	number = buf->bufsize;
	max = min = buf->dat[0];
	sum = 0;
	for (index = 0; index < number; index++)
	{
		dat = buf->dat[index];
		sum += dat;
		max = (dat > max) ? dat : max;
		min = (dat < min) ? dat : min;
	}
	sum -= (max + min);
	sum /= (number - 2);
	buf->result = sum;
}


/* **************************************************
 fucntion:      moving-average
 input:     *buf: pointer of data structure
			weight: weight of new data
 output:
 describe:  put result using weighted-average
***************************************************/
void APP_common_average_weighted(APP_SENSBUF_TypeDef *buf, float weight)
{
	float   tmp1, tmp2;

	weight = weight > 1.0f ? 1.0f : weight;
	weight = weight < 0.0f ? 0.0f : weight;

	tmp1 = buf->newdat * weight;
	tmp2 = buf->result * (1.0f - weight);
	tmp1 += tmp2;
	buf->result = tmp1;
}


/* ***********************  sensor data functions  ****************************/

/* **************************************************
 fucntion:      APP_sensor_gpio_ini
 input:
 output:
 describe:  initialize gpio port
***************************************************/

void APP_sensor_gpio_ini(void)
{
	_RLY1_OFF();
	_RLY2_OFF();
	_RLY3_OFF();
	_RLY4_OFF();
}


/* **************************************************
 fucntion:      APP_sensor_gpio_update
 input:
 output:
 describe:  update gpio ports
***************************************************/

void APP_sensor_gpio_update(uint32_t step)
{
	// output update
	if (valve_stat.valve1_en)
	{
		_RLY1_ON();
	}
	else
	{
		_RLY1_OFF();
	}
	if (valve_stat.valve2_en)
	{
		_RLY2_ON();
	}
	else
	{
		_RLY2_OFF();
	}
	if (valve_stat.valve3_en)
	{
		_RLY3_ON();
	}
	else
	{
		_RLY3_OFF();
	}
	if (valve_stat.valve4_en)
	{
		_RLY4_ON();
	}
	else
	{
		_RLY4_OFF();
	}
}



/* ************************  sensor value functions  **************************/

/* **************************************************
 fucntion:      APP_sensor_speedupdate
 input:
 output:
 describe:  update speed
***************************************************/

void APP_sensor_speedupdate(float height)
{
#define SPEED_CUTOFF    0.15f       // cutoff of speed, m/s
#define SPEED_DATSIZE   8

	typedef struct
	{
		uint32_t time;
		float height;
	}
	APP_SPEEDDAT_TypeDef;

	typedef struct
	{
		uint32_t number;
		uint32_t index;
		APP_SPEEDDAT_TypeDef dat[SPEED_DATSIZE];
	}
	APP_SPEEDBUF_TypeDef;

	static APP_SPEEDBUF_TypeDef speed_buf;
	static uint32_t tick_last;
	static float height_last;
	static BOOL valid = FALSE;

	uint32_t idx;
	uint32_t tick;
	uint32_t time, time_max, time_min;
	float height0, height_max, height_min;

	uint32_t time_sum;
	float height_sum;
	float speed;

	tick = HAL_GetTick();
	if (valid)
	{
		speed_buf.dat[speed_buf.index].height = height - height_last;
		speed_buf.dat[speed_buf.index].time = tick - tick_last;
		if (++speed_buf.index >= SPEED_DATSIZE) speed_buf.index = 0;

		if (speed_buf.number < SPEED_DATSIZE)
		{
			speed_buf.number++;
		}
		else
		{
			time_sum = 0;
			height_sum = 0.0f;
			time_max = time_min = speed_buf.dat[0].time;
			height_max = height_min = speed_buf.dat[0].height;

			for (idx = 0; idx < SPEED_DATSIZE; idx++)
			{
				time = speed_buf.dat[idx].time;
				height0 = speed_buf.dat[idx].height;

				time_sum += time;
				height_sum += height0;

				if (height_max < height0)
				{
					height_max = height0;
					time_max = time;
				}
				else if (height_min > height0)
				{
					height_min = height0;
					time_min = time;
				}
			}
			time_sum -= (time_max + time_min);
			height_sum -= (height_max + height_min);
			if (time_sum == 0) time_sum = 1;
			speed = height_sum / time_sum * 1000.0f;
			if (speed > (SPEED_CUTOFF))
			{
				elivator_stat.move =  MOVESTAT_UP;
				elivator_stat.goingdown = FALSE;
			}
			else if (speed < (-SPEED_CUTOFF))
			{
				elivator_stat.move =  MOVESTAT_DOWN;
				elivator_stat.goingdown = TRUE;
			}
			else
			{
				speed =  0.0f;
				elivator_stat.move =  MOVESTAT_STOP;
			}

			sensor_value[SENS_SPEED].value = speed;
			sensor_value[SENS_SPEED].valid = TRUE;
		}

	}
	else
	{
		valid = TRUE;
		speed_buf.number = 0;
		speed_buf.index = 0;
	}

	height_last = height;
	tick_last = tick;

}


/* **************************************************
 fucntion:      APP_sensor_floorupdate
 input:
 output:
 describe:  update floor & flag
***************************************************/

void APP_sensor_floorupdate(float height)
{
#define OFFSET  0.1f        // offset of floor-aligned

	uint32_t max_type, max_number;
	uint32_t index_type, index_number;
	float next_height;

	uint32_t floor;
	BOOL aligned;
	//BOOL overhead = FALSE;

	index_type = index_number = 0;
	floor = 1;

	max_type = floor_tbl.tblsize;
	max_number = floor_tbl.type[index_type].number;
	next_height = floor_tbl.type[index_type].height;

	height += OFFSET;

	while (height >= next_height)
	{
		floor++;
		height -= next_height;

		index_number++;
		if (index_number >= max_number)
		{
			index_type++;
			index_number = 0;

			if (index_type >= max_type)
			{
				//overhead = TRUE;
				break;
			}
			next_height = floor_tbl.type[index_type].height;
			max_number = floor_tbl.type[index_type].number;
		}
	}

	if (height <= (OFFSET * 2.0f))
	{
		aligned = TRUE;
	}
	else
	{
		aligned = FALSE;
	}

/* 
	if (overhead)
	{
		;   // floor limit
	}
	else
*/
	{
		elivator_stat.floor = floor;
		elivator_stat.aligned = aligned;
	}
}

/* **************************************************
 fucntion:      APP_sensor_valvestat_ini
 input:
 output:
 describe:  set initialize stat for valve state
***************************************************/
void APP_sensor_valvestat_ini(void)
{
	valve_stat.valve1_en = FALSE;
	valve_stat.valve2_en = FALSE;
	valve_stat.valve3_en = FALSE;
	valve_stat.valve4_en = FALSE;
}

/* **************************************************
 fucntion:      APP_sensor_stateupdate
 input:
 output:
 describe:  update stat of elivator
***************************************************/

void APP_sensor_stateupdate()
{
	typedef enum
	{
		FINGERSTEP_STDBY = 0,
		FINGERSTEP_WAIT,            // wait to check
		FINGERSTEP_CHK,         // finger checking
		FINGERSTEP_VALID,           // finger valid

		FINGERSTEP_MAX
	}
	APP_FINGERSTEP_TypeDef;

	static APP_FINGERSTEP_TypeDef finger_step = FINGERSTEP_STDBY;
	static uint32_t finger_tick = 0;            // tick in mili-second


	switch (finger_step)
	{
	case FINGERSTEP_STDBY:
		if (elivator_stat.floor == 1 && elivator_stat.aligned)
		{
			finger_tick = HAL_GetTick();
			finger_step = FINGERSTEP_WAIT;
		}
		break;

	case FINGERSTEP_WAIT:
		if ((HAL_GetTick() - finger_tick) >= 2000)
		{
			finger_tick = HAL_GetTick();
			finger_step = FINGERSTEP_CHK;

			driver.finger_valid = FALSE;
			driver.name_id = 0;

			elivator_stat.finger_alarm = FALSE;
		}
		break;

	case FINGERSTEP_CHK:        // finger checking
		if (driver.finger_valid)
		{
			finger_tick = HAL_GetTick();
			finger_step = FINGERSTEP_VALID;
		}
		else if (elivator_stat.floor > 1)
		{
			elivator_stat.finger_alarm = TRUE;
			finger_step = FINGERSTEP_STDBY;
		}
		break;

	case FINGERSTEP_VALID:      // finger valid
		if (elivator_stat.floor > 1 || !elivator_stat.aligned)
		{
			finger_step = FINGERSTEP_STDBY;
		}
		else if ((HAL_GetTick() - finger_tick) >= 30000)
		{
			//finger_tick = HAL_GetTick();
			finger_step = FINGERSTEP_CHK;
			driver.finger_valid = FALSE;
		}
		break;

	default:
		finger_step = FINGERSTEP_STDBY;
		break;

	}

	// relay out update
/* 
	if (finger_step == FINGERSTEP_VALID)
*/
	if (finger_step == FINGERSTEP_VALID ||\
			finger_step == FINGERSTEP_STDBY)
	{
		elivator_stat.relay1_on = TRUE;
	}
	else     if (finger_step == FINGERSTEP_CHK)
	{
		elivator_stat.relay1_on = FALSE;
	}

}

/* **************************************************
 fucntion:      raw2value
 input:
 output:
 describe:  translate raw-data of sensors into engineering values
***************************************************/

void APP_sensor_raw2value(void)
{
	static SENSOR_CHANNELS channel = SENSOR_MINCHANNEL;
	float tmp;

	// translate raw data into real-time values
	switch (channel)
	{
	case SENS_UPSNR1:
	case SENS_UPSNR2:
	case SENS_UPSNR3:
	case SENS_UPSNR4:
	case SENS_UPSNR5:
	case SENS_UPSNR6:
	case SENS_UPSNR7:
	case SENS_UPSNR8:
	case SENS_UPSNR9:
	case SENS_UPSNR10:
	case SENS_UPSNR11:
	case SENS_UPSNR12:
	case SENS_UPSNR13:
	case SENS_UPSNR14:
	case SENS_UPSNR15:
	case SENS_UPSNR16:
	case SENS_UPWEIGHT:
		if (sensor_dat[channel].update)
		{
			sensor_dat[channel].update = FALSE;

			APP_common_average_weighted(&sensor_dat[channel], 1.0f);

			//APP_common_average_moving(&sensor_dat[channel]);

			if (sensor_dat[channel].count < sensor_dat[channel].bufsize)
			{
				sensor_dat[channel].count++;
				sensor_dat[channel].valid = FALSE;
				sensor_value[channel].valid = FALSE;
			}
			else
			{
				if (channel == SENS_UPWEIGHT || channel == SENS_CABL || channel == SENS_CABR)
				{
					tmp = APP_sensor_linear_interpolation(sensor_dat[channel].result,\
															  &cali_tbl.chdat[channel]);
					tmp = (tmp > 50.0f) ? tmp : 0.0f;
				}
				else
				{
					tmp = sensor_dat[channel].result;
				}

				//tmp = sensor_dat[channel].result;

				sensor_value[channel].value = tmp;
				sensor_value[channel].valid = TRUE;
				sensor_dat[channel].valid = TRUE;

				APP_sensor_alarmcheck(channel);
			}
		}
		else
		{
			if (sensor_dat[channel].timeout >= SENSOR_TMOUT * 10)
			{
				sensor_dat[channel].count = 0;
				sensor_dat[channel].valid = FALSE;
				sensor_value[channel].valid = FALSE;
			}
		}
		break;
	default:
		break;
	}

	if (++channel > SENSOR_MAXCHANNEL) channel = SENSOR_MINCHANNEL;

}



/* **************************************************
 fucntion:      sensor data initialize
 input:
 output:
 describe:  set default data for all sensor channels
***************************************************/

void APP_sensor_dat_ini(void)
{
	sensor_dat[CH_PWM1].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_PWM1].bufindex = 0;
	sensor_dat[CH_PWM1].scale = 1000;
	sensor_dat[CH_PWM1].update = FALSE;
	sensor_dat[CH_PWM1].valid = FALSE;

	sensor_dat[CH_PWM2].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_PWM2].bufindex = 0;
	sensor_dat[CH_PWM2].scale = 1000;
	sensor_dat[CH_PWM2].update = FALSE;
	sensor_dat[CH_PWM2].valid = FALSE;

	sensor_dat[CH_PWM3].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_PWM3].bufindex = 0;
	sensor_dat[CH_PWM3].scale = 100000;
	sensor_dat[CH_PWM3].update = FALSE;
	sensor_dat[CH_PWM3].valid = FALSE;

	sensor_dat[CH_PWM4].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_PWM4].bufindex = 0;
	sensor_dat[CH_PWM4].scale = 100000;
	sensor_dat[CH_PWM4].update = FALSE;
	sensor_dat[CH_PWM4].valid = FALSE;

	sensor_dat[CH_PWM5].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_PWM5].bufindex = 0;
	sensor_dat[CH_PWM5].scale = 100000;
	sensor_dat[CH_PWM5].update = FALSE;
	sensor_dat[CH_PWM5].valid = FALSE;


	sensor_dat[CH_FRQ1].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_FRQ1].bufindex = 0;
	sensor_dat[CH_FRQ1].scale = 100;        // 100000;
	sensor_dat[CH_FRQ1].update = FALSE;
	sensor_dat[CH_FRQ1].valid = FALSE;

	sensor_dat[CH_FRQ2].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_FRQ2].bufindex = 0;
	sensor_dat[CH_FRQ2].scale = 100;        //100000;
	sensor_dat[CH_FRQ2].update = FALSE;
	sensor_dat[CH_FRQ2].valid = FALSE;
	//sensor_dat[CH_FRQ2].count = 8;


	sensor_dat[CH_ANA1].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_ANA1].bufindex = 0;
	sensor_dat[CH_ANA1].scale = 10;     // 1000;
	sensor_dat[CH_ANA1].update = FALSE;
	sensor_dat[CH_ANA1].valid = FALSE;

	sensor_dat[CH_ANA2].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_ANA2].bufindex = 0;
	sensor_dat[CH_ANA2].scale = 10;     // 1000;
	sensor_dat[CH_ANA2].update = FALSE;
	sensor_dat[CH_ANA2].valid = FALSE;

	sensor_dat[CH_SSI].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_SSI].bufindex = 0;
	sensor_dat[CH_SSI].scale = 1;
//  sensor_dat[CH_SSI].newdat = ROTAT_BASE * sensor_dat[CH_SSI].scale;
	sensor_dat[CH_SSI].update = FALSE;
	sensor_dat[CH_SSI].valid = FALSE;

// flexible channel
	sensor_dat[CH_WEIGHT].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_WEIGHT].bufindex = 0;
	sensor_dat[CH_WEIGHT].scale = 10;
	sensor_dat[CH_WEIGHT].update = FALSE;
	sensor_dat[CH_WEIGHT].valid = FALSE;

	sensor_dat[CH_TEMP].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_TEMP].bufindex = 0;
	sensor_dat[CH_TEMP].scale = 10;
	sensor_dat[CH_TEMP].update = FALSE;
	sensor_dat[CH_TEMP].valid = FALSE;

	sensor_dat[CH_HUMIDITY].bufsize = SENS_BUFSIZE;
	sensor_dat[CH_HUMIDITY].bufindex = 0;
	sensor_dat[CH_HUMIDITY].scale = 10;
	sensor_dat[CH_HUMIDITY].update = FALSE;
	sensor_dat[CH_HUMIDITY].valid = FALSE;

// uploading platform sensor channel
	for (uint8_t i = 0; i < 17; i++)
	{
		sensor_dat[CH_UPSNR1 + i].bufsize = SENS_BUFSIZE;
		sensor_dat[CH_UPSNR1 + i].bufindex = 0;
		sensor_dat[CH_UPSNR1 + i].scale = 1;
		sensor_dat[CH_UPSNR1 + i].update = FALSE;
		sensor_dat[CH_UPSNR1 + i].valid = FALSE;
	}
}


/* **************************************************
 fucntion:      sensor value initialize
 input:
 output:
 describe:  set initialize value for sensors
***************************************************/

void APP_sensor_value_ini(void)
{
	sensor_value[SENS_PM25].value = 0.0;            // pm2.5 in ug/m^3
	sensor_value[SENS_PM25].valid = FALSE;
	sensor_value[SENS_PM25].alarm = SENSOR_NOERR;

	sensor_value[SENS_PM10].value = 0.0;            // pm10 in ug/m^3
	sensor_value[SENS_PM10].valid = FALSE;
	sensor_value[SENS_PM10].alarm = SENSOR_NOERR;

	sensor_value[SENS_TEMP].value = 0.0;            // temperature in ℃
	sensor_value[SENS_TEMP].valid = FALSE;
	sensor_value[SENS_TEMP].alarm = SENSOR_NOERR;

	sensor_value[SENS_HMDT].value = 0.0;            // humidity in %
	sensor_value[SENS_HMDT].valid = FALSE;
	sensor_value[SENS_HMDT].alarm = SENSOR_NOERR;

	sensor_value[SENS_WIND].value = 0.0;            // wind speed in m/s
	sensor_value[SENS_WIND].valid = FALSE;
	sensor_value[SENS_WIND].alarm = SENSOR_NOERR;

	sensor_value[SENS_VANE].value = 0.0;            // wind vane in direction
	sensor_value[SENS_VANE].valid = FALSE;
	sensor_value[SENS_VANE].alarm = SENSOR_NOERR;

	sensor_value[SENS_NOISE].value = 0.0;           // noise in dB
	sensor_value[SENS_NOISE].valid = FALSE;
	sensor_value[SENS_NOISE].alarm = SENSOR_NOERR;

// uploading platfor sensor channel
	for (uint8_t i = 0; i < 17; i++)
	{
		sensor_value[SENS_UPSNR1 + i].value = 0.0;
		sensor_value[SENS_UPSNR1 + i].valid  = FALSE;
		sensor_value[SENS_UPSNR1 + i].alarm = SENSOR_NOERR;
	}
}


/* **************************************************
 fucntion:      APP_sensor_sendprdvalue
 input:
 output:
 describe:  periodically send values of sensor
***************************************************/

void APP_sensor_sendprdvalue(void)
{
	uint8_t flag;
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

#ifdef UPPLAT
	APP_UPPRDVALUE_TypeDef *pv;
	pv = &upperiod_value;
#else
	APP_PRDVALUE_TypeDef *pv;
	pv = &period_value;
#endif
	pv->sn++;
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

	pv->sec = sTime.Seconds;
	pv->min = sTime.Minutes;
	pv->hour = sTime.Hours;
	pv->date = sDate.Date;
	pv->month = sDate.Month;
	pv->month |= ((sDate.WeekDay & 0x07) << 5);    // assemble with weekday
	pv->year = sDate.Year;

	pv->attrib = 0;

#ifdef UPPLAT

	// 平台称重
	if (sensor_value[SENS_UPWEIGHT].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_UPWEIGHT]) flag |= 0x02;
	pv->weight_flag = flag;
	pv->weight_alarm = sensor_value[SENS_UPWEIGHT].alarm;
	pv->weight_value = sensor_value[SENS_UPWEIGHT].value;

	// 斜拉索left
	if (sensor_value[SENS_CABL].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABL]) flag |= 0x02;
	pv->cableleft_flag = flag;
	pv->cableleft_alarm = sensor_value[SENS_CABL].alarm;
	pv->cableleft_value = sensor_value[SENS_CABL].value;

	// 斜拉索right
	if (sensor_value[SENS_CABR].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABR]) flag |= 0x02;
	pv->cableright_flag = flag;
	pv->cableright_alarm = sensor_value[SENS_CABR].alarm;
	pv->cableright_value = sensor_value[SENS_CABR].value;

	for (uint8_t i = 0; i < SENSORMAX; i++)
	{
		if (sensor_value[SENS_UPSNR1 + i].valid) flag = 0x01;
		else flag = 0x00;
		if (device_info.sensor_en[SENS_UPSNR1 + i]) flag |= 0x02;
		pv->weightsensor_flag[i] = flag;
		pv->weightsensor_alarm[i] = sensor_value[SENS_UPSNR1 + i].alarm;
		pv->weightsensor_value[i] = sensor_value[SENS_UPSNR1 + i].value;
	}

	pv->spare1_flag = 0;       // reserved
	pv->spare1_alarm = SENSOR_NOERR;
	pv->spare1_value = 0;
	pv->spare2_flag = 0;       // reserved
	pv->spare2_alarm = SENSOR_NOERR;
	pv->spare2_value = 0;
	pv->spare3_flag = 0;       // reserved
	pv->spare3_alarm = SENSOR_NOERR;
	pv->spare3_value = 0;
	pv->spare4_flag = 0;       // reserved
	pv->spare4_alarm = SENSOR_NOERR;
	pv->spare4_value = 0;

	pv->spare_other[0] = 0;
	pv->spare_other[1] = 0;
	pv->spare_other[2] = 0;
	pv->spare_other[3] = 0;
	pv->spare_other[4] = 0;
	pv->spare_other[5] = 0;

#else
	// pm2.5
	if (sensor_value[SENS_PM25].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_PM25]) flag |= 0x02;
	pv->pm25_flag = flag;
	pv->pm25_alarm = sensor_value[SENS_PM25].alarm;
	pv->pm25_value = sensor_value[SENS_PM25].value;

	// pm10
	if (sensor_value[SENS_PM10].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_PM10]) flag |= 0x02;
	pv->pm10_flag = flag;
	pv->pm10_alarm = sensor_value[SENS_PM10].alarm;
	pv->pm10_value = sensor_value[SENS_PM10].value;

	// wind speed
	if (sensor_value[SENS_WIND].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_WIND]) flag |= 0x02;
	pv->wind_flag = flag;
	pv->wind_alarm = sensor_value[SENS_WIND].alarm;
	pv->wind_value = sensor_value[SENS_WIND].value;

	// temperature
	if (sensor_value[SENS_TEMP].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_TEMP]) flag |= 0x02;
	pv->temperature_flag = flag;
	pv->temperature_alarm = sensor_value[SENS_TEMP].alarm;
	pv->temperature_value = sensor_value[SENS_TEMP].value;

	// humidity
	if (sensor_value[SENS_HMDT].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_HMDT]) flag |= 0x02;
	pv->humidity_flag = flag;
	pv->humidity_alarm = sensor_value[SENS_HMDT].alarm;
	pv->humidity_value = sensor_value[SENS_HMDT].value;

	// valve 1
	pv->valve1_flag = (dustmon_info.thrshdflag[0] << 1) | 0x01;
	if (pv->valve1_flag)
	{
		pv->valve1_alarm = valve_stat.valve1_en;
	}
	else pv->valve1_alarm = 0;
	// valve 2
	pv->valve2_flag = (dustmon_info.thrshdflag[1] << 1) | 0x01;
	if (pv->valve2_flag)
	{
		pv->valve2_alarm = valve_stat.valve2_en;
	}
	else pv->valve2_alarm = 0;
	// valve 3
	pv->valve3_flag = (dustmon_info.thrshdflag[2] << 1) | 0x01;
	if (pv->valve3_flag)
	{
		pv->valve3_alarm = valve_stat.valve3_en;
	}
	else pv->valve3_alarm = 0;
	// valve 4
	pv->valve4_flag = (dustmon_info.thrshdflag[3] << 1) | 0x01;
	if (pv->valve4_flag)
	{
		pv->valve4_alarm = valve_stat.valve4_en;
	}
	else pv->valve4_alarm = 0;

	// noise
	if (sensor_value[SENS_NOISE].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_NOISE]) flag |= 0x02;
	pv->noise_flag = flag;
	pv->noise_alarm = sensor_value[SENS_NOISE].alarm;
	pv->noise_value = 35.0f * log10(0.00523f * sensor_value[SENS_NOISE].value + 4.708f) + 25; //35 * log10(0.00523*sensor_value[SENS_NOISE].value+4.708) + 25

	// wind vane
	if (sensor_value[SENS_VANE].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_VANE]) flag |= 0x02;
	pv->vane_flag = flag;
	pv->vane_alarm = sensor_value[SENS_VANE].alarm;
	pv->vane_value = sensor_value[SENS_VANE].value;

	pv->spare1_flag = 0;       // reserved
	pv->spare1_alarm = SENSOR_NOERR;
	pv->spare1_value = 0;


	pv->spare2_flag = 0;       // reserved
	pv->spare2_alarm = SENSOR_NOERR;
	pv->spare2_value = 0;

	pv->spare_other[0] = 0;
	pv->spare_other[1] = 0;
	pv->spare_other[2] = 0;
	pv->spare_other[3] = 0;
	pv->spare_other[4] = 0;
	pv->spare_other[5] = 0;

#endif

	pv->flag = BUFSTAT_READY;
}


/* **************************************************
 fucntion:      APP_sensor_savprdvalue
 input:
 output:
 describe:  periodically save values of sensor
***************************************************/

void APP_sensor_savprdvalue(void)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	uint8_t flag;

#ifdef UPPLAT
	APP_UPPRDVALUE_TypeDef *sv;
	sv = &upsave_value;
#else
	APP_PRDVALUE_TypeDef *sv;
	sv = &save_value;
#endif

	sv->sn++;
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

	sv->sec = sTime.Seconds;
	sv->min = sTime.Minutes;
	sv->hour = sTime.Hours;
	sv->date = sDate.Date;
	sv->month = sDate.Month;
	sv->month |= ((sDate.WeekDay & 0x07) << 5);  // assemble with weekday
	sv->year = sDate.Year;

	sv->attrib = 0;

#ifdef UPPLAT
	// 平台称重
	if (sensor_value[SENS_UPWEIGHT].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_UPWEIGHT]) flag |= 0x02;
	sv->weight_flag = flag;
	sv->weight_alarm = sensor_value[SENS_UPWEIGHT].alarm;
	sv->weight_value = sensor_value[SENS_UPWEIGHT].value;

	// 斜拉索left
	if (sensor_value[SENS_CABL].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABL]) flag |= 0x02;
	sv->cableleft_flag = flag;
	sv->cableleft_alarm = sensor_value[SENS_CABL].alarm;
	sv->cableleft_value = sensor_value[SENS_CABL].value;

	// 斜拉索right
	if (sensor_value[SENS_CABR].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABR]) flag |= 0x02;
	sv->cableright_flag = flag;
	sv->cableright_alarm = sensor_value[SENS_CABR].alarm;
	sv->cableright_value = sensor_value[SENS_CABR].value;

	for (uint8_t i = 0; i < SENSORMAX; i++)
	{
		if (sensor_value[SENS_UPSNR1 + i].valid) flag = 0x01;
		else flag = 0x00;
		if (device_info.sensor_en[SENS_UPSNR1 + i]) flag |= 0x02;
		sv->weightsensor_flag[i] = flag;
		sv->weightsensor_alarm[i] = sensor_value[SENS_UPSNR1 + i].alarm;
		sv->weightsensor_value[i] = sensor_value[SENS_UPSNR1 + i].value;
	}

	sv->spare1_flag = 0;       // reserved
	sv->spare1_alarm = SENSOR_NOERR;
	sv->spare1_value = 0;
	sv->spare2_flag = 0;       // reserved
	sv->spare2_alarm = SENSOR_NOERR;
	sv->spare2_value = 0;
	sv->spare3_flag = 0;       // reserved
	sv->spare3_alarm = SENSOR_NOERR;
	sv->spare3_value = 0;
	sv->spare4_flag = 0;       // reserved
	sv->spare4_alarm = SENSOR_NOERR;
	sv->spare4_value = 0;

	sv->spare_other[0] = 0;
	sv->spare_other[1] = 0;
	sv->spare_other[2] = 0;
	sv->spare_other[3] = 0;
	sv->spare_other[4] = 0;
	sv->spare_other[5] = 0;

#else
	// pm2.5
	if (sensor_value[SENS_PM25].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_PM25]) flag |= 0x02;
	sv->pm25_flag = flag;
	sv->pm25_alarm = sensor_value[SENS_PM25].alarm;
	sv->pm25_value = sensor_value[SENS_PM25].value;

	// pm10
	if (sensor_value[SENS_PM10].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_PM10]) flag |= 0x02;
	sv->pm10_flag = flag;
	sv->pm10_alarm = sensor_value[SENS_PM10].alarm;
	sv->pm10_value = sensor_value[SENS_PM10].value;

	// wind speed
	if (sensor_value[SENS_WIND].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_WIND]) flag |= 0x02;
	sv->wind_flag = flag;
	sv->wind_alarm = sensor_value[SENS_WIND].alarm;
	sv->wind_value = sensor_value[SENS_WIND].value;

	// temperature
	if (sensor_value[SENS_TEMP].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_TEMP]) flag |= 0x02;
	sv->temperature_flag = flag;
	sv->temperature_alarm = sensor_value[SENS_TEMP].alarm;
	sv->temperature_value = sensor_value[SENS_TEMP].value;

	// humidity
	if (sensor_value[SENS_HMDT].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_HMDT]) flag |= 0x02;
	sv->humidity_flag = flag;
	sv->humidity_alarm = sensor_value[SENS_HMDT].alarm;
	sv->humidity_value = sensor_value[SENS_HMDT].value;

	// valve 1
	sv->valve1_flag = (dustmon_info.thrshdflag[0] << 1) | 0x01;
	if (save_value.valve1_flag)
	{
		sv->valve1_alarm = valve_stat.valve1_en;
	}
	else
		sv->valve1_alarm = 0;
	// valve 2
	sv->valve2_flag = (dustmon_info.thrshdflag[1] << 1) | 0x01;
	if (save_value.valve2_flag)
	{
		sv->valve2_alarm = valve_stat.valve2_en;
	}
	else sv->valve2_alarm = 0;
	// valve 3
	sv->valve3_flag = (dustmon_info.thrshdflag[2] << 1) | 0x01;
	if (save_value.valve3_flag)
	{
		sv->valve3_alarm = valve_stat.valve3_en;
	}
	else sv->valve3_alarm = 0;
	// valve 4
	sv->valve4_flag = (dustmon_info.thrshdflag[3] << 1) | 0x01;
	if (save_value.valve4_flag)
	{
		sv->valve4_alarm = valve_stat.valve4_en;
	}
	else sv->valve4_alarm = 0;

	// noise
	if (sensor_value[SENS_NOISE].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_NOISE]) flag |= 0x02;
	sv->noise_flag = flag;
	sv->noise_alarm = sensor_value[SENS_NOISE].alarm;
	sv->noise_value = 35.0f * log10(0.00523f * sensor_value[SENS_NOISE].value + 4.708f) + 25; //35 * log10(0.00523*sensor_value[SENS_NOISE].value+4.708) + 25

	// wind vane
	if (sensor_value[SENS_VANE].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_VANE]) flag |= 0x02;
	sv->vane_flag = flag;
	sv->vane_alarm = sensor_value[SENS_VANE].alarm;
	sv->vane_value = sensor_value[SENS_VANE].value;

	sv->spare1_flag = 0;     // reserved
	sv->spare1_alarm = SENSOR_NOERR;
	sv->spare1_value = 0;


	sv->spare2_flag = 0;     // reserved
	sv->spare2_alarm = SENSOR_NOERR;
	sv->spare2_value = 0;

	sv->spare_other[0] = 0;
	sv->spare_other[1] = 0;
	sv->spare_other[2] = 0;
	sv->spare_other[3] = 0;
	sv->spare_other[4] = 0;
	sv->spare_other[5] = 0;
#endif
	sv->flag = BUFSTAT_READY;
}


/* **************************************************
 fucntion:      APP_wklpvalue_update
 input:
 output:
 describe:  update workloop max-min value
***************************************************/

void APP_sensor_wklpvalue_update()
{
	float value;

	value = sensor_value[SENS_HEIGHT].value;
	if (value > workloop.height_max)
	{
		workloop.height_max = value;
	}

	value = elivator_stat.floor;
	if (value > workloop.floor_max)
	{
		workloop.floor_max = value;
	}

	if (sensor_value[SENS_WEIGHT].valid &&\
			device_info.sensor_en[SENS_WEIGHT])
	{
		value = sensor_value[SENS_WEIGHT].value;
		if (value > workloop.weight_max)
		{
			workloop.weight_max = value;
		}

		value = sensor_value[SENS_PEOPLE].value;
		if (value > workloop.people_max)
		{
			workloop.people_max = value;
		}
	}
}


/* **************************************************
 fucntion:      work-loop check
 input:
 output:
 describe:  check work-loop using weight
***************************************************/

void APP_sensor_wklpchk(void)
{
#define HEIGHT_WKLPON       1.0f        // height threshold, meter
#define TIME_WKLP   10      // work loop ack-time, based on check interval.

	typedef enum
	{
		WKLPSTEP_IDLE = 0,      // work-loop not found

		WKLPSTEP_ON_FOUND,  // found begining of work-loop
		WKLPSTEP_ON_ACK,        // ack begining of work-loop
		WKLPSTEP_OFF_FOUND, // found ending of work-loop
		WKLPSTEP_OFF_ACK,       // ack ending of work-loop
		WKLPSTEP_TOTAL_ACK, // work-loop ack
		WKLPSTEP_TOTAL_NCK, // work-loop nck

		WKLPSTEP_MAX
	}
	APP_WKLPSTEP_TypeDef;

	static APP_WKLPSTEP_TypeDef step = WKLPSTEP_IDLE;

	static bool load_now = FALSE;
	static uint32_t wklpcnt = 0;

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	if (sensor_value[SENS_HEIGHT].valid &&\
			device_info.sensor_en[SENS_HEIGHT])
	{
		if (elivator_stat.floor == 1 &&\
				elivator_stat.aligned) load_now = FALSE;
		else load_now = TRUE;

		switch (step)
		{
		case WKLPSTEP_IDLE:

			wklpcnt = 0;

			if (load_now)
			{
				step = WKLPSTEP_ON_FOUND;
				//wklpcnt = HAL_GetTick();

				// fill begin time

				HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
				HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

				workloop.sec_begin = sTime.Seconds;
				workloop.min_begin = sTime.Minutes;
				workloop.hour_begin = sTime.Hours;
				workloop.date_begin = sDate.Date;
				workloop.month_begin = sDate.Month;
				workloop.month_begin |= ((sDate.WeekDay & 0x07) << 5);  // assemble with weekday
				workloop.year_begin = sDate.Year;

				// ini max-min value
				workloop.weight_max = sensor_value[SENS_WEIGHT].value;
				workloop.people_max = sensor_value[SENS_PEOPLE].value;

				workloop.height_max = sensor_value[SENS_HEIGHT].value;
				workloop.floor_max = elivator_stat.floor;
			}

			break;


		case WKLPSTEP_ON_FOUND:
			if (load_now)
			{
				if (++wklpcnt >= TIME_WKLP)
				{
					step = WKLPSTEP_ON_ACK;
				}
				APP_sensor_wklpvalue_update();
			}
			else
			{
				step = WKLPSTEP_IDLE;
			}

			break;


		case WKLPSTEP_ON_ACK:

			wklpcnt = 0;

			if (!load_now)
			{
				step = WKLPSTEP_OFF_FOUND;

				// fill end time

				HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
				HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

				workloop.sec_end = sTime.Seconds;
				workloop.min_end = sTime.Minutes;
				workloop.hour_end = sTime.Hours;
				workloop.date_end = sDate.Date;
				workloop.month_end = sDate.Month;
				workloop.month_end |= ((sDate.WeekDay & 0x07) << 5);    // assemble with weekday
				workloop.year_end = sDate.Year;
			}
			else
			{
				APP_sensor_wklpvalue_update();
			}

			break;


		case WKLPSTEP_OFF_FOUND:
			if (!load_now)
			{
				if (++wklpcnt >= TIME_WKLP)
				{
					step = WKLPSTEP_OFF_ACK;
				}
			}
			else
			{
				step = WKLPSTEP_ON_ACK;
			}

			break;


		case WKLPSTEP_OFF_ACK:
			if (workloop.height_max >= HEIGHT_WKLPON)
			{
				workloop.flag = BUFSTAT_READY;
				workloop.sn++;

				if (driver.finger_valid)
				{
					workloop.name_id = driver.name_id;
				}
				else
				{
					workloop.name_id = 0;
				}
			}
			else
			{
				workloop.flag = BUFSTAT_NULL;
			}

			step = WKLPSTEP_IDLE;

			break;


		default:
			step = WKLPSTEP_IDLE;
			workloop.flag = BUFSTAT_NULL;

			break;

		}
	}
	else
	{
		step = WKLPSTEP_IDLE;
		workloop.flag = BUFSTAT_NULL;
	}
}


/* **************************************************
 fucntion:      APP_sensor_alarm_ini
 input:
 output:
 describe:  initialize alarm_shadow
***************************************************/
void APP_sensor_alarm_ini(void)
{
	alarm_shadow.flag = FALSE;
	alarm_shadow.sn = 0;

	alarm_shadow.pm25_valid = TRUE;
	alarm_shadow.pm25_alarm = SENSOR_NOERR;

	alarm_shadow.weight_valid = TRUE;
	alarm_shadow.weight_alarm = SENSOR_NOERR;

	alarm_shadow.height_valid = TRUE;
	alarm_shadow.height_alarm = SENSOR_NOERR;

	alarm_shadow.speed_valid = TRUE;
	alarm_shadow.speed_alarm = SENSOR_NOERR;

	alarm_shadow.wind_valid = TRUE;
	alarm_shadow.wind_alarm = SENSOR_NOERR;

	alarm_shadow.tilt_valid = TRUE;
	alarm_shadow.tilt_alarm = SENSOR_NOERR;

	alarm_shadow.motor1_valid = TRUE;
	alarm_shadow.motor1_alarm = SENSOR_NOERR;

	alarm_shadow.motor2_valid = TRUE;
	alarm_shadow.motor2_alarm = SENSOR_NOERR;

	alarm_shadow.motor3_valid = TRUE;
	alarm_shadow.motor3_alarm = SENSOR_NOERR;

	alarm_shadow.people_valid = TRUE;
	alarm_shadow.people_alarm = SENSOR_NOERR;

	alarm_shadow.floor = 1;
	alarm_shadow.aligned = FALSE;

	alarm_shadow.move = MOVESTAT_STOP;
	alarm_shadow.goingdown = FALSE;

	alarm_shadow.fingersensor_valid = TRUE;
	//alarm_shadow.finger_valid = TRUE;
	alarm_shadow.finger_alarm = TRUE;
	//alarm_shadow.finger_step = FINGERSTEP_STDBY;

	alarm_shadow.alarm_doorin = FALSE;
	alarm_shadow.alarm_doorout = FALSE;
	alarm_shadow.alarm_top = FALSE;
	alarm_shadow.alarm_bottom = FALSE;

	alarm_shadow.upweight_valid = TRUE;
	alarm_shadow.upweight_alarm = SENSOR_NOERR;

	alarm_shadow.cabl_valid = TRUE;
	alarm_shadow.cabl_alarm = SENSOR_NOERR;

	alarm_shadow.cabr_valid = TRUE;
	alarm_shadow.cabr_alarm = SENSOR_NOERR;

	driver.sensor_valid = TRUE;
	driver.finger_valid = FALSE;
	driver.id = FINGERSTAT_NULL;
	strcpy((char *)driver.name, "未授权");
	driver.name_id = 0;
}


/* **************************************************
 fucntion:      alarm check
 input:     channel: sensor channel
 output:
 describe:  check alarm for specified channel
			set/reset alarm flag
***************************************************/

void APP_sensor_alarmcheck(SENSOR_CHANNELS channel)
{
//#define _TEST_

#ifdef _TEST_
#define _STATIC static  // for test only
#else
#define _STATIC
#endif

	// alarm check

	float err,err_back;
	float alarm,alarm_back;
	float warn,warn_back;
	_STATIC float value;
	uint8_t i;
	float alrm[8], alrm_bak[8];

	value = sensor_value[channel].value;
	switch (channel)
	{
	case SENS_UPWEIGHT:
		if (sensor_value[SENS_UPWEIGHT].valid && \
			device_info.sensor_en[SENS_UPWEIGHT])
		{
			value = sensor_value[SENS_UPWEIGHT].value;

			// upper-limit check
			alarm = limit_tbl.limit[SENS_UPWEIGHT].hilimit;
			warn = limit_tbl.limit[SENS_UPWEIGHT].hiwarn;
			warn_back = warn - 0.2f;

			alarm -= 0.03f ;
			alarm_back = alarm - 0.2f;

			if (value >= alarm)
				SET_BIT(sensor_value[SENS_UPWEIGHT].alarm, BIT_HIALARM);
			else if (value < alarm_back)
				CLEAR_BIT(sensor_value[SENS_UPWEIGHT].alarm, BIT_HIALARM);

			if (value >= warn)
				SET_BIT(sensor_value[SENS_UPWEIGHT].alarm, BIT_HIWARN);
			else if (value < warn_back)
				CLEAR_BIT(sensor_value[SENS_UPWEIGHT].alarm, BIT_HIWARN);
		}
		else
		{
			sensor_value[SENS_UPWEIGHT].alarm = SENSOR_NOERR;
		}

		break;
	case SENS_CABL:
		if (sensor_value[SENS_CABL].valid && \
			device_info.sensor_en[SENS_CABL])
		{
			value = sensor_value[SENS_CABL].value;

			// upper-limit check
			alarm = limit_tbl.limit[SENS_CABL].hilimit;
			warn = limit_tbl.limit[SENS_CABL].hiwarn;
			warn_back = warn - 0.2f;

			alarm -= 0.03f ;
			alarm_back = alarm - 0.2f;

			if (value >= alarm)
				SET_BIT(sensor_value[SENS_CABL].alarm, BIT_HIALARM);
			else if (value < alarm_back)
				CLEAR_BIT(sensor_value[SENS_CABL].alarm, BIT_HIALARM);

			if (value >= warn)
				SET_BIT(sensor_value[SENS_CABL].alarm, BIT_HIWARN);
			else if (value < warn_back)
				CLEAR_BIT(sensor_value[SENS_CABL].alarm, BIT_HIWARN);
		}
		else
		{
			sensor_value[SENS_CABL].alarm = SENSOR_NOERR;
		}

		break;
	case SENS_CABR:
		if (sensor_value[SENS_CABR].valid && \
			device_info.sensor_en[SENS_CABR])
		{
			value = sensor_value[SENS_CABR].value;

			// upper-limit check
			alarm = limit_tbl.limit[SENS_CABR].hilimit;
			warn = limit_tbl.limit[SENS_CABR].hiwarn;
			warn_back = warn - 0.2f;

			alarm -= 0.03f ;
			alarm_back = alarm - 0.2f;

			if (value >= alarm)
				SET_BIT(sensor_value[SENS_CABR].alarm, BIT_HIALARM);
			else if (value < alarm_back)
				CLEAR_BIT(sensor_value[SENS_CABR].alarm, BIT_HIALARM);

			if (value >= warn)
				SET_BIT(sensor_value[SENS_CABR].alarm, BIT_HIWARN);
			else if (value < warn_back)
				CLEAR_BIT(sensor_value[SENS_CABR].alarm, BIT_HIWARN);
		}
		else
		{
			sensor_value[SENS_CABR].alarm = SENSOR_NOERR;
		}

		break;
	default:
		break;
	}
#undef _STATIC
#undef _TEST_

}


/* **************************************************
 fucntion:      APP_sensor_findalarm
 input:
 output:
 describe:  find alarm, send to server and play sound
***************************************************/
void APP_sensor_findalarm(void)
{
	uint8_t buf_idx, idx2;
	//BOOL playback = FALSE;      // flag to play audio

	//// clear alarm buffer
	alarm_shadow.status = 0;
	for (buf_idx = 0; buf_idx < RPT_ALARMID_MAX; buf_idx++)
	{
		for (idx2 = 0; idx2 < RPT_ALARMID_SIZE; idx2++) alarm_shadow.alarm[buf_idx][idx2] = 0;
	}
	for (buf_idx = 0; buf_idx < RPT_AGAINSTID_MAX; buf_idx++)
	{
		for (idx2 = 0; idx2 < RPT_AGAINSTID_SIZE; idx2++) alarm_shadow.against[buf_idx][idx2] = 0;
	}
	for (buf_idx = 0; buf_idx < RPT_ERRORID_MAX; buf_idx++)
	{
		alarm_shadow.error[buf_idx] = 0;
	}



	// find activation of alarm, against & error
	//// uploading platform weight
	if (device_info.sensor_en[SENS_UPWEIGHT])
	{
		if (sensor_value[SENS_UPWEIGHT].valid)
		{
			switch (sensor_value[SENS_UPWEIGHT].alarm)
			{
/* 
				case SENSOR_HIERR:
				if (alarm_shadow.pm25_alarm != SENSOR_HIERR)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ERROR;
					alarm_shadow.error[RPT_ERRORID_LIMITPM25] = TRUE;
				}
				break;
*/

			case SENSOR_HIALARM:
				if (alarm_shadow.upweight_alarm != SENSOR_HIERR &&\
						alarm_shadow.upweight_alarm != SENSOR_HIALARM)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;

					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_ALARM;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_UPWEIGHT_ALARM;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;
			case SENSOR_HIWARN:
				if (alarm_shadow.upweight_alarm != SENSOR_HIERR && \
					alarm_shadow.upweight_alarm != SENSOR_HIALARM && \
					alarm_shadow.upweight_alarm != SENSOR_HIWARN)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;
					
					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_WARN;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_UPWEIGHT_WARN;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;
			default:
				APP_UPSNR_CLOSEALARM();
				break;
			}
		}
		else if (alarm_shadow.upweight_valid)
		{
			alarm_shadow.status |= RPT_STATUSBIT_ERROR;
			alarm_shadow.error[RPT_ERRORID_UPW] = TRUE;
			APP_UPSNR_CLOSEALARM();
		}
		alarm_shadow.upweight_valid = sensor_value[SENS_UPWEIGHT].valid;
		alarm_shadow.upweight_alarm = sensor_value[SENS_UPWEIGHT].alarm;
	}
	else
	{
		alarm_shadow.upweight_valid = TRUE;
		alarm_shadow.upweight_alarm = SENSOR_NOERR;
		APP_UPSNR_CLOSEALARM();
	}

	if (device_info.sensor_en[SENS_CABL])
	{
		if (sensor_value[SENS_CABL].valid)
		{
			switch (sensor_value[SENS_CABL].alarm)
			{
/* 
				case SENSOR_HIERR:
				if (alarm_shadow.pm25_alarm != SENSOR_HIERR)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ERROR;
					alarm_shadow.error[RPT_ERRORID_LIMITPM25] = TRUE;
				}
				break;
*/

			case SENSOR_HIALARM:
				if (alarm_shadow.cabl_alarm != SENSOR_HIERR &&\
						alarm_shadow.cabl_alarm != SENSOR_HIALARM)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;

					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_ALARM;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_CABL_ALARM;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;
			case SENSOR_HIWARN:
				if (alarm_shadow.cabl_alarm != SENSOR_HIERR && \
					alarm_shadow.cabl_alarm != SENSOR_HIALARM && \
					alarm_shadow.cabl_alarm != SENSOR_HIWARN)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;
					
					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_WARN;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_CABL_WARN;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;

			default:
				APP_UPSNR_CLOSEALARM();
				break;
			}
		}
		else if (alarm_shadow.cabl_valid)
		{
			alarm_shadow.status |= RPT_STATUSBIT_ERROR;
			alarm_shadow.error[RPT_ERRORID_UPW] = TRUE;
			APP_UPSNR_CLOSEALARM();
		}
		alarm_shadow.cabl_valid = sensor_value[SENS_CABL].valid;
		alarm_shadow.cabl_alarm = sensor_value[SENS_CABL].alarm;
	}
	else
	{
		alarm_shadow.cabl_valid = TRUE;
		alarm_shadow.cabl_alarm = SENSOR_NOERR;
		APP_UPSNR_CLOSEALARM();
	}

	if (device_info.sensor_en[SENS_CABR])
	{
		if (sensor_value[SENS_CABR].valid)
		{
			switch (sensor_value[SENS_CABR].alarm)
			{
/* 
				case SENSOR_HIERR:
				if (alarm_shadow.pm25_alarm != SENSOR_HIERR)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ERROR;
					alarm_shadow.error[RPT_ERRORID_LIMITPM25] = TRUE;
				}
				break;
*/

			case SENSOR_HIALARM:
				if (alarm_shadow.cabr_alarm != SENSOR_HIERR &&\
						alarm_shadow.cabr_alarm != SENSOR_HIALARM)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;

					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_ALARM;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_CABR_ALARM;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;
			case SENSOR_HIWARN:
				if (alarm_shadow.cabr_alarm != SENSOR_HIERR && \
					alarm_shadow.cabr_alarm != SENSOR_HIALARM && \
					alarm_shadow.cabr_alarm != SENSOR_HIWARN)
				{
					alarm_shadow.status |= RPT_STATUSBIT_ALARM;
					
					alarm_shadow.alarm[RPT_ALARMID_UPPLAT][0] = RPT_ALARM_WARN;
					alarm_shadow.alarm[RPT_ALARMID_LIMIT][1] = RPT_ALARM_CABR_WARN;
					// start voice&light alarm
					APP_UPSNR_OPENALARM();
				}
				break;

			default:
				APP_UPSNR_CLOSEALARM();
				break;
			}
		}
		else if (alarm_shadow.cabr_valid)
		{
			alarm_shadow.status |= RPT_STATUSBIT_ERROR;
			alarm_shadow.error[RPT_ERRORID_UPW] = TRUE;
			APP_UPSNR_CLOSEALARM();
		}
		alarm_shadow.cabr_valid = sensor_value[SENS_CABR].valid;
		alarm_shadow.cabr_alarm = sensor_value[SENS_CABR].alarm;
	}
	else
	{
		alarm_shadow.cabr_valid = TRUE;
		alarm_shadow.cabr_alarm = SENSOR_NOERR;
		APP_UPSNR_CLOSEALARM();
	}

	//// set data when message found
	if (alarm_shadow.status)
	{
#ifdef UPPLAT
		APP_UPALARMDAT_TypeDef *buf;
#else
		APP_ALARMDAT_TypeDef *buf;
#endif
		uint8_t flag;
		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef sDate;

		// find first empty buffer
		// use last buffer when full
		for (buf_idx = 0; buf_idx < RPT_ALARM_BUFSIZE; buf_idx++)
		{
			if (upalarm_dat[buf_idx].flag == BUFSTAT_NULL) break;
		}
		if (buf_idx >= RPT_ALARM_BUFSIZE) buf_idx = RPT_ALARM_BUFSIZE - 1;
		buf = upalarm_dat + buf_idx;

		APP_sensor_clearalarmbuf(buf);

		// fill report buffer
		alarm_shadow.sn++;

		buf->sn = alarm_shadow.sn;          // serial number

		HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
		HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

		buf->sec = sTime.Seconds;
		buf->min = sTime.Minutes;
		buf->hour = sTime.Hours;
		buf->date = sDate.Date;
		buf->month = sDate.Month;
		buf->month |= ((sDate.WeekDay & 0x07) << 5);    // assemble with weekday
		buf->year = sDate.Year;         // 20xx

		if (driver.finger_valid)
		{
			buf->name_id = driver.name_id;
		}
		else
		{
			buf->name_id = 0;
		}

		buf->attrib = 0;        // reserved

		flag = (sensor_value[SENS_UPWEIGHT].valid) ? 0x01 : 0x00;
		if (device_info.sensor_en[SENS_UPWEIGHT]) flag |= 0x02;
		buf->weight_flag = flag;
		buf->weight_alarm = sensor_value[SENS_UPWEIGHT].alarm;
		buf->weight_value = sensor_value[SENS_UPWEIGHT].value;

		flag = (sensor_value[SENS_CABL].valid) ? 0x01 : 0x00;
		if (device_info.sensor_en[SENS_CABL]) flag |= 0x02;
		buf->cableleft_flag = flag;
		buf->cableleft_alarm = sensor_value[SENS_CABL].alarm;
		buf->cableleft_value = sensor_value[SENS_CABL].value;

		flag = (sensor_value[SENS_CABR].valid) ? 0x01 : 0x00;
		if (device_info.sensor_en[SENS_CABR]) flag |= 0x02;
		buf->cableright_flag = flag;
		buf->cableright_alarm = sensor_value[SENS_CABR].alarm;
		buf->cableright_value = sensor_value[SENS_CABR].value;


		// status of alarm, against, error.
		buf->alarm_stat = alarm_shadow.status;

		for (buf_idx = 0; buf_idx < RPT_ALARMID_MAX; buf_idx++)
		{
			flag = 0;
			for (idx2 = 0; idx2 < RPT_ALARMID_SIZE; idx2++)
			{
				flag |= alarm_shadow.alarm[buf_idx][idx2];
			}
			if (flag)
			{
				buf->alarm[buf->alarm_num].alarm_code = buf_idx;
				for (idx2 = 0; idx2 < RPT_ALARMID_SIZE; idx2++)
				{
					buf->alarm[buf->alarm_num].alarm_byte[idx2] = alarm_shadow.alarm[buf_idx][idx2];
				}
				buf->alarm_num++;
			}
		}

		for (buf_idx = 0; buf_idx < RPT_AGAINSTID_MAX; buf_idx++)
		{
			flag = 0;
			for (idx2 = 0; idx2 < RPT_AGAINSTID_SIZE; idx2++)
			{
				flag |= alarm_shadow.against[buf_idx][idx2];
			}
			if (flag)
			{
				buf->against[buf->against_num].against_code = buf_idx;
				for (idx2 = 0; idx2 < RPT_AGAINSTID_SIZE; idx2++)
				{
					buf->against[buf->against_num].against_byte[idx2] = alarm_shadow.against[buf_idx][idx2];
				}
				buf->against_num++;
			}
		}

		for (buf_idx = 0; buf_idx < RPT_ERRORID_MAX; buf_idx++)
		{
			if (alarm_shadow.error[buf_idx])
			{
				buf->error[buf->error_num].error_code = buf_idx;
				buf->error_num++;
			}
		}

		buf->flag = BUFSTAT_READY;
	}
}


/* **************************************************
 fucntion:      APP_sensor_clearalarmbuf
 input:
 output:
 describe:  clear alarm buffer
***************************************************/
void APP_sensor_clearalarmbuf(APP_UPALARMDAT_TypeDef *buf)
{
	buf->flag = BUFSTAT_READY;


	buf->alarm_stat = 0;
	buf->alarm_num = 0;
	buf->against_num = 0;
	buf->error_num = 0;
}


/* **************************************************
 fucntion:      APP_sensor_sendrawdat
 input:
 output:
 describe:  periodically send raw-data for remote-calibration
***************************************************/
void APP_sensor_sendrawdat(void)
{
	uint8_t flag;
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BCD);

	upcali_dat.sec = sTime.Seconds;
	upcali_dat.min = sTime.Minutes;
	upcali_dat.hour = sTime.Hours;
	upcali_dat.date = sDate.Date;
	upcali_dat.month = sDate.Month;
	upcali_dat.month |= ((sDate.WeekDay & 0x07) << 5);    // assemble with weekday
	upcali_dat.year = sDate.Year;

	upcali_dat.attrib = 0;

	if (sensor_value[SENS_UPWEIGHT].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_UPWEIGHT]) flag |= 0x02;
	//period_value.weight_flag = flag;
	upcali_dat.upweight = sensor_dat[SENS_UPWEIGHT].result;

	if (sensor_value[SENS_CABL].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABL]) flag |= 0x02;
	//period_value.weight_flag = flag;
	upcali_dat.cabl = sensor_dat[SENS_CABL].result;

	if (sensor_value[SENS_CABR].valid) flag = 0x01;
	else flag = 0x00;
	if (device_info.sensor_en[SENS_CABR]) flag |= 0x02;
	//period_value.weight_flag = flag;
	upcali_dat.cabr = sensor_dat[SENS_CABR].result;

	upcali_dat.spare1_flag = 0;       // reserved
	upcali_dat.spare1_alarm = 0;      // reserved
	upcali_dat.spare1_dat = 0;        // reserved

	upcali_dat.spare2_flag = 0;       // reserved
	upcali_dat.spare2_alarm = 0;      // reserved
	upcali_dat.spare2_dat = 0;        // reserved

	upcali_dat.spare3_flag = 0;       // reserved
	upcali_dat.spare3_alarm = 0;      // reserved
	upcali_dat.spare3_dat = 0;        // reserved

	upcali_dat.spare4_flag = 0;       // reserved
	upcali_dat.spare4_alarm = 0;      // reserved
	upcali_dat.spare4_dat = 0;        // reserved

	upcali_dat.flag = BUFSTAT_READY;
}


/* **************************************************
 fucntion:      linear interpolation
 input:     dat: data to be processed
			buf: pointer of calibration table structure
 output:        value been precessed
 describe:  putout value from data using linear interpolation
			with specified table, data outside the range
			used as the nearest region.
***************************************************/

float APP_sensor_linear_interpolation(uint32_t dat, APP_CALICHDAT_TypeDef *buf)
{
	uint32_t index, tblsize;
	float value;
	float x0, x1, y0, y1;
	float dltx, dlty;
	float k, b;

	tblsize = buf->tblsize;     // if (tblsize <2) ????
//  value = buf->dat[0].x;

	index  = 1;
	while (index < (tblsize - 1))
	{
		if (dat <= buf->dat[index].x) break;
		index++;
	}
//  index = (index < tblsize) ? index : (tblsize - 1);

	x0 = buf->dat[index - 1].x;
	x1 = buf->dat[index].x;
	dltx =  x1 - x0;

	y0 = buf->dat[index - 1].y;
	y1 = buf->dat[index].y;
	dlty = y1 - y0;

	k = dlty / dltx;
	b = dat - x0;
	b *= k;
	value = b + y0;

	return (value);

}


#undef  _LOCAL_SENSOR

/* ****************************  END OF FILE  *********************************/


