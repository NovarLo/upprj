#ifndef _APP_DHT22_H
#define _APP_DHT22_H 
 
#ifdef _LOCAL_DHT22
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

//Set GPIO Direction
#define DHT22_IO_IN()  {GPIOE->MODER&=0XFFFFFCFF;}    //PE4 input
#define DHT22_IO_OUT() {GPIOE->MODER&=0XFFFFFCFF;GPIOE->MODER|=GPIO_MODE_OUTPUT_PP<<8;}  //PE4 output pp
											   
//#define	DHT22_DQ_OUT {HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);}
//#define	DHT22_DQ_IN  PAin(3)   

uint8_t DHT22_Init(void); //Init DHT22
uint8_t DHT22_Read_Data(float *temp,float *humi); //Read DHT22 Value
uint8_t DHT22_Read_Byte(void);//Read One Byte
uint8_t DHT22_Read_Bit(void);//Read One Bit
uint8_t DHT22_Check(void);//Chack DHT22
void DHT22_Rst(void);//Reset DHT22    
void Get_AM2301_Data(void);


#undef	_EXTERN
#endif
