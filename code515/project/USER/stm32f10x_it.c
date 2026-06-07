#include "stm32f10x_it.h"
#include "read.h"
#include "timer.h"
#include "sys.h"

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
}

// 编码器1脉冲 EXTI2 (PA2)
void EXTI2_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		if (PAin(3))	// PA3方向引脚，高电平为正转
			encoder1_count++;
		else
			encoder1_count--;
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

// 左侧超声波Echo EXTI3 (PB3)
void EXTI3_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		us_echo_handler(3, &us_zuo);
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}

// 右侧超声波Echo EXTI9 (PB9) + 编码器2脉冲 EXTI5 (PB5)
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		if (PBin(6))	// PB6方向引脚，高电平为正转
			encoder2_count++;
		else
			encoder2_count--;
		EXTI_ClearITPendingBit(EXTI_Line5);
	}

	if (EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		us_echo_handler(9, &us_you);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}

// 正前超声波Echo EXTI12 (PA12)
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		us_echo_handler(12, &us_qian);
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}
