


#ifndef __UPDATE_FLAG_H__
#define __UPDATE_FLAG_H__
#include "gd32f10x.h"

typedef struct update_flag
{
	uint16_t need_update;    //��Ҫ������0xff����0x0���ǲ�����������ֵ��Ҫ����
	uint16_t update_success;  //�����ɹ�����0xff��δ�ɹ�������ֵ��ʾ�ɹ�
	uint16_t update_where;    //������������0xff��ʾ���Դ��ڣ�����ֵ��ʾ��3399ͨ�ŵĴ���
}update_flag_t;


//��Ƭ���Ƿ���Ҫ������
//1.need_update ��־����0xff����0x0
//2.update_success ��־����0xff����0x0 ��ʾ��һ������ʧ�ܡ�
//����ֵ��0��ʾ����Ҫ��������0��ʾҪ����
uint8_t is_mcu_need_update(void);


//������ɣ�����23k���ÿ�ȫ��Ϊ0xff
void mcu_update_done(void);


//����ֵ1��ʾ�ӵ��Դ���������0���ʾ��ͨ�Ŵ�������
uint8_t is_update_from_debug_uart(void);


//ѭ������׼�����ձ�־
void send_readyto_recive(void);
#endif
