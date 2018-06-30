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



struct WSNSTATE  //位段，保存各组件状态
{
unsigned diskok:1; //初始化磁盘和文件系统成功标志位BIT0    0:未初始化或初始化失败   1:已经初始化成功
unsigned fileok:1; //创建文件成功标志位BIT1                0:未创建文件或者创建失败 1:文件创建成功
unsigned record:1; //
unsigned position:1; //
};

/*********************全局变量定义区************/

volatile struct WSNSTATE state; //节点状态
FATFS fs;            // Work area (file system object) for logical drive
FRESULT res;         // FatFs function common result code
UINT br,bw;         // File Write count

FIL faddata;  //创建的文件对象


volatile unsigned char pi[720]={0}; //文件缓存
volatile unsigned int filebuffoffset=0;
unsigned char read_buff[512];
/*********************变量定义区end************/
/***************功能函数******************/
void SCI0_putchar(unsigned char ch)			   //SCI0发送函数
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
	SCI0_putchar(0x0D);	  //换行
	SCI0_putchar(0x0A);  //回车
}


void init_disk_fatfs(void)   //初始化磁盘和文件系统，成功了state.diskok==1;
{
	if(state.diskok==0)  //如果磁盘未初始化 
	{
		if(disk_initialize(0))
		{
			state.diskok=0;
		}
		else
		{	
			res = f_mount(0,&fs);      // 注册文件系统工作区for logical drive 0
//			SCI0_putchar(res);
			state.diskok=1;
		}
	}
}
/***************功能函数end******************/
void main(void) {
  /* put your own code here */
  
    unsigned char r;
    unsigned int i;
    DIR hstdir;
    DWORD Len;
    char *str,*str1,*str2;
    str = "1,2,3,4,a,b,c,d";
    str1 = "Hello,world!\n你好，世界！\n念怒娇.赤壁怀古\n大江东去，浪淘尽，千古风流人物。\n";
    //for(i = 0;i < 2000;i+2) (str2 + i)  = "和";
    SYS_Int();
    init_disk_fatfs();
    if(state.diskok)            //SD卡初始化成功，PB0亮
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
 	  res = f_mkdir("HstData");     //新建的dir的name长度不能超过8,如使用"historydata"时，f_mkdir会报错invalid_name
 	  res = f_opendir(&hstdir,"HstData");	
 	} 	
 	//存在则覆盖，不存在则创建
 	res = f_open(&faddata,"HstData/data.txt", FA_CREATE_ALWAYS | FA_WRITE);     //创建一个名为data.txt的文件
// 	SCI0_putchar(res);
 	res = f_lseek(&faddata,f_size(&faddata)); //将光标移至文件最后
 	res = f_write(&faddata,str1,strlen(str1), &bw); //将文件缓冲区里的数据写入data.txt
// 	SCI0_putchar(res); 	
 	res = f_close(&faddata);   //关闭data.txt
 	//
 	res = f_open(&faddata,"HstData/data.txt", FA_OPEN_ALWAYS | FA_WRITE);     //创建一个名为data.txt的文件
 	res = f_lseek(&faddata,f_size(&faddata)); //将光标移至文件最后
 	res = f_write(&faddata,str1,strlen(str1), &bw);//将文件缓冲区里的数据写入data.txt
 	res = f_close(&faddata);                                        //关闭data.txt
 	//
 	res = f_open(&faddata,"data1.txt", FA_CREATE_ALWAYS | FA_WRITE);     //创建一个名为data.txt的文件 	
 	res = f_write(&faddata,str2,strlen(str2), &bw);            //将文件缓冲区里的数据写入data.txt
 	res = f_close(&faddata);         
  //	
 	res = f_open(&faddata,"test.txt", FA_READ);                    //以读的形式打开test.txt文件
// 	SCI0_putchar(res);    
 	 	
 	res = f_read(&faddata,read_buff,512,&br);                     //从test.txt中读取512个字节的数据，存入read_buff这个数组中
// 	SCI0_putchar(res);
 	for(i=0;i<512;i++)
 	{
 	    SCI0_putchar(read_buff[i]);                              //串口发回读出的数据
 	}
 	
 	


  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
