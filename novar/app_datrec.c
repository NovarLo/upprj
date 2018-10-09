#include "stm32f4xx_hal.h"
#include "app_datrec.h"
#include "app_user.h"
#include "app_sensor.h"
#include "app_finger.h"
#include "string.h"

/*����NAND Flash��������ַ���������Ӳ�������� */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR

/*�������NAND Flash�õ�3���� */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

// ��ϻ�Ӽ�¼���ݵ�������
#define BLACKBOX_SIZE 512

/*�߼����ӳ������ÿ�������2%���ڱ��������������ά������1024�� LUT = Look Up Table */
static uint16_t s_usLUT[NAND_BLOCK_COUNT];

static uint16_t s_usValidDataBlockCount;	/*��Ч�����ݿ���� */

static uint8_t s_ucTempBuf[NAND_PAGE_TOTAL_SIZE];	/*�󻺳�����2112�ֽ�. ���ڶ����Ƚ� */

static uint8_t NAND_BuildLUT(void);
static uint32_t FSMC_NAND_GetStatus(void);
static uint16_t NAND_FindFreeBlock (void);
static uint8_t NAND_MarkUsedBlock(uint32_t _ulBlockNo);
static void NAND_MarkBadBlock(uint32_t _ulBlockNo);
static uint16_t NAND_AddrToPhyBlockNo(uint32_t _ulMemAddr);
static uint8_t NAND_IsBufOk(uint8_t *_pBuf, uint32_t _ulLen, uint8_t _ucValue);
uint8_t NAND_WriteToNewBlock(uint32_t _ulPhyPageNo, uint8_t *_pWriteBuf, uint16_t _usOffset, uint16_t _usSize);
static uint8_t NAND_IsFreeBlock(uint32_t _ulBlockNo);

static uint16_t NAND_LBNtoPBN(uint32_t _uiLBN);

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Init
*	����˵��: ����FSMC��GPIO����NAND Flash�ӿڡ�������������ڶ�дnand flashǰ������һ�Ρ�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void FSMC_NAND_Ini(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	FSMC_NAND_PCC_TimingTypeDef p;

	/*--NAND Flash GPIOs ����  ------
		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE
		PD5/FSMC_NWE
		PD7/FSMC_NCE2
		PD11/FSMC_A16
		PD12/FSMC_A17
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7

		PG6/FSMC_INT2	(�������ò�ѯ��ʽ��æ���˿�����Ϊ��ͨGPIO���빦��ʹ��)
	*/

	/*ʹ�� GPIO ʱ�� */
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);
	  /*GPIO Ports Clock Enable */	
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	__GPIOG_CLK_ENABLE();

	/*ʹ�� FSMC ʱ�� */
	//RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
	__FSMC_CLK_ENABLE();

	/** FSMC GPIO Configuration  
	PE7   ------> FSMC_D4
	PE8   ------> FSMC_D5
	PE9   ------> FSMC_D6
	PE10   ------> FSMC_D7
	PD11   ------> FSMC_CLE
	PD12   ------> FSMC_ALE
	PD14   ------> FSMC_D0
	PD15   ------> FSMC_D1
	PD0   ------> FSMC_D2
	PD1   ------> FSMC_D3
	PD4   ------> FSMC_NOE
	PD5   ------> FSMC_NWE
	PD6   ------> FSMC_NWAIT
	PD7   ------> FSMC_NCE2
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15 
						  |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
						  |GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/*
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
*/
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


	/*���� FSMC ʱ�� */
	/*
		Defines the number of HCLK cycles to setup address before the command assertion for NAND-Flash
		read or write access  to common/Attribute or I/O memory space (depending on the memory space
		timing to be configured).This parameter can be a value between 0 and 0xFF.
	*/
  	//p.FSMC_SetupTime = 0x01;
  	p.SetupTime = 0x2;

	/*
		Defines the minimum number of HCLK cycles to assert the command for NAND-Flash read or write
		access to common/Attribute or I/O memory space (depending on the memory space timing to be
		configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_WaitSetupTime = 0x03;
	p.WaitSetupTime = 0x5;

	/*
		Defines the number of HCLK clock cycles to hold address (and data for write access) after the
		command deassertion for NAND-Flash read or write access to common/Attribute or I/O memory space
		 (depending on the memory space timing to be configured).
		This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HoldSetupTime = 0x02;
	p.HoldSetupTime = 0x3;

	/*
		Defines the number of HCLK clock cycles during which the databus is kept in HiZ after the start
		of a NAND-Flash  write access to common/Attribute or I/O memory space (depending on the memory
		space timing to be configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HiZSetupTime = 0x01;
	p.HiZSetupTime = 0x2;

	/** Perform the NAND1 memory initialization sequence
	*/
	hnand1.Instance = FSMC_NAND_DEVICE;
	/*hnand1.Init */
	hnand1.Init.NandBank = FSMC_NAND_BANK2;
	hnand1.Init.Waitfeature = FSMC_NAND_PCC_WAIT_FEATURE_DISABLE;
	hnand1.Init.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
	hnand1.Init.EccComputation = FSMC_NAND_ECC_ENABLE;
	hnand1.Init.ECCPageSize = FSMC_NAND_ECC_PAGE_SIZE_2048BYTE;
	hnand1.Init.TCLRSetupTime = 1;
	hnand1.Init.TARSetupTime = 1;
	/*hnand1.Info */

	HAL_NAND_Init(&hnand1, &p, &p);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadID
