# FATFS-XEP100
***********************************************************
Time:  2018-6-30
Autor: Judee
Email: Judee@139.com
本工程基于飞思卡尔MC9S12XEP100单片机，具有FATFS功能，已经调试通过，可以直接使用。
This project is based on (FSL)Freescale's MC9S12XEP100 MCU with FATFS function. It has been debugged and can be used directly.
***********************************************************
==========================NOTICE===========================

1.本软件工程从网上下载而来，原工程使用的是XS128型号的MCU；
2.下载后，通过CW的MCU变更功能，将工程从XS128变更为了XEP100；
3.结合硬件，对和SD卡通信的SPI，电源进行了初始化；
4.sd_driver.c文件里的SD_ReadSingleBlock函数有个bug，导致f_open函数返回错误，
  bug:
     sector = sector << 9;
  应该修改成下述：
     if()SD_Type != SD_Type_V2HC)
	 {
	   sector = sector << 9;
	 }
5.修复了第4点描述的bug后，即可成功读写文件。
===========================================================
	 
