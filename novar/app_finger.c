#define _LOCAL_FINGER

#include "stm32f4xx_hal.h"
#include "app_user.h"
#include "app_network.h"
#include "app_mg2639d.h"
#include "app_sim7600ce.h"
#include "app_audio.h"
#include "app_wave.h"
#include "app_sensor.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "app_finger.h"

extern uint8_t mrtnbuffer[];
uint8_t fngrbuf[512];
/* 
*********************************************************************************************************
*   函 数 名: APP_UART4_Init
*
*   功能说明: 指纹模块串口重新初始化
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_USART6_Init(void)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 57600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart6);
}

/* 
*********************************************************************************************************
*   函 数 名: GetSum
*
*   功能说明: 获取累加校验和
*
*   形   参: uint8_t *ptr —— 数组首指针
*            uint16_t size —— 大小
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint16_t GetSum(uint8_t *ptr, uint16_t size)
{
    uint16_t i;
    uint16_t sum=0;

    for (i = 0; i < size; i++)
    {
        sum += ptr[i];
    }
    return sum;
}

/* 
*********************************************************************************************************
*   函 数 名: struct2frame
*
*   功能说明: 结构转帧数组
*
*   形   参: pFrmFngr ptr —— 结构
*            uint8_t *buffer —— 数组
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void struct2frame(pFrmFngr ptr, uint8_t *buffer)
{
    uint16_t i;
    uint8_t *dptr;

    dptr = buffer;
    *dptr++ = (ptr->start>>8)&0xff;
    *dptr++ = ptr->start&0xff;
    *dptr++ = (ptr->addr>>24)&0xff;
    *dptr++ = (ptr->addr>>16)&0xff;
    *dptr++ = (ptr->addr>>8)&0xff;
    *dptr++ = ptr->addr;
    *dptr++ = ptr->pid;
    *dptr++ = (ptr->length>>8)&0xff;
    *dptr++ = ptr->length;
    for (i = 0; i < ptr->length-2; i++)
    {
        *dptr++ = ptr->buf[i];
    }
    *dptr++ = (ptr->sum >> 8) & 0xff;
    *dptr++ = ptr->sum;

}

/* 
*********************************************************************************************************
*   函 数 名: frame2struct
*
*   功能说明: 帧数组转结构
*
*   形   参: pFrmFngr ptr —— 结构
*            uint8_t *buffer —— 数组
*
*   返 回 值: none
*
*********************************************************************************************************
*/
BOOL frame2struct(pFrmFngr ptr, uint8_t *buffer)
{
    static uint16_t i,sum=0;
    BOOL ret=FALSE;

    ptr->start = ((buffer[0] << 8) & 0xff00)|buffer[1];
    ptr->addr = ((buffer[2] << 24) & 0xff000000) |
                ((buffer[3] << 16) & 0x00ff0000) |
                ((buffer[4] << 8)  & 0x0000ff00) |
                buffer[5];
    ptr->pid = buffer[6];
    ptr->length = ((buffer[7] << 8) & 0xff00) | buffer[8];
    for (i = 0; i < ptr->length - 2; i++)
    {
        ptr->buf[i] = buffer[9 + i];
    }

    ptr->sum = ((buffer[9 + ptr->length - 2] << 8) & 0xff00) | buffer[9 + ptr->length - 1];
    sum = GetSum(buffer + 6, ptr->length + 1);

    if (ptr->start != START || ptr->addr != ADDR || sum != ptr->sum) ret = (BOOL)FALSE;
    ret = (BOOL)TRUE;

    return ret;
}
/* 
*********************************************************************************************************
*   函 数 名: FNGR_Send_Frame
*
*   功能说明: 下发指纹模块帧发送函数
*
*   形   参: pFrmFngr —— 帧结构
*
*   返 回 值: none
*
*********************************************************************************************************
*/
HAL_StatusTypeDef FNGR_Send_Frame(pFrmFngr ptr)
{
    HAL_StatusTypeDef ret;

    struct2frame(ptr, fngr_send_frm);

    // 计算累加和
    ptr->sum = GetSum(fngr_send_frm + 6, ptr->length + 1);

    fngr_send_frm[ptr->length + 7] = (ptr->sum >> 8) & 0xff;
    fngr_send_frm[ptr->length + 8] = ptr->sum & 0xff;

    // 从结构中取出帧

    ret = HAL_UART_Transmit(HDL_UART_LED, fngr_send_frm, (uint8_t)ptr->length + 9, 500);
    return ret;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_Recv_Frame
*
*   功能说明: 接收指纹模块帧
*
*   形   参: pFrmFngr —— 帧结构
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint8_t* FNGR_Recv_Frame(pFrmFngr ptr,uint16_t length)
{
    HAL_StatusTypeDef ret;

    ret = HAL_UART_Receive(HDL_UART_LED, fngr_recv_frm, length, 1000);

    if (ret == HAL_OK)
    {
        frame2struct(ptr, fngr_recv_frm);
        return ptr->buf;    //  返回确认码
    }
    else return NULL;   // 返回失败
}

/* 
*********************************************************************************************************
*   函 数 名: Fngr_packhead
*
*   功能说明: 帧结构头打包
*
*   形   参: pFrmFngr ptr —— 帧结构
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void Fngr_packhead(pFrmFngr ptr)
{
    ptr->start = START;
    ptr->addr = ADDR;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_VfyPwd
*
*   功能说明: 发送验证密码帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*
*   返 回 值: 确认码
*
*********************************************************************************************************
*/
uint8_t FNGR_VfyPwd(pFrmFngr ptr)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 7;
    ptr->buf[0] = VfyPwd;
    ptr->buf[1] = (PWD>>24)&0xff;
    ptr->buf[2] = (PWD>>16)&0xff;
    ptr->buf[3] = (PWD>>8)&0xff;
    ptr->buf[4] = PWD&0xff;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 12);
    if (!ret) return 0;

    // 发送接收都正常
    // 查看返回确认码是否正常
    return ret[0];
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_GenImg
*
*   功能说明: 发送录入图像帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*
*   返 回 值: 确认码 —— 0x00表示录入成功；0x01表示收包有错；0x02表示传感器上无手指；0x03表示录入不成功；
*
*********************************************************************************************************
*/
uint8_t FNGR_GenImg(pFrmFngr ptr)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 3;
    ptr->buf[0] = GenImg;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 12);
    if (!ret) return 0xff;

    return ret[0];
}


