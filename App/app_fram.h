/*   */
/*   */

#ifdef	_LOCAL_SENSOR
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

/////////// type & variables

// fram command
#define FRAMCMD_WREN	0x06	// write enable
#define FRAMCMD_WRDI	0x04	// write disable
#define FRAMCMD_RDSR	0x05	// read status
#define FRAMCMD_WRSR	0x01	// write status
#define FRAMCMD_READ	0x03	// read data
#define FRAMCMD_WRITE	0x02	// write data


/////////// functions
void APP_fram_ini(void);
void APP_fram_WREN(SPI_HandleTypeDef* hspi);
void APP_fram_WRDI(SPI_HandleTypeDef* hspi);
void APP_fram_RDSR(SPI_HandleTypeDef* hspi, uint8_t* buf);
void APP_fram_WRSR(SPI_HandleTypeDef* hspi, uint8_t status);
void APP_fram_readdata(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t size);
void APP_fram_writedata(SPI_HandleTypeDef* hspi, uint32_t addr, uint8_t* buf, uint32_t size);


#undef	_EXTERN

/* ******************************  END OF FILE  *******************************/


