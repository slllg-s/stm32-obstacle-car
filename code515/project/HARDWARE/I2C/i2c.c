#include "i2c.h"

void I2C2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_Init(I2C2, &I2C_InitStructure);

	I2C_Cmd(I2C2, ENABLE);
}

void I2C2_Start(I2C_TypeDef *I2Cx, u8 addr, u8 direction)
{
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2Cx, addr, direction);
	if (direction == I2C_Direction_Transmitter)
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	else
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

void I2C2_Stop(I2C_TypeDef *I2Cx)
{
	I2C_GenerateSTOP(I2Cx, ENABLE);
}

void I2C2_WriteByte(I2C_TypeDef *I2Cx, u8 data)
{
	I2C_SendData(I2Cx, data);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

u8 I2C2_ReadByte(I2C_TypeDef *I2Cx, u8 ack)
{
	if (ack)
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
	else
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	return I2C_ReceiveData(I2Cx);
}

u8 I2C2_WriteReg(I2C_TypeDef *I2Cx, u8 dev_addr, u8 reg, u8 data)
{
	I2C2_Start(I2Cx, dev_addr, I2C_Direction_Transmitter);
	I2C2_WriteByte(I2Cx, reg);
	I2C2_WriteByte(I2Cx, data);
	I2C2_Stop(I2Cx);
	return 0;
}

u8 I2C2_ReadReg(I2C_TypeDef *I2Cx, u8 dev_addr, u8 reg, u8 *buf, u8 len)
{
	u8 i;
	I2C2_Start(I2Cx, dev_addr, I2C_Direction_Transmitter);
	I2C2_WriteByte(I2Cx, reg);
	I2C2_Start(I2Cx, dev_addr, I2C_Direction_Receiver);
	for (i = 0; i < len; i++) {
		buf[i] = I2C2_ReadByte(I2Cx, (i < len - 1) ? 1 : 0);
	}
	I2C2_Stop(I2Cx);
	return 0;
}
