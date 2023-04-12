/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.1
  * @date    01/31/2023
  * @brief   Main program body
  ******************************************************************************
  本版本仅针对gd32f103VBT6 内存20K，flash 128K的情况，其他情况没有测试
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


const char* g_build_time_str = "Buildtime :"__DATE__" "__TIME__;   //获得编译时间



/*
	工作指示灯   PB4 低电平点亮
*/



void Led_Show_Work_init(void)
{
	//时钟使能
	rcu_periph_clock_enable(RCU_GPIOB);	
		
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4);	
	//熄灭
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


//翻转
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
	//1. 时钟使能
	rcu_periph_clock_enable(RCU_GPIOC);
		
	//2.0 上电控制引脚
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  //控制输出	
	//2. 初始化后，默认输出高
	gpio_bit_reset(GPIOC, GPIO_PIN_2);  //OE3 输出低
	
	gpio_bit_set(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3);  //其他 输出高
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
	
//	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x1d000);   //注意变化！！！2023-02-01
	
	//0. 中断分组初始化
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);    //2022-09-09 优先级被我修改了，现在只有抢占优先级了！！
	
	//0.1 复用功能模块通电
    rcu_periph_clock_enable(RCU_AF);
	
	//只保留sw接口，其他用于GPIO端口
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	
	OePins_Control_Init();    //OE引脚的初始化
	
	//串口初始化
	IAP_Init(USART0);
	
	//led引脚初始化部分
	Led_Show_Work_init();
	
	SystickConfig();
	
	/* Flash unlock */
	fmc_unlock();
	
	
	//判断是否需要通过rk3399升级
	if(is_mcu_need_update())
	{		
		SerialPutString("=is_mcu_need_update == 1  ====\r\n");
		
		if(is_update_from_debug_uart() == 0)  //从rk3399串口升级？
		{
			IAP_Init(USART1);   //初始化串口
			
			//send_readyto_recive();  //发送开始接收标志
			/* Download user application in the Flash */
			SerialDownload();
		}		
	}
	else  //调试串口是否需要升级
	{
		//__enable_irq(); // 开启总中断
		
		SerialPutString("=gd32 bootloader start======\r\n");
		//enter_iap = 1;
	  /* Initialize Key Button mounted on STM3210X-EVAL board */
	  //STM_EVAL_PBInit();
		while(count++ < 200)  //等待1000次
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
	

	  /*读串口数据，看是否有*/
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
	
	
	//启动app
	SerialPutString("= gd32 bootloader auto boot======\r\n");
	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
	{ 
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* 设置程序运行的栈 */
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
