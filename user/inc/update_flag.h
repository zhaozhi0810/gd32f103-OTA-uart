


#ifndef __UPDATE_FLAG_H__
#define __UPDATE_FLAG_H__
#include "gd32f10x.h"

typedef struct update_flag
{
	uint16_t need_update;    //需要升级吗？0xff或者0x0都是不升级，其他值需要升级
	uint16_t update_success;  //升级成功了吗？0xff是未成功，其他值表示成功
	uint16_t update_where;    //升级的渠道，0xff表示调试串口，其他值表示与3399通信的串口
}update_flag_t;


//单片机是否需要升级？
//1.need_update 标志不是0xff或者0x0
//2.update_success 标志不是0xff或者0x0 表示上一次升级失败。
//返回值是0表示不需要升级，非0表示要升级
uint8_t is_mcu_need_update(void);


//升级完成，擦除23k，该块全部为0xff
void mcu_update_done(void);


//返回值1表示从调试串口升级，0则表示从通信串口升级
uint8_t is_update_from_debug_uart(void);


//循环发送准备接收标志
void send_readyto_recive(void);
#endif