*	����˵��: ��NAND Flash��ID��ID�洢���β�ָ���Ľṹ������С�
*	��    ��:  ��
*	�� �� ֵ: 32bit��NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;

	// �������� Command to the command area 
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;

	// ˳���ȡNAND Flash��ID 
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) |
			((data << 8 ) & 0x00FF0000) |
			((data >> 8 ) & 0x0000FF00) |
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_PageCopyBack
*	����˵��: ��һҳ���ݸ��Ƶ�����һ��ҳ��Դҳ��Ŀ��ҳ����ͬΪż��ҳ��ͬΪ����ҳ��
*	��    ��:  - _ulSrcPageNo: Դҳ��
*             - _ulTarPageNo: Ŀ��ҳ��
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*
*	˵    ���������ֲ��Ƽ�����ҳ����֮ǰ����У��Դҳ��λУ�飬������ܻ����λ���󡣱�����δʵ�֡�
*
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_PageCopyBack(uint32_t _ulSrcPageNo, uint32_t _ulTarPageNo)
{
	uint8_t i;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_A;

	/*����Դҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulSrcPageNo;
	NAND_ADDR_AREA = (_ulSrcPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_B;

	/*����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	for (i = 0; i < 20; i++);
	while( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0 );//GPIDG novar

	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;

	/*����Ŀ��ҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulTarPageNo;
	NAND_ADDR_AREA = (_ulTarPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_D;

	/*������״̬ */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_PageCopyBackEx
*	����˵��: ��һҳ���ݸ��Ƶ�����һ��ҳ,������Ŀ��ҳ�еĲ������ݡ�Դҳ��Ŀ��ҳ����ͬΪż��ҳ��ͬΪ����ҳ��
*	��    ��:  - _ulSrcPageNo: Դҳ��
*             - _ulTarPageNo: Ŀ��ҳ��
*			  - _usOffset: ҳ��ƫ�Ƶ�ַ��pBuf�����ݽ�д�������ַ��ʼ��Ԫ
*			  - _pBuf: ���ݻ�����
*			  - _usSize: ���ݴ�С
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*
*	˵    ���������ֲ��Ƽ�����ҳ����֮ǰ����У��Դҳ��λУ�飬������ܻ����λ���󡣱�����δʵ�֡�
*
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_PageCopyBackEx(uint32_t _ulSrcPageNo, uint32_t _ulTarPageNo, uint8_t *_pBuf, uint16_t _usOffset, uint16_t _usSize)
{
	uint16_t i;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_A;

	/*����Դҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulSrcPageNo;
	NAND_ADDR_AREA = (_ulSrcPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_COPYBACK_B;

	/*����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	for (i = 0; i < 20; i++);
	while( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0 );	// novar GPIOG

	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;

	/*����Ŀ��ҳ��ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = 0;
	NAND_ADDR_AREA = _ulTarPageNo;
	NAND_ADDR_AREA = (_ulTarPageNo & 0xFF00) >> 8;

	/*�м����������, Ҳ����ȴ� */

	NAND_CMD_AREA = NAND_CMD_COPYBACK_C;

	NAND_ADDR_AREA = _usOffset;
	NAND_ADDR_AREA = _usOffset >> 8;

	/*�������� */
	for(i = 0; i < _usSize; i++)
	{
		NAND_DATA_AREA = _pBuf[i];
	}

	NAND_CMD_AREA = NAND_CMD_COPYBACK_D;

	/*������״̬ */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WritePage
*	����˵��: дһ��������NandFlashָ��ҳ���ָ��λ�ã�д������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint16_t i;

	/*����ҳд���� */
	NAND_CMD_AREA = NAND_CMD_WRITE0;

	/*����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	/*д���� */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;

	/*������״̬ */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadPage
*	����˵��: ��NandFlashָ��ҳ���ָ��λ�ö�һ�����ݣ����������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: �ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint16_t i;

    /*����ҳ������� */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/*����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;

	 /*����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	for (i = 0; i < 20; i++);
	while( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == 0); // novar GPIOD

	/*�����ݵ�������pBuffer */
	for(i = 0; i < _usByteCount; i++)
	{
		_pBuffer[i] = NAND_DATA_AREA;
	}

	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WriteSpare
*	����˵��: ��1��PAGE��Spare��д������
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-63
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WriteSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}

	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadSpare
*	����˵��: ��1��PAGE��Spare��������
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-63
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}

	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WriteData
*	����˵��: ��1��PAGE����������д������
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-2047
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WriteData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_PAGE_SIZE)
	{
		return NAND_FAIL;
	}

	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadData
*	����˵��: ��1��PAGE�������ݵ�����
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-2047
*             - _usByteCount: �������ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	if (_usByteCount > NAND_PAGE_SIZE)
	{
		return NAND_FAIL;
	}

	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_EraseBlock
*	����˵��: ����NAND Flashһ���飨block��
*	��    ��:  - _ulBlockNo: ��ţ���ΧΪ��0 - 1023
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	/*���Ͳ������� */
	NAND_CMD_AREA = NAND_CMD_ERASE0;

	_ulBlockNo <<= 6;	/*���ת��Ϊҳ��� */
	NAND_ADDR_AREA = _ulBlockNo;
	NAND_ADDR_AREA = _ulBlockNo >> 8;

	NAND_CMD_AREA = NAND_CMD_ERASE1;

	return (FSMC_NAND_GetStatus());
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Reset
*	����˵��: ��λNAND Flash
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;

		/*������״̬ */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}

	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadStatus
*	����˵��: ʹ��Read statuc �����NAND Flash�ڲ�״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_BUSY: �ڲ���æ
*             - NAND_READY: �ڲ����У����Խ����²�����
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;

	/*��״̬���� */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);

	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	}
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY;
	}

	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_GetStatus
