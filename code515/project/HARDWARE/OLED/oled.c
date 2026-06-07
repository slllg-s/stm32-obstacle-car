#include "oled.h"
#include "delay.h"

// 软件I2C：PB8=SCL, PB9=SDA
// 注意：PB8/PB9与右侧超声波(Trig/Echo)共用，测距时会切换模式
// OLED仅在需要显示时调用，平时不影响超声波工作

#define OLED_SCL_H  GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define OLED_SCL_L  GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define OLED_SDA_H  GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define OLED_SDA_L  GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define OLED_SDA_IN  PBin(9)

static void OLED_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void OLED_SDA_INPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void OLED_SCL_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void I2C_Start(void)
{
	OLED_SDA_OUT();
	OLED_SCL_OUT();
	OLED_SDA_H;
	OLED_SCL_H;
	delay_us(5);
	OLED_SDA_L;
	delay_us(5);
	OLED_SCL_L;
}

static void I2C_Stop(void)
{
	OLED_SDA_OUT();
	OLED_SCL_OUT();
	OLED_SDA_L;
	OLED_SCL_H;
	delay_us(5);
	OLED_SDA_H;
	delay_us(5);
}

static u8 I2C_WaitAck(void)
{
	u8 timeout = 0;
	OLED_SDA_INPUT();
	OLED_SCL_H;
	delay_us(1);
	while (OLED_SDA_IN && timeout < 200) timeout++;
	OLED_SCL_L;
	OLED_SDA_OUT();
	return (timeout >= 200) ? 1 : 0;
}

static void I2C_SendByte(u8 data)
{
	u8 i;
	OLED_SDA_OUT();
	OLED_SCL_OUT();
	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			OLED_SDA_H;
		else
			OLED_SDA_L;
		data <<= 1;
		OLED_SCL_H;
		delay_us(2);
		OLED_SCL_L;
		delay_us(2);
	}
	I2C_WaitAck();
}

static void OLED_WriteCmd(u8 cmd)
{
	I2C_Start();
	I2C_SendByte(0x78);	// SSD1306地址
	I2C_SendByte(0x00);	// 命令模式
	I2C_SendByte(cmd);
	I2C_Stop();
}

static void OLED_WriteData(u8 data)
{
	I2C_Start();
	I2C_SendByte(0x78);
	I2C_SendByte(0x40);	// 数据模式
	I2C_SendByte(data);
	I2C_Stop();
}

static void OLED_SetPos(u8 x, u8 y)
{
	OLED_WriteCmd(0xB0 + y);
	OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);
	OLED_WriteCmd(x & 0x0F);
}

void OLED_Init(void)
{
	delay_ms(100);

	OLED_WriteCmd(0xAE);	// 关闭显示
	OLED_WriteCmd(0x00);	// 设置低列地址
	OLED_WriteCmd(0x10);	// 设置高列地址
	OLED_WriteCmd(0x40);	// 设置起始行
	OLED_WriteCmd(0x81);	// 对比度
	OLED_WriteCmd(0xCF);
	OLED_WriteCmd(0xA1);	// 段重映射
	OLED_WriteCmd(0xC8);	// COM扫描方向
	OLED_WriteCmd(0xA6);	// 正常显示
	OLED_WriteCmd(0xA8);	// 多路复用比
	OLED_WriteCmd(0x3F);
	OLED_WriteCmd(0xD3);	// 显示偏移
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(0xD5);	// 显示时钟
	OLED_WriteCmd(0x80);
	OLED_WriteCmd(0xD9);	// 预充电周期
	OLED_WriteCmd(0xF1);
	OLED_WriteCmd(0xDA);	// COM引脚配置
	OLED_WriteCmd(0x12);
	OLED_WriteCmd(0xDB);	// VCOMH
	OLED_WriteCmd(0x40);
	OLED_WriteCmd(0x8D);	// 电荷泵
	OLED_WriteCmd(0x14);
	OLED_WriteCmd(0xAF);	// 开启显示
}

void OLED_Clear(void)
{
	u8 i, j;
	for (i = 0; i < 8; i++) {
		OLED_SetPos(0, i);
		for (j = 0; j < 128; j++)
			OLED_WriteData(0x00);
	}
}

void OLED_ShowString(u8 x, u8 y, const char *str)
{
	OLED_SetPos(x, y);
	while (*str) {
		OLED_WriteData(*str++);
	}
}
