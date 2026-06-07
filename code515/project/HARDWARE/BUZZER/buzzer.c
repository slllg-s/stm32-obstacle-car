#include "buzzer.h"
#include "delay.h"

void Buzzer_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);

	BUZZER_PORT->BSRR = BUZZER_PIN;     // 默认高电平(不响)
}

void Buzzer_On(void)
{
	BUZZER_PORT->BRR = BUZZER_PIN;      // 低电平响
}

void Buzzer_Off(void)
{
	BUZZER_PORT->BSRR = BUZZER_PIN;     // 高电平关
}

void Buzzer_Beep(void)
{
	Buzzer_On();
	delay_ms(100);
	Buzzer_Off();
}