*	����˵��: ��ȡNAND Flash����״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint32_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x10000;
	uint32_t ucStatus = NAND_READY;

	ucStatus = FSMC_NAND_ReadStatus();

	/*�ȴ�NAND������������ʱ����˳� */
	while ((ucStatus != (uint8_t)NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		ulTimeout--;
	}

	if(ulTimeout == 0x00)
	{
		ucStatus =  NAND_TIMEOUT_ERROR;
	}

	/*���ز���״̬ */
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Init
*	����˵��: ��ʼ��NAND Flash�ӿ�
*	��    ��:  ��
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Init(void)
{
	uint8_t Status;

	FSMC_NAND_Ini();			/*����FSMC��GPIO����NAND Flash�ӿ� */

	FSMC_NAND_Reset();			/*ͨ����λ���λNAND Flash����״̬ */

	//Status = NAND_BuildLUT();	/*����������� LUT = Look up table */
	return Status;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_WriteToNewBlock
*	����˵��: ���ɿ�����ݸ��Ƶ��¿飬�����µ����ݶ�д������¿�
*	��    ��:  	_ulPhyPageNo : Դҳ��
*				_pWriteBuf �� ���ݻ�����
*				_usOffset �� ҳ��ƫ�Ƶ�ַ
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_WriteToNewBlock(uint32_t _ulPhyPageNo, uint8_t *_pWriteBuf, uint16_t _usOffset, uint16_t _usSize)
{
	uint16_t n, i;
	uint16_t usNewBlock;
	uint16_t ulSrcBlock;
	uint16_t usOffsetPageNo;

	ulSrcBlock = _ulPhyPageNo / NAND_BLOCK_SIZE;		/*��������ҳ�ŷ��ƿ�� */
	usOffsetPageNo = _ulPhyPageNo % NAND_BLOCK_SIZE;	/*��������ҳ�ż�������ҳ���ڿ���ƫ��ҳ�� */
	/*����ѭ����Ŀ���Ǵ���Ŀ���Ϊ�������� */
	for (n = 0; n < 10; n++)
	{
		/*�������ȫ0xFF�� ����ҪѰ��һ�����п��ÿ飬����ҳ�ڵ�����ȫ���Ƶ��¿��У�Ȼ���������� */
		usNewBlock = NAND_FindFreeBlock();	/*�����һ��Block��ʼ����Ѱһ�����ÿ� */
		if (usNewBlock >= NAND_BLOCK_COUNT)
		{
			return NAND_FAIL;	/*���ҿ��п�ʧ�� */
		}

		/*ʹ��page-copy���ܣ�����ǰ�飨usPBN��������ȫ�����Ƶ��¿飨usNewBlock�� */
		for (i = 0; i < NAND_BLOCK_SIZE; i++)
		{
			if (i == usOffsetPageNo)
			{
				/*���д��������ڵ�ǰҳ������Ҫʹ�ô�������ݵ�Copy-Back���� */
				if (FSMC_NAND_PageCopyBackEx(ulSrcBlock * NAND_BLOCK_SIZE + i, usNewBlock * NAND_BLOCK_SIZE + i,
					_pWriteBuf, _usOffset, _usSize) == NAND_FAIL)
				{
					NAND_MarkBadBlock(usNewBlock);	/*���¿���Ϊ���� */
					NAND_BuildLUT();				/*�ؽ�LUT�� */
					break;
				}
			}
			else
			{
				/*ʹ��NAND Flash �ṩ����ҳCopy-Back���ܣ�����������߲���Ч�� */
				if (FSMC_NAND_PageCopyBack(ulSrcBlock * NAND_BLOCK_SIZE + i,
					usNewBlock * NAND_BLOCK_SIZE + i) == NAND_FAIL)
				{
					NAND_MarkBadBlock(usNewBlock);	/*���¿���Ϊ���� */
					NAND_BuildLUT();				/*�ؽ�LUT�� */
					break;
				}
			}
		}
		/*Ŀ�����³ɹ� */
		if (i == NAND_BLOCK_SIZE)
		{
			/*����¿�Ϊ���ÿ� */
			if (NAND_MarkUsedBlock(usNewBlock) == NAND_FAIL)
			{
				NAND_MarkBadBlock(usNewBlock);	/*���¿���Ϊ���� */
				NAND_BuildLUT();				/*�ؽ�LUT�� */
				continue;
			}

			/*����ԴBLOCK */
			if (FSMC_NAND_EraseBlock(ulSrcBlock) != NAND_READY)
			{
				NAND_MarkBadBlock(ulSrcBlock);	/*��Դ����Ϊ���� */
				NAND_BuildLUT();				/*�ؽ�LUT�� */
				continue;
			}
			NAND_BuildLUT();				/*�ؽ�LUT�� */
			break;
		}
	}

	return NAND_OK;	/*д��ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Write
*	����˵��: дһ������
*	��    ��:  	_MemAddr : �ڴ浥Ԫƫ�Ƶ�ַ
*				_pReadbuff ����Ŵ�д���ݵĻ�������ָ��
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Write(uint32_t _ulMemAddr, uint32_t *_pWriteBuf, uint16_t _usSize)
{
	uint16_t usPBN;			/*������� */
	uint32_t ulPhyPageNo;	/*����ҳ�� */
	uint16_t usAddrInPage;	/*ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/*���ݳ��ȱ�����4�ֽ������� */
	if ((_usSize % 4) != 0)
	{
		return NAND_FAIL;
	}
	/*���ݳ��Ȳ��ܳ���512�ֽ�(��ѭ Fat��ʽ) */
	if (_usSize > 512)
	{
		//return NAND_FAIL;
	}

	usPBN = NAND_AddrToPhyBlockNo(_ulMemAddr);	/*��ѯLUT������������ */

	ulTemp = _ulMemAddr % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
	ulPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/*��������ҳ�� */
	usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/*����ҳ��ƫ�Ƶ�ַ */

	/*�������������ݣ��ж��Ƿ�ȫFF */
	if (FSMC_NAND_ReadData(s_ucTempBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		return NAND_FAIL;	/*��NAND Flashʧ�� */
	}
	/*�������ȫ0xFF, �����ֱ��д�룬������� */
	if (NAND_IsBufOk(s_ucTempBuf, _usSize, 0xFF) == NAND_OK)
	{
		if (FSMC_NAND_WriteData((uint8_t *)_pWriteBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
		{
			/*������д�뵽����һ���飨���п飩 */
			return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
		}

		/*��Ǹÿ����� */
		if (NAND_MarkUsedBlock(ulPhyPageNo) == NAND_FAIL)
		{
			/*���ʧ�ܣ�������д�뵽����һ���飨���п飩 */
			return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
		}
		return NAND_OK;	/*д��ɹ� */
	}

	/*������д�뵽����һ���飨���п飩 */
	return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
}
/*
*********************************************************************************************************
*	�� �� ��: NAND_DirectWrite
*	����˵��: дһ������
*	��    ��:  	_MemAddr : �ڴ浥Ԫƫ�Ƶ�ַ
*				_pReadbuff ����Ŵ�д���ݵĻ�������ָ��
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_DirectWrite(uint32_t _ulMemAddr, uint32_t *_pWriteBuf, uint16_t _usSize)
{
	uint16_t usPBN;			/*������� */
	uint32_t ulPhyPageNo;	/*����ҳ�� */
	uint16_t usAddrInPage;	/*ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/*���ݳ��ȱ�����4�ֽ������� */
	if ((_usSize % 4) != 0)
	{
		return NAND_FAIL;
	}
	/*���ݳ��Ȳ��ܳ���512�ֽ�(��ѭ Fat��ʽ) */
	if (_usSize > 512)
	{
		//return NAND_FAIL;
	}

	usPBN = NAND_AddrToPhyBlockNo(_ulMemAddr);	/*��ѯLUT������������ */

	ulTemp = _ulMemAddr % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
	ulPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/*��������ҳ�� */
	usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/*����ҳ��ƫ�Ƶ�ַ */

	/*�������������ݣ��ж��Ƿ�ȫFF */
	if (FSMC_NAND_ReadData(s_ucTempBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		return NAND_FAIL;	/*��NAND Flashʧ�� */
	}
	/*�����д���ַ����ȫ0xFF, ���׵�ַλ�ڸÿ�ĵ�0ҳ��ƫ�Ƶ�ַ0�������ֱ�Ӳ��� */
	if (NAND_IsBufOk(s_ucTempBuf, _usSize, 0xFF) != NAND_OK && ulPhyPageNo == 0 && usAddrInPage == 0)
	{
		/*��1������������� */
		if (FSMC_NAND_EraseBlock(ulTemp) != NAND_READY)
		{
			return NAND_FAIL;
		}
	}

	// д��ָ���������ݵ�ָ��λ��
	if (FSMC_NAND_WriteData((uint8_t *)_pWriteBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		/*������д�뵽����һ���飨���п飩 */
		return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
	}

	/*��Ǹÿ����� */
	if (NAND_MarkUsedBlock(ulPhyPageNo) == NAND_FAIL)
	{
		/*���ʧ�ܣ�������д�뵽����һ���飨���п飩 */
		return NAND_WriteToNewBlock(ulPhyPageNo, (uint8_t *)_pWriteBuf, usAddrInPage, _usSize);
	}
	return NAND_OK;	/*д��ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Read
*	����˵��: ��һ������
*	��    ��:  	_MemAddr : �ڴ浥Ԫƫ�Ƶ�ַ
*				_pReadbuff ����Ŷ������ݵĻ�������ָ��
*				_usSize �����ݳ��ȣ�������4�ֽڵ�������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_Read(uint32_t _ulMemAddr, uint32_t *_pReadBuf, uint16_t _usSize)
{
	static uint16_t usPBN;			/*������� */
	uint32_t ulPhyPageNo;	/*����ҳ�� */
	uint16_t usAddrInPage;	/*ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/*���ݳ��ȱ�����4�ֽ������� */
	if ((_usSize % 4) != 0)
	{
		return NAND_FAIL;
	}

	usPBN = NAND_AddrToPhyBlockNo(_ulMemAddr);	/*��ѯLUT������������ */
	if (usPBN >= NAND_BLOCK_COUNT)
	{
		/*û�и�ʽ����usPBN = 0xFFFF */
		return NAND_FAIL;
	}

	ulTemp = _ulMemAddr % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
	ulPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/*��������ҳ�� */
	usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/*����ҳ��ƫ�Ƶ�ַ */

	if (FSMC_NAND_ReadData((uint8_t *)_pReadBuf, ulPhyPageNo, usAddrInPage, _usSize) == NAND_FAIL)
	{
		return NAND_FAIL;	/*��NAND Flashʧ�� */
	}

	/*�ɹ� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_WriteMultiSectors
*	����˵��: �ú��������ļ�ϵͳ������д����������ݡ�������С������512�ֽڻ�2048�ֽ�
*	��    ��:  	_pBuf : ������ݵĻ�������ָ��
*				_SectorNo ��������
*				_SectorSize ��ÿ�������Ĵ�С
*				_SectorCount : ��������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_WriteMultiSectors(uint8_t *_pBuf, uint32_t _SectorNo, uint16_t _SectorSize, uint32_t _SectorCount)
{
	uint32_t i;
	uint32_t usLBN;			/*�߼���� */
	uint32_t usPBN;			/*������� */
	uint32_t uiPhyPageNo;	/*����ҳ�� */
	uint16_t usAddrInPage;	/*ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;
	uint8_t ucReturn;

	/*
		HY27UF081G2A = 128M Flash.  �� 1024��BLOCK, ÿ��BLOCK����64��PAGE�� ÿ��PAGE����2048+64�ֽڣ�
		������С��λ��BLOCK�� �����С��λ���ֽڡ�

		ÿ��PAGE���߼��Ͽ��Է�Ϊ4��512�ֽ�������
	*/

	for (i = 0; i < _SectorCount; i++)
	{
		/*�����߼������ź�������С�����߼���� */
		//usLBN = (_SectorNo * _SectorSize) / (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
		/*(_SectorNo * _SectorSize) �˻����ܴ���32λ����˻���������д�� */
		usLBN = (_SectorNo + i) / (NAND_BLOCK_SIZE * (NAND_PAGE_SIZE / _SectorSize));
		usPBN = NAND_LBNtoPBN(usLBN);	/*��ѯLUT������������ */
		if (usPBN >= NAND_BLOCK_COUNT)
		{
			/*û�и�ʽ����usPBN = 0xFFFF */
			return NAND_FAIL;
		}

		//ulTemp = ((uint64_t)(_SectorNo + i) * _SectorSize) % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
		ulTemp = ((_SectorNo + i) % (NAND_BLOCK_SIZE * (NAND_PAGE_SIZE / _SectorSize))) *  _SectorSize;
		uiPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/*��������ҳ�� */
		usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/*����ҳ��ƫ�Ƶ�ַ */

		/*��� _SectorCount > 0, ������ҳ���׵�ַ������Խ����Ż� */
		if (usAddrInPage == 0)
		{
			/*��δ���� */
		}

		/*�������������ݣ��ж��Ƿ�ȫFF */
		if (FSMC_NAND_ReadData(s_ucTempBuf, uiPhyPageNo, usAddrInPage, _SectorSize) == NAND_FAIL)
		{
			return NAND_FAIL;	/*ʧ�� */
		}

		/*�������ȫ0xFF, �����ֱ��д�룬������� */
		if (NAND_IsBufOk(s_ucTempBuf, _SectorSize, 0xFF) == NAND_OK)
		{
			if (FSMC_NAND_WriteData(&_pBuf[i * _SectorSize], uiPhyPageNo, usAddrInPage, _SectorSize) == NAND_FAIL)
			{
				/*������д�뵽����һ���飨���п飩 */
				ucReturn = NAND_WriteToNewBlock(uiPhyPageNo, &_pBuf[i * _SectorSize], usAddrInPage, _SectorSize);
				if (ucReturn != NAND_OK)
				{
					return NAND_FAIL;	/*ʧ�� */
				}
				continue;
			}

			/*��Ǹÿ����� */
			if (NAND_MarkUsedBlock(uiPhyPageNo) == NAND_FAIL)
			{
				/*���ʧ�ܣ�������д�뵽����һ���飨���п飩 */
				ucReturn = NAND_WriteToNewBlock(uiPhyPageNo, &_pBuf[i * _SectorSize], usAddrInPage, _SectorSize);
				if (ucReturn != NAND_OK)
				{
					return NAND_FAIL;	/*ʧ�� */
				}
				continue;
			}
		}
		else	/*Ŀ�������Ѿ������ݣ�����ȫFF, ��ֱ�ӽ�����д������һ�����п� */
		{
			/*������д�뵽����һ���飨���п飩 */
			ucReturn = NAND_WriteToNewBlock(uiPhyPageNo, &_pBuf[i * _SectorSize], usAddrInPage, _SectorSize);
			if (ucReturn != NAND_OK)
			{
				return NAND_FAIL;	/*ʧ�� */
			}
			continue;
		}
	}
	return NAND_OK;		/*�ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadMultiSectors
*	����˵��: �ú��������ļ�ϵͳ�������������ݡ���1������������������С������512�ֽڻ�2048�ֽ�
*	��    ��:  	_pBuf : ��Ŷ������ݵĻ�������ָ��
*				_SectorNo ��������
*				_SectorSize ��ÿ�������Ĵ�С
*				_SectorCount : ��������
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t NAND_ReadMultiSectors(uint8_t *_pBuf, uint32_t _SectorNo, uint16_t _SectorSize, uint32_t _SectorCount)
{
	uint32_t i;
	uint32_t usLBN;			/*�߼���� */
	uint32_t usPBN;			/*������� */
	uint32_t uiPhyPageNo;	/*����ҳ�� */
	uint16_t usAddrInPage;	/*ҳ��ƫ�Ƶ�ַ */
	uint32_t ulTemp;

	/*
		HY27UF081G2A = 128M Flash.  �� 1024��BLOCK, ÿ��BLOCK����64��PAGE�� ÿ��PAGE����2048+64�ֽڣ�
		������С��λ��BLOCK�� �����С��λ���ֽڡ�

		ÿ��PAGE���߼��Ͽ��Է�Ϊ4��512�ֽ�������
	*/

	for (i = 0; i < _SectorCount; i++)
	{
		/*�����߼������ź�������С�����߼���� */
		//usLBN = (_SectorNo * _SectorSize) / (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
		/*(_SectorNo * _SectorSize) �˻����ܴ���32λ����˻���������д�� */
		usLBN = (_SectorNo + i) / (NAND_BLOCK_SIZE * (NAND_PAGE_SIZE / _SectorSize));
		usPBN = NAND_LBNtoPBN(usLBN);	/*��ѯLUT������������ */
		if (usPBN >= NAND_BLOCK_COUNT)
		{
			/*û�и�ʽ����usPBN = 0xFFFF */
			return NAND_FAIL;
		}

		ulTemp = ((uint64_t)(_SectorNo + i) * _SectorSize) % (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
		uiPhyPageNo = usPBN * NAND_BLOCK_SIZE + ulTemp / NAND_PAGE_SIZE;	/*��������ҳ�� */
		usAddrInPage = ulTemp % NAND_PAGE_SIZE;	/*����ҳ��ƫ�Ƶ�ַ */

		if (FSMC_NAND_ReadData((uint8_t *)&_pBuf[i * _SectorSize], uiPhyPageNo, usAddrInPage, _SectorSize) == NAND_FAIL)
		{
			return NAND_FAIL;	/*��NAND Flashʧ�� */
		}
	}

	/*�ɹ� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_BuildLUT
*	����˵��: ���ڴ��д������������
*	��    ��:  ZoneNbr ������
*	�� �� ֵ: NAND_OK�� �ɹ��� 	NAND_FAIL��ʧ��
*********************************************************************************************************
*/
static uint8_t NAND_BuildLUT(void)
{
	uint16_t i;
	uint8_t buf[VALID_SPARE_SIZE];
	uint16_t usLBN;	/* �߼���� */

	/* */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		s_usLUT[i] = 0xFFFF;	/* �����Чֵ�������ؽ�LUT���ж�LUT�Ƿ���� */
	}
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/* ��ÿ����ĵ�1��PAGE��ƫ�Ƶ�ַΪLBN0_OFFSET������ */
		FSMC_NAND_ReadSpare(buf, i * NAND_BLOCK_SIZE, 0, VALID_SPARE_SIZE);

		/* ����Ǻÿ飬���¼LBN0 LBN1 */
		if (buf[BI_OFFSET] == 0xFF)
		{
			usLBN = buf[LBN0_OFFSET] + buf[LBN1_OFFSET] * 256;	/* ����������߼���� */
			if (usLBN < NAND_BLOCK_COUNT)
			{
				/* ����Ѿ��Ǽǹ��ˣ����ж�Ϊ�쳣 */
				if (s_usLUT[usLBN] != 0xFFFF)
				{
						return NAND_FAIL;
				}

				s_usLUT[usLBN] = i;	/* ����LUT�� */
			}
		}
	}

	/* LUT������ϣ�����Ƿ���� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (s_usLUT[i] >= NAND_BLOCK_COUNT)
		{
			s_usValidDataBlockCount = i;
			break;
		}
	}
	if (s_usValidDataBlockCount < 100)
	{
		/* ���� ������Ч�߼����С��100��������û�и�ʽ�� */
		return NAND_FAIL;
	}
	for (; i < s_usValidDataBlockCount; i++)
	{
		if (s_usLUT[i] != 0xFFFF)
		{
			return NAND_FAIL;	/* ����LUT���߼���Ŵ�����Ծ���󣬿�����û�и�ʽ�� */
		}
	}

	/* �ؽ�LUT���� */
	return NAND_OK;
}
/*
*********************************************************************************************************
*	�� �� ��: NAND_AddrToPhyBlockNo
*	����˵��: �ڴ��߼���ַת��Ϊ�������
*	��    ��:  _ulMemAddr���߼��ڴ��ַ
*	�� �� ֵ: ����ҳ�ţ� ����� 0xFFFFFFFF ���ʾ����
*********************************************************************************************************
*/
static uint16_t NAND_AddrToPhyBlockNo(uint32_t _ulMemAddr)
{
	uint16_t usLBN;		/*�߼���� */
	uint16_t usPBN;		/*������� */

	usLBN = _ulMemAddr / (NAND_BLOCK_SIZE * NAND_PAGE_SIZE);	/*�����߼���� */
	/*����߼���Ŵ�����Ч�����ݿ������̶�����0xFFFF, ���øú����Ĵ���Ӧ�ü������ִ��� */
	if (usLBN >= s_usValidDataBlockCount)
	{
		return 0xFFFF;
	}
	/*��ѯLUT�������������� */
	usPBN = s_usLUT[usLBN];
	return usPBN;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_LBNtoPBN
*	����˵��: �߼����ת��Ϊ�������
*	��    ��: _uiLBN : �߼���� Logic Block No
*	�� �� ֵ: ������ţ� ����� 0xFFFFFFFF ���ʾ����
*********************************************************************************************************
*/
static uint16_t NAND_LBNtoPBN(uint32_t _uiLBN)
{
	uint16_t usPBN;		/*������� */

	/*����߼���Ŵ�����Ч�����ݿ������̶�����0xFFFF, ���øú����Ĵ���Ӧ�ü������ִ��� */
	if (_uiLBN >= s_usValidDataBlockCount)
	{
		return 0xFFFF;
	}
	/*��ѯLUT�������������� */
	usPBN = s_usLUT[_uiLBN];
	return usPBN;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_FindFreeBlock
*	����˵��: �����һ���鿪ʼ������һ�����õĿ顣
*	��    ��:  ZoneNbr ������
*	�� �� ֵ: ��ţ������0xFFFF��ʾʧ��
*********************************************************************************************************
*/
static uint16_t NAND_FindFreeBlock (void)
{
	uint16_t i;
	uint16_t n;

	n = NAND_BLOCK_COUNT - 1;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (NAND_IsFreeBlock(n))
		{
			return n;
		}
		n--;
	}
	return 0xFFFF;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsBufOk
*	����˵��: �ж��ڴ滺�����������Ƿ�ȫ��Ϊָ��ֵ
*	��    ��:  - _pBuf : ���뻺����
*			  - _ulLen : ����������
*			  - __ucValue : ������ÿ����Ԫ����ȷ��ֵ
*	�� �� ֵ: 1 ��ȫ����ȷ�� 0 ������ȷ
*********************************************************************************************************
*/
static uint8_t NAND_IsBufOk(uint8_t *_pBuf, uint32_t _ulLen, uint8_t _ucValue)
{
	uint32_t i;

	for (i = 0; i < _ulLen; i++)
	{
		if (_pBuf[i] != _ucValue)
		{
			return NAND_FAIL;
		}
	}

	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsBadBlock
*	����˵��: ���ݻ����Ǽ��NAND Flashָ���Ŀ��Ƿ񻵿�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: 0 ���ÿ���ã� 1 ���ÿ��ǻ���
*********************************************************************************************************
*/
 uint8_t NAND_IsBadBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;

	/*���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	FSMC_NAND_ReadSpare(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, BI_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}

	FSMC_NAND_ReadSpare(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE + 1, BI_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}
	return 0;	/*�Ǻÿ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsFreeBlock
*	����˵��: ���ݻ����Ǻ�USED��־����Ƿ���ÿ�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: 1 ���ÿ���ã� 0 ���ÿ��ǻ��������ռ��
*********************************************************************************************************
*/
static uint8_t NAND_IsFreeBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;

	/*���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	if (NAND_IsBadBlock(_ulBlockNo))
	{
		return 0;
	}

	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, USED_OFFSET, 1);
	if (ucFlag == 0xFF)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ScanBlock
*	����˵��: ɨ�����NAND Flashָ���Ŀ�
*			��ɨ������㷨��
*			1) ��1���飨�������������ͱ����������������������Ƿ�ȫ0xFF, ��ȷ�Ļ��������Ըÿ飬����ÿ�
				�ǻ���,��������
*			2) ��ǰ��д��ȫ 0x00��Ȼ���ȡ��⣬��ȷ�Ļ��������ԸĿ飬�����˳�
*			3) �ظ��ڣ�2���������ѭ��������50�ζ�û�з���������ô�ÿ�����,�������أ�����ÿ��ǻ��飬
*				��������
*			��ע�⡿
*			1) �ú���������Ϻ󣬻�ɾ�������������ݣ�����Ϊȫ0xFF;
*			2) �ú������˲������������⣬Ҳ�Ա������������в��ԡ�
*			3) ��д����ѭ���������Ժ�ָ����#define BAD_BALOK_TEST_CYCLE 50
*	��    ��:  _ulPageNo ��ҳ�� 0 - 65535 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: NAND_OK ���ÿ���ã� NAND_FAIL ���ÿ��ǻ���
*********************************************************************************************************
*/
uint8_t NAND_ScanBlock(uint32_t _ulBlockNo)
{
	uint32_t i, k;
	uint32_t ulPageNo;

	#if 1
	/*���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	if (NAND_IsBadBlock(_ulBlockNo))
	{
		return NAND_FAIL;
	}
	#endif

	/*����Ĵ��뽫ͨ��������������̵ķ�ʽ������NAND Flashÿ����Ŀɿ��� */
	memset(s_ucTempBuf, 0x00, NAND_PAGE_TOTAL_SIZE);
	for (i = 0; i < BAD_BALOK_TEST_CYCLE; i++)
	{
		/*��1������������� */
		if (FSMC_NAND_EraseBlock(_ulBlockNo) != NAND_READY)
		{
			return NAND_FAIL;
		}

		/*��2������������ÿ��page�����ݣ����ж��Ƿ�ȫ0xFF */
		ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/*����ÿ��1��ҳ��ҳ�� */
		for (k = 0; k < NAND_BLOCK_SIZE; k++)
		{
			/*������ҳ���� */
			FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);

			/*�жϴ洢��Ԫ�ǲ���ȫ0xFF */
			if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0xFF) != NAND_OK)
			{
				return NAND_FAIL;
			}

			ulPageNo++;		/*����д��һ��ҳ */
		}

		/*��2����дȫ0���������ж��Ƿ�ȫ0 */
		ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/*����ÿ��1��ҳ��ҳ�� */
		for (k = 0; k < NAND_BLOCK_SIZE; k++)
		{
			/*���buf[]������Ϊȫ0,��д��NAND Flash */
			memset(s_ucTempBuf, 0x00, NAND_PAGE_TOTAL_SIZE);
			if (FSMC_NAND_WritePage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE) != NAND_OK)
			{
				return NAND_FAIL;
			}

			/*������ҳ����, �жϴ洢��Ԫ�ǲ���ȫ0x00 */
			FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);
			if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0x00) != NAND_OK)
			{
				return NAND_FAIL;
			}

			ulPageNo++;		/*����һ��ҳ */
		}
	}

	/*���һ�������������� */
	if (FSMC_NAND_EraseBlock(_ulBlockNo) != NAND_READY)
	{
		return NAND_FAIL;
	}
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/*����ÿ��1��ҳ��ҳ�� */
	for (k = 0; k < NAND_BLOCK_SIZE; k++)
	{
		/*������ҳ���� */
		FSMC_NAND_ReadPage(s_ucTempBuf, ulPageNo, 0, NAND_PAGE_TOTAL_SIZE);

		/*�жϴ洢��Ԫ�ǲ���ȫ0xFF */
		if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_TOTAL_SIZE, 0xFF) != NAND_OK)
		{
			return NAND_FAIL;
		}

		ulPageNo++;		/*����д��һ��ҳ */
	}

	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_MarkUsedBlock
