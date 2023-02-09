#include "driver_I2C.h"
#include "driver_delay.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_rcc.h"


void SDA_IN_2(void)
{
		Pad_Config(IIC_GPIO_SDA_PIN_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
		Pinmux_Config(IIC_GPIO_SDA_PIN_2, DWGPIO);

	
	RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
 
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin		= GPIO_GetPin(IIC_GPIO_SDA_PIN_2);
	GPIO_InitStruct.GPIO_Mode		= GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_ITCmd		= DISABLE;
	GPIO_InitStruct.GPIO_ITTrigger	= GPIO_INT_Trigger_EDGE;
	GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
	GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
	GPIO_InitStruct.GPIO_DebounceTime = 10;
	GPIO_Init(&GPIO_InitStruct);


}

void SDA_OUT_2(void)
{

	Pad_Config(IIC_GPIO_SDA_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
//	Pinmux_Config(IIC_GPIO_SDA_PIN_2, DWGPIO);
}

void IIC_SCL_2(uint8_t Sel)
{
	if (0 == Sel)
	{
		Pad_Config(IIC_GPIO_SCL_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
//		Pinmux_Config(IIC_GPIO_SCL_PIN_2, DWGPIO);
	}
	else
	{
		Pad_Config(IIC_GPIO_SCL_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
//		Pinmux_Config(IIC_GPIO_SCL_PIN_2, DWGPIO);
	}	
}

void IIC_SDA_2(uint8_t Sel)
{
	if (0 == Sel)
	{
		Pad_Config(IIC_GPIO_SDA_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
//		Pinmux_Config(IIC_GPIO_SCL_PIN_2, DWGPIO);
	}
	else
	{
		
		Pad_Config(IIC_GPIO_SDA_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
//		Pinmux_Config(IIC_GPIO_SCL_PIN_2, DWGPIO);
	}	

}

uint8_t READ_SDA_2(void)	
{
	return GPIO_ReadInputDataBit(GPIO_GetPin(IIC_GPIO_SDA_PIN_2));
}

void IIC_Init_2(void)
{
	Pad_Config(IIC_GPIO_SDA_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	Pad_Config(IIC_GPIO_SCL_PIN_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 
	
	Pinmux_Config(IIC_GPIO_SDA_PIN_2, DWGPIO);
	Pinmux_Config(IIC_GPIO_SCL_PIN_2, DWGPIO);

	IIC_SCL_2(1);
	IIC_SDA_2(1);
	
	delay_us(3);
}

//??IIC????
void IIC_Start_2(void)
{
	SDA_OUT_2();     //sda???
	IIC_SDA_2(1);
	IIC_SCL_2(1);
	delay_us(4);
 	IIC_SDA_2(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL_2(0);
}	  
//??IIC????
void IIC_Stop_2(void)
{
	SDA_OUT_2();//sda???
	IIC_SDA_2(0);//STOP:when CLK is high DATA change form low to high
	IIC_SCL_2(1);
 	delay_us(4);
//	IIC_SCL_2(1); 
	IIC_SDA_2(1);//??I2C??????
	delay_us(4);							   	
}

uint8_t IIC_Wait_Ack_2(void)
{
	uint8_t ucErrTime=0;
	SDA_IN_2();      //SDA?????  
//	IIC_SDA_2(1);
//	delay_us(2);	   
	IIC_SCL_2(1);
	delay_us(1);	
//	IIC_SDA_2(0);
//	SDA_IN_2();
	while(READ_SDA_2())
	{
		ucErrTime++;
		delay_us(1);
		if(ucErrTime>250)
		{
			IIC_Stop_2();
			return 1;
		}
	}
	IIC_SCL_2(0);//????0 	   
	return 0;  
} 
//??ACK??
void IIC_Ack_2(void)
{
	IIC_SCL_2(0);
	SDA_OUT_2();
	IIC_SDA_2(0);
	delay_us(2);
	IIC_SCL_2(1);
	delay_us(2);
	IIC_SCL_2(0);
}
//???ACK??		    
void IIC_NAck_2(void)
{
	IIC_SCL_2(0);
	SDA_OUT_2();
	IIC_SDA_2(1);
	delay_us(2);
	IIC_SCL_2(1);
	delay_us(2);
	IIC_SCL_2(0);
}					 				     

	  
void IIC_Send_Byte_2(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT_2(); 	    
    IIC_SCL_2(0);
    for(t=0;t<8;t++)
    {              
        IIC_SDA_2((txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);
		IIC_SCL_2(1);
		delay_us(2); 
		IIC_SCL_2(0);	
		delay_us(2);
    }	 
}

void IIC_Send_Byte_2_NEW(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT_2(); 	    
    IIC_SCL_2(0);
    for(t=0;t<8;t++)
    {              
        IIC_SDA_2((txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);
		IIC_SCL_2(1);
		delay_us(2); 
		IIC_SCL_2(0);	
		delay_us(2);
    }	 
//
//	IIC_SCL_2(1);
}


//?1???,ack=1?,??ACK,ack=0,??nACK   
uint8_t IIC_Read_Byte_2(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN_2();//SDA?????
  	for(i=0;i<8;i++ )
	{
	    IIC_SCL_2(0); 
	    delay_us(2);
		IIC_SCL_2(1);
	    receive<<=1;
	    if(READ_SDA_2())receive++;   
			delay_us(1); 
	}	
	
	if (!ack)
		IIC_NAck_2();//??nACK
	else
		IIC_Ack_2(); //??ACK   
	return receive;
}

void I2C_WritenByte_2(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr)
{
	int i;
	IIC_Start_2();  
	
	if(device_addr==0xA0) //eeprom????1??
		IIC_Send_Byte_2(0xA0 + ((addr/256)<<1));//?????
	else
		IIC_Send_Byte_2(device_addr);	    //?????
	if(IIC_Wait_Ack_2() == 1) return; 
	IIC_Send_Byte_2(addr&0xFF);   //?????
	if(IIC_Wait_Ack_2() == 1) return; 

	for (i=0 ; i<len ; i++)
	{
		IIC_Send_Byte_2(data[i]);	   
		IIC_Wait_Ack_2(); 
	}
	IIC_Stop_2();

	if(device_addr==0xA0) 
		delay_ms(10);
	else
		delay_us(2);
}

uint8_t I2C_ReadnByte_2(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr)
{
	uint16_t i;
	IIC_Start_2();  
	if(device_addr==0xA0)
		IIC_Send_Byte_2(0xA0 + ((addr/256)<<1));
	else
		IIC_Send_Byte_2(device_addr);	
	if(IIC_Wait_Ack_2() == 1) return 0;
	IIC_Send_Byte_2(addr&0xFF);   //?????
	if(IIC_Wait_Ack_2() == 1) return 0;
	
	IIC_Start_2();
	IIC_Send_Byte_2(device_addr+1);	
	if(IIC_Wait_Ack_2() == 1) return 0;

	for (i=0 ; i<len ; i++)
	{
		if (i == (len-1))
		{
			*(data+i) = IIC_Read_Byte_2(IIC_READ_NACK_2);
		}
		else
		{
			*(data+i) = IIC_Read_Byte_2(IIC_READ_ACK_2);
		}		
	}
	IIC_Stop_2();
	return 1;
}

uint8_t I2C_ReadnByte_2_NEW(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr)
{
	uint16_t i;
	IIC_Start_2();  
	if(device_addr==0xA0)
		IIC_Send_Byte_2(0xA0 + ((addr/256)<<1));
	else
		IIC_Send_Byte_2(device_addr);	
	if(IIC_Wait_Ack_2() == 1) return 0;
	IIC_Send_Byte_2(addr&0xFF);   //?????
	if(IIC_Wait_Ack_2() == 1) return 0;
	
	IIC_Start_2();
	IIC_Send_Byte_2(device_addr+1);	
	if(IIC_Wait_Ack_2() == 1) return 0;

	for (i=0 ; i<len ; i++)
	{
		if (i == (len-1))
		{
			*(data+i) = 0;//IIC_Read_Byte_2(IIC_READ_NACK_2);
			
		}
		else
		{
			*(data+i) = IIC_Read_Byte_2(IIC_READ_ACK_2);
		}		
	}
	IIC_Stop_2();
	return 1;

}

#if 0
unsigned char dxif_transfer_2(unsigned char *buf, unsigned short len)
{	
	if ((buf[0]&0x01) == 0)
	{
		I2C_WritenByte_2(buf[0] , buf+1 , len-1);
	}
	else
	{
		I2C_ReadnByte_2(buf[0] , buf+1 , len-1);
	}
	return 0;
}
#endif


void I2C_WriteByte_2(uint16_t addr,uint8_t data,uint8_t device_addr)
{
	IIC_Start_2();  
	
	if(device_addr==0xA0) //eeprom????1??
		IIC_Send_Byte_2(0xA0 + ((addr/256)<<1));//?????
	else
		IIC_Send_Byte_2(device_addr);	    //?????
	if(IIC_Wait_Ack_2() == 1) return; 
	IIC_Send_Byte_2(addr&0xFF);   //?????
	if(IIC_Wait_Ack_2() == 1) return; 
	IIC_Send_Byte_2(data);     //????							   
	if(IIC_Wait_Ack_2() == 1) return;  		    	   
	IIC_Stop_2();//???????? 
	if(device_addr==0xA0) 
		delay_ms(10);
	else
		delay_us(2);
}
 
uint16_t I2C_ReadByte_2(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead)  //????????
{	
	uint16_t data;
	IIC_Start_2();  
	if(device_addr==0xA0)
		IIC_Send_Byte_2(0xA0 + ((addr/256)<<1));
	else
		IIC_Send_Byte_2(device_addr);	
	IIC_Wait_Ack_2();
	IIC_Send_Byte_2(addr&0xFF);   //?????
	IIC_Wait_Ack_2();

	IIC_Start_2();  	
	IIC_Send_Byte_2(device_addr+1);	    //?????
	IIC_Wait_Ack_2();
	if(ByteNumToRead == 1)//LM75?????11bit
	{
		data=IIC_Read_Byte_2(0);
	}
	else
		{
			data=IIC_Read_Byte_2(1);
			data=(data<<8)+IIC_Read_Byte_2(0);
		}
	IIC_Stop_2();   
	return data;
}
