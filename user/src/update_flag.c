

#include "gd32f10x.h"
#include "update_flag.h"
#include "common.h"
/*
VBT6  总共有128Kflash，把前面24K拿出来用于iap，剩下108K用于app(起始地址0x6000)
其中0-22K共23k用于iap程序，23k(0x5c00)这个位置用于保存一些标志位update_flag_t结构体中的内容，

这个flash 是1k页，擦除和编程都是1k为单位。
*/


#define UPDATE_FLAG_START_ADDR   (ApplicationAddress-PAGE_SIZE)     //设置起始地址，0x805c00







//起始地址
static update_flag_t *g_updateflag = (void*)UPDATE_FLAG_START_ADDR;

//单片机是否需要升级？
//1.need_update 标志不是0xff或者0x0
//2.update_success 标志不是0xff或者0x0 表示上一次升级失败。
//返回值是0表示不需要升级，非0表示要升级
uint8_t is_mcu_need_update(void)
{
	if(g_updateflag->need_update != 0xffff && g_updateflag->need_update != 0)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //需要升级
	}
	else if(g_updateflag->update_success != 0xffff)	
	{
		SerialPutString("=is_mcu_need_update 2 ====\r\n");
		return 1;   //升级未成功，需要升级
	}
	
	return 0;  //不需要升级
}
	

//升级完成，擦除23k，该块全部为0xff
void mcu_update_done(void)
{
	SerialPutString("=mcu_update_done====\r\n");
	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //升级标志被清除，未成功标识被设置（需要app去清除）
}


//返回值1表示从调试串口升级，0则表示从通信串口升级
uint8_t is_update_from_debug_uart(void)
{
	return g_updateflag->update_where == 0xffff;
}



//循环发送准备接收标志
void send_readyto_recive(void)
{
	printf("send_readyto_recive flag\r\n");
	SerialPutChar(0x43);
//	SerialPutChar(0xaa);	
}




