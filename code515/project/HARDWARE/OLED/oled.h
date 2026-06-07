#ifndef __OLED_H
#define __OLED_H
#include "sys.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(u8 x, u8 y, const char *str);

#endif
