


#ifndef __UPDATE_FLAG_H__
#define __UPDATE_FLAG_H__
#include "gd32f10x.h"

typedef struct update_flag
{
	uint16_t need_update;    //需要升级（从back区拷贝到app区）吗？0xffff是需要升级(同时表示升级不成功)，0x00ff表示升级成功
	uint16_t need_download;    //需要升级吗？0xffff是不需要下载(同时表示下载成功)，0x00ff表示需要下载，
	uint32_t firm_size;       //固件大小，下载的值
}update_flag_t;


//单片机是否需要升级？
//1.need_update 标志不是0xff或者0x0
//2.update_success 标志不是0xff或者0x0 表示上一次升级失败。
//返回值是0表示不需要升级，非0表示要升级
uint8_t is_mcu_need_update(void);


//升级完成，擦除23k，该块全部为0xff
void mcu_update_done(void);


//返回值1表示从调试串口升级，0则表示从通信串口升级
//uint8_t is_update_from_debug_uart(void);


//返回1表示需要下载，0表示不需要
uint8_t is_mcu_need_download(void);


//flash 拷贝
uint8_t flash_download_copyto_app(void);


//下载完成
void mcu_download_done(void);
//循环发送准备接收标志
//void send_readyto_recive(void);
#endif