/* 
*********************************************************************************************************
*   函 数 名: FNGR_Img2Tz
*
*   功能说明: 发送生成特征码帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid  —— 缓冲区ID，缓冲区CharBuffer1、CharBuffer2的BufferID分别为0x01和0x02，
*                                   如果指定其它值，按照CharBuffer2处理
*
*   返 回 值: 确认码=0x00表示生成特征成功；
*                   =0x01表示收包有错；
*                   =0x06表示指纹图像太乱而生不成特征；
*                   =0x07表示指纹图像正常，但特征点太少而生不成特征；
*                   =0x15表示图像缓冲区内没有有效原始图而生不成图像；
*
*********************************************************************************************************
*/
uint8_t FNGR_Img2Tz(pFrmFngr ptr, uint8_t bufferid)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 4;
    ptr->buf[0] = Img2Tz;
    ptr->buf[1] = bufferid;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 12);
    if (!ret) return 0xff;

    return ret[0];
}


/* 
*********************************************************************************************************
*   函 数 名: FNGR_Match
*
*   功能说明: 发送特征比对帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*
*   返 回 值: 确认码=0x00表示指纹匹配；
*                   =0x01表示收包有错；
*                   =0x08表示指纹不匹配；
*
*********************************************************************************************************
*/
uint8_t FNGR_Match(pFrmFngr ptr)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 3;
    ptr->buf[0] = Match;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 14);
    if (!ret) return 0xff;

    return ret[0];
}


