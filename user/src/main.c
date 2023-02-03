/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.1
  * @date    01/31/2023
  * @brief   Main program body
  ******************************************************************************
  ���汾�����gd32f103VBT6 �ڴ�20K��flash 128K��������������û�в���
  */ 

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "key.h"
#include "systick.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

/* Private function prototypes -----------------------------------------------*/
static void IAP_Init(void);

/* Private functions ---------------------------------------------------------*/


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //��ñ���ʱ��



/*
	����ָʾ��   PB4 �͵�ƽ����
*/



void Led_Show_Work_init(void)
{
		//ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOB);	
		
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	
	//Ϩ��
	gpio_bit_reset(GPIOB, GPIO_PIN_4);
}



static void Led_Show_Work_On(void)
{
	gpio_bit_reset(GPIOB, GPIO_PIN_4);
}

static void Led_Show_Work_Off(void)
{
	gpio_bit_set(GPIOB, GPIO_PIN_4);
}


//��ת
void Led_Show_Work_ToggleOut(void)
{
	uint8_t status = gpio_output_bit_get(GPIOB, GPIO_PIN_4);
	if(!status)
		Led_Show_Work_Off();
	else
		Led_Show_Work_On();

}





/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	uint8_t key;
	uint16_t count = 0;
	uint8_t enter_iap = 0;	
	
//	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x1d000);   //ע��仯������2023-02-01
	
	//0. �жϷ����ʼ��
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);    //2022-09-09 ���ȼ������޸��ˣ�����ֻ����ռ���ȼ��ˣ���
	
	//0.1 ���ù���ģ��ͨ��
    rcu_periph_clock_enable(RCU_AF);
	
	//ֻ����sw�ӿڣ���������GPIO�˿�
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	//���ڳ�ʼ��
	IAP_Init();
	
	//led���ų�ʼ������
	Led_Show_Work_init();
	
	SystickConfig();
	
	/* Flash unlock */
	fmc_unlock();

	//__enable_irq(); // �������ж�
	
	SerialPutString("=gd32 bootloader start======\r\n");
	//enter_iap = 1;
  /* Initialize Key Button mounted on STM3210X-EVAL board */
  //STM_EVAL_PBInit();
	while(count++ < 200)  //�ȴ�1000��
	{
		if (SerialKeyPressed((uint8_t*)&key))
		{
			if(key == 3)//(key == 'c' || key == 'C')  //ctrl + c
			{
				enter_iap = 1;
				break;
			}
		}
		Delay1ms(1)	;	
	}
  

  /*���������ݣ����Ƿ���*/
	if(enter_iap)
	{
		/* If Key is pressed */
		/* Execute the IAP driver in order to re-program the Flash */
		SerialPutString("\r\n========================================");
		SerialPutString("\r\n=           (C) COPYRIGHT 2023 HuNanHTJC BY dazhi                  =");
		SerialPutString("\r\n=                                                                  =");
		SerialPutString("\r\n=     In-Application Programming Application  (Version 3.3.1)      =");
		SerialPutString("\r\n=                                                                  =");
		SerialPutString("\r\n=     By Dazhi ");
		SerialPutString(g_build_time_str);
		SerialPutString("                     =");
		SerialPutString("\r\n========================================");
		SerialPutString("\r\n\r\n");
		Main_Menu ();
	}
  /* Keep the user application running */
	else
	{
		SerialPutString("= gd32 bootloader auto boot======\r\n");
		/* Test if user code is programmed starting from address "ApplicationAddress" */
		if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
		{ 
			/* Jump to user application */
			JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
			Jump_To_Application = (pFunction) JumpAddress;
			/* ���ó������е�ջ */
			__set_MSP(*(__IO uint32_t*) ApplicationAddress);
			Jump_To_Application();  //
		}
	}

  while (1)
  {}
}


//void gd32_disable_phy(void)
//{
//	rcu_periph_clock_disable(RCU_GPIOA);
//	rcu_periph_clock_disable(RCU_GPIOB);
//	rcu_periph_clock_disable(RCU_GPIOC);
//	rcu_periph_clock_disable(RCU_GPIOD);
//	rcu_periph_clock_disable(RCU_GPIOE);
//	rcu_periph_clock_disable(RCU_GPIOF);
//	rcu_periph_clock_disable(RCU_GPIOG);
//	rcu_periph_clock_disable(RCU_USART0);
//	rcu_periph_clock_disable(RCU_USART1);
//}



/**
  * @brief  Initialize the IAP: Configure RCC, USART and GPIOs.
  * @param  None
  * @retval None

	2023-01-31
	��Ҫ�ǳ�ʼ�����ڣ����ղ�ʹ���жϡ�GPA9,GPA10

  */
void IAP_Init(void)
{
    uint32_t com = USART0;
	
//	gd32_disable_phy();
	
    /*1. ʹ��GPIOʱ�� enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /*2. ʹ��uartʱ�� enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /*3. ���ų�ʼ��Ϊ���ù���ģʽ connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);

    /*4. ���ų�ʼ��Ϊ���ù���ģʽ connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_10);

    /*4. uart��������ʼ�� USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, 115200);
    usart_word_length_set(com, USART_WL_8BIT);   //����λ
    usart_stop_bit_set(com, USART_STB_1BIT);    //ֹͣλ
    usart_parity_config(com, USART_PM_NONE);    //У��λ
    usart_hardware_flow_rts_config(com, USART_RTS_DISABLE);   //����
    usart_hardware_flow_cts_config(com, USART_CTS_DISABLE);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
	
	//5. �����жϵĳ�ʼ����
	//usart_interrupt_enable(com, USART_INT_RBNE);    //�����ж�
//	usart_interrupt_enable(com, USART_INT_ERR);

	//6. nvic������
	//nvic_irq_enable(COM_NVIC[com_id],  COM_PRIO[com_id]>>2, COM_PRIO[com_id]&0x3);   //�����жϣ����������ȼ�
	//nvic_irq_enable(USART0_IRQn,  3, 0);	//ȫ������ռ���ȼ�

}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
