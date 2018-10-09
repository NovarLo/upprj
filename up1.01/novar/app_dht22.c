#define _LOCAL_DHT22

#include "stm32f4xx_hal.h"
#include "app_dht22.h"
#include "app_user.h"
      
//float temperature = 0;  	    
//float humidity = 0; 
//Reset DHT11
void DHT22_Rst(void)	   
{                 
	DHT22_IO_OUT(); 	//SET OUTPUT
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);	//GPIOE.4=0
    delay_ms(20);    	//Pull down Least 18ms
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET); 	//GPIOE.4=1 
	delay_us(30);     	//Pull up 20~40us
}

uint8_t DHT22_Check(void) 	   
{   
	uint8_t retry=0;

	DHT22_IO_IN();//SET INPUT	 
    while (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)&&retry<100)//DHT22 Pull down 40~80us
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=100)
    {
		return 1;
    }
	else 
		retry=0;
    while (!HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)&&retry<100)//DHT22 Pull up 40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)
    {
		return 1;//chack error	 
    }        
	return 0;
}

uint8_t DHT22_Read_Bit(void) 			 
{
 	uint8_t retry=0;

	while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)&&retry<100)//wait become Low level
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)&&retry<100)//wait become High level
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//wait 40us
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4))
		return 1;
	else 
		return 0;		   
}

uint8_t DHT22_Read_Byte(void)    
{        
    uint8_t i,dat;

    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT22_Read_Bit();
    }						    
    return dat;
}

uint8_t DHT22_Read_Data(float *temp,float *humi)    
{        
 	uint8_t buf[5];
	uint8_t i;

	DHT22_Rst();
	if(DHT22_Check()==0)
	{
		for(i=0;i<5;i++)
		{
			buf[i]=DHT22_Read_Byte();
		}
		if(buf[4]==(uint8_t)(buf[0]+buf[1]+buf[2]+buf[3]))
		{
			*humi=(float)((buf[0]<<8)+buf[1])/10;

			if (buf[2] >> 7)
			{
				*temp = -(float)(((buf[2]&0x7F) << 8) + buf[3]) / 10;
			}
			else
			{
				*temp = (float)((buf[2] << 8) + buf[3]) / 10;
			}
		}
	}
	else 
		return 1;
	return 0;	    
}
	 
uint8_t DHT22_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;
	uint32_t tick;
	static uint32_t timestamp;
	uint8_t rtn;
 	
	  /*  GPIO Ports Clock Enable */
	__GPIOE_CLK_ENABLE();

	GPIO_InitStructure.Pin = GPIO_PIN_4;	// PE4				 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW; 
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
			    
	DHT22_Rst();  

	// initialize variable
	temperature = 0.0;   
	humidity = 0.0;     
	 
	tick = timestamp = HAL_GetTick();
	while (tick - timestamp < 5000)
	{
		tick = HAL_GetTick();
		rtn = DHT22_Check();
		if (!rtn)break;
	}
	return rtn;
} 

#undef _LOCAL_DHT22
