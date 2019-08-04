#include "action_flash.h"
#include "stm32f407xx.h"
#include "action_sys.h"


//����STM32��FLASH
void STMFLASH_Unlock(void)
{
	FLASH->KEYR=FLASH_KEY1;	//д���������.
	FLASH->KEYR=FLASH_KEY2; 
}
//flash����
void STMFLASH_Lock(void)
{
	FLASH->CR|=(uint32_t)1<<31;//����
}
//�õ�FLASH״̬
//����ֵ:
//0,�������
//1,æ 
//2,�����쳣 
uint8_t STMFLASH_GetStatus(void)
{	
	 uint32_t res=0;		
	res=FLASH->SR;  
	if(res&(1<<16))return 1;   		//æ
	else if(res&(1<<4))return 2;	//�����쳣 
	else if(res&(1<<5))return 2;	//�����쳣 
	else if(res&(1<<6))return 2;	//�����쳣 
	else if(res&(1<<7))return 2;	//�����쳣 
	return 0;						//û���κ�״̬/�������.
} 
//�ȴ��������
//time:Ҫ��ʱ�ĳ���(��λ:10us)
//����ֵ:
//0,���
//2,�����쳣
//0XFF,��ʱ       
uint8_t STMFLASH_WaitDone(uint32_t time)
{
	uint8_t res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		Delay_Ms(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
//��������
//sectoraddr:������ַ,��Χ��:0~11.
//0~3,16K����;4,64K����;5~11,128K����.
//����ֵ:ִ�����
uint8_t STMFLASH_EraseSector(uint32_t sectoraddr)
{
	uint8_t res=0;
	res=STMFLASH_WaitDone(200000);//�ȴ��ϴβ�������,���2s    
	if(res==0)
	{ 
		FLASH->CR&=~(3<<8);	//���PSIZEԭ��������
		FLASH->CR|=2<<8;	//����Ϊ32bit��,ȷ��VCC=2.7~3.6V֮��!!
		FLASH->CR&=~(0X1F<<3);//���ԭ��������
		FLASH->CR|=sectoraddr<<3;//����Ҫ���������� 
		FLASH->CR|=1<<1;	//�������� 
		FLASH->CR|=1<<16;	//��ʼ����		  
		res=STMFLASH_WaitDone(200000);//�ȴ���������,���2s  
		if(res!=1)			//��æ
		{
			FLASH->CR&=~(1<<1);//�������������־.
		}
	}
	return res;
}
//��FLASHָ����ַдһ����
//faddr:ָ����ַ(�˵�ַ����Ϊ4�ı���!!)
//dat:Ҫд�������
//����ֵ:0,д��ɹ�
//    ����,д��ʧ��
uint8_t STMFLASH_WriteWord(uint32_t faddr, uint32_t dat)
{
	uint8_t res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR&=~(3<<8);	//���PSIZEԭ��������
		FLASH->CR|=2<<8;	//����Ϊ32bit��,ȷ��VCC=2.7~3.6V֮��!!
 		FLASH->CR|=1<<0;	//���ʹ��
		*(__IO uint32_t *)faddr=dat;	//д������
		res=STMFLASH_WaitDone(0XFF);//�ȴ��������,һ���ֱ��,���100us.
		if(res!=1)//�����ɹ�
		{
			FLASH->CR&=~(1<<0);//���PGλ.
		}
	} 
	return res;
} 
//��ȡָ����ַ��һ����(32λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(__IO uint32_t *)faddr; 
}  
//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint8_t STMFLASH_GetFlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return 0;
	else if(addr<ADDR_FLASH_SECTOR_2)return 1;
	else if(addr<ADDR_FLASH_SECTOR_3)return 2;
	else if(addr<ADDR_FLASH_SECTOR_4)return 3;
	else if(addr<ADDR_FLASH_SECTOR_5)return 4;
	else if(addr<ADDR_FLASH_SECTOR_6)return 5;
	else if(addr<ADDR_FLASH_SECTOR_7)return 6;
	else if(addr<ADDR_FLASH_SECTOR_8)return 7;
	else if(addr<ADDR_FLASH_SECTOR_9)return 8;
	else if(addr<ADDR_FLASH_SECTOR_10)return 9;
	else if(addr<ADDR_FLASH_SECTOR_11)return 10; 
	return 11;	
}
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F(ע�⣺���16�ֽڣ�����OTP���ݿ�����������д����)
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
void STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{ 
	uint8_t status=0;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
  	if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
	STMFLASH_Unlock();									//���� 
 	FLASH->ACR&=~(1<<10);			//FLASH�����ڼ�,�����ֹ���ݻ���!!!�����������ϲŷ����������!
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
	{
		while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
				status=STMFLASH_EraseSector(STMFLASH_GetFlashSector(addrx));
				if(status)break;	//����������
			}else addrx+=4;
		} 
	}
	if(status==0)
	{
		while(WriteAddr<endaddr)//д����
		{
			if(STMFLASH_WriteWord(WriteAddr,*pBuffer))//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
	FLASH->ACR|=1<<10;		//FLASH��������,��������fetch
	STMFLASH_Lock();//����
} 

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(32λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}

//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(uint32_t WriteAddr,uint32_t WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}


