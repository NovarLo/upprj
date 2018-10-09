
#define	_LOCAL_FRAM

#include "stm32f4xx_hal.h"
#include "app_user.h"
#include "app_fram.h"



/* **************************************************
 fucntion:		APP_fram_WREN
 input:
 output:
 describe:	FRAM write enable
***************************************************/
void APP_fram_ini(void)
{
	_FM_NCS_DISABLE();
}


/* **************************************************
 fucntion:		APP_fram_WREN
 input:
 output:
 describe:	FRAM write enable
***************************************************/
void APP_fram_WREN(SPI_HandleTypeDef* hspi)
{
	uint8_t cmd = FRAMCMD_WREN;

	_FM_NCS_ENABLE();
	delay(20);
	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);
	delay(20);
	_FM_NCS_DISABLE();
}

 

/* **************************************************
 fucntion:		APP_fram_WRDI
 input:
 output:
 describe:	FRAM write disable
***************************************************/
void APP_fram_WRDI(SPI_HandleTypeDef* hspi)
{
	uint8_t cmd = FRAMCMD_WRDI;

	_FM_NCS_ENABLE();
	delay(20);
	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);
	delay(20);
	_FM_NCS_DISABLE();
}


/* **************************************************
 fucntion:		fram_RDSR
 input:
 output:
 describe: read status register of FRAM
***************************************************/
void APP_fram_RDSR(SPI_HandleTypeDef* hspi, uint8_t* buf)
{
	uint8_t cmd = FRAMCMD_RDSR;

	_FM_NCS_ENABLE();
	delay(20);
	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);
	HAL_SPI_Receive(hspi, buf, 1, 1000);
	delay(20);
	_FM_NCS_DISABLE();
}

 

/* **************************************************
 fucntion:		fram_WRSR
 input:
 output:
 describe: set status register of FRAM
***************************************************/
void APP_fram_WRSR(SPI_HandleTypeDef* hspi, uint8_t status)
{
	uint8_t buf[2];
	
	_FM_NCS_ENABLE();
	buf[0] = FRAMCMD_WRDI;
	buf[1] = status;
	delay(20);
	HAL_SPI_Transmit(hspi, buf, 2, 1000);
	delay(20);
	_FM_NCS_DISABLE();
}
 

/* **************************************************
 fucntion:		fram_readdata
 input:			addr - start of FRAM to read
 				buf - where data to store
 				size - number of data to read in byte
 output:
 describe: read data from FRAM to buf
***************************************************/
void APP_fram_readdata(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t size)
{
	uint8_t cmd[3];

	_FM_NCS_ENABLE();
	cmd[0] = FRAMCMD_READ;
	cmd[1] = (uint8_t)(addr >> 8);
	cmd[2] = (uint8_t)addr;
	delay(20);
	HAL_SPI_Transmit(hspi, cmd, 3, 1000);
	HAL_SPI_Receive(hspi, buf, size, 1000);
	delay(20);
	_FM_NCS_DISABLE();
}

 

/* **************************************************
 fucntion:		fram_writedata
 input:			addr - start of FRAM to write
 				buf - where data read from
 				size - number of data to write in byte
 output:
 describe:
***************************************************/
void APP_fram_writedata(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t size)
{
	uint8_t cmd[3];

	APP_fram_WREN(hspi);
	delay(20);
	
	_FM_NCS_ENABLE();
	cmd[0] = FRAMCMD_WRITE;
	cmd[1] = (uint8_t)(addr >> 8);
	cmd[2] = (uint8_t)addr;
	delay(20);
	HAL_SPI_Transmit(hspi, cmd, 3, 1000);
	HAL_SPI_Transmit(hspi, buf, size, 1000);
	delay(20);
	_FM_NCS_DISABLE();
	
	delay(10);
	APP_fram_WREN(hspi);
}

 

#undef	_LOCAL_FRAM

/* ****************************  END OF FILE  *********************************/


