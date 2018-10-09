#define _LOCAL_UPSNR

#include "stm32f4xx_hal.h"
#include "app_user.h"
#include "app_network.h"
#include "app_mg2639d.h"
#include "app_sim7600ce.h"
#include "app_audio.h"
#include "app_wave.h"
#include "app_sensor.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "app_upsnr.h"
#include "app_crc.h"

uint8_t upsnr_rcvbyte;
APP_upsnrsendbuf_TypeDef upsnr_sendbuf;
APP_upsnrrcvbuf_TypeDef upsnr_rcvbuf;

/* 
*********************************************************************************************************
*   函 数 名: APP_UART6_Init
*
*   功能说明: 称重传感器模块串口重新初始化
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_USARTUP_Init(void)
{
	huart6.Instance = USART6;
	huart6.Init.BaudRate = 115200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart6);
}

/* 
*********************************************************************************************************
*   函 数 名: APP_UPSNR_Init
*
*   功能说明: 称重传感器模块初始化 —— 包含串口6的重新初始化和其他状态位
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_UPSNR_Init(void)
{

	// 初始化发送和接收环形缓冲区
	recvfifo_6.buffer = recvbuf_6;
	memset(recvfifo_6.buffer, 0, QUEUE_REV_SIZE);
	recvfifo_6.size = QUEUE_REV_SIZE;
	recvfifo_6.in = 0;
	recvfifo_6.out = 0;

	sendfifo_6.buffer = sendbuf_6;
	memset(sendfifo_6.buffer, 0, BUFFERSIZE);
	sendfifo_6.size = BUFFERSIZE;
	sendfifo_6.in = 0;
	sendfifo_6.out = 0;
	APP_USARTUP_Init();
	HAL_UART_Receive_IT(HDL_UART_UP, recvbuf_6, QUEUE_REV_SIZE);

	//HAL_UART_Receive_IT(HDL_UART_UP, &upsnr_rcvbyte, 1);
}

/* **************************************************
 fucntion:      APP_upsnr_reciveisr
 input:     byte: data received form uart
 output:
 describe:  uart receive isr of upsnr
***************************************************/

