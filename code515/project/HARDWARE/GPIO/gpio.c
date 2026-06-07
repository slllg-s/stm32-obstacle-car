#include "gpio.h"

void gpio_Init(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// 禁用JTAG，保留SWD（释放PB3用作左侧超声Echo）
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// 电机方向控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// PA6=A1, PA7=A2

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);			// PB0=B1, PB1=B2

	// 超声波Trig引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;		// 正前Trig
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		// 右侧Trig
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		// 左侧Trig
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 超声波Echo引脚 和 编码器输入（初始化为下拉输入）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;		// 正前Echo PA12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		// 右侧Echo PB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		// 左侧Echo PB3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 编码器输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // 编码器1 PA2(脉冲) PA3(方向)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; // 编码器2 PB5(脉冲) PB6(方向)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// I2C: PB10=SCL, PB11=SDA (由i2c.c配置)
	// OLED: PB8=SCL, PB9=SDA (由oled.c配置)
}

void GO(int L, int R)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_6);		// PA6=1  A正转
	GPIO_SetBits(GPIOB, GPIO_Pin_0);		// PB0=1  B正转
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);		// PA7=0
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);		// PB1=0
	TIM_SetCompare1(TIM2, L);
	TIM_SetCompare2(TIM2, R);
}

void Left(int L, int R)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_6);		// PA6=1  A正转
	GPIO_SetBits(GPIOB, GPIO_Pin_1);		// PB1=1  B反转
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);		// PA7=0
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);		// PB0=0
	TIM_SetCompare1(TIM2, L);
	TIM_SetCompare2(TIM2, R);
}

void Right(int L, int R)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_7);		// PA7=1  A反转
	GPIO_SetBits(GPIOB, GPIO_Pin_0);		// PB0=1  B正转
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);		// PA6=0
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);		// PB1=0
	TIM_SetCompare1(TIM2, L);
	TIM_SetCompare2(TIM2, R);
}

void Behind(int L, int R)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_7);		// PA7=1  A反转
	GPIO_SetBits(GPIOB, GPIO_Pin_1);		// PB1=1  B反转
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);		// PA6=0
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);		// PB0=0
	TIM_SetCompare1(TIM2, L);
	TIM_SetCompare2(TIM2, R);
}

void Stop()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}
