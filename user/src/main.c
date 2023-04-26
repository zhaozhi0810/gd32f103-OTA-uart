/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.1
  * @date    01/31/2023
  * @brief   Main program body
  ******************************************************************************
  本版本仅针对gd32f103VBT6 内存20K，flash 128K的情况，其他情况没有测试
  0-23K : iap                (0~0x5c00-1)
  23-24K  : record flags    (0x5c00~0x6000-1)
  24-76K : app              (0x6000~0x13000-1)，所以二进制文件最大不能超过52k（0xd000）
  76-128K : back download   (0x13000~0x20000-1)
  
  2023-04-12
  1. 先下载到  download 分区，然后下载成功（设置下载成功标志），再复制到 app分区 （设置复制成功标志）
  2. 记录下载区的md5和app区的md5，不相同下次重启的时候完成升级
  3. 下载前比较下载区的md5与实际需要下载的md5
  4. 第一个128字节的包，改为文件名（最大64字节，包括\0）,文件长度（最大16字节，包括\0），增加下载文件的md5（33个字节，包括\0，md5需要32个字节存储）
  5. 如果md5相同，则终止下载，直接进入app
  6. 下载失败时，不予升级，不更新下载区的md5码
  
  2023-04-13
  1.一个重要的问题，是调试串口下载的时候是没有md5码的。需要注意处理一下。（目前调试串口还没有测试）
  2.进入下载模式后，多久退出下载模式。2分钟？
  3.如何表示现在app分区的程序是正常的？
  
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
	rk3399的升级先下载到back区域，同时更新下载区的md5
	如果是串口升级，直接下载到app区，更新下载区的md5和app的md5
*/
uint8_t is_cpu_update_cmd = 0;   //是rk3399的升级吗？





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



void enter_main_menu(void)
{
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




//800ms 看门狗
static void iwdog_init(void)
{	
	fwdgt_write_enable();	
	fwdgt_config(0xfff,FWDGT_PSC_DIV64);    //设置分配系数,FWDGT_PSC_DIV8最长800ms，FWDGT_PSC_DIV32最大3s		

	//	fwdgt_config(0xfff,FWDGT_PSC_DIV64);    //设置分配系数,最长6s	
	//fwdgt_counter_reload();  //等待时间约3s
	fwdgt_enable(); //使能看门狗

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
	
	//只保留sw接口，其他用于GPIO端口，否则led无法闪烁
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	SystickConfig();
	
	OePins_Control_Init();    //OE引脚的初始化，允许串口输出
	
	//串口初始化
	IAP_Init(USART0);
	IAP_Init(USART1);   //初始化串口1，通讯串口

	//led引脚初始化部分
	Led_Show_Work_init();
			
	/* Flash unlock */
	fmc_unlock();
	
	
	SerialPutString("\r\niap start(by dazhi@jc), init ok!!\r\n");
	SerialPutString((char*)g_build_time_str);   //此时还没有打印任务，不要打印太多数据
	SerialPutString("\r\n");

	//判断是否需要通过rk3399升级
	if(is_mcu_need_download())
	{
		set_download_uart(1);
		SerialPutString("=is_mcu_need_download == 1  ====\r\n");
		{			
			is_cpu_update_cmd = 1;   //设置一个标志
			goto_ota_update();
			//enter_main_menu();   //进入菜单，等待
//			if(0 == SerialDownload())
//			if(0 == goto_ota_update())
//			{
//				mcu_download_done();
//				flash_download_copyto_app();  //下载成功后完成一次拷贝。
//			}
		}
	}
	//判断是否需要从down区拷贝到app区
	else if(is_mcu_need_update())
	{		
		SerialPutString("=is_mcu_need_update == 1  ====\r\n");
		flash_download_copyto_app();				
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
			if (SerialKeyPressed_Uart0((uint8_t*)&key)) //是否有调试串口的命令？？
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
			enter_main_menu();
		}
		/* Keep the user application running */
		else
		{
			//nothing to do;
		}
	}
	
	
	if (((*(__IO uint32_t*)ApplicationAddress) & 0xfFFE0000 ) != 0x20000000)
	{
		Main_Menu ();
	}
	else if(((*(__IO uint32_t*)(ApplicationAddress+4)) & 0xfFFff000 ) != ApplicationAddress)
	{
		Main_Menu ();
	}
	
	
	//启动app
	SerialPutString("= gd32 bootloader auto boot======\r\n");
	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
	{ 
		/* Jump to user application */
		//1.开启看门狗，防止app程序卡死，在app中一定要喂狗，否则cup重启，
		iwdog_init();
		//2.启动app
		JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* 设置程序运行的栈 */
		__set_MSP(*(__IO uint32_t*) ApplicationAddress);
		Jump_To_Application();  //
	}
	
	SerialPutString("= gd32 bootloader error while 1======\r\n");
	
	//启动失败的处理，进入到菜单，这时应该还需要接受cpu的串口指令。
	while (1)
	{
		SerialPutString("= gd32 bootloader error while 1======\r\n");
		enter_main_menu();
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