void APP_upsnr_rcvisr(void)
{
	float value;
	enum
	{
		UPSNR_RCVSTEP_IDLE = 0,
		UPSNR_RCVSTEP_HEAD,
		UPSNR_RCVSTEP_DAT,            // receiving data
		UPSNR_RCVSTEP_CRC,            //
		UPSNR_RCVSTEP_END,           // find 1st byte of end data

		UPSNR_RCVSTEP_MAX
	};
	static uint8_t upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;

	uint32_t tick;
	uint32_t index;
	uint8_t byte = upsnr_rcvbyte;
	uint8_t crc;

	HAL_UART_Receive_IT(HDL_UART_UP, &upsnr_rcvbyte, 1);

	tick = HAL_GetTick();
	if ((tick - upsnr_rcvbuf.timestamp) > 200)
	{
		upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
		upsnr_rcvbuf.rcvindex = 0;
	}
	upsnr_rcvbuf.timestamp = tick;

	switch (upsnr_rcvstep)
	{
	case UPSNR_RCVSTEP_IDLE:
		upsnr_rcvbuf.rcvindex = 0;
		if (byte == UPSNR_RCVSTART1)
		{       // find start1 byte
			upsnr_rcvstep = UPSNR_RCVSTEP_HEAD;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		break;

	case UPSNR_RCVSTEP_HEAD:
		upsnr_rcvbuf.rcvindex = 0;
		if (byte == UPSNR_RCVSTART2)
		{       // find start2 byte
			upsnr_rcvstep = UPSNR_RCVSTEP_DAT;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		break;

	case UPSNR_RCVSTEP_DAT:
		if (upsnr_rcvbuf.rcvindex >= (UPSNR_RCVDATLEN - 3))
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_CRC;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		else
		{
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		break;

	case UPSNR_RCVSTEP_CRC:
		crc = GetCRC7ByLeftByTable(upsnr_rcvbuf.rcvbyte, UPSNR_DATBUFSIZE + 2);
		if (crc == byte)
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_END;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		else
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
			upsnr_rcvbuf.rcvindex = 0;
		}
		break;

	case UPSNR_RCVSTEP_END:
		upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;

		if (byte == UPSNR_RCVEND)
		{   // recieve complete frame
			for (index = 0; index < upsnr_rcvbuf.rcvindex; index++)
			{
				upsnr_rcvbuf.rcvdat[index] = upsnr_rcvbuf.rcvbyte[index];
			}
			for (index = 0; index < UPSNR_DATBUFSIZE / 2; index++)
			{
				value = upsnr_rcvbuf.rcvdat[2 * index + 2] | upsnr_rcvbuf.rcvdat[2 * index + 3] << 8;
				_APP_SET_SENSOR_DAT(CH_UPSNR1 + index, value);
			}
			upsnr_rcvbuf.valid = TRUE;

			upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
			upsnr_rcvbuf.rcvindex = 0;
		}
		break;

	default :
		upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
		upsnr_rcvbuf.rcvindex = 0;
		break;

	}
}

void APP_UPSNR_OPENALARM(void)
{
	uint8_t send_buf[5] = {0x55, 0xaa, 0x01, 0x00, 0x16 };

	send_buf[3] =  GetCRC7ByLeftByTable(send_buf, 3);
	//HAL_UART_Transmit_IT(HDL_UART_UP, send_buf, 5);
	HAL_UART_Transmit(HDL_UART_UP, send_buf, 5, 50);

}

void APP_UPSNR_CLOSEALARM(void)
{
	uint8_t send_buf[5] = {0x55, 0xaa, 0x00, 0x00, 0x16 };

	send_buf[3] =  GetCRC7ByLeftByTable(send_buf, 3);
	//HAL_UART_Transmit_IT(HDL_UART_UP, send_buf, 5);
	HAL_UART_Transmit(HDL_UART_UP, send_buf, 5, 50);
}

void APP_UPSNR_TASK(void)
{
	double value,alpha;
	enum
	{
		UPSNR_RCVSTEP_IDLE = 0,
		UPSNR_RCVSTEP_HEAD,
		UPSNR_RCVSTEP_DAT,            // receiving data
		UPSNR_RCVSTEP_CRC,            //
		UPSNR_RCVSTEP_END,           // find 1st byte of end data

		UPSNR_RCVSTEP_MAX
	};
	static uint8_t upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;

	uint32_t tick;
	uint32_t index;
	uint8_t byte = upsnr_rcvbyte;
	uint8_t crc;
	//float sum = 0.0f;

	//HAL_UART_Receive_IT(HDL_UART_UP, &upsnr_rcvbyte, 1);

	// 缓冲区读指针掉头
	if (recvfifo_6.out == QUEUE_REV_SIZE)
	{
		recvfifo_6.out = 0;
	}

	// 如果读指针离写指针超过限值，调整读指针到写指针，保证数据的实时性
	if (abs(recvfifo_6.out - recvfifo_6.in) > 100)
	{
		recvfifo_6.out = recvfifo_6.in;
	}

	// 接收缓冲区无数据退出
	if (recvfifo_6.out == recvfifo_6.in) return;

	byte = recvbuf_6[recvfifo_6.out++];

	tick = HAL_GetTick();
	if ((tick - upsnr_rcvbuf.timestamp) > 200)
	{
		upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
		upsnr_rcvbuf.rcvindex = 0;
	}
	upsnr_rcvbuf.timestamp = tick;

	switch (upsnr_rcvstep)
	{
	case UPSNR_RCVSTEP_IDLE:
		upsnr_rcvbuf.rcvindex = 0;
		if (byte == UPSNR_RCVSTART1)
		{       // find start1 byte
			upsnr_rcvstep = UPSNR_RCVSTEP_HEAD;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		break;

	case UPSNR_RCVSTEP_HEAD:
		if (byte == UPSNR_RCVSTART2)
		{       // find start2 byte
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
			upsnr_rcvstep = UPSNR_RCVSTEP_DAT;
		}
		break;

	case UPSNR_RCVSTEP_DAT:
		if (upsnr_rcvbuf.rcvindex >= (UPSNR_RCVDATLEN - 3))
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_CRC;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		else
		{
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		break;

	case UPSNR_RCVSTEP_CRC:
		crc = GetCRC7ByLeftByTable(upsnr_rcvbuf.rcvbyte, UPSNR_DATBUFSIZE + 2);
		if (crc == byte)
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_END;
			upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;
		}
		else
		{
			upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
			upsnr_rcvbuf.rcvindex = 0;
		}
		break;

	case UPSNR_RCVSTEP_END:
		upsnr_rcvbuf.rcvbyte[upsnr_rcvbuf.rcvindex++] = byte;

		if (byte == UPSNR_RCVEND)
		{   // recieve complete frame
			for (index = 0; index < upsnr_rcvbuf.rcvindex; index++)
			{
				upsnr_rcvbuf.rcvdat[index] = upsnr_rcvbuf.rcvbyte[index];
			}
			for (index = 0; index < UPSNR_DATBUFSIZE / 2; index++)
			{
				value = upsnr_rcvbuf.rcvdat[2 * index + 2] | upsnr_rcvbuf.rcvdat[2 * index + 3] << 8;
				_APP_SET_SENSOR_DAT(CH_UPSNR1 + index, value);
			}
			{
				value = 0;
				for (index = 0; index < 8; index++)
				{
					if (cali_tbl.algorithm_enable) alpha = cali_tbl.alpha[index];
					else alpha = 1;
					value += alpha * (upsnr_rcvbuf.rcvdat[2 * index + 2] | upsnr_rcvbuf.rcvdat[2 * index + 3] << 8);
				}
				_APP_SET_SENSOR_DAT(CH_UPWEIGHT, value);
			}
			upsnr_rcvbuf.valid = TRUE;

			upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
			upsnr_rcvbuf.rcvindex = 0;
		}
		break;

	default :
		upsnr_rcvstep = UPSNR_RCVSTEP_IDLE;
		upsnr_rcvbuf.rcvindex = 0;
		break;

	}
}

#undef _LOCAL_UPSNR
/* *************************** end line ****************************/

