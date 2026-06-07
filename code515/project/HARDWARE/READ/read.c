#include "read.h"
#include "sys.h"
#include "delay.h"
#include "pwm.h"
#include "timer.h"

// 超声波捕获状态
// bit7: 捕获完成标志
// bit6: 已捕获到上升沿
// bit5~0: 保留
struct us_status us_qian = {0, 0, 0};
struct us_status us_you  = {0, 0, 0};
struct us_status us_zuo  = {0, 0, 0};

// 超声波EXTI初始化
void Ultrasonic_EXTI_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// PB3 -> EXTI3 (左侧超声Echo)
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// PB9 -> EXTI9 (右侧超声Echo)
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// PA12 -> EXTI12 (正前超声Echo)
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// EXTI3中断
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// EXTI9_5中断(PB9+PB5共用)
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// EXTI15_10中断
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}

// 复位EXTI为上升沿触发
static void exti_set_rising(u8 line)
{
	switch (line) {
	case 3:
		EXTI->FTSR &= ~EXTI_Line3;
		EXTI->RTSR |= EXTI_Line3;
		break;
	case 9:
		EXTI->FTSR &= ~EXTI_Line9;
		EXTI->RTSR |= EXTI_Line9;
		break;
	case 12:
		EXTI->FTSR &= ~EXTI_Line12;
		EXTI->RTSR |= EXTI_Line12;
		break;
	}
}

// 超声波测距通用函数
static float read_distance(struct us_status *us, GPIO_TypeDef *GPIOx, u16 Pin, u8 exti_line)
{
	float temp;
	u32 timeout = 0;

	// 复位EXTI为上升沿触发
	exti_set_rising(exti_line);
	// 清状态
	us->sta = 0;
	us->start_cnt = 0;
	us->val = 0;

	// 发送Trig脉冲: 高20us
	GPIO_SetBits(GPIOx, Pin);
	delay_us(20);
	GPIO_ResetBits(GPIOx, Pin);

	// 重置TIM4计数器作为计时起点
	TIM_SetCounter(TIM4, 0);

	// 等待捕获完成或超时(~100ms)
	while (!(us->sta & 0x80) && timeout < 100000)
		timeout++;

	if (us->sta & 0x80) {
		// val = 脉宽(us) = 下降沿计数值 - 上升沿计数值
		temp = us->val;
		temp = temp * 170 * 0.0001;
		return temp;
	}
	return 99;
}

float read_qian()
{
	return read_distance(&us_qian, GPIOA, GPIO_Pin_11, 12);
}

float read_you()
{
	return read_distance(&us_you, GPIOB, GPIO_Pin_8, 9);
}

float read_zuo()
{
	return read_distance(&us_zuo, GPIOC, GPIO_Pin_13, 3);
}

// 超声波ECHO EXTI处理：上升沿记录起点，下降沿计算脉宽
void us_echo_handler(u8 exti_line, struct us_status *us)
{
	if ((us->sta & 0x80) == 0)	// 未完成捕获
	{
		if (us->sta & 0x40)		// 已捕获上升沿，本次为下降沿
		{
			u16 now = TIM_GetCounter(TIM4);
			us->sta |= 0x80;			// 标记捕获完成
			if (now >= us->start_cnt)
				us->val = now - us->start_cnt;	// 脉宽(us)
			else
				us->val = 0xFFFF - us->start_cnt + now + 1;
			exti_set_rising(exti_line);	// 恢复上升沿触发
		}
		else	// 上升沿
		{
			us->sta = 0;
			us->sta |= 0x40;		// 标记已捕获上升沿
			us->start_cnt = TIM_GetCounter(TIM4);
			// 切换为下降沿触发
			switch (exti_line) {
			case 3:
				EXTI->RTSR &= ~EXTI_Line3;
				EXTI->FTSR |= EXTI_Line3;
				break;
			case 9:
				EXTI->RTSR &= ~EXTI_Line9;
				EXTI->FTSR |= EXTI_Line9;
				break;
			case 12:
				EXTI->RTSR &= ~EXTI_Line12;
				EXTI->FTSR |= EXTI_Line12;
				break;
			}
		}
	}
}
