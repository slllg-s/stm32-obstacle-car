#include "ai.h"
#include "gpio.h"
#include "delay.h"
#include "pwm.h"
#include "speed.h"

void calculate(float head, float left, float right)
{
	// 1. 紧急后退：任意方向极近
	if ((head < THR_HEAD_EMRG) || (left < THR_SIDE_EMRG) || (right < THR_SIDE_EMRG))
	{
		Behind(SPEED_BACK_L, SPEED_BACK_R);
		delay_ms(DELAY_EMRG);
	}
	// 2. 前方有障碍物：比较左右空间，向更开阔的一侧转弯
	else if (head < THR_HEAD_NEAR)
	{
		if (left > right)
			Left(SPEED_TURN_L, SPEED_TURN_R);
		else
			Right(SPEED_TURN_L, SPEED_TURN_R);
		delay_ms(DELAY_TURN);
	}
	// 3. 左侧有障碍物
	else if (left < THR_LEFT_NEAR)
	{
		Right(SPEED_TURN_L, SPEED_TURN_R);
		delay_ms(DELAY_TURN);
	}
	// 4. 右侧有障碍物
	else if (right < THR_RIGHT_NEAR)
	{
		Left(SPEED_TURN_L, SPEED_TURN_R);
		delay_ms(DELAY_TURN);
	}
	// 5. 前方中等距离、左侧非常开阔 → 可能是弯道，左转探索
	else if ((head > THR_CURVE_HEAD_LO) && (head < THR_CURVE_HEAD_HI) && (left > THR_CURVE_LEFT))
	{
		Left(SPEED_TURN_L, SPEED_TURN_R);
		delay_ms(DELAY_CURVE);
	}
	// 6. 开阔区域，直行
	else
	{
		GO(SPEED_GO_L, SPEED_GO_R);
	}
}
