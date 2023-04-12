

#include "gd32f10x.h"
#include "update_flag.h"
#include "common.h"
/*
VBT6  总共有128Kflash，把前面24K拿出来用于iap，剩下108K用于app(起始地址0x6000)
其中0-22K共23k用于iap程序，23k(0x5c00)这个位置用于保存一些标志位update_flag_t结构体中的内容，

这个flash 是1k页，擦除和编程都是1k为单位。
*/

extern uint8_t tab_1024[1024];
extern uint8_t md5sum_down[34];  //存放md5值
extern int32_t Size;

//起始地址
static update_flag_t *g_updateflag = (void*)UPDATE_FLAG_START_ADDR;

//单片机是否需要升级？
//1.need_update 标志不是0xff或者0x0
//2.update_success 标志不是0xff或者0x0 表示上一次升级失败。
//返回值是0表示不需要升级，非0表示要升级
uint8_t is_mcu_need_update(void)
{
	if(g_updateflag->need_update == 0xffff)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //需要升级
	}	
	return 0;  //不需要升级
}
	


//单片机是否需要下载，0xffff就是不需要下载,其他就是需要下载，
//返回1表示需要下载，0表示不需要
uint8_t is_mcu_need_download(void)
{
	if(g_updateflag->need_download != 0xffff)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //需要升级
	}	
	return 0;  //不需要升级
}


void mcu_download_done(void)
{
	uint8_t i;
	uint32_t md5sum_addr = UPDATE_FLAG_START_ADDR + DOWN_MD5_OFFET; //md5的起始地址
	
	fmc_unlock();
	SerialPutString("=mcu_download_done====\r\n");
	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //升级标志被设置，未成功标识被设置（需要app去清除）
	
	//设置需要升级，写入md5到download分区
	for(i=0;i<32;i+=4)
	{
		fmc_word_program(md5sum_addr, *(uint32_t*)(md5sum_down+i));
		md5sum_addr += 4;   //app_addr 一直再更新
	}
	
	if(Size > 1024)  //至少大于1k
	{
		fmc_word_program(UPDATE_FLAG_START_ADDR+4, (uint32_t)(Size));
	}
	
}



//升级完成
void mcu_update_done(void)
{
	SerialPutString("=mcu_update_done====\r\n");
//	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //升级标志被清除，未成功标识被设置（需要app去清除）
	
	fmc_unlock();
	fmc_halfword_program(UPDATE_FLAG_START_ADDR, 0xff);   //设置为0xff，表示不需要升级了
}


//返回值1表示从调试串口升级，0则表示从通信串口升级
uint8_t is_update_from_debug_uart(void)
{
	return 0;
//	return g_updateflag->update_where == 0xffff;
}





//flash 拷贝
uint8_t flash_download_copyto_app(void)
{
	uint16_t i,j;
	uint8_t count = 0;
	uint8_t *download_addr = (void*) ApplicationDownAddress;   //flash的大小，实际是76k(0x8013000)的位置，download的起始地址
	uint32_t app_addr = ApplicationAddress; //app的起始地址 
	uint32_t size,size_read = 0;
	fmc_unlock();
	
	if (((*(__IO uint32_t*)download_addr) & 0xfFFE0000 ) != 0x20000000)
	{
		printf("not update\r\n");
		mcu_update_done();   //更新标志，不再升级
		return 2;   //不能升级，rom不对
	}
	else if(((*(__IO uint32_t*)(download_addr+4)) & 0xfFFff000 ) != ApplicationAddress)
	{
		printf("not update 3\r\n");
		mcu_update_done();   //更新标志，不再升级
		return 2;   //不能升级，rom不对
	}
	
	
	size = Size > 1024? Size:g_updateflag->firm_size;
	if(size<1024 || size > FLASH_IMAGE_SIZE)
	{
		size = FLASH_IMAGE_SIZE;
	}
	printf("flash_download_copyto_app size = %#x // %d\r\n",size,size);
	
	while(size_read< size)
	{
		for(i=0;(i<1024);i++,size_read++)  //从download区读取出来
		{
			tab_1024[i] = download_addr[size_read];
			if(tab_1024[i] == 0xff)  //连续出现10次0xff，拷贝结束
			{
				count++;
				if(count >= 16)
					break;
			}
			else
			{
				count = 0;
			}			
		}

		//写入到app区
		fmc_page_erase(app_addr);
		
		//不一定是写入1024个字节		
		for(j=0;j<i;j+=4)
		{
			fmc_word_program(app_addr, *(uint32_t*)(tab_1024+j));
			app_addr += 4;   //app_addr 一直再更新
		}

		if(count >= 16)  //完成
		{
			break;
		}
	}
	
	//设置更新成功的标志，修改app md5值。
	mcu_update_done();
	
	return 0;
}