/* 
*********************************************************************************************************
*   函 数 名: FNGR_RegModel
*
*   功能说明: 发送特征比对帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*
*   返 回 值: 确认码=0x00表示合并成功；
*                   =0x01表示收包有错；
*                   =0x0a表示合并失败（两枚指纹不属于同一手指）；
*
*********************************************************************************************************
*/
uint8_t FNGR_RegModel(pFrmFngr ptr)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 3;
    ptr->buf[0] = RegModel;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 12);
    if (!ret) return 0xff;

    return ret[0];
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_Img2Tz
*
*   功能说明: 发送搜索特征码帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid  —— 缓冲区ID，缓冲区CharBuffer1、CharBuffer2的BufferID分别为0x01和0x02，
*                                   如果指定其它值，按照CharBuffer2处理
*
*   返 回 值: 确认码=0x00表示搜索到；
*                   =0x01表示收包有错；
*                   =0x09表示没搜索到；
*
*********************************************************************************************************
*/
uint8_t* FNGR_Seach(pFrmFngr ptr, uint8_t bufferid, uint16_t startpage, uint16_t pagenum)
{
    uint8_t *ret;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 8;
    ptr->buf[0] = Search;
    ptr->buf[1] = bufferid;
    ptr->buf[2] = (startpage >> 8) & 0xff;
    ptr->buf[3] = startpage & 0xff;
    ptr->buf[4] = (pagenum >> 8) & 0xff;
    ptr->buf[5] = pagenum & 0xff;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return NULL; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr, 16);
    if (!ret) return NULL;

    return ret;
}
/* 
*********************************************************************************************************
*   函 数 名: FNGR_WriteNotepad
*
*   功能说明: 发送写记事本帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t pagenum —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint8_t FNGR_WriteNotepad(uint8_t pagenum,uint8_t *buffer)
{
    uint16_t i;
    uint8_t *ret;
    uint16_t length;

    fngrbuf[0] = (START >> 0x08) & 0xff;
    fngrbuf[1] = START & 0xff;
    fngrbuf[2] = (ADDR >> 0x18) & 0xff;
    fngrbuf[3] = (ADDR >> 0x10) & 0xff;
    fngrbuf[4] = (ADDR >> 0x8) & 0xff;
    fngrbuf[5] = ADDR & 0xff;
    fngrbuf[6] = PID_CMD;
    fngrbuf[7] = 0x00;
    fngrbuf[8] = 0x24;
    fngrbuf[9] = WriteNotepad;
    fngrbuf[10] = pagenum;
    length = fngrbuf[8] | ((fngrbuf[7] << 8)&0xff00);
    for (i = 0; i < length - 4; i++)
    {
        fngrbuf[i+2] = buffer[i];
    }
    fngr_struct_send.sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(&fngr_struct_send) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(&fngr_struct_send, 12);
    if (!ret) return 0xff;

    // 发送接收都正常
    // 查看返回确认码是否正常
    return ret[0];
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_ReadNotepad
*
*   功能说明: 发送读记事本帧
*
*   形   参:
*            uint8_t pagenum —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: none
*
*********************************************************************************************************
*/
uint8_t FNGR_ReadNotepad(uint8_t pagenum, uint8_t *buffer)
{
    uint8_t *ret,i;

    Fngr_packhead(&fngr_struct_send);
    fngr_struct_send.pid = PID_CMD;
    fngr_struct_send.length = 0x04;
    fngr_struct_send.buf[0] = ReadNotepad;
    fngr_struct_send.buf[1] = pagenum;
    fngr_struct_send.sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(&fngr_struct_send) != HAL_OK) return NULL; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(&fngr_struct_send,44);
    if (!ret) return NULL;

    for (i = 0; i < 32; i++)
    {
        buffer[i] = fngr_struct_send.buf[i+1];
    }
    // 发送接收都正常
    // 查看返回确认码是否正常
    return ret[0];
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_DowChar
*
*   功能说明: 发送下载特征或模版帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: 确认码 —— 0x00表示可以接收后续数据包；0x01表示收包有错；0x0e表示不能接收后续数据包；
*
*********************************************************************************************************
*/
uint8_t FNGR_DowChar(pFrmFngr ptr, uint8_t bufferid, uint8_t *fingerbuf, uint16_t length)
{
    uint16_t i,j,a,b;
    uint8_t *ret=NULL;
    HAL_StatusTypeDef rtn;


    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 0x04;
    ptr->buf[0] = DownChar;
    ptr->buf[1] = bufferid;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误

    ret = FNGR_Recv_Frame(ptr,12);
    if (!ret) return 0xff;

    if (ret[0] != ACK_OK) return 0xff;

    // 组织发送帧到一个包
    a = length / FRM_NUM;
    b = length % FRM_NUM;

    for (i = 0; i < a; i++)
    {
        Fngr_packhead(ptr);

        if (!b && i == (a-1)) ptr->pid = PID_END;
        else ptr->pid = PID_DAT;
        ptr->length = FRM_NUM + 2;
        for (j = 0; j < FRM_NUM; j++)
        {
            ptr->buf[j] = fingerbuf[i*FRM_NUM+j];
        }
        ptr->sum = 0;   // 校验和在发送数据处计算

        struct2frame(ptr, fngr_send_frm + (FRM_NUM + 11) * i);
        // 计算累加和
        ptr->sum = GetSum(fngr_send_frm + (FRM_NUM + 11) * i + 6, ptr->length + 1);

        fngr_send_frm[ptr->length + (FRM_NUM + 11) * i + 7] = (ptr->sum >> 8) & 0xff;
        fngr_send_frm[ptr->length + (FRM_NUM + 11) * i + 8] = ptr->sum & 0xff;

        // if (FNGR_Send_Frame(ptr) != HAL_OK) return 0xff; // 发送失败直接返回错误
    }
    if (b)
    {
        Fngr_packhead(ptr);
        ptr->pid = PID_END;
        ptr->length = b + 2;
        for (i = 0; i < b; i++)
        {
            ptr->buf[i] = fingerbuf[i];
        }
        ptr->sum = 0;   // 校验和在发送数据处计算
        struct2frame(ptr, fngr_send_frm + (FRM_NUM + 11) * a);
        // 计算累加和
        ptr->sum = GetSum(fngr_send_frm + (FRM_NUM + 11) * a + 6, ptr->length + 1);

        fngr_send_frm[ptr->length + (FRM_NUM + 11) * a + 7] = (ptr->sum >> 8) & 0xff;
        fngr_send_frm[ptr->length + (FRM_NUM + 11) * a + 8] = ptr->sum & 0xff;
    }

    rtn = HAL_UART_Transmit(HDL_UART_LED, fngr_send_frm, (FRM_NUM + 11) * a + b + 11, 500);

    if (rtn == HAL_OK)
    {
        return ACK_OK;
    }
    else return 0xff;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_UpChar
*
*   功能说明: 发送上传特征或模版帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: 确认码 —— 0x00表示随后发数据包；0x01表示收包有错；0x0d表示指令执行失败；
*
*********************************************************************************************************
*/
uint8_t FNGR_UpChar(pFrmFngr ptr, uint8_t bufferid, uint8_t *fingerbuf, uint16_t length)
{
    uint16_t i,j,len,cnt = 0;
    uint8_t *ret=NULL;
    uint8_t *dptr,*sptr;
    //HAL_StatusTypeDef rtn;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 0x04;
    ptr->buf[0] = UpChar;
    ptr->buf[1] = bufferid;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return ACK_DatPackErr; // 发送失败直接返回错误
    ret = FNGR_Recv_Frame(ptr,12);
    if (!ret) return ACK_DatPackErr;
    if (ret[0] != ACK_OK) return ACK_DatPackErr;

    while (1)
    {
        HAL_UART_Receive(HDL_UART_LED, fngr_recv_frm + (FRM_NUM + 11) * cnt, FRM_NUM + 11, 500);
        if (fngr_recv_frm[(FRM_NUM + 11) * cnt + 6] == PID_END) break;
        cnt++;
    }

    // 组织数据到指定缓冲区
    dptr = fingerbuf;
    for (i = 0; i < cnt+1; i++)
    {
        sptr = fngr_recv_frm + (FRM_NUM + 11) * i + 9;
        len = fngr_recv_frm[(FRM_NUM + 11) * i + 7] << 8 | fngr_recv_frm[(FRM_NUM + 11) * i + 8] - 2;
        for (j = 0; j < len; j++)
        {
            *dptr++ = *sptr++;
        }
    }

    return ACK_OK;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_Store
*
*   功能说明: 发送上传特征或模版帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: 确认码=0x00表示储存成功；
*                   =0x01表示收包有错；
*                   =0x0b表示PageID超出指纹库范围；
*                   =0x18表示写FLASH出错；
*
*********************************************************************************************************
*/
uint8_t FNGR_Store(pFrmFngr ptr, uint8_t bufferid, uint16_t pageid)
{
    uint8_t *ret=NULL;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 0x06;
    ptr->buf[0] = Store;
    ptr->buf[1] = bufferid;
    ptr->buf[2] = (pageid >> 8) & 0xff;
    ptr->buf[3] = pageid & 0xff;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return ACK_DatPackErr; // 发送失败直接返回错误
    ret = FNGR_Recv_Frame(ptr,12);
    if (!ret) return ACK_DatPackErr;
    if (ret[0] != ACK_OK) return ACK_DatPackErr;

    return ACK_OK;
}


/* 
*********************************************************************************************************
*   函 数 名: FNGR_LoadChar
*
*   功能说明: 发送读出特征或模版帧
*
*   形   参: pFrmFngr ptr —— 帧结构
*            uint8_t bufferid —— 写入页号，共16页 0x00~0x0e
*            uint8_t *buf —— 用户信息
*
*   返 回 值: 确认码=0x00表示读出成功；
*                   =0x01表示收包有错；
*                   =0x0c表示读出有错或模板无效；
*                   =0x0b表示PageID超出指纹库范围；
*
*********************************************************************************************************
*/
uint8_t FNGR_LoadChar(pFrmFngr ptr, uint8_t bufferid, uint16_t pageid)
{
    uint8_t *ret=NULL;

    Fngr_packhead(ptr);
    ptr->pid = PID_CMD;
    ptr->length = 0x06;
    ptr->buf[0] = LoadChar;
    ptr->buf[1] = bufferid;
    ptr->buf[2] = (pageid >> 8) & 0xff;
    ptr->buf[3] = pageid & 0xff;
    ptr->sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(ptr) != HAL_OK) return ACK_DatPackErr; // 发送失败直接返回错误
    ret = FNGR_Recv_Frame(ptr,12);
    if (!ret) return ACK_DatPackErr;
    if (ret[0] != ACK_OK) return ACK_DatPackErr;

    return ACK_OK;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_DeletChar
*
*   功能说明: 发送删除模版帧
*
*   形   参: uint16_t pageid —— 删除页码
*            uint16_t num —— 删除个数
*
*   返 回 值: 确认码=0x00表示删除模板成功；
*                   =0x01表示收包有错；
*                   =0x10表示删除模板失败；
*
*********************************************************************************************************
*/
uint8_t FNGR_DeletChar(uint16_t pageid,uint16_t num)
{
    uint8_t *ret=NULL;

    Fngr_packhead(&fngr_struct_send);
    fngr_struct_send.pid = PID_CMD;
    fngr_struct_send.length = 0x07;
    fngr_struct_send.buf[0] = DeletChar;
    fngr_struct_send.buf[1] = (pageid >> 8) & 0xff;
    fngr_struct_send.buf[2] = pageid & 0xff;
    fngr_struct_send.buf[3] = (num >> 8) & 0xff;
    fngr_struct_send.buf[4] = num & 0xff;
    fngr_struct_send.sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(&fngr_struct_send) != HAL_OK) return ACK_DatPackErr; // 发送失败直接返回错误
    ret = FNGR_Recv_Frame(&fngr_struct_send,12);
    if (!ret) return ACK_DatPackErr;
    if (ret[0] != ACK_OK) return ACK_DatPackErr;

    return ACK_OK;
}

/* 
*********************************************************************************************************
*   函 数 名: FNGR_Empty
*
*   功能说明: 发送清空指纹库帧
*
*   形   参: none
*
*   返 回 值: 确认码=0x00表示清空成功；
*                   =0x01表示收包有错；
*                   =0x11表示清空失败；
*
*********************************************************************************************************
*/
uint8_t FNGR_Empty(void)
{
    uint8_t *ret=NULL;

    Fngr_packhead(&fngr_struct_send);
    fngr_struct_send.pid = PID_CMD;
    fngr_struct_send.length = 0x03;
    fngr_struct_send.buf[0] = Empty;
    fngr_struct_send.sum = 0;   // 校验和在发送数据处计算

    if (FNGR_Send_Frame(&fngr_struct_send) != HAL_OK) return ACK_DatPackErr; // 发送失败直接返回错误
    ret = FNGR_Recv_Frame(&fngr_struct_send,12);
    if (!ret) return ACK_DatPackErr;
    if (ret[0] != ACK_OK) return ACK_DatPackErr;

    return ACK_OK;
}

/* 
*********************************************************************************************************
*   函 数 名: APP_FNGR_Init
*
*   功能说明: 指纹模块初始化 —— 包含串口6的重新初始化和其他状态位
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_FNGR_Init(void)
{
    uint8_t i,buf[32],ret;

    APP_USART6_Init();
    fngr_stat.STAT_FNGRSEND_BUSY = 0;
    fngr_stat.STAT_FNGRRECV_BUSY = 0;
    fngr_stat.STAT_10MIN_EN = 0;
    fngr_stat.STAT_10MIN_TIMEOUT = 0;
    fngr_stat.STAT_2HOUR_EN = 0;
    fngr_stat.STAT_2HOUR_TIMEOUT = 0;

    ret = FNGR_VfyPwd(&fngr_struct_send);

    if (ret == ACK_OK)
    {
        novar_print((uint8_t *)"verify password ok!\r\n", 21);
    }
    else novar_print((uint8_t *)"verify password fail!\r\n", 23);
    // 清除指纹存储状态标识

    memset(buf, 0, 32);

/* 
    for (i = 0; i < 16; i++)
    {
        if (FNGR_WriteNotepad(i, buf) == ACK_OK) novar_print((uint8_t *)"Write notepad ok!\r\n", 19);

    }
    for (i = 0; i < 4; i++)
    {
        if (FNGR_LoadChar(&fngr_struct_send, CHARBUF1, i) == ACK_OK)
        {
            if (FNGR_UpChar(&fngr_struct_send,1,fngrbuf,512) == ACK_OK)
            {
                novar_print((uint8_t *)"upchar success!\r\n", 17);
            }
            else novar_print((uint8_t *)"upchar failed !\r\n", 17);
        }
    }

*/
    for (i = 0; i < 16; i++)
    {
        if (FNGR_ReadNotepad(i, buf) == ACK_OK) novar_print((uint8_t *)"Read notepad ok!\r\n", 18);

    }
    //
    //if (FNGR_Empty() == ACK_OK) novar_print((uint8_t *)"Empty is ok!\r\n", 14);
}

