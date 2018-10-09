#define _LOCAL_FIFO
/*  	仿照 Linux kfifo 写的 ring buffer
	@novar date：2015-07-08
	ring_buffer.h
   */
   
#include "stm32f4xx_hal.h"
#include "app_fifo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// 判断x是否是2的指数函数
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

// 取 a 和 b中的最小值
#define min(a,b)	(((a) < (b)) ? (a) : (b))


/* 
// 定义接收和发送缓冲区 8K Bytes                              
uint8_t recvbuf_5[QUEUE_REV_SIZE];                            
uint8_t sendbuf_5[BUFFERSIZE];                                
uint8_t mfappbuf_5[BUFFERSIZE];                               
                                                              
RingBuf recvfifo_5;   // 接收FIFO                             
RingBuf sendfifo_5;   // 发送FIFO                             
RingBuf mfappfifo_5;  // 多帧应用数据存放FIFO                 
                                                              
RingBuf recvfifo_6;   // 接收FIFO                             
RingBuf sendfifo_6;   // 发送FIFO                             
RingBuf mfappfifo_6;  // 多帧应用数据存放FIFO                 
                                                              
uint8_t recvbuf_6[];                                          
uint8_t sendbuf_6[];                                          
uint8_t mfappbuf_6[];                                         
 */

// 初始化缓冲区
/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
pRingBuf ring_buffer_init(uint8_t *buffer, uint32_t size)//, pthread_mutex_t *f_lock)
{
	//assert(buffer);

	pRingBuf ring_buf = NULL;
	if (!is_power_of_2(size)) 
	{
		//fprintf(stderr, "size must be power of 2.\n");
		return ring_buf;
	}
	ring_buf = (pRingBuf)malloc(sizeof(RingBuf));

	if (!ring_buf) 
	{
		//fprintf(stderr, "Failed to malloc memory,errno:%u,reason:%s", errno, strerror(errno));
		return ring_buf;
	}

	memset(ring_buf, 0, sizeof(RingBuf));
	ring_buf->buffer = buffer;
	ring_buf->size = size;
	ring_buf->in = 0;
	ring_buf->out = 0;
	//ring_buf->f_lock = f_lock;

	return ring_buf;
}

// 释放缓冲区
/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
void ring_buffer_free(pRingBuf ring_buf) 
{
	if (ring_buf) 
	{
		if (ring_buf->buffer) 
		{
			free(ring_buf->buffer);
			ring_buf->buffer = NULL;
		}
		free(ring_buf);
		ring_buf = NULL;
	}
}

// 缓冲区的长度
/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_len(const pRingBuf ring_buf) 
{
	return (ring_buf->in - ring_buf->out);
}

// 从缓冲区中取数据
/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	//assert(ring_buf || buffer);
	uint32_t len = 0;

	size = min(size, ring_buf->in - ring_buf->out);

	// 首先从fifo->out获得数据直到缓冲区末尾
	len = min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
	memcpy(buffer, (ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1))), len);

	// 然后从缓冲区起始位置获得剩余空间
	memcpy(buffer + len, ring_buf->buffer, size - len);

	ring_buf->out += size;
	ring_buf->out &= (ring_buf->size - 1);
	return size;
}

// 向缓冲区中存放数据
/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_put(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	//assert(ring_buf || buffer);
	uint32_t len = 0;

	size = min(size, ring_buf->size - ring_buf->in + ring_buf->out);

	// 首先把数据从fifo->in到缓冲区尾部依次放入
	len = min(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
	memcpy(ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), buffer, len);

	// 然后把剩余数据放入缓冲区起始位置
	memcpy(ring_buf->buffer, buffer + len, size - len);

	ring_buf->in += size;

	return size;
}


/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
uint32_t ring_buffer_len(const pRingBuf ring_buf) 
{
	uint32_t len = 0;

	//pthread_mutex_t_lock(ring_buffer->f_lock);

	len = __ring_buffer_len(ring_buf);

	//pthread_mutex_unlock(ring_buf->f_lock);

	return len;
}


/* 
*********************************************************************************************************
*   函 数 名:
*           
*   功能说明:
* 
*   形   参: 
*            
*   返 回 值:
*            
*********************************************************************************************************
*/
uint32_t ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	uint32_t ret;
	
	//pthread_mutex_lock(ring_buf->f_lock);
	
	ret = __ring_buffer_get(ring_buf, buffer, size);
	// buffer中没有数据
    if (ring_buf->in == ring_buf->out)   
    {                                    
        ring_buf->in = ring_buf->out = 0;
    }                                    

	//pthread_mutex_unlock(ring_buf->f_lock);

	return ret;
}

/* 
*********************************************************************************************************
*   函 数 名: ring_buffer_put
*           
*   功能说明: 往环形缓冲区中放入指定缓冲区数据
* 
*   形   参: pRingBuf ring_buf : 目标环形缓冲区
*   		uint8_t *buffer : 数据存放缓冲区指针
*   		uint32_t size : 放入fifo中的数据字节长度
*            
*   返 回 值: 实际写入数据长度
*            
*********************************************************************************************************
*/
uint32_t ring_buffer_put(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	uint32_t ret;

	//pthread_mutex_lock(ring_buf->f_lock);
	
	ret = __ring_buffer_put(ring_buf, buffer, size);

	//pthread_mutex_unlock(ring_buf->f_lock);
	return ret;
}

#undef _LOCAL_FIFO

/* ************************ end line***************/
