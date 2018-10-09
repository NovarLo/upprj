#define _LOCAL_FIFO
/*  	���� Linux kfifo д�� ring buffer
	@novar date��2015-07-08
	ring_buffer.h
   */
   
#include "stm32f4xx_hal.h"
#include "app_fifo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// �ж�x�Ƿ���2��ָ������
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

// ȡ a �� b�е���Сֵ
#define min(a,b)	(((a) < (b)) ? (a) : (b))


/* 
// ������պͷ��ͻ����� 8K Bytes                              
uint8_t recvbuf_5[QUEUE_REV_SIZE];                            
uint8_t sendbuf_5[BUFFERSIZE];                                
uint8_t mfappbuf_5[BUFFERSIZE];                               
                                                              
RingBuf recvfifo_5;   // ����FIFO                             
RingBuf sendfifo_5;   // ����FIFO                             
RingBuf mfappfifo_5;  // ��֡Ӧ�����ݴ��FIFO                 
                                                              
RingBuf recvfifo_6;   // ����FIFO                             
RingBuf sendfifo_6;   // ����FIFO                             
RingBuf mfappfifo_6;  // ��֡Ӧ�����ݴ��FIFO                 
                                                              
uint8_t recvbuf_6[];                                          
uint8_t sendbuf_6[];                                          
uint8_t mfappbuf_6[];                                         
 */

// ��ʼ��������
/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
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

// �ͷŻ�����
/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
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

// �������ĳ���
/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_len(const pRingBuf ring_buf) 
{
	return (ring_buf->in - ring_buf->out);
}

// �ӻ�������ȡ����
/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	//assert(ring_buf || buffer);
	uint32_t len = 0;

	size = min(size, ring_buf->in - ring_buf->out);

	// ���ȴ�fifo->out�������ֱ��������ĩβ
	len = min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
	memcpy(buffer, (ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1))), len);

	// Ȼ��ӻ�������ʼλ�û��ʣ��ռ�
	memcpy(buffer + len, ring_buf->buffer, size - len);

	ring_buf->out += size;
	ring_buf->out &= (ring_buf->size - 1);
	return size;
}

// �򻺳����д������
/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
*            
*********************************************************************************************************
*/
uint32_t __ring_buffer_put(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	//assert(ring_buf || buffer);
	uint32_t len = 0;

	size = min(size, ring_buf->size - ring_buf->in + ring_buf->out);

	// ���Ȱ����ݴ�fifo->in��������β�����η���
	len = min(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
	memcpy(ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), buffer, len);

	// Ȼ���ʣ�����ݷ��뻺������ʼλ��
	memcpy(ring_buf->buffer, buffer + len, size - len);

	ring_buf->in += size;

	return size;
}


/* 
*********************************************************************************************************
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
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
*   �� �� ��:
*           
*   ����˵��:
* 
*   ��   ��: 
*            
*   �� �� ֵ:
*            
*********************************************************************************************************
*/
uint32_t ring_buffer_get(pRingBuf ring_buf, uint8_t *buffer, uint32_t size) 
{
	uint32_t ret;
	
	//pthread_mutex_lock(ring_buf->f_lock);
	
	ret = __ring_buffer_get(ring_buf, buffer, size);
	// buffer��û������
    if (ring_buf->in == ring_buf->out)   
    {                                    
        ring_buf->in = ring_buf->out = 0;
    }                                    

	//pthread_mutex_unlock(ring_buf->f_lock);

	return ret;
}

/* 
*********************************************************************************************************
*   �� �� ��: ring_buffer_put
*           
*   ����˵��: �����λ������з���ָ������������
* 
*   ��   ��: pRingBuf ring_buf : Ŀ�껷�λ�����
*   		uint8_t *buffer : ���ݴ�Ż�����ָ��
*   		uint32_t size : ����fifo�е������ֽڳ���
*            
*   �� �� ֵ: ʵ��д�����ݳ���
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
