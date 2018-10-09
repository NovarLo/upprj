#ifndef _APP_FIFO_H
#define _APP_FIFO_H

#ifdef _LOCAL_FIFO
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

#define BUFFERSIZE	1024*8
#define QUEUE_REV_SIZE 1024*8

typedef struct 
{
	uint8_t *buffer;		// 缓冲区
	uint32_t size;		// 大小
	__IO uint32_t in;		// 入口位置
	__IO uint32_t out;		// 出口位置
	//pthread_mutex_t *f_lock;	// 互斥锁
}RingBuf,*pRingBuf;
// for uart 5
_EXTERN RingBuf recvfifo_5;   // 接收FIFO            
_EXTERN RingBuf sendfifo_5;   // 发送FIFO            
_EXTERN RingBuf mfappfifo_5;  // 多帧应用数据存放FIFO

_EXTERN uint8_t recvbuf_5[QUEUE_REV_SIZE];          
_EXTERN uint8_t sendbuf_5[BUFFERSIZE];                  
_EXTERN uint8_t mfappbuf_5[BUFFERSIZE];      
// for uart 6           
_EXTERN RingBuf recvfifo_6;   // 接收FIFO            
_EXTERN RingBuf sendfifo_6;   // 发送FIFO            
_EXTERN RingBuf mfappfifo_6;  // 多帧应用数据存放FIFO

_EXTERN uint8_t recvbuf_6[QUEUE_REV_SIZE];          
_EXTERN uint8_t sendbuf_6[BUFFERSIZE];                  
_EXTERN uint8_t mfappbuf_6[BUFFERSIZE];     

pRingBuf ring_buffer_init(uint8_t *buffer, uint32_t size);
void ring_buffer_free(pRingBuf ring_buf);
uint32_t __ring_buffer_len(const pRingBuf ring_buf);
uint32_t __ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size);
uint32_t __ring_buffer_put(pRingBuf ring_buf, uint8_t *buffer, uint32_t size);
uint32_t ring_buffer_len(const pRingBuf ring_buf );
uint32_t ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size);
uint32_t ring_buffer_put(pRingBuf ring_buf, uint8_t *buffer, uint32_t size);

#undef _EXTERN

#endif