*	����˵��: ���NAND Flashָ���Ŀ�Ϊ���ÿ�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: NAND_OK:��ǳɹ��� NAND_FAIL�����ʧ�ܣ��ϼ�����Ӧ�ý��л��鴦����
*********************************************************************************************************
*/
static uint8_t NAND_MarkUsedBlock(uint32_t _ulBlockNo)
{
	uint32_t ulPageNo;
	uint8_t ucFlag;

	/*�����ĵ�1��ҳ�� */
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/*����ÿ��1��ҳ��ҳ�� */

	/*���ڵ�1��page�������ĵ�6���ֽ�д���0xFF���ݱ�ʾ���� */
	ucFlag = NAND_USED_BLOCK_FLAG;
	if (FSMC_NAND_WriteSpare(&ucFlag, ulPageNo, USED_OFFSET, 1) == NAND_FAIL)
	{
		/*������ʧ�ܣ�����Ҫ��ע�����Ϊ���� */
		return NAND_FAIL;
	}
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_MarkBadBlock
*	����˵��: ���NAND Flashָ���Ŀ�Ϊ����
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: �̶�NAND_OK
*********************************************************************************************************
*/
static void NAND_MarkBadBlock(uint32_t _ulBlockNo)
{
	uint32_t ulPageNo;
	uint8_t ucFlag;

	/*�����ĵ�1��ҳ�� */
	ulPageNo = _ulBlockNo * NAND_BLOCK_SIZE;	/*����ÿ��1��ҳ��ҳ�� */

	/*���ڵ�1��page�������ĵ�6���ֽ�д���0xFF���ݱ�ʾ���� */
	ucFlag = NAND_BAD_BLOCK_FLAG;
	if (FSMC_NAND_WriteSpare(&ucFlag, ulPageNo, BI_OFFSET, 1) == NAND_FAIL)
	{
		/*�����1��ҳ���ʧ�ܣ����ڵ�2��ҳ��� */
		FSMC_NAND_WriteSpare(&ucFlag, ulPageNo + 1, BI_OFFSET, 1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Format
*	����˵��: NAND Flash��ʽ�����������е����ݣ��ؽ�LUT
*	��    ��:  ��
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint8_t NAND_Format(void)
{
	uint16_t i, n;
	static uint16_t usGoodBlockCount;

	/*����ÿ���� */
	usGoodBlockCount = 0;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/*����Ǻÿ飬����� */
		if (!NAND_IsBadBlock(i))
		{
			FSMC_NAND_EraseBlock(i);
			usGoodBlockCount++;
		}
	}

	/*����ÿ����������100����NAND Flash���� */
	if (usGoodBlockCount < 100)
	{
		return NAND_FAIL;
	}

	usGoodBlockCount = (usGoodBlockCount * 98) / 100;	/*98%�ĺÿ����ڴ洢���� */

	/*��������һ�� */
	n = 0; /*ͳ���ѱ�ע�ĺÿ� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (!NAND_IsBadBlock(i))
		{
			/*����Ǻÿ飬���ڸÿ�ĵ�1��PAGE��LBN0 LBN1��д��nֵ (ǰ���Ѿ�ִ���˿������ */
			FSMC_NAND_WriteSpare((uint8_t *)&n, i * NAND_BLOCK_SIZE, LBN0_OFFSET, 2);
			n++;

			/*���㲢д��ÿ��������ECCֵ ����ʱδ����*/

			if (n == usGoodBlockCount)
			{
				break;
			}
		}
	}

	NAND_BuildLUT();	/*��ʼ��LUT�� */
	return NAND_OK;
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_FormatCapacity
*	����˵��: NAND Flash��ʽ�������Ч����
*	��    ��:  ��
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint32_t NAND_FormatCapacity(void)
{
	uint16_t usCount;

	/*�������ڴ洢���ݵ����ݿ��������������Ч������98%������ */
	usCount = (s_usValidDataBlockCount * DATA_BLOCK_PERCENT) / 100;

	return (usCount * NAND_BLOCK_SIZE * NAND_PAGE_SIZE);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_DispBadBlockInfo
*	����˵��: ͨ�����ڴ�ӡ��NAND Flash�Ļ�����Ϣ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_DispBadBlockInfo(void)
{
	uint32_t id;
	uint32_t i;
	uint32_t n;
//	uint8_t str[20];

	FSMC_NAND_Ini();	/*��ʼ��FSMC */

	id = NAND_ReadID();

	////printf("NAND Flash ID = 0x%04X, Type = ", id);
	if (id == HY27UF081G2A)
	{
		////printf("HY27UF081G2A\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else if (id == K9F1G08U0A)
	{
		////printf("K9F1G08U0A\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else if (id == K9F1G08U0B)
	{
		////printf("K9F1G08U0B\r\n  1024 Blocks, 64 pages per block, 2048 + 64 bytes per page\r\n");
	}
	else
	{
		////printf("unkonow\r\n");
		return;
	}

	////printf("Block Info : 0 is OK, * is Bad\r\n");
	n = 0;	/*����ͳ�� */
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		if (NAND_IsBadBlock(i))
		{
			//printf("*");
			n++;	
			//sprintf(str,"Bad Block N0. = %d\r\n", i);
			//novar_print(str,strlen(str));
		}
		else
		{
			//printf("0");
		}

		if (((i + 1) % 8) == 0)
		{
			//printf(" ");
		}

		if (((i + 1) % 64) == 0)
		{
			//printf("\r\n");
		}
	}

}

/*
*********************************************************************************************************
*	�� �� ��: NAND_DispPhyPageData
*	����˵��: ͨ�����ڴ�ӡ��ָ��ҳ�����ݣ�2048+64��
*	��    ��: _uiPhyPageNo �� ����ҳ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_DispPhyPageData(uint32_t _uiPhyPageNo)
{
	uint32_t i, n;
//	uint32_t ulBlockNo;
//	uint16_t usOffsetPageNo;

//	ulBlockNo = _uiPhyPageNo / NAND_BLOCK_SIZE;		/*��������ҳ�ŷ��ƿ�� */
//	usOffsetPageNo = _uiPhyPageNo % NAND_BLOCK_SIZE;	/*��������ҳ�ż�������ҳ���ڿ���ƫ��ҳ�� */

	if (NAND_OK != FSMC_NAND_ReadPage(s_ucTempBuf, _uiPhyPageNo, 0, NAND_PAGE_TOTAL_SIZE))
	{
		//printf("FSMC_NAND_ReadPage Failed() \r\n");
		return;
	}

	//printf("Block = %d, Page = %d\r\n", ulBlockNo, usOffsetPageNo);

	/*��ӡǰ�� 2048�ֽ����ݣ�ÿ512�ֽڿ�һ�� */
	for (n = 0; n < 4; n++)
	{
		for (i = 0; i < 512; i++)
		{
			//printf(" %02X", s_ucTempBuf[i + n * 512]);

			if ((i & 31) == 31)
			{
				//printf("\r\n");	/*ÿ����ʾ32�ֽ����� */
			}
			else if ((i & 31) == 15)
			{
				//printf(" - ");
			}
		}
		//printf("\r\n");
	}

	/*��ӡǰ�� 2048�ֽ����ݣ�ÿ512�ֽڿ�һ�� */
	for (i = 0; i < 64; i++)
	{
		//printf(" %02X", s_ucTempBuf[i + 2048]);

		if ((i & 15) == 15)
		{
			//printf("\r\n");	/*ÿ����ʾ32�ֽ����� */
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_DispLogicPageData
*	����˵��: ͨ�����ڴ�ӡ��ָ��ҳ�����ݣ�2048+64��
*	��    ��: _uiLogicPageNo �� �߼�ҳ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_DispLogicPageData(uint32_t _uiLogicPageNo)
{
	uint32_t uiPhyPageNo;
	uint16_t usLBN;	/*�߼���� */
	uint16_t usPBN;	/*������� */

	usLBN = _uiLogicPageNo / NAND_BLOCK_SIZE;
	usPBN = NAND_LBNtoPBN(usLBN);	/*��ѯLUT������������ */
	if (usPBN >= NAND_BLOCK_COUNT)
	{
		/*û�и�ʽ����usPBN = 0xFFFF */
		return;
	}

	//printf("LogicBlock = %d, PhyBlock = %d\r\n", _uiLogicPageNo, usPBN);

	/*��������ҳ�� */
	uiPhyPageNo = usPBN * NAND_BLOCK_SIZE + _uiLogicPageNo % NAND_BLOCK_SIZE;
	NAND_DispPhyPageData(uiPhyPageNo);	/*��ʾָ��ҳ���� */
}


/*
*********************************************************************************************************
*	�� �� ��: DATREC_EXC
*	����˵��: ��¼��ϻ������
*   ��    ��:  	uint32_t *ptr ���� ��ϻ������д��ָ�룬ΪBLOCKBAD_SIZE������������ǰһ����¼Ϊ256Bytes
*   							   ��ָ��ָ���ַΪNAND Flashͳһ��ַ�룬��Χ0 ~ 1024 * 64 * 2048
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/

uint8_t DATREC_EXC(uint32_t *ptr)
{
	uint32_t blkno;
	uint32_t pageno;
	uint16_t offset;
	static uint16_t i,len;
	uint8_t _pWriteBuf[BLACKBOX_SIZE],_pReadBuf[BLACKBOX_SIZE];

	// ͳһ��ַ����ɿ�+ҳ+ƫ�Ƶ�ַ
	blkno = *ptr / (NAND_PAGE_SIZE * NAND_BLOCK_SIZE);
	if (blkno >= NAND_ZONE_SIZE)
	{
		blkno = 0;
		*ptr = 0;
	}
	pageno = (*ptr % (NAND_PAGE_SIZE * NAND_BLOCK_SIZE)) / NAND_PAGE_SIZE;
	offset = (*ptr % (NAND_PAGE_SIZE * NAND_BLOCK_SIZE)) % NAND_PAGE_SIZE;

	// �жϻ��������Ƿ񳬹�24�飬�������ֱ���˳�
	

	// �жϵ�ǰƫ�Ƶ�ַȷ��дָ���Ƿ�Ϊ��ǰ���׵�ַ

	if (pageno == 0 && offset == 0)
	{
		// дָ��Ϊ���׵�ַ

		// �жϵ�ǰ���Ƿ�Ϊ����
		if (NAND_IsBadBlock(blkno))
		{
			*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
			return NAND_FAIL;
		}
		else
		{
			// �ж������Ƿ�ȫΪ0xff
			for (i = 0; i < NAND_BLOCK_SIZE; i++)
			{
				/*������ҳ���� */
				FSMC_NAND_ReadPage(s_ucTempBuf, blkno * NAND_BLOCK_SIZE + i, 0, NAND_PAGE_SIZE);

				/*�жϴ洢��Ԫ�ǲ���ȫ0xFF */
				if (NAND_IsBufOk(s_ucTempBuf, NAND_PAGE_SIZE, 0xFF) != NAND_OK)
				{
					FSMC_NAND_EraseBlock(blkno);
					break;
				}
			}
		}

	}

	// 
	/*�������������ݣ��ж��Ƿ�ȫFF */
	if (FSMC_NAND_ReadData(s_ucTempBuf, blkno * NAND_BLOCK_SIZE + pageno, offset, BLACKBOX_SIZE) == NAND_FAIL)
	{
		*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
		NAND_MarkBadBlock(blkno);	/*�¿���Ϊ���� */
		return NAND_FAIL;	/*��NAND Flashʧ�� */
	}

	/*�������ȫ0xFF, �����ֱ��д�룬������� */
	if (NAND_IsBufOk(s_ucTempBuf, BLACKBOX_SIZE, 0xFF) == NAND_OK)
	{
		memset(_pWriteBuf, 0xFF, BLACKBOX_SIZE);
		len = sizeof(APP_PRDVALUE_TypeDef) / sizeof(uint8_t);
		memcpy(_pWriteBuf, &save_value, len);

		if (FSMC_NAND_WriteData((uint8_t *)_pWriteBuf, blkno * NAND_BLOCK_SIZE + pageno, offset, BLACKBOX_SIZE) == NAND_FAIL)
		{
			*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
			NAND_MarkBadBlock(blkno);	/*���Ϊ���� */
			return NAND_FAIL;
		}

		if (FSMC_NAND_ReadData((uint8_t *)_pReadBuf, blkno * NAND_BLOCK_SIZE + pageno, offset, BLACKBOX_SIZE) == NAND_FAIL)
		{
			*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
			NAND_MarkBadBlock(blkno);	/*���Ϊ���� */
			return NAND_FAIL;
		}

		for (i = 0; i < BLACKBOX_SIZE;i++)
		{
			if (_pWriteBuf[i] != _pReadBuf[i])
			{
				*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
				NAND_MarkBadBlock(blkno);	/*���Ϊ���� */
				return NAND_FAIL;
			}
		}

		*ptr += BLACKBOX_SIZE;
		return NAND_OK; /*д��ɹ� */
	}
	else
	{
		*ptr = (blkno + 1) * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;  // ������һ�����׵�ַ
		NAND_MarkBadBlock(blkno);	/*�¿���Ϊ���� */
		return NAND_FAIL;	/*��NAND Flashʧ�� */
	}
}

#define BUFFER_SIZE 512
void APP_NAND_TEST(void)
{
	uint32_t i;
	//uint32_t nand_id;
	uint8_t status;
	uint32_t TxBuffer[BUFFER_SIZE],RxBuffer[BUFFER_SIZE];

/*
    if (NAND_Init() == NAND_OK){}
    else                         
    {                            
        NAND_Format();           
    }                            
                                 
    if (status == NAND_OK)       
*/
	{
		//nand_id = NAND_ReadID();

		for (i = 0; i < BUFFER_SIZE; i++)	//��������Բ���
		{
			TxBuffer[i] = i;
			RxBuffer[i] = 0;
		}
		status = NAND_Read(2048, RxBuffer, BUFFER_SIZE*2);
		status = NAND_Write(2048, TxBuffer, BUFFER_SIZE*2);

		if (status == NAND_OK)
		{
			status = NAND_Read(2048, RxBuffer, BUFFER_SIZE*2);
		}
	}
}

// novar modify
void APP_NAND_MDF(void)
{
	uint16_t i;

	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		//FSMC_NAND_EraseBlock(i);

		if (!NAND_IsBadBlock(i))
		{
			/*����Ǻÿ飬���ڸÿ�ĵ�1��PAGE��LBN0 LBN1��д��nֵ (ǰ���Ѿ�ִ���˿������ */
			FSMC_NAND_WriteSpare((uint8_t *)&i, i * NAND_BLOCK_SIZE, LBN0_OFFSET, 2);
		}
	}
}
/***************************** (END OF FILE) *********************************/