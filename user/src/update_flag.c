

#include "gd32f10x.h"
#include "update_flag.h"
#include "common.h"
/*
VBT6  �ܹ���128Kflash����ǰ��24K�ó�������iap��ʣ��108K����app(��ʼ��ַ0x6000)
����0-22K��23k����iap����23k(0x5c00)���λ�����ڱ���һЩ��־λupdate_flag_t�ṹ���е����ݣ�

���flash ��1kҳ�������ͱ�̶���1kΪ��λ��
*/

extern uint8_t tab_1024[1024];
extern uint8_t md5sum_down[34];  //���md5ֵ
extern int32_t Size;

//��ʼ��ַ
static update_flag_t *g_updateflag = (void*)UPDATE_FLAG_START_ADDR;

//��Ƭ���Ƿ���Ҫ������
//1.need_update ��־����0xff����0x0
//2.update_success ��־����0xff����0x0 ��ʾ��һ������ʧ�ܡ�
//����ֵ��0��ʾ����Ҫ��������0��ʾҪ����
uint8_t is_mcu_need_update(void)
{
	if(g_updateflag->need_update == 0xffff)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //��Ҫ����
	}	
	return 0;  //����Ҫ����
}
	


//��Ƭ���Ƿ���Ҫ���أ�0xffff���ǲ���Ҫ����,����������Ҫ���أ�
//����1��ʾ��Ҫ���أ�0��ʾ����Ҫ
uint8_t is_mcu_need_download(void)
{
	if(g_updateflag->need_download != 0xffff)
	{
		SerialPutString("=is_mcu_need_update 1 ====\r\n");
		return 1;	 //��Ҫ����
	}	
	return 0;  //����Ҫ����
}


void mcu_download_done(void)
{
	uint8_t i;
	uint32_t md5sum_addr = UPDATE_FLAG_START_ADDR + DOWN_MD5_OFFET; //md5����ʼ��ַ
	
	fmc_unlock();
	SerialPutString("=mcu_download_done====\r\n");
	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //������־�����ã�δ�ɹ���ʶ�����ã���Ҫappȥ�����
	
	//������Ҫ������д��md5��download����
	for(i=0;i<32;i+=4)
	{
		fmc_word_program(md5sum_addr, *(uint32_t*)(md5sum_down+i));
		md5sum_addr += 4;   //app_addr һֱ�ٸ���
	}
	
	if(Size > 1024)  //���ٴ���1k
	{
		fmc_word_program(UPDATE_FLAG_START_ADDR+4, (uint32_t)(Size));
	}
	
}



//�������
void mcu_update_done(void)
{
	SerialPutString("=mcu_update_done====\r\n");
//	fmc_page_erase(UPDATE_FLAG_START_ADDR);  //������־�������δ�ɹ���ʶ�����ã���Ҫappȥ�����
	
	fmc_unlock();
	fmc_halfword_program(UPDATE_FLAG_START_ADDR, 0xff);   //����Ϊ0xff����ʾ����Ҫ������
}


//����ֵ1��ʾ�ӵ��Դ���������0���ʾ��ͨ�Ŵ�������
uint8_t is_update_from_debug_uart(void)
{
	return 0;
//	return g_updateflag->update_where == 0xffff;
}





//flash ����
uint8_t flash_download_copyto_app(void)
{
	uint16_t i,j;
	uint8_t count = 0;
	uint8_t *download_addr = (void*) ApplicationDownAddress;   //flash�Ĵ�С��ʵ����76k(0x8013000)��λ�ã�download����ʼ��ַ
	uint32_t app_addr = ApplicationAddress; //app����ʼ��ַ 
	uint32_t size,size_read = 0;
	fmc_unlock();
	
	if (((*(__IO uint32_t*)download_addr) & 0xfFFE0000 ) != 0x20000000)
	{
		printf("not update\r\n");
		mcu_update_done();   //���±�־����������
		return 2;   //����������rom����
	}
	else if(((*(__IO uint32_t*)(download_addr+4)) & 0xfFFff000 ) != ApplicationAddress)
	{
		printf("not update 3\r\n");
		mcu_update_done();   //���±�־����������
		return 2;   //����������rom����
	}
	
	
	size = Size > 1024? Size:g_updateflag->firm_size;
	if(size<1024 || size > FLASH_IMAGE_SIZE)
	{
		size = FLASH_IMAGE_SIZE;
	}
	printf("flash_download_copyto_app size = %#x // %d\r\n",size,size);
	
	while(size_read< size)
	{
		for(i=0;(i<1024);i++,size_read++)  //��download����ȡ����
		{
			tab_1024[i] = download_addr[size_read];
			if(tab_1024[i] == 0xff)  //��������10��0xff����������
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

		//д�뵽app��
		fmc_page_erase(app_addr);
		
		//��һ����д��1024���ֽ�		
		for(j=0;j<i;j+=4)
		{
			fmc_word_program(app_addr, *(uint32_t*)(tab_1024+j));
			app_addr += 4;   //app_addr һֱ�ٸ���
		}

		if(count >= 16)  //���
		{
			break;
		}
	}
	
	//���ø��³ɹ��ı�־���޸�app md5ֵ��
	mcu_update_done();
	
	return 0;
}



