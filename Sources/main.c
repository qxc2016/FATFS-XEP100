/*====================================================================================================
*||||||||||||||Project      : FATFS-XEP100                                ||||||||||||||||||||||||||||
*||||||||||||||Author       : judee                                       ||||||||||||||||||||||||||||
*||||||||||||||Email        : judee@139.com                               ||||||||||||||||||||||||||||
*||||||||||||||Created      : 2018-6-30                                   ||||||||||||||||||||||||||||
*||||||||||||||Target       : MC9S12XEP100                                ||||||||||||||||||||||||||||
*||||||||||||||IDE          : CodeWarrior Development Studio for the S12(X) Version 5.1, build 10221 |
*||||||||||||||Simulator    : TBDML                                       ||||||||||||||||||||||||||||
======================================================================================================*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "common.h"



struct WSNSTATE  //λ�Σ���������״̬
{
unsigned diskok:1; //��ʼ�����̺��ļ�ϵͳ�ɹ���־λBIT0    0:δ��ʼ�����ʼ��ʧ��   1:�Ѿ���ʼ���ɹ�
unsigned fileok:1; //�����ļ��ɹ���־λBIT1                0:δ�����ļ����ߴ���ʧ�� 1:�ļ������ɹ�
unsigned record:1; //
unsigned position:1; //
};

/*********************ȫ�ֱ���������************/

volatile struct WSNSTATE state; //�ڵ�״̬
FATFS fs;            // Work area (file system object) for logical drive
FRESULT res;         // FatFs function common result code
UINT br,bw;         // File Write count

FIL faddata;  //�������ļ�����


volatile unsigned char pi[720]={0}; //�ļ�����
volatile unsigned int filebuffoffset=0;
unsigned char read_buff[512];
/*********************����������end************/
/***************���ܺ���******************/
void SCI0_putchar(unsigned char ch)			   //SCI0���ͺ���
{ 
  while(!(SCI0SR1&0x80)) ; 		    //keep waiting when not empty  
  SCI0DRL=ch;
}

void SCI0_puts(unsigned char *ptr)
{
	while (*ptr)
	{
		SCI0_putchar(*ptr++);
	}
}

void SCI0_put_Enter(void)
{
	SCI0_putchar(0x0D);	  //����
	SCI0_putchar(0x0A);  //�س�
}


void init_disk_fatfs(void)   //��ʼ�����̺��ļ�ϵͳ���ɹ���state.diskok==1;
{
	if(state.diskok==0)  //�������δ��ʼ�� 
	{
		if(disk_initialize(0))
		{
			state.diskok=0;
		}
		else
		{	
			res = f_mount(0,&fs);      // ע���ļ�ϵͳ������for logical drive 0
//			SCI0_putchar(res);
			state.diskok=1;
		}
	}
}
/***************���ܺ���end******************/
void main(void) {
  /* put your own code here */
  
    unsigned char r;
    unsigned int i;
    DIR hstdir;
    DWORD Len;
    char *str,*str1,*str2;
    str = "1,2,3,4,a,b,c,d";
    str1 = "Hello,world!\n��ã����磡\n��ŭ��.��ڻ���\n�󽭶�ȥ�����Ծ���ǧ�ŷ������\n";
    //for(i = 0;i < 2000;i+2) (str2 + i)  = "��";
    SYS_Int();
    init_disk_fatfs();
    if(state.diskok)            //SD����ʼ���ɹ���PB0��
        PORTB_PB0 = 0;
  //SD_ReadSingleBlock(0,fs.win);  
 	EnableInterrupts;
 	
 	for(i=0;i<=256;i++)
 	{
 	    filebuffoffset++;
 	    pi[i]='s';
 	}
 	if(f_opendir(&hstdir,"HstData") != RES_OK)  
 	{
 	  res = f_mkdir("HstData");     //�½���dir��name���Ȳ��ܳ���8,��ʹ��"historydata"ʱ��f_mkdir�ᱨ��invalid_name
 	  res = f_opendir(&hstdir,"HstData");	
 	} 	
 	//�����򸲸ǣ��������򴴽�
 	res = f_open(&faddata,"HstData/data.txt", FA_CREATE_ALWAYS | FA_WRITE);     //����һ����Ϊdata.txt���ļ�
// 	SCI0_putchar(res);
 	res = f_lseek(&faddata,f_size(&faddata)); //����������ļ����
 	res = f_write(&faddata,str1,strlen(str1), &bw); //���ļ��������������д��data.txt
// 	SCI0_putchar(res); 	
 	res = f_close(&faddata);   //�ر�data.txt
 	//
 	res = f_open(&faddata,"HstData/data.txt", FA_OPEN_ALWAYS | FA_WRITE);     //����һ����Ϊdata.txt���ļ�
 	res = f_lseek(&faddata,f_size(&faddata)); //����������ļ����
 	res = f_write(&faddata,str1,strlen(str1), &bw);//���ļ��������������д��data.txt
 	res = f_close(&faddata);                                        //�ر�data.txt
 	//
 	res = f_open(&faddata,"data1.txt", FA_CREATE_ALWAYS | FA_WRITE);     //����һ����Ϊdata.txt���ļ� 	
 	res = f_write(&faddata,str2,strlen(str2), &bw);            //���ļ��������������д��data.txt
 	res = f_close(&faddata);         
  //	
 	res = f_open(&faddata,"test.txt", FA_READ);                    //�Զ�����ʽ��test.txt�ļ�
// 	SCI0_putchar(res);    
 	 	
 	res = f_read(&faddata,read_buff,512,&br);                     //��test.txt�ж�ȡ512���ֽڵ����ݣ�����read_buff���������
// 	SCI0_putchar(res);
 	for(i=0;i<512;i++)
 	{
 	    SCI0_putchar(read_buff[i]);                              //���ڷ��ض���������
 	}
 	
 	


  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
