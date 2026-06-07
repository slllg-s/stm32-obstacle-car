#ifndef __BUZZER_H
#define __BUZZER_H
#include "sys.h"

#define BUZZER_PIN   GPIO_Pin_12
#define BUZZER_PORT  GPIOB

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(void);

#endif
