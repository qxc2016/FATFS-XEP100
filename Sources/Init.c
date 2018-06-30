#include "Init.h"

/************ʱ�ӳ�ʼ��***********/
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

/************SCI0��ʼ��***********/ 
void SCI0_INT(void)
{
	SCI0CR1 =0x00;	//��ͨģʽ����У�飬1����ʼλ��8������λ��1��ֹͣλ
    SCI0CR2 =0x2c; //enable Receive Full Interrupt,RX enable,Tx enable �����ж�ʹ�ܣ����շ���ʹ��     
    SCI0BD  =40000000/38400/16; //SCI0BDL=busclk/(16*SCI0BDL)������38400
}

/************Spi��ʼ��***********/
void SPI_Init(void)
{
    MODRR_MODRR4=0; //ʹ��m��
    //DDRM = 0x31; //SCK0=1,MOSI=1,pm0Ϊcs 0b00110001    
    SPI0CR1 = 0x5e; //CPOL=1,CPHA=1 0b01011110
    SPI0CR2 = 0x10; //0b00010000
    SPI0BR  = 0x07;  // 80M / 256 = 312.5k
}

/***********�˿ڳ�ʼ��***************/
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

/**************ϵͳ��ʼ��*****************/
void SYS_Int(void)
{
    SetBusCLK_40M();
    PORT_Int();
    SCI0_INT();
    SPI_Init();
}



