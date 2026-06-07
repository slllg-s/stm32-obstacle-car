#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

void TIM4_FreeRun_Init(u16 arr, u16 psc);
void Encoder_Init(void);

extern volatile s32 encoder1_count;
extern volatile s32 encoder2_count;

#endif
