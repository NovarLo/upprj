/*
*********************************************************************************************************
*
*	ģ������ : ������������ģ��
*	�ļ����� : bsp_key.c
*	��    �� : V1.0
*	˵    �� : ɨ�������������������˲����ƣ����а���FIFO�����Լ�������¼���
*				(1) ��������
*				(2) ��������
*				(3) ������
*				(4) ����ʱ�Զ�����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*
*
*	Copyright (C), 2018-2020, ̩�²������ܿƼ� http://www.htachina.com/
*
*********************************************************************************************************
*/

#include "stm32f4xx_hal.h"
#include "app_user.h"
#include "app_key.h"
#include "app_sensor.h"

/*
	�ó�������̩�²������İ�

	�����������Ӳ�������޸�GPIO����� IsKeyDown1 - IsKeyDown8 ����

	����û��İ�������С��8��������Խ�����İ���ȫ������Ϊ�͵�1������һ��������Ӱ�������
	#define KEY_COUNT    8	  ����� bsp_key.h �ļ��ж���
*/

#ifdef STM32_TXBK		/* ̩�²������İ� */
/*

		K1��       : PA1  (�͵�ƽ��ʾ����)
		K2��       : PA1  (�͵�ƽ��ʾ����)
		K3��       : PA1  (�͵�ƽ��ʾ����)
		ҡ��UP��   : PA1  (�͵�ƽ��ʾ����)
		ҡ��DOWN�� : PA1  (�͵�ƽ��ʾ����)
		ҡ��LEFT�� : PA1  (�͵�ƽ��ʾ����)
		ҡ��RIGHT��: PA1  (�͵�ƽ��ʾ����)
		ҡ��OK��   : PA1  (�͵�ƽ��ʾ����)
*/
//#define RCC_ALL_KEY 	(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC)	/* �����ڶ�Ӧ��RCCʱ�� */

#define GPIO_PORT_K1    GPIOA
#define GPIO_PIN_K1	    GPIO_PIN_1

#define GPIO_PORT_K2    GPIOA
#define GPIO_PIN_K2	    GPIO_PIN_3

#define GPIO_PORT_K3    GPIOA
#define GPIO_PIN_K3	    GPIO_PIN_3

#define GPIO_PORT_K4    GPIOA
#define GPIO_PIN_K4	    GPIO_PIN_3

#define GPIO_PORT_K5    GPIOA
#define GPIO_PIN_K5	    GPIO_PIN_3

#define GPIO_PORT_K6    GPIOA
#define GPIO_PIN_K6	    GPIO_PIN_3

#define GPIO_PORT_K7    GPIOA
#define GPIO_PIN_K7	    GPIO_PIN_3

#define GPIO_PORT_K8    GPIOA
#define GPIO_PIN_K8	    GPIO_PIN_3

#else	/* STM32_XX */
/*
	������STM32-XX �������߷��䣺
		K1 ��      : Pxx  (�͵�ƽ��ʾ����)
		K2 ��      : Pxx  (�͵�ƽ��ʾ����)
		K3 ��      : Pxx  (�͵�ƽ��ʾ����)
		ҡ��UP��   : Pxx  (�͵�ƽ��ʾ����)
		ҡ��DOWN�� : Pxx  (�͵�ƽ��ʾ����)
		ҡ��LEFT�� : Pxx  (�͵�ƽ��ʾ����)
		ҡ��RIGHT��: Pxx  (�͵�ƽ��ʾ����)
		ҡ��OK��   : Pxx  (�͵�ƽ��ʾ����)
*/

/* �����ڶ�Ӧ��RCCʱ�� */
//#define RCC_ALL_KEY 	(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOG)

#define GPIO_PORT_K1    GPIOA
#define GPIO_PIN_K1	    GPIO_PIN_1

#define GPIO_PORT_K2    GPIOA
#define GPIO_PIN_K2	    GPIO_PIN_1

