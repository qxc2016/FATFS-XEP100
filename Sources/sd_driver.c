/*******************************************************************************
*  ���ļ�ΪSPI����SD���ĵײ������ļ�
*  ����SPIģ�鼰���IO�ĳ�ʼ����SPI��дSD����дָ������ݵȣ�
*******************************************************************************/

#include <hidef.h>      /* common defines and macros */
#include <MC9S12XEP100.h>     /* derivative information */
#include "sd_driver.h"
//#include "uart.h"

unsigned char SD_Type=0 ;

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

void SPI_Configuration(void)
{
    MODRR_MODRR4=1; //ʹ��m��
    DDRM = 0x31; //SCK0=1,MOSI=1,pm0Ϊcs 0b00110001
    SPI0CR1 = 0x5e; //CPOL=1,CPHA=1 0b01011110
    SPI0CR2 = 0x10; //0b00010000
    SPI0BR  = 0x07;  // 80M / 256 = 312.5k
}
   

/*******************************************************************************
* Function Name  : SPI_SetSpeed
* Description    : SPI�����ٶ�Ϊ����
* Input          : unsigned char SpeedSet
*                  ����ٶ���������0�������ģʽ����0�����ģʽ
*                  SPI_SPEED_HIGH   1
*                  SPI_SPEED_LOW    0
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_SetSpeed(unsigned char SpeedSet)
{
    //����ٶ���������0�������ģʽ����0�����ģʽ
    if(SpeedSet==SPI_SPEED_LOW)
    {
     // set low speed
        SPI0CR1 = 0x5e; //CPOL=1,CPHA=1 0b01011110
        SPI0CR2 = 0x10; //0b00010000
        SPI0BR  = 0x07;  // 40M / 256 = 156.25k
    }
    else
    {
	    SPI0BR  = 0x02;  // 40M / 8 = 5M       ���Ը��ߵ�~~
    }
    return ;
}

/*******************************************************************************
* Function Name  : SPI_ReadWriteByte
* Description    : SPI��дһ���ֽڣ�������ɺ󷵻ر���ͨѶ��ȡ�����ݣ�
* Input          : unsigned char TxData �����͵���
* Output         : None
* Return         : unsigned char RxData �յ�����
*******************************************************************************/
unsigned char SPI_ReadWriteByte(unsigned char TxData)
{
    unsigned char spi_tmp;
  
    while (!SPI0SR_SPTEF);
    SPI0DRL = TxData;
    while (!SPI0SR_SPIF);
    spi_tmp = SPI0DRL;
    return spi_tmp;
}

/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : �ȴ�SD��Ready
* Input          : None
* Output         : None
* Return         : unsigned char
*               0�� �ɹ�
*           other��ʧ��
*******************************************************************************/

unsigned char SD_WaitReady(void)
{
    unsigned char r1 ;
    unsigned char retry ;
    retry=0 ;
    do 
    {
        r1=SPI_ReadWriteByte(0xFF);
        if(retry++==200)//������쳣������ѭ����
        {
            return 1 ;
        }
    }
    while(r1!=0xFF);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : ��SD������һ������
* Input          : unsigned char cmd   ����
*                  unsigned long arg  �������
*                  unsigned char crc   crcУ��ֵ
* Output         : None
* Return         : unsigned char r1 SD�����ص���Ӧ
*******************************************************************************/
unsigned char SD_SendCommand(unsigned char cmd,unsigned long arg,unsigned char crc)
{
    unsigned char r1 ;
    unsigned char Retry=0 ;
    

    SPI_ReadWriteByte(0xff);
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS_ENABLE();
    
    //����
    SPI_ReadWriteByte(cmd|0x40);
    //�ֱ�д������
    SPI_ReadWriteByte(arg>>24);
    SPI_ReadWriteByte(arg>>16);
    SPI_ReadWriteByte(arg>>8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc);
    
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)
        {
        break ;
        }
    }

    //�ر�Ƭѡ
    SD_CS_DISABLE();
    //�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
    SPI_ReadWriteByte(0xFF);
    
    //����״ֵ̬
    return r1 ;
}


