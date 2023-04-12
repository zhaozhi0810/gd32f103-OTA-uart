/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.1
  * @date    01/31/2023
  * @brief   Main program body
  ******************************************************************************
  ���汾�����gd32f103VBT6 �ڴ�20K��flash 128K��������������û�в���
  0-23K : iap                (0~0x5c00-1)
  23-24K  : record flags    (0x5c00~0x6000-1)
  24-76K : app              (0x6000~0x13000-1)�����Զ������ļ�����ܳ���52k��0xd000��
  76-128K : back download   (0x13000~0x20000-1)
  
  1. �����ص�  download ������Ȼ�����سɹ����������سɹ���־�����ٸ��Ƶ� app���� �����ø��Ƴɹ���־��
  2. ��¼��������md5��app����md5������ͬ�´�������ʱ���������
  3. ����ǰ�Ƚ���������md5��ʵ����Ҫ���ص�md5
  4. ��һ��128�ֽڵİ�����Ϊ�ļ��������64�ֽڣ�����\0��,�ļ����ȣ����16�ֽڣ�����\0�������������ļ���md5��33���ֽڣ�����\0��md5��Ҫ32���ֽڴ洢��
  5. ���md5��ͬ������ֹ���أ�ֱ�ӽ���app
  6. ����ʧ��ʱ��������������������������md5��
  */ 

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "key.h"
#include "systick.h"
#include "uart.h"
#include "update_flag.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;


/* Private functions ---------------------------------------------------------*/


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //��ñ���ʱ��

/*
	rk3399�����������ص�back����ͬʱ������������md5
	����Ǵ���������ֱ�����ص�app����������������md5��app��md5
*/
uint8_t is_cpu_update_cmd = 0;   //��rk3399��������





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


static void OePins_Control_Init(void)
{
	//1. ʱ��ʹ��
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2.0 �ϵ��������
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  //�������	
	//2. ��ʼ����Ĭ�������
	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 �����
	
	gpio_bit_set(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3);  //���� �����
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
	
	
	OePins_Control_Init();    //OE���ŵĳ�ʼ��
	
	//���ڳ�ʼ��
	IAP_Init(USART0);
	
	//led���ų�ʼ������
	Led_Show_Work_init();
	
	SystickConfig();
	
	/* Flash unlock */
	fmc_unlock();
	
	
	//�ж��Ƿ���Ҫͨ��rk3399����
	if(is_mcu_need_download())
	{
		SerialPutString("=is_mcu_need_download == 1  ====\r\n");
		{
			IAP_Init(USART1);   //��ʼ������
			is_cpu_update_cmd = 1;   //����һ����־
			if(0 == SerialDownload())
			{
				mcu_download_done();
				flash_download_copyto_app();  //���سɹ������һ�ο�����
			}
		}
	}
	//�ж��Ƿ���Ҫ��down��������app��
	else if(is_mcu_need_update())
	{		
		SerialPutString("=is_mcu_need_update == 1  ====\r\n");
		flash_download_copyto_app();
				
	}
	else  //���Դ����Ƿ���Ҫ����
	{
		//__enable_irq(); // �������ж�
		
		SerialPutString("=gd32 bootloader start======\r\n");
		//enter_iap = 1;
	  /* Initialize Key Button mounted on STM3210X-EVAL board */
	  //STM_EVAL_PBInit();
		while(count++ < 200)  //�ȴ�1000��
		{
			if (SerialKeyPressed_Uart0((uint8_t*)&key))
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
			
		}
	}
	
	
	//����app
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
	
	SerialPutString("= gd32 bootloader error while 1======\r\n");
	while (1)
	{
		SerialPutString("= gd32 bootloader error while 1======\r\n");
		Delay1ms(1000);
	}
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
