

#ifndef __UART_H__
#define __UART_H__

#include "gd32f10x.h"

//com = USART0,USART1
void IAP_Init(uint32_t com);


/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c);

void SerialPutChar_uart1(uint8_t c);

void SerialPutChar_uart0(uint8_t c);  //���Դ��ڴ�ӡ��Ϣ
/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key);

//���Դ��ڰ������
uint32_t SerialKeyPressed_Uart0(uint8_t *key);

//��ͨѶ��������
uint32_t SerialKeyPressed_Uart1(uint8_t *key);


//�������ش��ڣ�index:0��ʾ���Դ��ڣ�����ֵ��ʾ��rk3399��ͨѶ����
void set_download_uart(uint8_t index);

#endif