/*******************************************************************************
* Function Name  : SD_SendCommand_NoDeassert
* Description    : ��SD������һ������(����ʱ��ʧ��Ƭѡ�����к������ݴ�����
* Input          : unsigned char cmd   ����
*                  unsigned long arg  �������
*                  unsigned char crc   crcУ��ֵ
* Output         : None
* Return         : unsigned char r1 SD�����ص���Ӧ
*******************************************************************************/
unsigned char SD_SendCommand_NoDeassert(unsigned char cmd,unsigned long arg,unsigned char crc)
{
    unsigned char r1 ;
    unsigned char Retry=0 ;
    
    //�ȸ���8��ʱ��
    SPI_ReadWriteByte(0xff);
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS_ENABLE();
    
    //����
    SPI_ReadWriteByte(cmd|0x40);
    //�ֱ�д������
    SPI_ReadWriteByte(arg>>24);
    SPI_ReadWriteByte(arg>>16);
    SPI_ReadWriteByte(arg>>8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc);
    
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)
        {
            break ;
        }
    }
    //������Ӧֵ
    return r1 ;
}

/*******************************************************************************
* Function Name  : SD_Init
* Description    : ��ʼ��SD��
* Input          : None
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  1��TIME_OUT
*                  99��NO_CARD
*******************************************************************************/
unsigned char SD_Init(void)
{

    unsigned int i ;
    // ����ѭ������
    unsigned char r1 ;
    // ���SD���ķ���ֵ
    unsigned int retry ;
    // �������г�ʱ����
    unsigned char buff[6];
    
//	put_s("SDinit...");

    // ����ʱ���ȴ�SD���ϵ����
    for(i=0;i<0xf00;i++);
    
    //�Ȳ���>74�����壬��SD���Լ���ʼ�����
    for(i=0;i<10;i++)
    {
        SPI_ReadWriteByte(0xFF);
    }

    //-----------------SD����λ��idle��ʼ-----------------
    //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
    //��ʱ��ֱ���˳�
    retry=0 ;
    do 
    {
        //����CMD0����SD������IDLE״̬
        r1=SD_SendCommand(CMD0,0,0x95);
        retry++;
    }
    while((r1!=0x01)&&(retry<200));
    //����ѭ���󣬼��ԭ�򣺳�ʼ���ɹ���or ���Գ�ʱ��
    if(retry==200)
    {   
//		put_s("SD Time out!");
        return 1 ;
        //��ʱ����1
    }
//	put_s("SdEnterIdle ");
    //-----------------SD����λ��idle����-----------------
    

    //��ȡ��Ƭ��SD�汾��Ϣ
    r1=SD_SendCommand_NoDeassert(8,0x1aa,0x87);
    
    //�����Ƭ�汾��Ϣ��v1.0�汾�ģ���r1=0x05����������³�ʼ��
    if(r1==0x05)
    {
        //���ÿ�����ΪSDV1.0����������⵽ΪMMC�������޸�ΪMMC
//		put_s("SD V1 ");
        SD_Type=SD_TYPE_V1 ;
        
        //�����V1.0����CMD8ָ���û�к�������
        //Ƭѡ�øߣ�������������
        SD_CS_DISABLE();
        //�෢8��CLK����SD������������
        SPI_ReadWriteByte(0xFF);
        
        //-----------------SD����MMC����ʼ����ʼ-----------------
        
        //������ʼ��ָ��CMD55+ACMD41
        // �����Ӧ��˵����SD�����ҳ�ʼ�����
        // û�л�Ӧ��˵����MMC�������������Ӧ��ʼ��
        retry=0 ;
        do 
        {
            //�ȷ�CMD55��Ӧ����0x01���������
            r1=SD_SendCommand(CMD55,0,0);
            if(r1!=0x01)
            {
                return r1 ;
            }
            //�õ���ȷ��Ӧ�󣬷�ACMD41��Ӧ�õ�����ֵ0x00����������400��
            r1=SD_SendCommand(ACMD41,0,0);
            retry++;
        }
        while((r1!=0x00)&&(retry<400));
        // �ж��ǳ�ʱ���ǵõ���ȷ��Ӧ
        // ���л�Ӧ����SD����û�л�Ӧ����MMC��
        
        //----------MMC�������ʼ��������ʼ------------
        if(retry==400)
        {
            retry=0 ;
            //����MMC����ʼ�����û�в��ԣ�
            do 
            {
                r1=SD_SendCommand(1,0,0);
                retry++;
            }
            while((r1!=0x00)&&(retry<400));
            if(retry==400)
            {
                return 1 ;
                //MMC����ʼ����ʱ
            }
            //д�뿨����
            SD_Type=SD_TYPE_MMC ;
        }
        //----------MMC�������ʼ����������------------
        
        //����SPIΪ����ģʽ
        SPI_SetSpeed(1);

        SPI_ReadWriteByte(0xFF);
        
        //��ֹCRCУ��
        /*
        		r1 = SD_SendCommand(CMD59, 0, 0x01);
                if(r1 != 0x00)
                {
                    return r1;  //������󣬷���r1
                }
                */
        //����Sector Size
        r1=SD_SendCommand(CMD16,512,0xff);
        if(r1!=0x00)
        {
            return r1 ;
            //������󣬷���r1
        }
        //-----------------SD����MMC����ʼ������-----------------
        
    }
    //SD��ΪV1.0�汾�ĳ�ʼ������
    
    
    //������V2.0���ĳ�ʼ��
    //������Ҫ��ȡOCR���ݣ��ж���SD2.0����SD2.0HC��
    else if(r1==0x01)
    {
//		put_s("SD V2 ");
        //V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
        buff[0]=SPI_ReadWriteByte(0xFF);
        //should be 0x00
        buff[1]=SPI_ReadWriteByte(0xFF);
        //should be 0x00
        buff[2]=SPI_ReadWriteByte(0xFF);
        //should be 0x01
        buff[3]=SPI_ReadWriteByte(0xFF);
        //should be 0xAA
        
        SD_CS_DISABLE();
        //the next 8 clocks
        SPI_ReadWriteByte(0xFF);
        
        //�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
        if(buff[2]==0x01&&buff[3]==0xAA)
        {
            //֧�ֵ�ѹ��Χ�����Բ���
            retry=0 ;
            //������ʼ��ָ��CMD55+ACMD41
            do 
            {
                r1=SD_SendCommand(CMD55,0,0);
                if(r1!=0x01)
                {
                    return r1 ;
                }
                r1=SD_SendCommand(ACMD41,0x40000000,0);
                if(retry>200)
                {
                    return r1 ;
                    //��ʱ�򷵻�r1״̬
                }
            }
            while(r1!=0);
            
            //��ʼ��ָ�����ɣ���������ȡOCR��Ϣ
            
            //-----------����SD2.0���汾��ʼ-----------
            r1=SD_SendCommand_NoDeassert(CMD58,0,0);
            if(r1!=0x00)
            {
                return r1 ;
                //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��
            }
            //��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
            buff[0]=SPI_ReadWriteByte(0xFF);
            buff[1]=SPI_ReadWriteByte(0xFF);
            buff[2]=SPI_ReadWriteByte(0xFF);
            buff[3]=SPI_ReadWriteByte(0xFF);
            
            //OCR������ɣ�Ƭѡ�ø�
            SD_CS_DISABLE();
            SPI_ReadWriteByte(0xFF);
            
            //�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
            //���CCS=1��SDHC   CCS=0��SD2.0
            //���CCS
            if(buff[0]&0x40)
            {
                SD_Type=SD_TYPE_V2HC ;
//				put_s("SDHC ");

            }
            else 
            {
                SD_Type=SD_TYPE_V2 ;
            }
            //-----------����SD2.0���汾����-----------
            
            
            //����SPIΪ����ģʽ
            SPI_SetSpeed(1);
        }
        
    }
    return r1 ;
}