/* 
*********************************************************************************************************
*   函 数 名: APP_FNGR_TASK
*
*   功能说明: 指纹模块任务
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_FNGR_TASK(void)
{
    #ifdef ELIVATOR
    uint8_t rtn;
    //static uint8_t snsrflag=0;
    uint8_t *ret;
    uint16_t pageno;
    uint32_t i = 0;
    static uint8_t step = 0;
    static uint32_t tick_delay = 0;
    uint8_t dbuf[32];

    if (device_info.sensor_en[SENS_FINGER] != TRUE)
    {
        step = 0;
        return;
    }

    switch (step)
    {
    case 0:
        APP_USART6_Init();
        fingerdata.flag = BUFSTAT_NULL;
        fingerdata.staffid = 0;
        tick_delay = HAL_GetTick();
        step = 1;
        break;
    case 1:// 上电等待15s后再提示指纹输入
        if (HAL_GetTick() - tick_delay > FNGR_RECHK_15SEC)  // 15seconds = 15000
        {
            step = 2;
        }
        break;
    case 2:
            rtn = FNGR_VfyPwd(&fngr_struct_send);

            if (driver.finger_valid) return;

            if (rtn == ACK_OK)
            {
                // 此处启动提示“请输入指纹”，启动之前置指纹违章标志为验证成功
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)qing, CNTqing);
                APP_audio_list_add((uint16_t*)shuru, CNTshuru);
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_play();
                step = 3;
                tick_delay = HAL_GetTick();
                driver.sensor_valid = TRUE;
            }
            break;
    case 3:// 采集一次指纹等待一秒，给主循环留出运行时间
        if (HAL_GetTick() - tick_delay > FNGR_RECHK_1SEC)  // 1seconds
        {
            step = 4;
        }
    break;
    case 4:
        rtn = FNGR_GenImg(&fngr_struct_send);
        if (rtn == ACK_OK)  // 检测到指纹，跳转到下一步
        {
            step = 5;
        }
        else if (rtn == ACK_NoFngr) // 未检测到指纹回到检测指纹
        {
            step = 3;
            driver.id = 1;  // 未验证
            strcpy((char*)driver.name, "未授权");
            tick_delay = HAL_GetTick();
        }
        else    // 无指纹输入设备
        {
            driver.sensor_valid = FALSE;
            step = 3;   // 操作失败，重新检测指纹
            tick_delay = HAL_GetTick();
        }
        break;
    case 5:
        rtn = FNGR_Img2Tz(&fngr_struct_send, CHARBUF1);
        if (rtn == ACK_OK)  // 生成特征码成功
        {
            step = 6;
        }
        else    // 生成特征码失败
        {
            step = 3;   // 操作失败，重新检测指纹
            tick_delay = HAL_GetTick();
        }
        break;
    case 6:
        ret = FNGR_Seach(&fngr_struct_send, CHARBUF1, 0, 32);

        if (ret[0] == ACK_OK)
        {
            pageno = ret[1] << 8 | ret[2];  // 对应存储的指纹页
            if (FNGR_ReadNotepad(pageno/2, dbuf) == ACK_OK)
            {
                // 获取匹配指纹页号
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_list_add((uint16_t*)shuru, CNTshuru);
                APP_audio_list_add((uint16_t*)zhengque, CNTzhengque);
                APP_audio_play();

                driver.finger_valid = TRUE;
                driver.id = 0;  // 验证通过

                fingerdata.flag = BUFSTAT_READY;
                // 司机ID放入缓冲区
                fingerdata.staffid = *((uint32_t *)dbuf);
                driver.name_id = *((uint32_t *)dbuf);
                // 显示司机名称信息
                for (i = 0; i < 12; i++) driver.name[i] = dbuf[i+4];
                step = 0;
            }
            else
            {
                step = 3;   // 操作失败，重新检测指纹
                tick_delay = HAL_GetTick();
            }
        }
        else
        {
            APP_audio_stop();
            APP_audio_list_ini();
            APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
            APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
            APP_audio_list_add((uint16_t*)shibai, CNTshibai);
            APP_audio_play();
            step = 3;   // 操作失败，重新检测指纹
            tick_delay = HAL_GetTick();
        }
        break;
    }
    #endif
    #ifdef TOWERBOX
    uint8_t rtn;
    static uint8_t snsrflag=0;
    uint8_t *ret;
    uint16_t pageno;
    uint32_t i = 0;
    static uint8_t step = 0, recnt = 0, waitcnt = 0, failcnt = 0;
    static uint32_t tick_delay = 0, tick_reinput = 0, tick_rechk = 0;
    uint8_t dbuf[32];

    if (device_info.sensor_en[SENS_FINGER] != TRUE)
    {
        step = 0;
        recnt = 0;
        tick_reinput = 0;
        tick_rechk = 0;
        return;
    }

    switch (step)
    {
    case 0:
        APP_USART6_Init();
        fingerdata.flag = BUFSTAT_NULL;
        fingerdata.staffid = 0;
        tick_delay = HAL_GetTick();
        step = 1;
        break;
    case 1:
        if (HAL_GetTick() - tick_delay > FNGR_RECHK_15SEC)  // 15seconds = 15000
        {
            step = 2;
        }
        break;
    case 2:
            rtn = FNGR_VfyPwd(&fngr_struct_send);

            driver.finger_valid = TRUE;
            if (rtn == ACK_OK)
            {
                // 此处启动提示“请输入指纹”，启动之前置指纹违章标志为验证成功
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)qing, CNTqing);
                APP_audio_list_add((uint16_t*)shuru, CNTshuru);
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_play();
                step = 3;
            }
            break;
        case 3:
            rtn = FNGR_GenImg(&fngr_struct_send);
            if (rtn == ACK_OK)  // 检测到指纹，跳转到下一步
            {
                step = 4;
                failcnt = 0;
            }
            else if (rtn == ACK_NoFngr) // 未检测到指纹回到检测指纹
            {
                waitcnt++;
                if (waitcnt >= 70)
                {
                    step = 2;
                    waitcnt = 0;
                    failcnt++;
                    if (failcnt >= 3)
                    {
                        step = 7;   // 进入延时2小时
                        tick_rechk = HAL_GetTick();
                        failcnt = 0;
                        APP_audio_stop();
                        APP_audio_list_ini();
                        APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                        APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                        APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                        APP_audio_play();
                        driver.finger_valid = FALSE;
                        driver.sensor_valid = TRUE;
                        driver.id = 1;  // 未验证
                        strcpy((char*)driver.name, "未授权");
                    }
                }
            }
            else    // 无指纹输入设备
            {
                step = 6;   // 操作失败，跳转到10秒钟延时
                snsrflag = 1;   // 指纹模块未装
                tick_reinput = HAL_GetTick();
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                APP_audio_play();
            }
            break;
        case 4:
            rtn = FNGR_Img2Tz(&fngr_struct_send, CHARBUF1);
            if (rtn == ACK_OK)  // 生成特征码成功
            {
                step = 5;
            }
            else    // 生成特征码失败
            {
                step = 6;   // 操作失败，跳转到10秒钟延时
                tick_reinput = HAL_GetTick();
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                APP_audio_play();
            }
            break;
        case 5:
            ret = FNGR_Seach(&fngr_struct_send, CHARBUF1, 0, 32);

            step = 7;   // 跳转到等待延时2小时
            tick_rechk = HAL_GetTick();
            if (ret[0] == ACK_OK)
            {
                // 获取匹配指纹页号
                pageno = ret[1] << 8 | ret[2];  // 对应存储的指纹页

                if (FNGR_ReadNotepad(pageno/2, dbuf) == ACK_OK)
                {
                    APP_audio_stop();
                    APP_audio_list_ini();
                    APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                    APP_audio_list_add((uint16_t*)shuru, CNTshuru);
                    APP_audio_list_add((uint16_t*)zhengque, CNTzhengque);
                    APP_audio_play();

                    driver.finger_valid = TRUE;
                    driver.sensor_valid = TRUE;
                    driver.id = 0;  // 验证通过
                    fingerdata.flag = BUFSTAT_READY;
                    // 司机ID放入缓冲区
                    fingerdata.staffid = *((uint32_t *)dbuf);
                    // 显示司机名称信息
                    for (i = 0; i < 12; i++) driver.name[i] = dbuf[i+4];
                }
                else
                {
                    fingerdata.flag = BUFSTAT_READY;
                    // 司机ID放入缓冲区
                    fingerdata.staffid = 0;
                    // 显示司机名称信息
                    strcpy((char*)driver.name, "未授权");
                }
            }
            else
            {
                step = 6;   // 操作失败，跳转到10秒钟延时
                tick_reinput = HAL_GetTick();
                APP_audio_stop();
                APP_audio_list_ini();
                APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                APP_audio_play();
            }
            break;
        case 6:
            if (HAL_GetTick() - tick_reinput > FNGR_RECHK_10SEC)  // 10 seconds = 10000
            {
                recnt++;
                if (recnt >= 3)
                {
                    step = 7;   // 三次指纹验证不通过，启动2hours延时
                    recnt = 0;
                    tick_reinput = 0;
                    tick_rechk = HAL_GetTick();
                    driver.finger_valid = FALSE;
                    driver.id = 2;  // 指纹验证未通过
                    if (snsrflag)
                    {
                        driver.sensor_valid = FALSE;
                        snsrflag = 0;
                    }
                    else
                    {
                        fingerdata.flag = BUFSTAT_READY;
                        // 司机ID置零
                        fingerdata.staffid = 0;
                        strcpy((char*)driver.name, "未授权");
                    }
                }
                else    // 10秒钟后指纹验证失败不够三次继续验证
                {
                    step = 2;
                }
            }
            break;
        case 7:
            if (HAL_GetTick() - tick_rechk > FNGR_RESET_2HOUR)  // 2 hours = 7200000
            {
                step = 0;
                recnt = 0;
                waitcnt = 0;
                failcnt = 0;
                tick_delay = 0;
                tick_reinput = 0;
                tick_rechk = 0;
            }
            break;
    }
    #endif
}

////// follow functions is for test //////////////

/* 
*********************************************************************************************************
*   函 数 名: APP_FNGR_TEST
*
*   功能说明: 指纹模块测试函数
*
*   形   参: none
*
*   返 回 值: none
*
*********************************************************************************************************
*/
void APP_FNGR_TEST(void)
{
    uint16_t i;
    uint8_t ret=0xff;
    uint8_t sbuf[32],dbuf[32];

    APP_FNGR_Init();


    ret = FNGR_VfyPwd(&fngr_struct_send);

    if (ret == ACK_OK)
    {
        novar_print((uint8_t *)"verify password ok!\r\n", 21);
    }
    else novar_print((uint8_t *)"verify password fail!\r\n", 23);

    for (i = 0; i < 32; i++)
    {
        sbuf[i] = i;
    }

    ret = FNGR_WriteNotepad(0, sbuf);

    if (ret == ACK_OK)
    {
        novar_print((uint8_t *)"Write notepad ok!\r\n", 19);
    }
    else novar_print((uint8_t *)"Write notepad fail!\r\n", 21);

    ret = FNGR_ReadNotepad(0, dbuf);
    if (ret == ACK_OK)
    {
        novar_print((uint8_t *)"Read notepad ok!\r\n", 18);
    }
    else novar_print((uint8_t *)"Read notepad fail!\r\n", 20);

    HAL_Delay(100);
    for (i = 0; i < 32; i++)
    {
        if(sbuf[i] != dbuf[i])
        {
            novar_print((uint8_t *)"notepad verify fail!\r\n", 22);
            break;
        }
        if (i==31)
        {
            novar_print((uint8_t *)"notepad verify success!\r\n", 25);
        }
    }

    for (i = 0; i < 512; i++)
    {
        mrtnbuffer[i + 17] = i;
    }

    if (FNGR_DowChar(&fngr_struct_send,1,mrtnbuffer+17,512) == ACK_OK)
    {
        novar_print((uint8_t *)"downchar success!\r\n", 19);
    }
    else novar_print((uint8_t *)"downchar failed !\r\n", 19);


    if (FNGR_UpChar(&fngr_struct_send,1,fngrbuf,512) == ACK_OK)
    {
        novar_print((uint8_t *)"upchar success!\r\n", 17);
    }
    else novar_print((uint8_t *)"upchar failed !\r\n", 17);


    for (i = 0; i < 512; i++)
    {
        if (mrtnbuffer[i+17] != fngrbuf[i])
        {
            novar_print((uint8_t *)"verify char fail!\r\n", 19);
        }
        if (i == 511) novar_print((uint8_t *)"verify char success!\r\n", 22);
    }

    // 录入指纹流程
    {
        uint8_t rtn,flag=0;
        static uint8_t step = 0;
        while (1)
        {
            switch (step)
            {
            case 0:
                rtn = FNGR_GenImg(&fngr_struct_send);
                if (rtn == ACK_OK)
                {
                    step = 1;
                    novar_print("finger input ok!\r\n", 18);
                }
                else if (rtn == ACK_NoFngr)
                {
                    step = 0;
                    novar_print("no finger!\r\n", 12);
                }
                else
                {
                    // 错误退出
                    novar_print("finger error!\r\n", 15);
                    while (1);
                }
                break;
            case 1:
                rtn = FNGR_Img2Tz(&fngr_struct_send, CHARBUF1);
                if (rtn == ACK_OK)
                {
                    step = 2;
                    novar_print("image to feature success!\r\n", 27);
                }
                else
                {
                    // 错误退出
                    step = 0;
                    novar_print("image to feature error!\r\n", 25);
                    while (1);
                }
                break;
            case 2:
                rtn = FNGR_GenImg(&fngr_struct_send);
                if (rtn == ACK_OK)
                {
                    step = 3;
                    novar_print("finger input ok!\r\n", 18);
                }
                else if (rtn == ACK_NoFngr)
                {
                    step = 0;
                    novar_print("no finger!\r\n", 12);
                }
                else
                {
                    // 错误退出
                    novar_print("finger error!\r\n", 15);
                    while (1);
                }
                break;
            case 3:
                rtn = FNGR_Img2Tz(&fngr_struct_send, CHARBUF2);
                if (rtn == ACK_OK)
                {
                    step = 4;
                    novar_print("image to feature success!\r\n", 27);
                }
                else
                {
                    // 错误退出
                    step = 0;
                    novar_print("image to feature error!\r\n", 25);
                    while (1);
                }
                break;
            case 4:
                rtn = FNGR_RegModel(&fngr_struct_send);
                if (rtn == ACK_OK)
                {
                    step = 5;
                    novar_print("RegModel success!\r\n", 19);
                }
                else
                {
                    step = 0;
                    novar_print("RegModel failed !\r\n", 19);
                    while (1);
                }
                break;
            case 5:
                if (FNGR_UpChar(&fngr_struct_send,CHARBUF1,fngrbuf,512) == ACK_OK)
                {
                    novar_print((uint8_t *)"upchar success!\r\n", 17);
                }
                else novar_print((uint8_t *)"upchar failed !\r\n", 17);
                rtn = FNGR_Store(&fngr_struct_send, CHARBUF1, 3);
                if (rtn == ACK_OK)
                {
                    step = 0;
                    flag = 1;
                    novar_print("finger Store success !\r\n", 24);

                }
                else
                {
                    novar_print("finger Store fail !\r\n", 21);
                }
                break;
            }
            if (flag)
            {
                break;
            }
        }
    }

    // 搜索指纹流程
    {
        uint8_t rtn;
        uint8_t *ret;
        //static uint16_t pageno;
        static uint8_t step = 0;
        // 此处启动提示“请输入指纹”，启动之前置指纹违章标志为验证成功
        // driver.xxflag = 1;
        APP_audio_stop();
        APP_audio_list_ini();
        APP_audio_list_add((uint16_t*)qing, CNTqing);
        APP_audio_list_add((uint16_t*)shuru, CNTshuru);
        APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
        APP_audio_play();
        while (1)
        {
            switch (step)
            {
            case 0:
                rtn = FNGR_GenImg(&fngr_struct_send);
                if (rtn == ACK_OK)
                {
                    step = 1;
                    novar_print("finger input ok!\r\n", 18);
                }
                else if (rtn == ACK_NoFngr)
                {
                    step = 0;
                    novar_print("no finger!\r\n", 12);
                }
                else
                {
                    // 错误退出
                    novar_print("finger error!\r\n", 15);
                    while (1);
                }
                break;
            case 1:
                rtn = FNGR_Img2Tz(&fngr_struct_send, CHARBUF1);
                if (rtn == ACK_OK)
                {
                    step = 2;
                    novar_print("image to feature success!\r\n", 27);
                }
                else
                {
                    // 错误退出
                    step = 0;
                    novar_print("image to feature error!\r\n", 25);
                    while (1);
                }
                break;
            case 2:
                ret = FNGR_Seach(&fngr_struct_send, CHARBUF1, 0, 4);

                if (ret[0] == ACK_OK)
                {
                    // 获取匹配指纹页号
                    step = 0;
                    novar_print((uint8_t *)"finger is match!\r\n", 18);
                    //pageno = ret[1] << 8 | ret[2];
                    APP_audio_stop();
                    APP_audio_list_ini();
                    APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                    APP_audio_list_add((uint16_t*)shuru, CNTshuru);
                    APP_audio_list_add((uint16_t*)zhengque, CNTzhengque);
                    APP_audio_play();
                    while (1);
                }
                else if (ret[0] == ACK_NoSearch)
                {
                    step = 0;
                    APP_audio_stop();
                    APP_audio_list_ini();
                    APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                    APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                    APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                    APP_audio_play();
                    novar_print("finger not match!\r\n", 19);
                    while (1);
                }
                else
                {
                    step = 0;
                    APP_audio_stop();
                    APP_audio_list_ini();
                    APP_audio_list_add((uint16_t*)zhiwen, CNTzhiwen);
                    APP_audio_list_add((uint16_t*)yanzheng, CNTyanzheng);
                    APP_audio_list_add((uint16_t*)shibai, CNTshibai);
                    APP_audio_play();
                    novar_print("finger match error!\r\n", 21);
                    while (1);
                }
            }
        }
    }
}

#undef _LOCAL_FINGER
/* *************************** end line ****************************/

