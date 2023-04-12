

#include "gd32f10x.h"
#include "update_flag.h"
#include "common.h"
/*
VBT6  �ܹ���128Kflash����ǰ��24K�ó�������iap��ʣ��108K����app(��ʼ��ַ0x6000)
����0-22K��23k����iap����23k(0x5c00)���λ�����ڱ���һЩ��־λupdate_flag_t�ṹ���е����ݣ�

���flash ��1kҳ�������ͱ�̶���1kΪ��λ��
*/


#define UPDATE_FLAG_START_ADDR   (ApplicationAddress-PAGE_SIZE)     //������ʼ��ַ��0x805c00







//��ʼ��ַ
static update_flag_t *g_updateflag = (void*)UPDATE_FLAG_START_ADDR;

//��Ƭ���Ƿ���Ҫ������
//1.need_update ��־����0xff����0x0
//2.update_success ��־����0xff����0x0 ��ʾ��һ������ʧ�ܡ�
//����ֵ��0��ʾ����Ҫ��������0��ʾҪ����
uint8_t is_mcu_need_update(void)
{
	if(g_updateflag->need_update != 0xffff && g_updateflag->need_update != 0)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //��Ҫ����
	}
	else if(g_updateflag->update_success != 0xffff)	
	{
		SerialPutString("=is_mcu_need_update 2 ====\r\n");
		return 1;   //����δ�ɹ�����Ҫ����
	}
	
	return 0;  //����Ҫ����
}
	

//������ɣ�����23k���ÿ�ȫ��Ϊ0xff
void mcu_update_done(void)
{
	SerialPutString("=mcu_update_done====\r\n");
	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //������־�������δ�ɹ���ʶ�����ã���Ҫappȥ�����
}


//����ֵ1��ʾ�ӵ��Դ���������0���ʾ��ͨ�Ŵ�������
uint8_t is_update_from_debug_uart(void)
{
	return g_updateflag->update_where == 0xffff;
}



//ѭ������׼�����ձ�־
void send_readyto_recive(void)
{
	printf("send_readyto_recive flag\r\n");
	SerialPutChar(0x43);
//	SerialPutChar(0xaa);	
}