/*******************************************************************************
* Function Name  : SD_ReceiveData
* Description    : ��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
* Input          : unsigned char *data(��Ŷ������ݵ��ڴ�>len)
*                  unsigned int len(���ݳ��ȣ�
*                  unsigned char release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  other��������Ϣ
*******************************************************************************/
unsigned char SD_ReceiveData(unsigned char*data,unsigned int len,unsigned char release)
{
    unsigned int retry ;
    unsigned char r1 ;
    
    // ����һ�δ���
    SD_CS_ENABLE();
    //�ȴ�SD������������ʼ����0xFE
    retry=0 ;
    do 
    {
        r1=SPI_ReadWriteByte(0xFF);
        //retry�εȴ���û��Ӧ���˳�����
        if(retry++>0xFFFE)
        {
//			put_s("SD tout");

            SD_CS_DISABLE();
            return 1 ;
        }
    }
    while(r1!=0xFE);
    //��ʼ��������
    while(len--)
    {
        *data=SPI_ReadWriteByte(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    SPI_ReadWriteByte(0xFF);
    SPI_ReadWriteByte(0xFF);
    //�����ͷ����ߣ���CS�ø�
    if(release==RELEASE)
    {
        //�������
        SD_CS_DISABLE();
        SPI_ReadWriteByte(0xFF);
    }

    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_GetCID
* Description    : ��ȡSD����CID��Ϣ��������������Ϣ
* Input          : unsigned char *cid_data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
unsigned char SD_GetCID(unsigned char*cid_data)
{
    unsigned char r1 ;
    
    //��CMD10�����CID
    r1=SD_SendCommand(CMD10,0,0xFF);
    if(r1!=0x00)
    {
        return r1 ;
        //û������ȷӦ�����˳�������
    }
    //����16���ֽڵ�����
    SD_ReceiveData(cid_data,16,RELEASE);

    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_GetCSD
* Description    : ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
* Input          : unsigned char *cid_data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
unsigned char SD_GetCSD(unsigned char *csd_data)
{
    unsigned char r1 ;
    
    //��CMD9�����CSD
    r1=SD_SendCommand(CMD9,0,0xFF);
    if(r1!=0x00)
    {
        return r1 ;
        //û������ȷӦ�����˳�������
    }
    //����16���ֽڵ�����
    SD_ReceiveData(csd_data,16,RELEASE);
    
    return 0 ;
}

/*******************************************************************************
* Function Name  : SD_GetCapacity
* Description    : ��ȡSD��������
* Input          : None
* Output         : None
* Return         : unsigned long capacity
*                   0�� ȡ��������
*******************************************************************************/
unsigned long SD_GetCapacity(void)
{
    unsigned char csd[16];
    unsigned long Capacity ;
    unsigned char r1 ;
    unsigned int i ;
    unsigned int temp ;
    
    //ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd)!=0)
    {
        return 0 ;
    }

    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)
    {
        Capacity=((((unsigned long)csd[8])<<8)+(unsigned long)csd[9]+1)*(unsigned long)1024 ;
    }
    else 
    {
        //�������Ϊ���ϰ汾
        ////////////formula of the capacity///////////////
        //
        //  memory capacity = BLOCKNR * BLOCK_LEN
        //
        //	BLOCKNR = (C_SIZE + 1)* MULT
        //
        //           C_SIZE_MULT+2
        //	MULT = 2
        //
        //               READ_BL_LEN
        //	BLOCK_LEN = 2
        /**********************************************/
        //C_SIZE
        i=csd[6]&0x03 ;
        i<<=8 ;
        i+=csd[7];
        i<<=2 ;
        i+=((csd[8]&0xc0)>>6);
        
        //C_SIZE_MULT
        r1=csd[9]&0x03 ;
        r1<<=1 ;
        r1+=((csd[10]&0x80)>>7);
        
        //BLOCKNR
        r1+=2 ;
        temp=1 ;
        while(r1)
        {
            temp*=2 ;
            r1--;
        }
        Capacity=((unsigned long)(i+1))*((unsigned long)temp);
        
        // READ_BL_LEN
        i=csd[5]&0x0f ;
        //BLOCK_LEN
        temp=1 ;
        while(i)
        {
            temp*=2 ;
            i--;
        }
        //The final result
        Capacity*=(unsigned long)temp ;
        //Capacity /= 512;
    }
    return (unsigned long)Capacity ;
}


/*******************************************************************************
* Function Name  : SD_ReadSingleBlock
* Description    : ��SD����һ��block
* Input          : unsigned long sector ������ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_ReadSingleBlock(unsigned long sector,unsigned char*buffer)
{
    unsigned char r1 ;
    //����Ϊ����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    
    //�������SDHC����sector��ַת��byte��ַ
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    
    r1=SD_SendCommand(CMD17,sector,0);
    //������
    
    if(r1!=0x00)
    {
  
        return r1 ;
    }
    
    r1=SD_ReceiveData(buffer,512,RELEASE);
    if(r1!=0)
    {
        return r1 ;
        //�����ݳ���
    }
    else 
    {
        return 0 ; //success

    }
}

/*******************************************************************************
* Function Name  : SD_WriteSingleBlock
* Description    : д��SD����һ��block
* Input          : unsigned long sector ������ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_WriteSingleBlock(unsigned long sector,const unsigned char *data)
{
    unsigned char r1 ;
    unsigned int i ;
    unsigned int retry ;
    
    //����Ϊ����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    
    r1=SD_SendCommand(CMD24,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS_ENABLE();
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    //����ʼ����0xFE
    SPI_ReadWriteByte(0xFE);
    
    //��һ��sector������
    for(i=0;i<512;i++)
    {
        SPI_ReadWriteByte(*data++);
    }
    //��2��Byte��dummy CRC
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    
    //�ȴ�SD��Ӧ��
    r1=SPI_ReadWriteByte(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS_DISABLE();
        return r1 ;
    }
    
    //�ȴ��������
    retry=0 ;
    while(!SPI_ReadWriteByte(0xff))
    {
        retry++;
        //�����ʱ��д��û����ɣ������˳�
        if(retry>0xfffe)
        {
            SD_CS_DISABLE();
            return 1 ;
            //д�볬ʱ����1
        }
    }
    
    //д����ɣ�Ƭѡ��1
    SD_CS_DISABLE();
    SPI_ReadWriteByte(0xff);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_ReadMultiBlock
* Description    : ��SD���Ķ��block
* Input          : unsigned long sector ȡ��ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
*                  unsigned char count ������count��block
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_ReadMultiBlock(unsigned long sector,unsigned char *buffer,unsigned char count)
{
    unsigned char r1 ;
    
    //����Ϊ����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    
    //�������SDHC����sector��ַת��byte��ַ
    sector=sector<<9 ;
    //SD_WaitReady();
    //�����������
    r1=SD_SendCommand(CMD18,sector,0);
    //������
    if(r1!=0x00)
    {
        return r1 ;
    }
    //��ʼ��������
    do 
    {
        if(SD_ReceiveData(buffer,512,NO_RELEASE)!=0x00)
        {
            break ;
        }
        buffer+=512 ;
    }
    while(--count);
    
    //ȫ��������ϣ�����ֹͣ����
    SD_SendCommand(CMD12,0,0);
    //�ͷ�����
    SD_CS_DISABLE();
    SPI_ReadWriteByte(0xFF);
    
    if(count!=0)
    {
        return count ;
        //���û�д��꣬����ʣ�����
    }
    else 
    {
        return 0 ;
    }
}


/*******************************************************************************
* Function Name  : SD_WriteMultiBlock
* Description    : д��SD����N��block
* Input          : unsigned long sector ������ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
*                  unsigned char count д���block��Ŀ
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_WriteMultiBlock(unsigned long sector,const unsigned char*data,unsigned char count)
{
    unsigned char r1 ;
    unsigned int i ;
    
    //����Ϊ����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    //���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����
    if(SD_Type!=SD_TYPE_MMC)
    {
        r1=SD_SendCommand(ACMD23,count,0x00);
    }
    //�����д��ָ��
    r1=SD_SendCommand(CMD25,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS_ENABLE();
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    
    //--------������N��sectorд���ѭ������
    do 
    {
        //����ʼ����0xFC �����Ƕ��д��
        SPI_ReadWriteByte(0xFC);
        
        //��һ��sector������
        for(i=0;i<512;i++)
        {
            SPI_ReadWriteByte(*data++);
        }
        //��2��Byte��dummy CRC
        SPI_ReadWriteByte(0xff);
        SPI_ReadWriteByte(0xff);
        
        //�ȴ�SD��Ӧ��
        r1=SPI_ReadWriteByte(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS_DISABLE();
            //���Ӧ��Ϊ��������������ֱ���˳�
            return r1 ;
        }
        
        //�ȴ�SD��д�����
        if(SD_WaitReady()==1)
        {
            SD_CS_DISABLE();
            //�ȴ�SD��д����ɳ�ʱ��ֱ���˳�����
            return 1 ;
        }
        
        //��sector���ݴ������
    }
    while(--count);
    
    //��������������0xFD
    r1=SPI_ReadWriteByte(0xFD);
    if(r1==0x00)
    {
        count=0xfe ;
    }
    
    if(SD_WaitReady())
    {
        while(1){}
    }
    
    //д����ɣ�Ƭѡ��1
    SD_CS_DISABLE();
    SPI_ReadWriteByte(0xff);
    
    return count ;
    //����countֵ�����д����count=0������count=1
}
