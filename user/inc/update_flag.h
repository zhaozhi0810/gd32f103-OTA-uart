


#ifndef __UPDATE_FLAG_H__
#define __UPDATE_FLAG_H__
#include "gd32f10x.h"

typedef struct update_flag
{
	uint16_t need_update;    //��Ҫ��������back��������app������0xffff����Ҫ����(ͬʱ��ʾ�������ɹ�)��0x00ff��ʾ�����ɹ�
	uint16_t need_download;    //��Ҫ������0xffff�ǲ���Ҫ����(ͬʱ��ʾ���سɹ�)��0x00ff��ʾ��Ҫ���أ�
	uint32_t firm_size;       //�̼���С�����ص�ֵ
}update_flag_t;


//��Ƭ���Ƿ���Ҫ������
//1.need_update ��־����0xff����0x0
//2.update_success ��־����0xff����0x0 ��ʾ��һ������ʧ�ܡ�
//����ֵ��0��ʾ����Ҫ��������0��ʾҪ����
uint8_t is_mcu_need_update(void);


//������ɣ�����23k���ÿ�ȫ��Ϊ0xff
void mcu_update_done(void);


//����ֵ1��ʾ�ӵ��Դ���������0���ʾ��ͨ�Ŵ�������
//uint8_t is_update_from_debug_uart(void);


//����1��ʾ��Ҫ���أ�0��ʾ����Ҫ
uint8_t is_mcu_need_download(void);


//flash ����
uint8_t flash_download_copyto_app(void);


//�������
void mcu_download_done(void);
//ѭ������׼�����ձ�־
//void send_readyto_recive(void);
#endif
