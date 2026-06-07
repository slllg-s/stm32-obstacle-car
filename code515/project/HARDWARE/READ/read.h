#ifndef __READ_H
#define __READ_H
#include "sys.h"

struct us_status {
	u8 sta;
	u16 start_cnt;
	u16 val;
};

void Ultrasonic_EXTI_Init(void);
float read_qian(void);
float read_you(void);
float read_zuo(void);
void us_echo_handler(u8 exti_line, struct us_status *us);

extern struct us_status us_qian;
extern struct us_status us_you;
extern struct us_status us_zuo;

#endif
