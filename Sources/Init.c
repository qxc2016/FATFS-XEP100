#include "Init.h"

/************时钟初始化***********/
void SetBusCLK_80M(void)
{   
    CLKSEL=0X00;				//disengage PLL to system
    PLLCTL_PLLON=1;			//turn on PLL
    SYNR =0xc0 | 0x09;                        
    REFDV=0xc0 | 0x01; 
    POSTDIV=0x00;       //pllclock=2*osc*(1+SYNR)/(1+REFDV)=80MHz;
    _asm(nop);          //BUS CLOCK=80M
    _asm(nop);
    while(!(CRGFLG_LOCK==1));	  //when pll is steady ,then use it;
    CLKSEL_PLLSEL =1;		        //engage PLL to system; 
}

void SetBusCLK_40M(void)
{   
    CLKSEL=0X00;				//disengage PLL to system
    PLLCTL_PLLON=1;			//turn on PLL
    SYNR =0xc0 | 0x04;                        
    REFDV=0xc0 | 0x01; 
    POSTDIV=0x00;       //pllclock=2*osc*(1+SYNR)/(1+REFDV)=80MHz;
    _asm(nop);          //BUS CLOCK=40M
    _asm(nop);
    while(!(CRGFLG_LOCK==1));	  //when pll is steady ,then use it;
    CLKSEL_PLLSEL =1;		        //engage PLL to system; 
}

/************SCI0初始化***********/ 
void SCI0_INT(void)
{
	SCI0CR1 =0x00;	//普通模式，无校验，1个起始位，8个数据位，1个停止位
    SCI0CR2 =0x2c; //enable Receive Full Interrupt,RX enable,Tx enable 接收中断使能，接收发送使能     
    SCI0BD  =40000000/38400/16; //SCI0BDL=busclk/(16*SCI0BDL)波特率38400
}

/************Spi初始化***********/
void SPI_Init(void)
{
    MODRR_MODRR4=0; //使用m口
    //DDRM = 0x31; //SCK0=1,MOSI=1,pm0为cs 0b00110001    
    SPI0CR1 = 0x5e; //CPOL=1,CPHA=1 0b01011110
    SPI0CR2 = 0x10; //0b00010000
    SPI0BR  = 0x07;  // 80M / 256 = 312.5k
}

/***********端口初始化***************/
void PORT_Int(void)
{
    DDRB = 0xff;
    PORTB = 0xff;
    DDRS_DDRS0 = 1;
    DDRS_DDRS1 = 0;
    DDRS_DDRS7 = 1;
    DDR0AD0_DDR0AD14 = 1;
    PT0AD0_PT0AD14 = 1;
}

/**************系统初始化*****************/
void SYS_Int(void)
{
    SetBusCLK_40M();
    PORT_Int();
    SCI0_INT();
    SPI_Init();
}



