#ifndef _IAP_H_
#define _IAP_H_

#include <stdint.h>

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   
#define FLASH_APP1_ADDR		0x08004000  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X08007FFF�Ŀռ�ΪBootloaderʹ��(��32KB)	   
void iap_load_app(uint32_t appxaddr);			//��ת��APP����ִ��
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t applen);	//��ָ����ַ��ʼ,д��bin
void MSR_MSP(uint32_t addr);
#endif