#define GPIO_PORT_K3    GPIOA
#define GPIO_PIN_K3	    GPIO_PIN_1

#define GPIO_PORT_K4    GPIOA
#define GPIO_PIN_K4	    GPIO_PIN_1

#define GPIO_PORT_K5    GPIOA
#define GPIO_PIN_K5	    GPIO_PIN_1

#define GPIO_PORT_K6    GPIOA
#define GPIO_PIN_K6	    GPIO_PIN_1

#define GPIO_PORT_K7    GPIOA
#define GPIO_PIN_K7	    GPIO_PIN_1

#define GPIO_PORT_K8    GPIOA
#define GPIO_PIN_K8	    GPIO_PIN_1
#endif

static KEY_T s_tBtn[KEY_COUNT];
static KEY_FIFO_T s_tKey;       /* ����FIFO����,�ṹ�� */

static void bsp_InitKeyVar(void);
static void bsp_InitKeyHard(void);
static void bsp_DetectKey(uint8_t i);

/*
*********************************************************************************************************
*	�� �� ��: IsKeyDownX
*	����˵��: �жϰ����Ƿ���
*	��    ��: ��
*	�� �� ֵ: ����ֵ1 ��ʾ���£�0��ʾδ����
*********************************************************************************************************
*/
#ifdef STM32_TXBK		/* ̩�²��� ���İ� */
static uint8_t IsKeyDown1(void) { if ((GPIO_PORT_K1->IDR & GPIO_PIN_K1) == 0) return 1;else return 0;}
static uint8_t IsKeyDown2(void) { if ((GPIO_PORT_K2->IDR & GPIO_PIN_K2) == 0) return 1;else return 0;}
static uint8_t IsKeyDown3(void) { if ((GPIO_PORT_K3->IDR & GPIO_PIN_K3) == 0) return 1;else return 0;}
static uint8_t IsKeyDown4(void) { if ((GPIO_PORT_K4->IDR & GPIO_PIN_K4) == 0) return 1;else return 0;}
static uint8_t IsKeyDown5(void) { if ((GPIO_PORT_K5->IDR & GPIO_PIN_K5) == 0) return 1;else return 0;}
static uint8_t IsKeyDown6(void) { if ((GPIO_PORT_K6->IDR & GPIO_PIN_K6) == 0) return 1;else return 0;}
static uint8_t IsKeyDown7(void) { if ((GPIO_PORT_K7->IDR & GPIO_PIN_K7) == 0) return 1;else return 0;}
static uint8_t IsKeyDown8(void) { if ((GPIO_PORT_K8->IDR & GPIO_PIN_K8) == 0) return 1;else return 0;}
#else				/*  ����Ӳ�� ���İ� */
static uint8_t IsKeyDown1(void) { if ((GPIO_PORT_K1->IDR & GPIO_PIN_K1) == 0) return 1;else return 0;}
static uint8_t IsKeyDown2(void) { if ((GPIO_PORT_K2->IDR & GPIO_PIN_K2) == 0) return 1;else return 0;}
static uint8_t IsKeyDown3(void) { if ((GPIO_PORT_K3->IDR & GPIO_PIN_K3) == 0) return 1;else return 0;}
static uint8_t IsKeyDown4(void) { if ((GPIO_PORT_K4->IDR & GPIO_PIN_K4) == 0) return 1;else return 0;}
static uint8_t IsKeyDown5(void) { if ((GPIO_PORT_K5->IDR & GPIO_PIN_K5) == 0) return 1;else return 0;}
static uint8_t IsKeyDown6(void) { if ((GPIO_PORT_K6->IDR & GPIO_PIN_K6) == 0) return 1;else return 0;}
static uint8_t IsKeyDown7(void) { if ((GPIO_PORT_K7->IDR & GPIO_PIN_K7) == 0) return 1;else return 0;}
static uint8_t IsKeyDown8(void) { if ((GPIO_PORT_K8->IDR & GPIO_PIN_K8) == 0) return 1;else return 0;}
#endif
static uint8_t IsKeyDown9(void) { if (IsKeyDown1() && IsKeyDown2()) return 1;else return 0;}
static uint8_t IsKeyDown10(void) { if (IsKeyDown1() && IsKeyDown2()) return 1;else return 0;}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitKey
*	����˵��: ��ʼ������. �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitKey(void)
{
	bsp_InitKeyVar();       /* ��ʼ���������� */
	bsp_InitKeyHard();      /* ��ʼ������Ӳ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_PutKey
*	����˵��: ��1����ֵѹ�밴��FIFO��������������ģ��һ��������
*	��    ��:  _KeyCode : ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_tKey.Buf[s_tKey.Write] = _KeyCode;

	if (++s_tKey.Write  >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetKey
*	����˵��: �Ӱ���FIFO��������ȡһ����ֵ��
*	��    ��:  ��
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetKey2
*	����˵��: �Ӱ���FIFO��������ȡһ����ֵ�������Ķ�ָ�롣
*	��    ��:  ��
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
uint8_t bsp_GetKey2(void)
{
	uint8_t ret;

	if (s_tKey.Read2 == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read2];

		if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
		{
			s_tKey.Read2 = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetKeyState
*	����˵��: ��ȡ������״̬
*	��    ��:  _ucKeyID : ����ID����0��ʼ
*	�� �� ֵ: 1 ��ʾ���£� 0 ��ʾδ����
*********************************************************************************************************
*/
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID)
{
	return s_tBtn[_ucKeyID].State;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetKeyParam
*	����˵��: ���ð�������
*	��    �Σ�_ucKeyID : ����ID����0��ʼ
*			_LongTime : �����¼�ʱ��
*			 _RepeatSpeed : �����ٶ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed)
{
	s_tBtn[_ucKeyID].LongTime = _LongTime;          /* ����ʱ�� 0 ��ʾ����ⳤ�����¼� */
	s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;            /* �����������ٶȣ�0��ʾ��֧������ */
	s_tBtn[_ucKeyID].RepeatCount = 0;                       /* ���������� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_ClearKey
*	����˵��: ��հ���FIFO������
*	��    �Σ���
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_ClearKey(void)
{
	s_tKey.Read = s_tKey.Write;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitKeyHard
*	����˵��: ���ð�����Ӧ��GPIO
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_InitKeyHard(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* ��1������GPIOʱ�� */
	__GPIOA_CLK_ENABLE();   // Ŀǰֻ��GPIOA

	/* ��2�����������еİ���GPIOΪ��������ģʽ(ʵ����CPU��λ���������״̬) */
	/* Configure GPIO pin : PA1 */
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     /* ��Ϊ����� */
	GPIO_InitStruct.Alternate = GPIO_MODE_AF_PP; /* ��Ϊ����ģʽ */
	GPIO_InitStruct.Pull = GPIO_NOPULL;         /* �������������� */
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;    /* IO������ٶ� */

	GPIO_InitStruct.Pin = GPIO_PIN_K1;
	HAL_GPIO_Init(GPIO_PORT_K1, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K2;
	HAL_GPIO_Init(GPIO_PORT_K2, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K3;
	HAL_GPIO_Init(GPIO_PORT_K3, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K4;
	HAL_GPIO_Init(GPIO_PORT_K4, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K5;
	HAL_GPIO_Init(GPIO_PORT_K5, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K6;
	HAL_GPIO_Init(GPIO_PORT_K6, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K7;
	HAL_GPIO_Init(GPIO_PORT_K7, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_K8;
	HAL_GPIO_Init(GPIO_PORT_K8, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitKeyVar
*	����˵��: ��ʼ����������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_InitKeyVar(void)
{
	uint8_t i;

	/* �԰���FIFO��дָ������ */
	s_tKey.Read = 0;
	s_tKey.Write = 0;
	s_tKey.Read2 = 0;

	/* ��ÿ�������ṹ���Ա������һ��ȱʡֵ */
	for (i = 0; i < KEY_COUNT; i++)
	{
		s_tBtn[i].LongTime = KEY_LONG_TIME;         /* ����ʱ�� 0 ��ʾ����ⳤ�����¼� */
		s_tBtn[i].Count = KEY_FILTER_TIME / 2;      /* ����������Ϊ�˲�ʱ���һ�� */
		s_tBtn[i].State = 0;                            /* ����ȱʡ״̬��0Ϊδ���� */
		//s_tBtn[i].KeyCodeDown = 3 * i + 1;				/* �������µļ�ֵ���� */
		//s_tBtn[i].KeyCodeUp   = 3 * i + 2;				/* ��������ļ�ֵ���� */
		//s_tBtn[i].KeyCodeLong = 3 * i + 3;				/* �������������µļ�ֵ���� */
		s_tBtn[i].RepeatSpeed = 0;                      /* �����������ٶȣ�0��ʾ��֧������ */
		s_tBtn[i].RepeatCount = 0;                      /* ���������� */
	}

	/* �����Ҫ��������ĳ�������Ĳ����������ڴ˵������¸�ֵ */
	/* ���磬����ϣ������1���³���1����Զ��ط���ͬ��ֵ */
	s_tBtn[KID_JOY_U].LongTime = 200;
	s_tBtn[KID_JOY_U].RepeatSpeed = 10;  /* ÿ��50ms�Զ����ͼ�ֵ */

	s_tBtn[KID_JOY_D].LongTime = 200;  
	s_tBtn[KID_JOY_D].RepeatSpeed = 10;  /* ÿ��50ms�Զ����ͼ�ֵ */

	s_tBtn[KID_JOY_L].LongTime = 200;  
	s_tBtn[KID_JOY_L].RepeatSpeed = 10;  /* ÿ��50ms�Զ����ͼ�ֵ */

	s_tBtn[KID_JOY_R].LongTime = 200;  
	s_tBtn[KID_JOY_R].RepeatSpeed = 10;  /* ÿ��50ms�Զ����ͼ�ֵ */

	/* �жϰ������µĺ��� */
	s_tBtn[0].IsKeyDownFunc = IsKeyDown1;
	s_tBtn[1].IsKeyDownFunc = IsKeyDown2;
	s_tBtn[2].IsKeyDownFunc = IsKeyDown3;
	s_tBtn[3].IsKeyDownFunc = IsKeyDown4;
	s_tBtn[4].IsKeyDownFunc = IsKeyDown5;
	s_tBtn[5].IsKeyDownFunc = IsKeyDown6;
	s_tBtn[6].IsKeyDownFunc = IsKeyDown7;
	s_tBtn[7].IsKeyDownFunc = IsKeyDown8;

	/* ��ϼ� */
	s_tBtn[8].IsKeyDownFunc = IsKeyDown9;
	s_tBtn[9].IsKeyDownFunc = IsKeyDown10;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DetectKey
*	����˵��: ���һ��������������״̬�����뱻�����Եĵ��á�
*	��    ��:  �����ṹ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_DetectKey(uint8_t i)
{
	KEY_T *pBtn;

	/*
		���û�г�ʼ�������������򱨴�
		if (s_tBtn[i].IsKeyDownFunc == 0)
		{
			printf("Fault : DetectButton(), s_tBtn[i].IsKeyDownFunc undefine");
		}
	*/

	pBtn = &s_tBtn[i];
	if (pBtn->IsKeyDownFunc())
	{
		if (pBtn->Count < KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count < 2 * KEY_FILTER_TIME)
		{
			pBtn->Count++;
		}
		else
		{
			if (pBtn->State == 0)
			{
				pBtn->State = 1;

				/* ���Ͱ�ť���µ���Ϣ */
				bsp_PutKey((uint8_t)(3 * i + 1));
			}

			if (pBtn->LongTime > 0)
			{
				if (pBtn->LongCount < pBtn->LongTime)
				{
					/* ���Ͱ�ť�������µ���Ϣ */
					if (++pBtn->LongCount == pBtn->LongTime)
					{
						/* ��ֵ���밴��FIFO */
						bsp_PutKey((uint8_t)(3 * i + 3));
					}
				}
				else
				{
					if (pBtn->RepeatSpeed > 0)
					{
						if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
						{
							pBtn->RepeatCount = 0;
							/* ��������ÿ��10ms����1������ */
							bsp_PutKey((uint8_t)(3 * i + 1));
						}
					}
				}
			}
		}
	}
	else
	{
		if (pBtn->Count > KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count != 0)
		{
			pBtn->Count--;
		}
		else
		{
			if (pBtn->State == 1)
			{
				pBtn->State = 0;

				/* ���Ͱ�ť�������Ϣ */
				bsp_PutKey((uint8_t)(3 * i + 2));
			}
		}

		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_KeyScan
*	����˵��: ɨ�����а���������������systick�ж������Եĵ���
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_KeyScan(void)
{
	uint8_t i;

	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_DetectKey(i);
	}
}

/*********************************************************************************************************
*   �� �� ��: APP_KEY_TASK
*
*   ����˵��: ����ģ������
*
*   ��   ��: none
*
*   �� �� ֵ: none
*
*********************************************************************************************************/
void APP_KEY_TASK(void)
{
	uint8_t ucKeyCode;  // ��������

	/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */
	ucKeyCode = bsp_GetKey();   /* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
	if (ucKeyCode != KEY_NONE)
	{
		switch (ucKeyCode)
		{
		case KEY_DOWN_K1:           /* K1������ */
			//bsp_LedOn(1);
			//printf("K1������, LED1����\r\n");
			break;

		case KEY_UP_K1:             /* K1������ */
			//bsp_LedOff(1);
			//printf("K1������, LED1Ϩ��\r\n");
			break;

		case KEY_LONG_K1:			/* K1������*/
			// tare
			device_info.weight_tare = sensor_value[SENS_UPWEIGHT].value;
			device_info.delay = 500;
			device_info.flag = CFGSTAT_SAVE;
			break;
		case KEY_DOWN_K2:           /* K2������ */
			//bsp_LedOn(2);
			//printf("K2������, LED2����\r\n");
			break;

		case KEY_UP_K2:             /* K2������ */
			//bsp_LedOff(2);
			//printf("K2������, LED2Ϩ��\r\n");
			break;

		case KEY_DOWN_K3:           /* K3������ */
			//bsp_LedOn(3);
			//printf("K3������, LED3����\r\n");
			break;

		case KEY_UP_K3:             /* K3������ */
			//bsp_LedOff(3);
			//printf("K3������, LED3Ϩ��\r\n");
			break;

		case JOY_DOWN_U:            /* ҡ��UP������ */
			//printf("ҡ���ϼ�����\r\n");
			break;

		case JOY_DOWN_D:            /* ҡ��DOWN������ */
			//printf("ҡ���¼�����\r\n");
			break;

		case JOY_DOWN_L:            /* ҡ��LEFT������ */
			//printf("ҡ���������\r\n");
			break;

		case JOY_DOWN_R:            /* ҡ��RIGHT������ */
			//printf("ҡ���Ҽ�����\r\n");
			break;

		case JOY_DOWN_OK:           /* ҡ��OK������ */
			//printf("ҡ��OK������\r\n");
			break;

		case JOY_UP_OK:             /* ҡ��OK������ */
			//printf("ҡ��OK������\r\n");
			break;

		default:
			/* �����ļ�ֵ������ */
			break;
		}
	}
}

/*****************************̩�²������ܿƼ� http://www.htachina.com/ (END OF FILE) *******************************/
