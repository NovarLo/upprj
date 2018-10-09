#ifndef _APP_UPSNR_H
#define _APP_UPSNR_H

/********************** macro ***********************/

#define UPSNR_RCVSTART1 0x55
#define UPSNR_RCVSTART2 0xAA
#define UPSNR_RCVEND	0x16
#define UPSNR_RCVDATLEN 0x24    // fixed frame length is 2+32+2=36

/******************** structure *********************/

#define UPSNR_sendbuf_SIZE 1000
typedef struct
{
    BOOL    valid;              // true when data ready
    uint32_t    length;             // length of data to send
    uint8_t dat[UPSNR_sendbuf_SIZE];
}APP_upsnrsendbuf_TypeDef;

#define UPSNR_RCVBUFSIZE  40
#define UPSNR_DATBUFSIZE  32
typedef struct
{       // UART receiver buffer
    uint32_t    timestamp;
    uint32_t    rcvindex;           // index of received byte
    uint8_t rcvbyte[UPSNR_RCVBUFSIZE];

    // protocol analyse buffer
    uint8_t valid;
    uint8_t cmd;        //
    uint8_t msg;
    uint8_t ctrtype;
    uint16_t    scrid;      //
    uint16_t    ctrid;      //
    uint8_t rcvdat[UPSNR_DATBUFSIZE];
}APP_upsnrrcvbuf_TypeDef;

/********************* variable *********************/
extern uint8_t upsnr_rcvbyte;
extern APP_upsnrsendbuf_TypeDef upsnr_sendbuf;
extern APP_upsnrrcvbuf_TypeDef upsnr_rcvbuf;

/********************* function *********************/

void APP_USARTUP_Init(void);
void APP_UPSNR_Init(void);
void APP_UPSNR_OPENALARM(void);
void APP_UPSNR_CLOSEALARM(void);
void APP_upsnr_rcvisr(void);
void APP_UPSNR_TASK(void);

#endif





















