/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive*/

#include "diskio.h"
#include "sd_driver.h"

struct tm {
   	int tm_sec;   // 秒 seconds after the minute, 0 to 60
   	int tm_min;   // 分 minutes after the hour, 0 to 59
	int tm_hour;  // 时 hours since midnight, 0 to 23*
	int tm_mday;  // 日 day of the month, 1 to 31
	int tm_mon;   // 月 months since January, 0 to 11
	int tm_year;  // 年 years since 1900
	int tm_wday;  // 星期 days since Sunday, 0 to 6
	int tm_yday;  // 从元旦起的天数 days since January 1, 0 to 365
 	int tm_isdst; // 夏令时？？Daylight Savings Time flag
 	};

DSTATUS disk_initialize ( 
BYTE drv				/* Physical drive nmuber (0..) */
)
{
    unsigned char state;

    if(drv)
    {
        return STA_NOINIT;  //仅支持磁盘0的操作
    }

    state = SD_Init();  //初始化SD卡，SD_driver层
    if(state == STA_NODISK)
    {
        return STA_NODISK;
    }
    else if(state != 0)
    {
        return STA_NOINIT;  //其他错误：初始化失败
    }
    else
    {
        return 0;           //初始化成功
    }
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
    if(drv)
    {
        return STA_NOINIT;  //仅支持磁盘0操作
    }

    //检查SD卡是否插入
//    if(!SD_DET())
//    {
//        return STA_NODISK;
//    }
    return 0;
}


FATFS fstest; 
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	unsigned char res=0;
    if (drv || !count) //不是0磁盘或者 Number of sectors 为0
    {    
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }

//    if(!SD_DET())
//    {
//        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
//    }

    if(count==1)            //1个sector的读操作      
    {                                                        
        SD_ReadSingleBlock(0, fstest.win);      //SD_driver层
        SD_ReadSingleBlock(8192, fstest.win);      //SD_driver层
        res = SD_ReadSingleBlock(sector, buff);      //SD_driver层
    }                                                
    else                    //多个sector的读操作     
    {                                                
        res = SD_ReadMultiBlock(sector, buff, count);
    }                                                

    //处理返回值，将SD_driver.c的返回值转成ff.c的返回值
    if(res == 0x00)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	unsigned char res;

    if (drv || !count)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }

//    if(!SD_DET())
//    {
//        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
//    }


    // 读写操作
    if(count == 1)
    {
        res = SD_WriteSingleBlock(sector, buff);
    }
    else
    {
        res = SD_WriteMultiBlock(sector, buff, count);
    }
    // 返回值转换
    if(res == 0)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)

{
    DRESULT res;


    if (drv)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，否则返回参数错误
    }
    
    //FATFS目前版本仅需处理CTRL_SYNC，GET_SECTOR_COUNT，GET_BLOCK_SIZ三个命令
    switch(ctrl)
    {
    case CTRL_SYNC:
        SD_CS_ENABLE();
        if(SD_WaitReady()==0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
        SD_CS_DISABLE();
        break;
        
    case GET_BLOCK_SIZE:
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(DWORD*)buff = SD_GetCapacity();
        res = RES_OK;
        break;
    default:
        res = RES_PARERR;
        break;
    }

    return res;
}


/*-----------------------------------------------------------------------*/
/* User defined function to give a current time to fatfs module      */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */ 

DWORD get_fattime (void)
{
 /*   struct tm t;
    DWORD date;
    t.tm_year=2009-1980;		//年份改为1980年起
    t.tm_mon=8;         	//0-11月改为1-12月
	t.tm_mday=3;
	t.tm_hour=15;
	t.tm_min=30;
    t.tm_sec=20;      	//将秒数改为0-29
	date =t.tm_year;
	date=date<<7;
	date+=t.tm_mon;
	date=date<<4;
	date+=t.tm_mday;
	date=date<<5;
	date+=t.tm_hour;
	date=date<<5;
	date+=t.tm_min;
	date=date<<5;
	date+=t.tm_sec;
	date=1950466004;
*/
    return 1950466005;
}


