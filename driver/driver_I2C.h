#ifndef _DRIVER_I2C_H_
#define _DRIVER_I2C_H_

#include <rtl876x.h>

//#define IIC_GPIO_PORT_2		GPIOA
#define IIC_GPIO_SDA_PIN_2	P3_0
#define IIC_GPIO_SCL_PIN_2	P3_1	

#define IIC_READ_NACK_2		0
#define IIC_READ_ACK_2		1

//IIC??????
void IIC_Init_2(void);                //???IIC?IO?				 
void IIC_Start_2(void);				//??IIC????
void IIC_Stop_2(void);	  			//??IIC????
void IIC_Send_Byte_2(uint8_t txd);			//IIC??????
uint8_t IIC_Read_Byte_2(unsigned char ack);//IIC??????
uint8_t IIC_Wait_Ack_2(void); 				//IIC??ACK??
void IIC_Ack_2(void);					//IIC??ACK??
void IIC_NAck_2(void);				//IIC???ACK??
 
void I2C_WriteByte_2(uint16_t addr,uint8_t data,uint8_t device_addr);
uint16_t I2C_ReadByte_2(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead);//?????,????,??????  		

void I2C_WritenByte_2(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr);
uint8_t I2C_ReadnByte_2(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr);
uint8_t I2C_ReadnByte_2_NEW(uint8_t device_addr , uint8_t *data , uint16_t len, uint16_t addr);
void IIC_Send_Byte_2_NEW(uint8_t txd);











#endif

