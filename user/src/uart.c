
#include "gd32f10x.h"
#include "common.h"
/**
  * @brief  Initialize the IAP: Configure RCC, USART and GPIOs.
  * @param  None
  * @retval None

	2023-01-31
	主要是初始化串口，接收不使用中断。GPA9,GPA10

	2023-04-07
	增加串口1，PA2，PA3，与3399连接，准备从3399使用ymodem方式更新软件
	但是调试信息仍然从串口0打印

  */
  
static uint32_t update_com_real = USART1;   //需要看看命令是从哪来的
  
  
void IAP_Init(uint32_t com)
{	
    /*1. 使能GPIO时钟 enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
//	com_real = USART0;   //实际使用的是串口0
	
	if(com == USART1)  //USART1 仍然需要USART0的协助
	{
		update_com_real = USART1; //实际使用的是串口1
		/*2. 使能uart时钟 enable USART clock */
		rcu_periph_clock_enable(RCU_USART1);
		/*3. 引脚初始化为复用功能模式 connect port to USARTx_Tx */
		gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
		/*4. 引脚初始化为复用功能模式 connect port to USARTx_Rx */
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_3);		
	}
	else{
		/*2. 使能uart时钟 enable USART clock */
		rcu_periph_clock_enable(RCU_USART0);
		/*3. 引脚初始化为复用功能模式 connect port to USARTx_Tx */
		gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
		/*4. 引脚初始化为复用功能模式 connect port to USARTx_Rx */
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	}
	
	/*4. uart控制器初始化 USART configure */
	usart_deinit(com);
	usart_baudrate_set(com, 115200);
	usart_word_length_set(com, USART_WL_8BIT);   //数据位
	usart_stop_bit_set(com, USART_STB_1BIT);    //停止位
	usart_parity_config(com, USART_PM_NONE);    //校验位
	usart_hardware_flow_rts_config(com, USART_RTS_DISABLE);   //流控
	usart_hardware_flow_cts_config(com, USART_CTS_DISABLE);
	usart_receive_config(com, USART_RECEIVE_ENABLE);
	usart_transmit_config(com, USART_TRANSMIT_ENABLE);
	usart_enable(com);	
}



void set_download_uart(uint8_t index)
{
	if(index == 0)
	{
		update_com_real = USART0;   //从调试串口下载更新。
	}
	else
		update_com_real = USART1;
}





/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key)
{
	if (SET == usart_flag_get(update_com_real, USART_FLAG_RBNE))
	{
		*key = (uint8_t)usart_data_receive(update_com_real);
	//	printf("key = %#x\r\n",*key);
		return 1;
	}
	else
	{
		return 0;
	}
}


//从通讯串口下载
uint32_t SerialKeyPressed_Uart1(uint8_t *key)
{
	if (SET == usart_flag_get(USART1, USART_FLAG_RBNE))
	{
		*key = (uint8_t)usart_data_receive(USART1);
	//	printf("key = %#x\r\n",*key);
		return 1;
	}
	else
	{
		return 0;
	}
}

//调试串口按键检测
uint32_t SerialKeyPressed_Uart0(uint8_t *key)
{
	if (SET == usart_flag_get(USART0, USART_FLAG_RBNE))
	{
		*key = (uint8_t)usart_data_receive(USART0);
	//	printf("key = %#x\r\n",*key);
		return 1;
	}
	else
	{
		return 0;
	}
//	return 0;
}

/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c)
{
	while(RESET == usart_flag_get(update_com_real, USART_FLAG_TC));
	usart_data_transmit(update_com_real, (uint8_t)c); 
}


void SerialPutChar_uart1(uint8_t c)
{
	while(RESET == usart_flag_get(USART1, USART_FLAG_TC));
	usart_data_transmit(USART1, (uint8_t)c); 
}

void SerialPutChar_uart0(uint8_t c)
{
	while(RESET == usart_flag_get(USART0, USART_FLAG_TC));
	usart_data_transmit(USART0, (uint8_t)c); 
}


//调试串口输出printf
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
//	if(update_com_real != USART0)
		SerialPutChar_uart0(ch);
	return ch;
}

